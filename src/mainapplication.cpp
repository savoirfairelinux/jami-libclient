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

#include "appsettingsmanager.h"
#include "connectivitymonitor.h"
#include "globalsystemtray.h"
#include "namedirectory.h"
#include "qmlregister.h"
#include "qrimageprovider.h"
#include "tintedbuttonimageprovider.h"
#include "avatarimageprovider.h"

#include <QAction>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QMenu>
#include <QQmlContext>
#include <QWindow>

#include <locale.h>

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

static void
vsConsoleDebug()
{
#ifdef _MSC_VER
    /*
     * Print debug to output window if using VS.
     */
    QObject::connect(&LRCInstance::behaviorController(),
                     &lrc::api::BehaviorController::debugMessageReceived,
                     [](const QString& message) {
                         OutputDebugStringA((message + "\n").toStdString().c_str());
                     });
#endif
}

static QString
getDebugFilePath()
{
    QDir logPath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    logPath.cdUp();
    return QString(logPath.absolutePath() + "/jami/jami.log");
}

static void
fileDebug(QFile* debugFile)
{
    QObject::connect(&LRCInstance::behaviorController(),
                     &lrc::api::BehaviorController::debugMessageReceived,
                     [debugFile](const QString& message) {
                         if (debugFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
                             auto msg = (message + "\n").toStdString().c_str();
                             debugFile->write(msg, qstrlen(msg));
                             debugFile->close();
                         }
                     });
}

MainApplication::MainApplication(int& argc, char** argv)
    : QApplication(argc, argv)
    , engine_(new QQmlApplicationEngine())
    , connectivityMonitor_(new ConnectivityMonitor(this))
{
    QObject::connect(this, &QApplication::aboutToQuit, [this] { cleanup(); });
}

bool
MainApplication::init()
{
    setWindowIcon(QIcon(":images/jami.ico"));

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

    initLrc(results[opts::UPDATEURL].toString(), connectivityMonitor_);

    connect(connectivityMonitor_, &ConnectivityMonitor::connectivityChanged, [] {
        LRCInstance::connectivityChanged();
    });

    QObject::connect(
        &LRCInstance::instance(),
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

    initSettings();
    initSystray();
    initQmlEngine();

    return true;
}

void
MainApplication::loadTranslations()
{
    auto appDir = qApp->applicationDirPath() + "/";
    const auto locale_name = QLocale::system().name();
    const auto locale_lang = locale_name.split('_')[0];

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
        if (lrcTranslator_lang->load(appDir + "share/libringclient/translations/lrc_" + locale_lang))
            installTranslator(lrcTranslator_lang);
    }
    if (lrcTranslator_name->load(appDir + "share/libringclient/translations/lrc_" + locale_name))
        installTranslator(lrcTranslator_name);

    QTranslator* mainTranslator_lang = new QTranslator(this);
    QTranslator* mainTranslator_name = new QTranslator(this);
    if (locale_name != locale_lang) {
        if (mainTranslator_lang->load(appDir + "share/ring/translations/ring_client_windows_"
                                      + locale_lang))
            installTranslator(mainTranslator_lang);
    }
    if (mainTranslator_name->load(appDir + "share/ring/translations/ring_client_windows_"
                                  + locale_name))
        installTranslator(mainTranslator_name);
}

void
MainApplication::initLrc(const QString& downloadUrl, ConnectivityMonitor* cm)
{
    /*
     * Init mainwindow and finish splash when mainwindow shows up.
     */
    std::atomic_bool isMigrating(false);
    LRCInstance::init(
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
        cm);
    LRCInstance::subscribeToDebugReceived();
    LRCInstance::getAPI().holdConferences = false;
}

const QVariantMap
MainApplication::parseArguments()
{
    QVariantMap results;
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    // This option is forced into the arg list.
    QCommandLineOption webSecurityDisableOption(QStringList() << "disable-web-security");
    parser.addOption(webSecurityDisableOption);

    QCommandLineOption webDebugOption(QStringList() << "remote-debugging-port",
                                      "Web debugging port.",
                                      "port");
    parser.addOption(webDebugOption);

    QCommandLineOption minimizedOption(QStringList() << "m"
                                                     << "minimized",
                                       "Start minimized.");
    parser.addOption(minimizedOption);

    QCommandLineOption debugOption(QStringList() << "d"
                                                 << "debug",
                                   "Debug out.");
    parser.addOption(debugOption);

#ifdef Q_OS_WINDOWS
    QCommandLineOption debugConsoleOption(QStringList() << "c"
                                                        << "console",
                                          "Debug out to IDE console.");
    parser.addOption(debugConsoleOption);

    QCommandLineOption debugFileOption(QStringList() << "f"
                                                     << "file",
                                       "Debug to file.");
    parser.addOption(debugFileOption);

    QCommandLineOption updateUrlOption(QStringList() << "u"
                                                     << "url",
                                       "Reference <url> for client versioning.",
                                       "url");
    parser.addOption(updateUrlOption);
#endif

    parser.process(*this);

    results[opts::STARTMINIMIZED] = parser.isSet(minimizedOption);
    results[opts::DEBUG] = parser.isSet(debugOption);
#ifdef Q_OS_WINDOWS
    results[opts::DEBUGCONSOLE] = parser.isSet(debugConsoleOption);
    results[opts::DEBUGFILE] = parser.isSet(debugFileOption);
    results[opts::UPDATEURL] = parser.value(updateUrlOption);
#endif
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
MainApplication::initQmlEngine()
{
    registerTypes();

    engine_->addImageProvider(QLatin1String("qrImage"), new QrImageProvider());
    engine_->addImageProvider(QLatin1String("tintedPixmap"), new TintedButtonImageProvider());
    engine_->addImageProvider(QLatin1String("avatarImage"), new AvatarImageProvider());

    engine_->setObjectOwnership(&LRCInstance::avModel(), QQmlEngine::CppOwnership);
    engine_->setObjectOwnership(&LRCInstance::pluginModel(), QQmlEngine::CppOwnership);
    engine_->setObjectOwnership(LRCInstance::getUpdateManager(), QQmlEngine::CppOwnership);
    engine_->setObjectOwnership(&LRCInstance::instance(), QQmlEngine::CppOwnership);
    engine_->setObjectOwnership(&NameDirectory::instance(), QQmlEngine::CppOwnership);

    engine_->load(QUrl(QStringLiteral("qrc:/src/MainApplicationWindow.qml")));
}

void
MainApplication::initSettings()
{
    AppSettingsManager::instance().initValues();
    auto downloadPath = AppSettingsManager::instance().getValue(Settings::Key::DownloadPath);
    LRCInstance::dataTransferModel().downloadDirectory = downloadPath.toString() + "/";
}

void
MainApplication::initSystray()
{
    GlobalSystemTray& sysIcon = GlobalSystemTray::instance();
    sysIcon.setIcon(QIcon(":images/jami.png"));

    QMenu* systrayMenu = new QMenu();

    QAction* exitAction = new QAction(tr("Exit"), this);
    connect(exitAction, &QAction::triggered, [this] {
        engine_->quit();
        cleanup();
    });
    connect(&sysIcon, &QSystemTrayIcon::activated, [](QSystemTrayIcon::ActivationReason reason) {
        if (reason != QSystemTrayIcon::ActivationReason::Context)
            emit LRCInstance::instance().restoreAppRequested();
    });

    systrayMenu->addAction(exitAction);
    sysIcon.setContextMenu(systrayMenu);
    sysIcon.show();
}

void
MainApplication::cleanup()
{
    GlobalSystemTray::instance().hide();
#ifdef Q_OS_WIN
    FreeConsole();
#endif
    QApplication::exit(0);
}
