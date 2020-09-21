/*!
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "qmlregister.h"

#include "accountadapter.h"
#include "accountstomigratelistmodel.h"
#include "mediacodeclistmodel.h"
#include "audioinputdevicemodel.h"
#include "audiomanagerlistmodel.h"
#include "audiooutputdevicemodel.h"
#include "avadapter.h"
#include "bannedlistmodel.h"
#include "calladapter.h"
#include "contactadapter.h"
#include "conversationsadapter.h"
#include "deviceitemlistmodel.h"
#include "distantrenderer.h"
#include "pluginadapter.h"
#include "mediahandleritemlistmodel.h"
#include "messagesadapter.h"
#include "namedirectory.h"
#include "preferenceitemlistmodel.h"
#include "pluginitemlistmodel.h"
#include "pluginlistpreferencemodel.h"
#include "previewrenderer.h"
#include "settingsadapter.h"
#include "utilsadapter.h"
#include "version.h"
#include "videoformatfpsmodel.h"
#include "videoformatresolutionmodel.h"
#include "videoinputdevicemodel.h"

#include <QMetaType>
#include <QQmlEngine>

// clang-format off
#define QML_REGISTERSINGLETONTYPE(N, T, MAJ, MIN) \
    qmlRegisterSingletonType<T>(N, MAJ, MIN, #T, \
                                [](QQmlEngine* e, QJSEngine* se) -> QObject* { \
                                    Q_UNUSED(e); Q_UNUSED(se); \
                                    T* obj = new T(); return obj; \
                                });

#define QML_REGISTERSINGLETONTYPE_WITH_INSTANCE(T, MAJ, MIN) \
    qmlRegisterSingletonType<T>("net.jami.Models", MAJ, MIN, #T, \
                                [](QQmlEngine* e, QJSEngine* se) -> QObject* { \
                                    Q_UNUSED(e); Q_UNUSED(se); \
                                    return &(T::instance()); \
                                });

#define QML_REGISTERSINGLETONTYPE_CUSTOM(N, T, MAJ, MIN, P) \
    qmlRegisterSingletonType<T>(N, MAJ, MIN, #T, \
                                [](QQmlEngine* e, QJSEngine* se) -> QObject* { \
                                    Q_UNUSED(e); Q_UNUSED(se); \
                                    return P; \
                                });

#define QML_REGISTERSINGLETONTYPE_URL(URL, T, MAJ, MIN) \
    qmlRegisterSingletonType(QUrl(QStringLiteral(URL)), "net.jami.Models", MAJ, MIN, #T);

#define QML_REGISTERTYPE(N, T, MAJ, MIN) qmlRegisterType<T>(N, MAJ, MIN, #T);

#define QML_REGISTERNAMESPACE(T, NAME, MAJ, MIN) \
    qmlRegisterUncreatableMetaObject(T, "net.jami.Models", MAJ, MIN, NAME, "")

#define QML_REGISTERUNCREATABLE(N, T, MAJ, MIN) \
    qmlRegisterUncreatableType<T>(N, MAJ, MIN, #T, "Don't try to add to a qml definition of " #T);

#define QML_REGISTERUNCREATABLE_IN_NAMESPACE(T, NAMESPACE, MAJ, MIN) \
    qmlRegisterUncreatableType<NAMESPACE::T>("net.jami.Models", \
                                             MAJ, MIN, #T, \
                                             "Don't try to add to a qml definition of " #T);

/*!
 * This function will expose custom types to the QML engine.
 */
void
registerTypes()
{
    /*
     * QAbstractListModels
     */
    QML_REGISTERTYPE("net.jami.Models", AccountListModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", DeviceItemListModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", PluginItemListModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", MediaHandlerItemListModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", PreferenceItemListModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", BannedListModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", MediaCodecListModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", AccountsToMigrateListModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", AudioInputDeviceModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", AudioOutputDeviceModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", AudioManagerListModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", VideoInputDeviceModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", VideoFormatResolutionModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", VideoFormatFpsModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", PluginListPreferenceModel, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", SmartListModel, 1, 0);

    /*
     * QQuickItems
     */
    QML_REGISTERTYPE("net.jami.Models", PreviewRenderer, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", VideoCallPreviewRenderer, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", DistantRenderer, 1, 0);
    QML_REGISTERTYPE("net.jami.Models", PhotoboothPreviewRender, 1, 0)

    /*
     * Adaptors
     */
    QML_REGISTERSINGLETONTYPE("net.jami.Models", CallAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", MessagesAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", ConversationsAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", AvAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", ContactAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", PluginAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Adapters", AccountAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Adapters", UtilsAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Adapters", SettingsAdapter, 1, 0);

    QML_REGISTERSINGLETONTYPE_CUSTOM("net.jami.Models", AVModel, 1, 0, &LRCInstance::avModel())
    QML_REGISTERSINGLETONTYPE_CUSTOM("net.jami.Models", PluginModel, 1, 0, &LRCInstance::pluginModel())
    QML_REGISTERSINGLETONTYPE_CUSTOM("net.jami.Models", RenderManager, 1, 0, LRCInstance::renderer())

    /*
     * Qml singleton components
     */
    QML_REGISTERSINGLETONTYPE_URL("qrc:/src/constant/JamiTheme.qml", JamiTheme, 1, 0);
    QML_REGISTERSINGLETONTYPE_URL("qrc:/src/constant/JamiQmlUtils.qml", JamiQmlUtils, 1, 0);
    QML_REGISTERSINGLETONTYPE_URL("qrc:/src/constant/JamiStrings.qml", JamiStrings, 1, 0);

    /*
     * C++ singletons
     */
    QML_REGISTERSINGLETONTYPE_WITH_INSTANCE(LRCInstance, 1, 0);
    QML_REGISTERSINGLETONTYPE_WITH_INSTANCE(NameDirectory, 1, 0);

    /*
     * lrc namespaces, models, and singletons
     */
    QML_REGISTERNAMESPACE(lrc::api::staticMetaObject, "Lrc", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::account::staticMetaObject, "Account", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::call::staticMetaObject, "Call", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::datatransfer::staticMetaObject, "Datatransfer", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::interaction::staticMetaObject, "Interaction", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::video::staticMetaObject, "Video", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::profile::staticMetaObject, "Profile", 1, 0);

    /*
     * same as QML_REGISTERUNCREATABLE but omit the namespace in Qml
     */
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewAccountModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(BehaviorController, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(DataTransferModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(ContactModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(ConversationModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewCallModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewDeviceModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewCodecModel, lrc::api, 1, 0);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(PeerDiscoveryModel, lrc::api, 1, 0);

    /*
     * Enums
     */
    QML_REGISTERUNCREATABLE("net.jami.Enums", Settings, 1, 0);
}
// clang-format on
