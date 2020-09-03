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
#include "audiocodeclistmodel.h"
#include "audioinputdevicemodel.h"
#include "audiomanagerlistmodel.h"
#include "audiooutputdevicemodel.h"
#include "avadapter.h"
#include "bannedlistmodel.h"
#include "calladapter.h"
#include "clientwrapper.h"
#include "contactadapter.h"
#include "conversationsadapter.h"
#include "deviceitemlistmodel.h"
#include "distantrenderer.h"
#include "mediahandleradapter.h"
#include "mediahandleritemlistmodel.h"
#include "mediahandlerlistpreferencemodel.h"
#include "messagesadapter.h"
#include "namedirectory.h"
#include "preferenceitemlistmodel.h"
#include "pluginitemlistmodel.h"
#include "pluginlistpreferencemodel.h"
#include "previewrenderer.h"
#include "settingsadapter.h"
#include "utils.h"
#include "version.h"
#include "videocodeclistmodel.h"
#include "videoformatfpsmodel.h"
#include "videoformatresolutionmodel.h"
#include "videoinputdevicemodel.h"

#include <QMetaType>
#include <QQmlEngine>

#define QML_REGISTERSINGLETONTYPE(N, T, MAJ, MIN) \
    qmlRegisterSingletonType<T>(N, MAJ, MIN, #T, \
                                [](QQmlEngine *e, QJSEngine *se) -> QObject * { \
                                    Q_UNUSED(e); \
                                    Q_UNUSED(se); \
                                    T *obj = new T(); \
                                    return obj; \
                                });
#define QML_REGISTERSINGLETONTYPE_WITH_INSTANCE(T, MAJ, MIN) \
    qmlRegisterSingletonType<T>("net.jami.Models", \
                                MAJ, \
                                MIN, \
                                #T, \
                                [](QQmlEngine *e, QJSEngine *se) -> QObject * { \
                                    Q_UNUSED(e); \
                                    Q_UNUSED(se); \
                                    return &(T::instance()); \
                                });

#define QML_REGISTERSINGLETONTYPE_URL(URL, T, MAJ, MIN) \
    qmlRegisterSingletonType(QUrl(URL), "net.jami.Models", MAJ, MIN, #T);

#define QML_REGISTERTYPE(T, MAJ, MIN) qmlRegisterType<T>("net.jami.Models", MAJ, MIN, #T);

#define QML_REGISTERNAMESPACE(T, NAME, MAJ, MIN) \
    qmlRegisterUncreatableMetaObject(T, "net.jami.Models", MAJ, MIN, NAME, "")

#define QML_REGISTERUNCREATABLE(N, T, MAJ, MIN) \
    qmlRegisterUncreatableType<T>(N, \
                                  MAJ, \
                                  MIN, \
                                  #T, \
                                  "Don't try to add to a qml definition of " #T);

#define QML_REGISTERUNCREATABLE_IN_NAMESPACE(T, NAMESPACE, MAJ, MIN) \
    qmlRegisterUncreatableType<NAMESPACE::T>("net.jami.Models", \
                                             MAJ, \
                                             MIN, \
                                             #T, \
                                             "Don't try to add to a qml definition of " #T);

/*!
 * This function will expose custom types to the QML engine.
 */
void registerTypes()
{
    /*
     * Register QAbstractListModel type.
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
     * Adaptors and qml singleton components - qmlRegisterSingletonType.
     */
    QML_REGISTERSINGLETONTYPE_URL(QStringLiteral("qrc:/src/constant/JamiTheme.qml"),
                                  JamiTheme, 1, 0);
    QML_REGISTERSINGLETONTYPE_URL(QStringLiteral("qrc:/src/constant/JamiQmlUtils.qml"),
                                  JamiQmlUtils, 1, 0);

    QML_REGISTERSINGLETONTYPE("net.jami.Models", CallAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", MessagesAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", ConversationsAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", AvAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", ContactAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", MediaHandlerAdapter, 1, 0);
    QML_REGISTERSINGLETONTYPE("net.jami.Models", ClientWrapper, 1, 0);


    QML_REGISTERSINGLETONTYPE("net.jami.Adapters", SettingsAdapter, 1, 0);
    QML_REGISTERUNCREATABLE("net.jami.Enums", Settings, 1, 0);

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
     * qmlRegisterUncreatableType & Q_DECLARE_METATYPE to expose models in qml.
     */
    QML_REGISTERUNCREATABLE("net.jami.Models", RenderManager, 1, 0);
    QML_REGISTERUNCREATABLE("net.jami.Models", AccountAdapter, 1, 0);
    QML_REGISTERUNCREATABLE("net.jami.Models", UtilsAdapter, 1, 0);
    QML_REGISTERUNCREATABLE("net.jami.Models", NameDirectory, 1, 0);
    QML_REGISTERUNCREATABLE("net.jami.Models", LRCInstance, 1, 0);

    /*
     * qmlRegisterUncreatableMetaObject to expose namespaces in qml
     */
    QML_REGISTERNAMESPACE(lrc::api::staticMetaObject, "Lrc", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::account::staticMetaObject, "Account", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::call::staticMetaObject, "Call", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::datatransfer::staticMetaObject, "Datatransfer", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::interaction::staticMetaObject, "Interaction", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::video::staticMetaObject, "Video", 1, 0);
    QML_REGISTERNAMESPACE(lrc::api::profile::staticMetaObject, "Profile", 1, 0);
}
