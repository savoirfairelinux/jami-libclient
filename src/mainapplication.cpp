/*
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainapplication.h"

#include "qmlregister.h"
#include "appsettingsmanager.h"
#include "connectivitymonitor.h"
#include "systemtray.h"
#include "previewengine.h"

#include <QAction>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QMenu>
#include <QQmlContext>
#include <QResource>
#include <QTranslator>
#include <QLibraryInfo>

#include <locale.h>
#include <thread>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_UNIX
#include "globalinstances.h"
#include "dbuserrorhandler.h"
#endif

#if defined _MSC_VER
#include <gnutls/gnutls.h>
#endif

namespace opts {
// Keys used to store command-line options.
constexpr static const char STARTMINIMIZED[] = "STARTMINIMIZED";
constexpr static const char DEBUG[] = "DEBUG";
constexpr static const char DEBUGCONSOLE[] = "DEBUGCONSOLE";
constexpr static const char DEBUGFILE[] = "DEBUGFILE";
constexpr static const char UPDATEURL[] = "UPDATEURL";
constexpr static const char MUTEDAEMON[] = "MUTEDAEMON";
} // namespace opts

static void
consoleDebug()
{
#ifdef Q_OS_WIN
    AllocConsole();
    SetConsoleCP(CP_UTF8);

    FILE* fpstdout = stdout;
    freopen_s(&fpstdout, "CONOUT$", "w", stdout);
    FILE* fpstderr = stderr;
    freopen_s(&fpstderr, "CONOUT$", "w", stderr);

    COORD coordInfo;
    coordInfo.X = 130;
    coordInfo.Y = 9000;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coordInfo);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
#endif
}

static QString
getDebugFilePath()
{
    QDir logPath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    logPath.cdUp();
    return QString(logPath.absolutePath() + "/jami/jami.log");
}

void
ScreenInfo::setCurrentFocusWindow(QWindow* window)
{
    if (window && !currentFocusWindow_) {
        currentFocusWindow_ = window;
        set_devicePixelRatio(currentFocusWindow_->screen()->devicePixelRatio());

        disconnect(devicePixelRatioConnection_);
        disconnect(currentFocusWindowScreenConnection_);

        currentFocusWindowScreenConnection_
            = connect(currentFocusWindow_, &QWindow::screenChanged, [this] {
                  currentFocusWindowScreen_ = currentFocusWindow_->screen();
                  set_devicePixelRatio(currentFocusWindowScreen_->devicePixelRatio());

                  devicePixelRatioConnection_ = connect(
                      currentFocusWindowScreen_, &QScreen::physicalDotsPerInchChanged, [this] {
                          set_devicePixelRatio(currentFocusWindowScreen_->devicePixelRatio());
                      });
              });
    }
}

void
MainApplication::vsConsoleDebug()
{
#ifdef _MSC_VER
    /*
     * Print debug to output window if using VS.
     */
    QObject::connect(&lrcInstance_->behaviorController(),
                     &lrc::api::BehaviorController::debugMessageReceived,
                     [](const QString& message) {
                         OutputDebugStringA((message + "\n").toStdString().c_str());
                     });
#endif
}

void
MainApplication::fileDebug(QFile* debugFile)
{
    QObject::connect(&lrcInstance_->behaviorController(),
                     &lrc::api::BehaviorController::debugMessageReceived,
                     [debugFile](const QString& message) {
                         if (debugFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
                             auto msg = (message + "\n").toStdString();
                             debugFile->write(msg.c_str(), qstrlen(msg.c_str()));
                             debugFile->close();
                         }
                     });
}

MainApplication::MainApplication(int& argc, char** argv)
    : QApplication(argc, argv)
    , engine_(new QQmlApplicationEngine())
    , connectivityMonitor_(new ConnectivityMonitor(this))
    , settingsManager_(new AppSettingsManager(this))
    , systemTray_(new SystemTray(settingsManager_.get(), this))
    , previewEngine_(new PreviewEngine(this))
{
    QObject::connect(this, &QApplication::aboutToQuit, [this] { cleanup(); });
}

MainApplication::~MainApplication()
{
    engine_.reset();
    lrcInstance_.reset();
}

bool
MainApplication::init()
{
    setWindowIcon(QIcon(":/images/jami.ico"));

    // Lrc web resources
    QResource::registerResource(QCoreApplication::applicationDirPath() + QDir::separator()
                                + "webresource.rcc");

#ifdef Q_OS_LINUX
    if (!getenv("QT_QPA_PLATFORMTHEME"))
        setenv("QT_QPA_PLATFORMTHEME", "gtk3", true);
#endif

    auto results = parseArguments();

    if (results[opts::DEBUG].toBool()) {
        consoleDebug();
    }

    Utils::removeOldVersions();
    loadTranslations();
    setApplicationFont();

#if defined _MSC_VER
    gnutls_global_init();
#endif

    initLrc(results[opts::UPDATEURL].toString(),
            connectivityMonitor_.get(),
            results[opts::DEBUG].toBool() && !results[opts::MUTEDAEMON].toBool());

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    using namespace Interfaces;
    GlobalInstances::setDBusErrorHandler(std::make_unique<DBusErrorHandler>());
    auto dBusErrorHandlerQObject = dynamic_cast<QObject*>(&GlobalInstances::dBusErrorHandler());
    QML_REGISTERSINGLETONTYPE_CUSTOM(NS_MODELS, DBusErrorHandler, dBusErrorHandlerQObject);
    if ((!lrc::api::Lrc::isConnected()) || (!lrc::api::Lrc::dbusIsValid())) {
        engine_->load(QUrl(QStringLiteral("qrc:/src/DaemonReconnectWindow.qml")));
        exec();

        if ((!lrc::api::Lrc::isConnected()) || (!lrc::api::Lrc::dbusIsValid()))
            return false;
        else
            engine_.reset(new QQmlApplicationEngine());
    }
#endif

    connect(connectivityMonitor_.get(), &ConnectivityMonitor::connectivityChanged, [this] {
        lrcInstance_->connectivityChanged();
    });

    connect(this, &QGuiApplication::focusWindowChanged, [this] {
        screenInfo_.setCurrentFocusWindow(this->focusWindow());
    });

    QObject::connect(
        lrcInstance_.get(),
        &LRCInstance::quitEngineRequested,
        this,
        [this] { engine_->quit(); },
        Qt::DirectConnection);

    if (results[opts::DEBUGFILE].toBool()) {
        debugFile_.reset(new QFile(getDebugFilePath()));
        debugFile_->open(QIODevice::WriteOnly | QIODevice::Truncate);
        debugFile_->close();
        fileDebug(debugFile_.get());
    }

    if (results[opts::DEBUGCONSOLE].toBool()) {
        vsConsoleDebug();
    }

    auto downloadPath = settingsManager_->getValue(Settings::Key::DownloadPath);
    auto allowTransferFromUntrusted = settingsManager_->getValue(Settings::Key::AllowFromUntrusted)
                                          .toBool();
    auto allowTransferFromTrusted = settingsManager_->getValue(Settings::Key::AutoAcceptFiles)
                                        .toBool();
    auto acceptTransferBelow = settingsManager_->getValue(Settings::Key::AcceptTransferBelow).toInt();
    lrcInstance_->accountModel().downloadDirectory = downloadPath.toString() + "/";
    lrcInstance_->accountModel().autoTransferFromUntrusted = allowTransferFromUntrusted;
    lrcInstance_->accountModel().autoTransferFromTrusted = allowTransferFromTrusted;
    lrcInstance_->accountModel().autoTransferSizeThreshold = acceptTransferBelow;

    initQmlLayer();
    initSystray();

    return true;
}

void
MainApplication::restoreApp()
{
    Q_EMIT lrcInstance_->restoreAppRequested();
}

void
MainApplication::loadTranslations()
{
#if defined(Q_OS_LINUX) && defined(JAMI_INSTALL_PREFIX)
    QString appDir = JAMI_INSTALL_PREFIX;
#else
    QString appDir = qApp->applicationDirPath() + QDir::separator() + "share";
#endif

    QString locale_name = QLocale::system().name();
    QString locale_lang = locale_name.split('_')[0];

    QTranslator* qtTranslator_lang = new QTranslator(this);
    QTranslator* qtTranslator_name = new QTranslator(this);
    if (locale_name != locale_lang) {
        if (qtTranslator_lang->load("qt_" + locale_lang,
                                    QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
            installTranslator(qtTranslator_lang);
    }

    if (qtTranslator_name->load("qt_" + locale_name,
                                QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        installTranslator(qtTranslator_name);
    }

    QTranslator* lrcTranslator_lang = new QTranslator(this);
    QTranslator* lrcTranslator_name = new QTranslator(this);
    if (locale_name != locale_lang) {
        if (lrcTranslator_lang->load(appDir + QDir::separator() + "libringclient" + QDir::separator()
                                     + "translations" + QDir::separator() + "lrc_" + locale_lang)) {
            installTranslator(lrcTranslator_lang);
        }
    }
    if (lrcTranslator_name->load(appDir + QDir::separator() + "libringclient" + QDir::separator()
                                 + "translations" + QDir::separator() + "lrc_" + locale_name)) {
        installTranslator(lrcTranslator_name);
    }

    QTranslator* mainTranslator_lang = new QTranslator(this);
    QTranslator* mainTranslator_name = new QTranslator(this);
    if (locale_name != locale_lang) {
        if (mainTranslator_lang->load(appDir + QDir::separator() + "ring" + QDir::separator()
                                      + "translations" + QDir::separator() + "ring_client_windows_"
                                      + locale_lang)) {
            installTranslator(mainTranslator_lang);
        }
    }
    if (mainTranslator_name->load(appDir + QDir::separator() + "ring" + QDir::separator()
                                  + "translations" + QDir::separator() + "ring_client_windows_"
                                  + locale_name)) {
        installTranslator(mainTranslator_name);
    }
}

void
MainApplication::initLrc(const QString& downloadUrl, ConnectivityMonitor* cm, bool logDaemon)
{
    lrc::api::Lrc::cacheAvatars.store(false);
    /*
     * Init mainwindow and finish splash when mainwindow shows up.
     */
    std::atomic_bool isMigrating(false);
    lrcInstance_.reset(new LRCInstance(
        [this, &isMigrating] {
            /*
             * TODO: splash screen for account migration.
             */
            isMigrating = true;
            while (isMigrating) {
                this->processEvents();
            }
        },
        [&isMigrating] {
            while (!isMigrating) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            isMigrating = false;
        },
        downloadUrl,
        cm,
        !logDaemon));
    lrcInstance_->subscribeToDebugReceived();
}

const QVariantMap
MainApplication::parseArguments()
{
    QVariantMap results;
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    // These options are potentially forced into the arg list.
    QCommandLineOption webSecurityDisableOption(QStringList() << "disable-web-security");
    parser.addOption(webSecurityDisableOption);

    QCommandLineOption noSandboxOption(QStringList() << "no-sandbox");
    parser.addOption(noSandboxOption);

    QCommandLineOption singleProcessOption(QStringList() << "single-process");
    parser.addOption(singleProcessOption);

    QCommandLineOption webDebugOption(QStringList() << "remote-debugging-port",
                                      "Web debugging port.",
                                      "port");
    parser.addOption(webDebugOption);

    QCommandLineOption minimizedOption({"m", "minimized"}, "Start minimized.");
    parser.addOption(minimizedOption);

    QCommandLineOption debugOption({"d", "debug"}, "Debug out.");
    parser.addOption(debugOption);

#ifdef Q_OS_WINDOWS
    QCommandLineOption debugConsoleOption({"c", "console"}, "Debug out to IDE console.");
    parser.addOption(debugConsoleOption);

    QCommandLineOption debugFileOption({"f", "file"}, "Debug to file.");
    parser.addOption(debugFileOption);

    QCommandLineOption updateUrlOption({"u", "url"}, "<url> for debugging version queries.", "url");
    parser.addOption(updateUrlOption);
#endif

    QCommandLineOption muteDaemonOption({"q", "quiet"}, "Mute daemon logging. (only if debug)");
    parser.addOption(muteDaemonOption);

    parser.process(*this);

    results[opts::STARTMINIMIZED] = parser.isSet(minimizedOption);
    results[opts::DEBUG] = parser.isSet(debugOption);
#ifdef Q_OS_WINDOWS
    results[opts::DEBUGCONSOLE] = parser.isSet(debugConsoleOption);
    results[opts::DEBUGFILE] = parser.isSet(debugFileOption);
    results[opts::UPDATEURL] = parser.value(updateUrlOption);
#endif
    results[opts::MUTEDAEMON] = parser.isSet(muteDaemonOption);

    return results;
}

void
MainApplication::setApplicationFont()
{
    QFont font;
    font.setFamily("Segoe UI");
    setFont(font);
    QFontDatabase::addApplicationFont(":/fonts/FontAwesome.otf");
}

void
MainApplication::initQmlLayer()
{
    // Expose custom types to the QML engine.
    Utils::registerTypes(engine_.get(),
                         systemTray_.get(),
                         lrcInstance_.get(),
                         settingsManager_.get(),
                         previewEngine_.get(),
                         &screenInfo_,
                         this);

    engine_->load(QUrl(QStringLiteral("qrc:/src/MainApplicationWindow.qml")));
}

void
MainApplication::initSystray()
{
    systemTray_->setIcon(QIcon(":/images/jami.svg"));

    QMenu* systrayMenu = new QMenu();

    QString quitString;
#ifdef Q_OS_WINDOWS
    quitString = tr("E&xit");
#else
    quitString = tr("&Quit");
#endif

    QAction* quitAction = new QAction(quitString, this);
    connect(quitAction, &QAction::triggered, this, &MainApplication::cleanup);

    QAction* restoreAction = new QAction(tr("&Show Jami"), this);
    connect(restoreAction, &QAction::triggered, this, &MainApplication::restoreApp);

    connect(systemTray_.get(),
            &QSystemTrayIcon::activated,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason != QSystemTrayIcon::ActivationReason::Context) {
#ifdef Q_OS_WINDOWS
                    restoreApp();
#else
                    QWindow* window = focusWindow();
                    if (window)
                        window->close();
                    else
                        restoreApp();
#endif
                }
            });

    systrayMenu->addAction(restoreAction);
    systrayMenu->addAction(quitAction);
    systemTray_->setContextMenu(systrayMenu);
    systemTray_->show();
}

void
MainApplication::cleanup()
{
#ifdef Q_OS_WIN
    FreeConsole();
#endif
    QApplication::exit(0);
}
