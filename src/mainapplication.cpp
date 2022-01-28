/*!
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
#include "namedirectory.h"
#include "qrimageprovider.h"
#include "tintedbuttonimageprovider.h"
#include "avatarimageprovider.h"
#include "avatarregistry.h"

#include "accountadapter.h"
#include "avadapter.h"
#include "calladapter.h"
#include "contactadapter.h"
#include "pluginadapter.h"
#include "messagesadapter.h"
#include "settingsadapter.h"
#include "utilsadapter.h"
#include "conversationsadapter.h"

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

#if defined _MSC_VER && !COMPILE_ONLY
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
        setDevicePixelRatio(currentFocusWindow_->screen()->devicePixelRatio());

        disconnect(devicePixelRatioConnection_);
        disconnect(currentFocusWindowScreenConnection_);

        currentFocusWindowScreenConnection_
            = connect(currentFocusWindow_, &QWindow::screenChanged, [this] {
                  currentFocusWindowScreen_ = currentFocusWindow_->screen();
                  setDevicePixelRatio(currentFocusWindowScreen_->devicePixelRatio());

                  devicePixelRatioConnection_ = connect(
                      currentFocusWindowScreen_, &QScreen::physicalDotsPerInchChanged, [this] {
                          setDevicePixelRatio(currentFocusWindowScreen_->devicePixelRatio());
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
{
    QObject::connect(this, &QApplication::aboutToQuit, [this] { cleanup(); });
}

MainApplication::~MainApplication()
{
    engine_.reset();
    systemTray_.reset();
    settingsManager_.reset();
    lrcInstance_.reset();
    connectivityMonitor_.reset();
}

bool
MainApplication::init()
{
    setWindowIcon(QIcon(":images/jami.ico"));

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

#if defined _MSC_VER && !COMPILE_ONLY
    gnutls_global_init();
#endif

    initLrc(results[opts::UPDATEURL].toString(),
            connectivityMonitor_.get(),
            results[opts::MUTEDAEMON].toBool());

#ifdef Q_OS_UNIX
    GlobalInstances::setDBusErrorHandler(std::make_unique<Interfaces::DBusErrorHandler>());
    auto dBusErrorHandlerQObject = dynamic_cast<QObject*>(&GlobalInstances::dBusErrorHandler());
    qmlRegisterSingletonType<Interfaces::DBusErrorHandler>("net.jami.Models",
                                                           1,
                                                           0,
                                                           "DBusErrorHandler",
                                                           [dBusErrorHandlerQObject](QQmlEngine* e,
                                                                                     QJSEngine* se)
                                                               -> QObject* {
                                                               Q_UNUSED(e)
                                                               Q_UNUSED(se)
                                                               return dBusErrorHandlerQObject;
                                                           });
    engine_->setObjectOwnership(dBusErrorHandlerQObject, QQmlEngine::CppOwnership);

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
                                    QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
            installTranslator(qtTranslator_lang);
    }
    qtTranslator_name->load("qt_" + locale_name,
                            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    installTranslator(qtTranslator_name);

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
MainApplication::initLrc(const QString& downloadUrl, ConnectivityMonitor* cm, bool muteDaemon)
{
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
        muteDaemon));
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

    QCommandLineOption muteDaemonOption({"q", "quiet"}, "Mute daemon logging.");
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
    QFontDatabase::addApplicationFont(":/images/FontAwesome.otf");
}

void
MainApplication::initQmlLayer()
{
    // setup the adapters (their lifetimes are that of MainApplication)
    auto callAdapter = new CallAdapter(systemTray_.get(), lrcInstance_.data(), this);
    auto messagesAdapter = new MessagesAdapter(settingsManager_.get(), lrcInstance_.data(), this);
    auto conversationsAdapter = new ConversationsAdapter(systemTray_.get(),
                                                         lrcInstance_.data(),
                                                         this);
    auto avAdapter = new AvAdapter(lrcInstance_.data(), this);
    auto contactAdapter = new ContactAdapter(lrcInstance_.data(), this);
    auto accountAdapter = new AccountAdapter(settingsManager_.get(), lrcInstance_.data(), this);
    auto utilsAdapter = new UtilsAdapter(systemTray_.get(), lrcInstance_.data(), this);
    auto settingsAdapter = new SettingsAdapter(settingsManager_.get(), lrcInstance_.data(), this);
    auto pluginAdapter = new PluginAdapter(lrcInstance_.data(), this);

    // qml adapter registration
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, callAdapter, "CallAdapter");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, messagesAdapter, "MessagesAdapter");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, conversationsAdapter, "ConversationsAdapter");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, avAdapter, "AvAdapter");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, contactAdapter, "ContactAdapter");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, accountAdapter, "AccountAdapter");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, utilsAdapter, "UtilsAdapter");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, settingsAdapter, "SettingsAdapter");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, pluginAdapter, "PluginAdapter");

    auto avatarRegistry = new AvatarRegistry(lrcInstance_.data(), this);
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_HELPERS, avatarRegistry, "AvatarRegistry");

    // TODO: remove these
    QML_REGISTERSINGLETONTYPE_CUSTOM(NS_MODELS, AVModel, &lrcInstance_->avModel())
    QML_REGISTERSINGLETONTYPE_CUSTOM(NS_MODELS, PluginModel, &lrcInstance_->pluginModel())
    QML_REGISTERSINGLETONTYPE_CUSTOM(NS_HELPERS, UpdateManager, lrcInstance_->getUpdateManager())

    // register other types that don't require injection(e.g. uncreatables, c++/qml singletons)
    Utils::registerTypes();

    engine_->addImageProvider(QLatin1String("qrImage"), new QrImageProvider(lrcInstance_.get()));
    engine_->addImageProvider(QLatin1String("tintedPixmap"),
                              new TintedButtonImageProvider(lrcInstance_.get()));
    engine_->addImageProvider(QLatin1String("avatarImage"),
                              new AvatarImageProvider(lrcInstance_.get()));

    engine_->rootContext()->setContextProperty("ScreenInfo", &screenInfo_);
    engine_->rootContext()->setContextProperty("LRCInstance", lrcInstance_.get());

    engine_->setObjectOwnership(&lrcInstance_->avModel(), QQmlEngine::CppOwnership);
    engine_->setObjectOwnership(&lrcInstance_->pluginModel(), QQmlEngine::CppOwnership);
    engine_->setObjectOwnership(lrcInstance_->getUpdateManager(), QQmlEngine::CppOwnership);
    engine_->setObjectOwnership(lrcInstance_.get(), QQmlEngine::CppOwnership);
    engine_->setObjectOwnership(&NameDirectory::instance(), QQmlEngine::CppOwnership);

    engine_->load(QUrl(QStringLiteral("qrc:/src/MainApplicationWindow.qml")));
}

void
MainApplication::initSystray()
{
    systemTray_->setIcon(QIcon(":images/jami.svg"));

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
