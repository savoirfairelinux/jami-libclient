/**
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

#include "accountadapter.h"
#include "accountlistmodel.h"
#include "accountstomigratelistmodel.h"
#include "audiocodeclistmodel.h"
#include "audioinputdevicemodel.h"
#include "audiomanagerlistmodel.h"
#include "audiooutputdevicemodel.h"
#include "pluginlistpreferencemodel.h"
#include "mediahandlerlistpreferencemodel.h"
#include "avadapter.h"
#include "bannedlistmodel.h"
#include "calladapter.h"
#include "clientwrapper.h"
#include "contactadapter.h"
#include "mediahandleradapter.h"
#include "conversationsadapter.h"
#include "deviceitemlistmodel.h"
#include "pluginitemlistmodel.h"
#include "mediahandleritemlistmodel.h"
#include "preferenceitemlistmodel.h"
#include "distantrenderer.h"
#include "globalinstances.h"
#include "globalsystemtray.h"
#include "messagesadapter.h"
#include "namedirectory.h"
#include "pixbufmanipulator.h"
#include "previewrenderer.h"
#include "qrimageprovider.h"
#include "settingsadaptor.h"
#include "tintedbuttonimageprovider.h"
#include "utils.h"
#include "version.h"
#include "videocodeclistmodel.h"
#include "videoformatfpsmodel.h"
#include "videoformatresolutionmodel.h"
#include "videoinputdevicemodel.h"

#include <QFontDatabase>
#include <QQmlContext>
#include <QtWebEngine>

#include <locale.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#if defined _MSC_VER && !COMPILE_ONLY
#include <gnutls/gnutls.h>
#endif

MainApplication::MainApplication(int& argc, char** argv)
    : QApplication(argc, argv)
    , engine_(new QQmlApplicationEngine())
{
    QObject::connect(this, &QApplication::aboutToQuit, [this] { exitApp(); });
}

void
MainApplication::applicationInitialization()
{
    /*
     * Some attributes are needed to be set before the creation of the application.
     */
    QApplication::setApplicationName("Ring");
    QApplication::setOrganizationDomain("jami.net");
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setQuitOnLastWindowClosed(false);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
    /*
     * Initialize QtWebEngine.
     */
    QtWebEngine::initialize();
#endif
}

void
MainApplication::consoleDebug()
{
#ifdef Q_OS_WIN
    AllocConsole();
    SetConsoleCP(CP_UTF8);

    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    COORD coordInfo;
    coordInfo.X = 130;
    coordInfo.Y = 9000;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coordInfo);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
#endif
}

void
MainApplication::vsConsoleDebug()
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

void
MainApplication::fileDebug(QFile* debugFile)
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

void
MainApplication::exitApp()
{
    GlobalSystemTray::instance().hide();
#ifdef Q_OS_WIN
    FreeConsole();
#endif
}

char**
MainApplication::parseInputArgument(int& argc, char* argv[], char* argToParse)
{
    /*
     * Forcefully append argToParse.
     */
    int oldArgc = argc;
    argc = argc + 1 + 1;
    char** newArgv = new char*[argc];
    for (int i = 0; i < oldArgc; i++) {
        newArgv[i] = argv[i];
    }
    newArgv[oldArgc] = argToParse;
    newArgv[oldArgc + 1] = nullptr;
    return newArgv;
}

QString
MainApplication::getDebugFilePath()
{
    QDir logPath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    /*
     * Since logPath will be .../Ring, we use cdUp to remove it.
     */
    logPath.cdUp();
    return QString(logPath.absolutePath() + "/jami/jami.log");
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
        [this, &isMigrating] {
            while (!isMigrating) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            isMigrating = false;
        });
    LRCInstance::subscribeToDebugReceived();
    LRCInstance::getAPI().holdConferences = false;
}

void
MainApplication::processInputArgument(bool& startMinimized)
{
    debugFile_ = std::make_unique<QFile>(getDebugFilePath());
    QString uri = "";

    for (auto string : QCoreApplication::arguments()) {
        if (string.startsWith("jami:")) {
            uri = string;
        } else {
            if (string == "-m" || string == "--minimized") {
                startMinimized = true;
            }
            auto dbgFile = string == "-f" || string == "--file";
            auto dbgConsole = string == "-c" || string == "--vsconsole";
            if (dbgFile || dbgConsole) {
                if (dbgFile) {
                    debugFile_->open(QIODevice::WriteOnly | QIODevice::Truncate);
                    debugFile_->close();
                    fileDebug(debugFile_.get());
                }
#ifdef _MSC_VER
                if (dbgConsole) {
                    vsConsoleDebug();
                }
#endif
            }
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
MainApplication::qmlInitialization()
{
    /*
     * Register accountListModel type.
     */
    QML_REGISTERTYPE(AccountListModel, 1, 0);
    QML_REGISTERTYPE(DeviceItemListModel, 1, 0);
    QML_REGISTERTYPE(PluginItemListModel, 1, 0);
    QML_REGISTERTYPE(MediaHandlerItemListModel, 1, 0);
    QML_REGISTERTYPE(PreferenceItemListModel, 1, 0);
    QML_REGISTERTYPE(BannedListModel, 1, 0);
    QML_REGISTERTYPE(VideoCodecListModel, 1, 0);
    QML_REGISTERTYPE(AudioCodecListModel, 1, 0);
    QML_REGISTERTYPE(AccountsToMigrateListModel, 1, 0);
    QML_REGISTERTYPE(AudioInputDeviceModel, 1, 0);
    QML_REGISTERTYPE(AudioOutputDeviceModel, 1, 0);
    QML_REGISTERTYPE(AudioManagerListModel, 1, 0);
    QML_REGISTERTYPE(VideoInputDeviceModel, 1, 0);
    QML_REGISTERTYPE(VideoFormatResolutionModel, 1, 0);
    QML_REGISTERTYPE(VideoFormatFpsModel, 1, 0);
    QML_REGISTERTYPE(PluginListPreferenceModel, 1, 0);
    QML_REGISTERTYPE(MediaHandlerListPreferenceModel, 1, 0);
    /*
     * Register QQuickItem type.
     */
    QML_REGISTERTYPE(PreviewRenderer, 1, 0);
    QML_REGISTERTYPE(VideoCallPreviewRenderer, 1, 0);
    QML_REGISTERTYPE(DistantRenderer, 1, 0);
    QML_REGISTERTYPE(PhotoboothPreviewRender, 1, 0)

    /*
     * Adapter - qmlRegisterSingletonType.
     * Note: in future, if lrc is fully compatible with qml (C++ struct
     *       is readable in qml), the adapters can be optimized away.
     */
    QML_REGISTERSINGLETONTYPE_URL(QStringLiteral("qrc:/src/constant/JamiTheme.qml"),
                                  JamiTheme,
                                  1,
                                  0);
    QML_REGISTERSINGLETONTYPE(CallAdapter, 1, 0);

    QML_REGISTERSINGLETONTYPE(MessagesAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE(ConversationsAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE(AvAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE(ContactAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE(MediaHandlerAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE(ClientWrapper, 1, 0);

    // QML_REGISTERSINGLETONTYPE_WITH_INSTANCE(AccountAdapter, 1, 0);
    // QML_REGISTERSINGLETONTYPE_WITH_INSTANCE(UtilsAdapter, 1, 0);
    QML_REGISTERUNCREATABLE(AccountAdapter, 1, 0);
    QML_REGISTERUNCREATABLE(UtilsAdapter, 1, 0);
    QML_REGISTERUNCREATABLE(SettingsAdaptor, 1, 0);
    QML_REGISTERUNCREATABLE(NameDirectory, 1, 0);
    QML_REGISTERUNCREATABLE(LRCInstance, 1, 0);

    /*
     * Lrc models - qmlRegisterUncreatableType & Q_DECLARE_METATYPE.
     * This to make lrc models recognizable in qml.
     */
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewAccountModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(BehaviorController, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(DataTransferModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(AVModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(ContactModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(ConversationModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewCallModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(PluginModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewDeviceModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewCodecModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(PeerDiscoveryModel, lrc::api, 1, 0);

    /*
     * Client models - qmlRegisterUncreatableType & Q_DECLARE_METATYPE.
     * This to make client models recognizable in qml.
     */
    QML_REGISTERUNCREATABLE(RenderManager, 1, 0);

    /*
     * Namespaces - qmlRegisterUncreatableMetaObject.
     */
    QML_REGISTERNAMESPACE(lrc::api::staticMetaObject, "Lrc", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::account::staticMetaObject, "Account", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::call::staticMetaObject, "Call", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::datatransfer::staticMetaObject, "Datatransfer", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::interaction::staticMetaObject, "Interaction", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::video::staticMetaObject, "Video", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::profile::staticMetaObject, "Profile", 1, 0);

    /*
     * Add image provider.
     */
    engine_->addImageProvider(QLatin1String("qrImage"), new QrImageProvider());
    engine_->addImageProvider(QLatin1String("tintedPixmap"), new TintedButtonImageProvider());

    engine_->load(QUrl(QStringLiteral("qrc:/src/MainApplicationWindow.qml")));
}

MainApplication::~MainApplication() {}

bool
MainApplication::applicationSetup()
{
#ifdef Q_OS_LINUX
    if (!getenv("QT_QPA_PLATFORMTHEME"))
        setenv("QT_QPA_PLATFORMTHEME", "gtk3", true);
#endif

    /*
     * Start debug console.
     */
    for (auto string : QCoreApplication::arguments()) {
        if (string == "-d" || string == "--debug") {
            consoleDebug();
        }
    }

    /*
     * Remove old version files.
     */
    Utils::removeOldVersions();

    /*
     * Load translations.
     */
    loadTranslations();

    /*
     * Set font.
     */
    setApplicationFont();

#if defined _MSC_VER && !COMPILE_ONLY
    gnutls_global_init();
#endif

    /*
     * Init pixmap manipulator.
     */
    GlobalInstances::setPixmapManipulator(std::make_unique<PixbufManipulator>());

    /*
     * Init lrc and its possible migration ui.
     */
    initLrc();

    /*
     * Process input argument.
     */
    bool startMinimized {false};
    processInputArgument(startMinimized);

    /*
     * Create jami.net settings in Registry if it is not presented.
     */
    QSettings settings("jami.net", "Jami");

    /*
     * Initialize qml components.
     */
    qmlInitialization();

    return true;
}
