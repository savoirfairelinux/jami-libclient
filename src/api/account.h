/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                       *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#pragma once

#include "profile.h"

#include "typedefs.h"

#include <memory>

#include <QString>

namespace lrc {

namespace api {

class ContactModel;
class ConversationModel;
class NewCallModel;
class NewAccountModel;
class NewDeviceModel;
class NewCodecModel;
class PeerDiscoveryModel;
class DataTransferModel;

namespace account {
Q_NAMESPACE
Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

enum class Type { INVALID, JAMI, SIP };
Q_ENUM_NS(Type)

#pragma push_macro("REGISTERED")
#undef REGISTERED

enum class Status { INVALID, ERROR_NEED_MIGRATION, INITIALIZING, UNREGISTERED, TRYING, REGISTERED };
Q_ENUM_NS(Status)

static inline account::Status
to_status(const QString& type)
{
    if (type == "INITIALIZING")
        return account::Status::INITIALIZING;
    else if (type == "UNREGISTERED")
        return account::Status::UNREGISTERED;
    else if (type == "TRYING")
        return account::Status::TRYING;
    else if (type == "REGISTERED" || type == "READY")
        return account::Status::REGISTERED;
    else if (type == "ERROR_NEED_MIGRATION")
        return account::Status::ERROR_NEED_MIGRATION;
    else
        return account::Status::INVALID;
}

#pragma pop_macro("REGISTERED")

enum class KeyExchangeProtocol { NONE, SDES };
Q_ENUM_NS(KeyExchangeProtocol)

enum class TlsMethod { DEFAULT, TLSv1, TLSv1_1, TLSv1_2 };
Q_ENUM_NS(TlsMethod)

struct ConfProperties_t
{
    QString mailbox;
    QString dtmfType;
    bool autoAnswer;
    bool receiveCallWhenBusy;
    bool sendReadReceipt;
    bool isRendezVous;
    int activeCallLimit;
    QString hostname;
    QString username;
    QString routeset;
    QString password;
    QString realm;
    QString localInterface;
    QString deviceId;
    QString deviceName;
    QString managerUri;
    QString managerUsername;
    bool publishedSameAsLocal;
    int localPort;
    int publishedPort;
    QString publishedAddress;
    QString userAgent;
    bool upnpEnabled;
    bool hasCustomUserAgent;
    bool allowIncoming;
    bool allowIPAutoRewrite;
    QString archivePassword;
    bool archiveHasPassword;
    QString archivePath;
    QString archivePin;
    bool proxyEnabled;
    QString proxyServer;
    QString proxyPushToken;
    bool peerDiscovery;
    bool accountDiscovery;
    bool accountPublish;
    int registrationExpire;
    bool keepAliveEnabled;
    QString bootstrapListUrl;
    QString dhtProxyListUrl;
    QString defaultModerators;
    bool localModeratorsEnabled;
    VectorMapStringString credentials;
    struct Audio_t
    {
        int audioPortMax;
        int audioPortMin;
    } Audio;
    struct Video_t
    {
        bool videoEnabled;
        int videoPortMax;
        int videoPortMin;
    } Video;
    struct STUN_t
    {
        QString server;
        bool enable;
    } STUN;
    struct TURN_t
    {
        QString server;
        bool enable;
        QString username;
        QString password;
        QString realm;
    } TURN;
    struct Presence_t
    {
        bool presencePublishSupported;
        bool presenceSubscribeSupported;
        bool presenceEnabled;
    } Presence;
    struct Ringtone_t
    {
        QString ringtonePath;
        bool ringtoneEnabled;
    } Ringtone;
    struct SRTP_t
    {
        KeyExchangeProtocol keyExchange;
        bool enable;
        bool rtpFallback;
    } SRTP;
    struct TLS_t
    {
        int listenerPort;
        bool enable;
        int port;
        QString certificateListFile;
        QString certificateFile;
        QString privateKeyFile;
        QString password;
        TlsMethod method;
        QString ciphers;
        QString serverName;
        bool verifyServer;
        bool verifyClient;
        bool requireClientCertificate;
        int negotiationTimeoutSec;
    } TLS;
    struct DHT_t
    {
        int port;
        bool PublicInCalls;
        bool AllowFromTrusted;
    } DHT;
    struct RingNS_t
    {
        QString uri;
        QString account;
    } RingNS;
    struct Registration_t
    {
        int expire;
    } Registration;

    MapStringString toDetails() const;
};

// Possible account export status
enum class ExportOnRingStatus { SUCCESS = 0, WRONG_PASSWORD = 1, NETWORK_ERROR = 2, INVALID };
Q_ENUM_NS(ExportOnRingStatus)

enum class RegisterNameStatus {
    SUCCESS = 0,
    WRONG_PASSWORD = 1,
    INVALID_NAME = 2,
    ALREADY_TAKEN = 3,
    NETWORK_ERROR = 4,
    INVALID
};
Q_ENUM_NS(RegisterNameStatus)

enum class LookupStatus { SUCCESS = 0, INVALID_NAME = 1, NOT_FOUND = 2, ERROR = 3, INVALID };
Q_ENUM_NS(LookupStatus)

struct Info
{
    bool freeable = false;
    bool valid = true;
    QString registeredName;
    Status status = account::Status::INVALID;
    std::unique_ptr<lrc::api::NewCallModel> callModel;
    std::unique_ptr<lrc::api::ContactModel> contactModel;
    std::unique_ptr<lrc::api::ConversationModel> conversationModel;
    std::unique_ptr<lrc::api::NewDeviceModel> deviceModel;
    std::unique_ptr<lrc::api::NewCodecModel> codecModel;
    std::unique_ptr<lrc::api::PeerDiscoveryModel> peerDiscoveryModel;
    std::unique_ptr<DataTransferModel> dataTransferModel;
    NewAccountModel* accountModel {nullptr};

    // daemon config
    QString id;
    profile::Info profileInfo; // contains: type, alias
    bool enabled;
    ConfProperties_t confProperties;

    // load/save
    void fromDetails(const MapStringString& details);
};

} // namespace account
} // namespace api
} // namespace lrc
