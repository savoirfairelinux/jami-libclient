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
#include "globalinstances.h"
#include "globalsystemtray.h"
#include "qmlregister.h"
#include "qrimageprovider.h"
#include "pixbufmanipulator.h"
#include "tintedbuttonimageprovider.h"

#include <QAction>
#include <QFontDatabase>
#include <QMenu>
#include <QQmlContext>
#include <QWindow>

#include <locale.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#if defined _MSC_VER && !COMPILE_ONLY
#include <gnutls/gnutls.h>
#endif

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
{
    QObject::connect(this, &QApplication::aboutToQuit, [this] { cleanup(); });
}

void
MainApplication::init()
{
#ifdef Q_OS_LINUX
    if (!getenv("QT_QPA_PLATFORMTHEME"))
        setenv("QT_QPA_PLATFORMTHEME", "gtk3", true);
#endif

    for (auto string : QCoreApplication::arguments()) {
        if (string == "-d" || string == "--debug") {
            consoleDebug();
        }
    }

    Utils::removeOldVersions();
    loadTranslations();
    setApplicationFont();

#if defined _MSC_VER && !COMPILE_ONLY
    gnutls_global_init();
#endif

    GlobalInstances::setPixmapManipulator(std::make_unique<PixbufManipulator>());
    initLrc();

    initConnectivityMonitor();

#ifdef Q_OS_WINDOWS
    QObject::connect(&LRCInstance::instance(), &LRCInstance::notificationClicked, [] {
        for (QWindow* appWindow : qApp->allWindows()) {
            if (appWindow->objectName().compare("mainViewWindow"))
                continue;
            // clang-format off
            ::SetWindowPos((HWND) appWindow->winId(),
                           HWND_TOPMOST, 0, 0, 0, 0,
                           SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            ::SetWindowPos((HWND) appWindow->winId(),
                           HWND_NOTOPMOST, 0, 0, 0, 0,
                           SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            // clang-format on
            return;
        }
    });
#endif

    bool startMinimized {false};
    parseArguments(startMinimized);

    initSettings();
    initSystray();
    initQmlEngine();
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
MainApplication::initLrc()
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
        });
    LRCInstance::subscribeToDebugReceived();
    LRCInstance::getAPI().holdConferences = false;
}

void
MainApplication::initConnectivityMonitor()
{
#ifdef Q_OS_WIN
    connectivityMonitor_.reset(new ConnectivityMonitor(this));
    connect(connectivityMonitor_.get(), &ConnectivityMonitor::connectivityChanged, [] {
        LRCInstance::connectivityChanged();
    });
#endif // Q_OS_WIN
}

void
MainApplication::parseArguments(bool& startMinimized)
{
    QString uri = "";

    for (auto string : QCoreApplication::arguments()) {
        if (string.startsWith("jami:")) {
            uri = string;
        } else {
            if (string == "-m" || string == "--minimized") {
                startMinimized = true;
            }
#ifdef Q_OS_WINDOWS
            debugFile_.reset(new QFile(getDebugFilePath()));
            auto dbgFile = string == "-f" || string == "--file";
            auto dbgConsole = string == "-c" || string == "--vsconsole";
            if (dbgFile || dbgConsole) {
                if (dbgFile) {
                    debugFile_->open(QIODevice::WriteOnly | QIODevice::Truncate);
                    debugFile_->close();
                    fileDebug(debugFile_.get());
                }
                if (dbgConsole) {
                    vsConsoleDebug();
                }
            }
#endif
        }
    }
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
