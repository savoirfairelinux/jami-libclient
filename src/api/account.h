/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                                  *
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

// Std
#include <string>
#include <memory>

// Data
#include "profile.h"

// old LRC
#include "typedefs.h"

namespace lrc
{

namespace api
{

class NewCallModel;
class ContactModel;
class ConversationModel;
class NewAccountModel;
class NewDeviceModel;
class NewCodecModel;

namespace account
{

enum class Type {
    INVALID,
    RING,
    SIP
};

#pragma push_macro("REGISTERED")
#undef REGISTERED

enum class Status {
    INVALID,
    ERROR_NEED_MIGRATION,
    INITIALIZING,
    UNREGISTERED,
    TRYING,
    REGISTERED
};

static inline account::Status
to_status(const std::string& type)
{
    if (type == "INITIALIZING")
        return account::Status::INITIALIZING;
    else if (type == "UNREGISTERED")
        return account::Status::UNREGISTERED;
    else if (type == "TRYING")
        return account::Status::TRYING;
    else if (type == "REGISTERED")
        return account::Status::REGISTERED;
    else if (type == "ERROR_NEED_MIGRATION")
        return account::Status::ERROR_NEED_MIGRATION;
    else
        return account::Status::INVALID;
}

#pragma pop_macro("REGISTERED")

enum class KeyExchangeProtocol {
    NONE,
    SDES
};

enum class TlsMethod {
    DEFAULT,
    TLSv1,
    TLSv1_1,
    TLSv1_2
};

struct ConfProperties_t {
    std::string             displayName;
    std::string             mailbox;
    std::string             dtmfType;
    bool                    autoAnswer;
    int                     activeCallLimit;
    std::string             hostname;
    std::string             username;
    std::string             routeset;
    std::string             password;
    std::string             realm;
    std::string             localInterface;
    std::string             deviceId;
    std::string             deviceName;
    bool                    publishedSameAsLocal;
    int                     localPort;
    int                     publishedPort;
    std::string             publishedAddress;
    std::string             userAgent;
    bool                    upnpEnabled;
    bool                    hasCustomUserAgent;
    bool                    allowIncoming;
    std::string             archivePassword;
    bool                    archiveHasPassword;
    std::string             archivePath;
    std::string             archivePin;
    bool                    proxyEnabled;
    std::string             proxyServer;
    std::string             proxyPushToken;
    int                     registrationExpire;
    struct Audio_t {
        int                 audioPortMax;
        int                 audioPortMin;
    } Audio;
    struct Video_t {
        bool                videoEnabled;
        int                 videoPortMax;
        int                 videoPortMin;
    } Video;
    struct STUN_t {
        std::string         server;
        bool                enable;
    } STUN;
    struct TURN_t {
        std::string         server;
        bool                enable;
        std::string         username;
        std::string         password;
        std::string         realm;
    } TURN;
    struct Presence_t {
        bool                presencePublishSupported;
        bool                presenceSubscribeSupported;
        bool                presenceEnabled;
    } Presence;
    struct Ringtone_t {
        std::string         ringtonePath;
        bool                ringtoneEnabled;
    } Ringtone;
    struct SRTP_t {
        KeyExchangeProtocol keyExchange;
        bool                enable;
        bool                rtpFallback;
    } SRTP;
    struct TLS_t {
        int                 listenerPort;
        bool                enable;
        int                 port;
        std::string         certificateListFile;
        std::string         certificateFile;
        std::string         privateKeyFile;
        std::string         password;
        TlsMethod           method;
        std::string         ciphers;
        std::string         serverName;
        bool                verifyServer;
        bool                verifyClient;
        bool                requireClientCertificate;
        int                 negotiationTimeoutSec;
    } TLS;
    struct DHT_t {
        int                 port;
        bool                PublicInCalls;
        bool                AllowFromTrusted;
    } DHT;
    struct RingNS_t {
        std::string         uri;
        std::string         account;
    } RingNS;
    struct Registration_t {
        int                 expire;
    } Registration;

    MapStringString         toDetails() const;
};

// Possible account export status
enum class ExportOnRingStatus {
    SUCCESS = 0,
    WRONG_PASSWORD = 1 ,
    NETWORK_ERROR = 2,
    INVALID
};

enum class RegisterNameStatus {
    SUCCESS = 0,
    WRONG_PASSWORD = 1,
    INVALID_NAME = 2,
    ALREADY_TAKEN = 3,
    NETWORK_ERROR = 4,
    INVALID
};

enum class LookupStatus {
    SUCCESS = 0,
    INVALID_NAME = 1,
    NOT_FOUND = 2,
    ERROR = 3,
    INVALID
};

struct Info
{
    bool freeable = false;
    bool valid = true;
    std::string registeredName;
    Status status = account::Status::INVALID;
    std::unique_ptr<lrc::api::NewCallModel> callModel;
    std::unique_ptr<lrc::api::ContactModel> contactModel;
    std::unique_ptr<lrc::api::ConversationModel> conversationModel;
    std::unique_ptr<lrc::api::NewDeviceModel> deviceModel;
    std::unique_ptr<lrc::api::NewCodecModel> codecModel;
    NewAccountModel* accountModel {nullptr};

    // daemon config
    std::string             id;
    profile::Info           profileInfo; // contains: type, alias
    bool                    enabled;
    ConfProperties_t        confProperties;

    // load/save
    void                    fromDetails(const MapStringString& details);
};

} // namespace account
} // namespace api
} // namespace lrc
