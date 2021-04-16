/*
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

#include "accountlistmodel.h"
#include "accountstomigratelistmodel.h"
#include "mediacodeclistmodel.h"
#include "audiodevicemodel.h"
#include "audiomanagerlistmodel.h"
#include "bannedlistmodel.h"
#include "moderatorlistmodel.h"
#include "deviceitemlistmodel.h"
#include "smartlistmodel.h"
#include "conversationlistmodelbase.h"

#include "appsettingsmanager.h"
#include "distantrenderer.h"
#include "namedirectory.h"
#include "updatemanager.h"
#include "pluginlistpreferencemodel.h"
#include "previewrenderer.h"
#include "version.h"
#include "videoformatfpsmodel.h"
#include "videoformatresolutionmodel.h"
#include "videoinputdevicemodel.h"

#include "api/peerdiscoverymodel.h"
#include "api/newcodecmodel.h"
#include "api/newdevicemodel.h"
#include "api/datatransfermodel.h"
#include "api/pluginmodel.h"

#include <QMetaType>
#include <QQmlEngine>

// clang-format off
// TODO: remove this
#define QML_REGISTERSINGLETONTYPE_WITH_INSTANCE(T) \
    qmlRegisterSingletonType<T>(NS_MODELS, VER_MAJ, VER_MIN, #T, \
                                [](QQmlEngine* e, QJSEngine* se) -> QObject* { \
                                    Q_UNUSED(e); Q_UNUSED(se); \
                                    return &(T::instance()); \
                                });

#define QML_REGISTERSINGLETONTYPE_URL(NS, URL, T) \
    qmlRegisterSingletonType(QUrl(QStringLiteral(URL)), NS, VER_MAJ, VER_MIN, #T);

#define QML_REGISTERTYPE(NS, T) qmlRegisterType<T>(NS, VER_MAJ, VER_MIN, #T);

#define QML_REGISTERNAMESPACE(NS, T, NAME) \
    qmlRegisterUncreatableMetaObject(T, NS, VER_MAJ, VER_MIN, NAME, "")

#define QML_REGISTERUNCREATABLE(N, T) \
    qmlRegisterUncreatableType<T>(N, VER_MAJ, VER_MIN, #T, "Don't try to add to a qml definition of " #T);

#define QML_REGISTERUNCREATABLE_IN_NAMESPACE(T, NAMESPACE) \
    qmlRegisterUncreatableType<NAMESPACE::T>(NS_MODELS, \
                                             VER_MAJ, VER_MIN, #T, \
                                             "Don't try to add to a qml definition of " #T);

namespace Utils {

/*!
 * This function will expose custom types to the QML engine.
 */
void
registerTypes()
{
    // Hack for QtCreator autocomplete (part 2)
    // https://bugreports.qt.io/browse/QTCREATORBUG-20569
    // Use a dummy object to register the import namespace.
    // This occurs when we register from within MainApplication
    QML_REGISTERNAMESPACE(NS_MODELS, dummy::staticMetaObject, "");
    QML_REGISTERNAMESPACE(NS_ADAPTERS, dummy::staticMetaObject, "");
    QML_REGISTERNAMESPACE(NS_CONSTANTS, dummy::staticMetaObject, "");
    QML_REGISTERNAMESPACE(NS_HELPERS, dummy::staticMetaObject, "");
    QML_REGISTERNAMESPACE(NS_ENUMS, dummy::staticMetaObject, "");

    // QAbstractListModels
    QML_REGISTERTYPE(NS_MODELS, AccountListModel);
    QML_REGISTERTYPE(NS_MODELS, DeviceItemListModel);
    QML_REGISTERTYPE(NS_MODELS, BannedListModel);
    QML_REGISTERTYPE(NS_MODELS, ModeratorListModel);
    QML_REGISTERTYPE(NS_MODELS, MediaCodecListModel);
    QML_REGISTERTYPE(NS_MODELS, AccountsToMigrateListModel);
    QML_REGISTERTYPE(NS_MODELS, AudioDeviceModel);
    QML_REGISTERTYPE(NS_MODELS, AudioManagerListModel);
    QML_REGISTERTYPE(NS_MODELS, VideoInputDeviceModel);
    QML_REGISTERTYPE(NS_MODELS, VideoFormatResolutionModel);
    QML_REGISTERTYPE(NS_MODELS, VideoFormatFpsModel);
    QML_REGISTERTYPE(NS_MODELS, PluginListPreferenceModel);
    QML_REGISTERTYPE(NS_MODELS, SmartListModel);

    // Roles & type enums for models
    QML_REGISTERNAMESPACE(NS_MODELS, ConversationList::staticMetaObject, "ConversationList");
    QML_REGISTERNAMESPACE(NS_MODELS, ContactList::staticMetaObject, "ContactList");

    // QQuickItems
    QML_REGISTERTYPE(NS_MODELS, PreviewRenderer);
    QML_REGISTERTYPE(NS_MODELS, VideoCallPreviewRenderer);
    QML_REGISTERTYPE(NS_MODELS, DistantRenderer);
    QML_REGISTERTYPE(NS_MODELS, PhotoboothPreviewRender)

    // Qml singleton components
    QML_REGISTERSINGLETONTYPE_URL(NS_CONSTANTS, "qrc:/src/constant/JamiTheme.qml", JamiTheme);
    QML_REGISTERSINGLETONTYPE_URL(NS_MODELS, "qrc:/src/constant/JamiQmlUtils.qml", JamiQmlUtils);
    QML_REGISTERSINGLETONTYPE_URL(NS_CONSTANTS, "qrc:/src/constant/JamiStrings.qml", JamiStrings);

    // C++ singletons
    // TODO: remove this
    QML_REGISTERSINGLETONTYPE_WITH_INSTANCE(NameDirectory);

    // Lrc namespaces, models, and singletons
    QML_REGISTERNAMESPACE(NS_MODELS, lrc::api::staticMetaObject, "Lrc");
    QML_REGISTERNAMESPACE(NS_MODELS, lrc::api::account::staticMetaObject, "Account");
    QML_REGISTERNAMESPACE(NS_MODELS, lrc::api::call::staticMetaObject, "Call");
    QML_REGISTERNAMESPACE(NS_MODELS, lrc::api::datatransfer::staticMetaObject, "Datatransfer");
    QML_REGISTERNAMESPACE(NS_MODELS, lrc::api::interaction::staticMetaObject, "Interaction");
    QML_REGISTERNAMESPACE(NS_MODELS, lrc::api::video::staticMetaObject, "Video");
    QML_REGISTERNAMESPACE(NS_MODELS, lrc::api::profile::staticMetaObject, "Profile");

    // Same as QML_REGISTERUNCREATABLE but omit the namespace in Qml
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewAccountModel, lrc::api);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(BehaviorController, lrc::api);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(DataTransferModel, lrc::api);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(ContactModel, lrc::api);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(ConversationModel, lrc::api);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewCallModel, lrc::api);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewDeviceModel, lrc::api);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(NewCodecModel, lrc::api);
    QML_REGISTERUNCREATABLE_IN_NAMESPACE(PeerDiscoveryModel, lrc::api);

    // Enums
    QML_REGISTERUNCREATABLE(NS_ENUMS, Settings);
    QML_REGISTERUNCREATABLE(NS_ENUMS, NetWorkManager);
}
// clang-format on
} // namespace Utils
