/*
 * Copyright (C) 2021 by Savoir-faire Linux
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

#pragma once

#include "lrcinstance.h"
#include "qtutils.h"
#include "appsettingsmanager.h"

#include <QObject>
#include <QString>

#define ACCOUNT_CONFIG_SETTINGS_PROPERTY_BASE(type, prop) \
    PROPERTY_GETTER_BASE(type, prop) \
    void set_##prop(const type& x = {}, bool initialize = false) \
    { \
        if (prop##_ != x) { \
            prop##_ = x; \
            if (!initialize) { \
                auto confProps = lrcInstance_->getCurrAccConfig(); \
                confProps.prop = x; \
                lrcInstance_->accountModel().setAccountConfig(lrcInstance_->get_currentAccountId(), \
                                                              confProps); \
            } \
            Q_EMIT prop##Changed(); \
        } \
    }

#define NEW_ACCOUNT_MODEL_SETTINGS_PROPERTY_BASE(type, prop, appSettingName) \
    PROPERTY_GETTER_BASE(type, prop) \
    void set_##prop(const type& x = {}, bool initialize = false) \
    { \
        if (prop##_ != x) { \
            prop##_ = x; \
            lrcInstance_->accountModel().prop = x; \
            if (!initialize) { \
                settingsManager_->setValue(Settings::Key::appSettingName, x); \
            } \
            Q_EMIT prop##Changed(); \
        } \
    }

#define ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY_BASE(type, prop, cate) \
    type prop##cate##_ {}; \
\
public: \
    Q_SIGNAL void prop##cate##Changed(); \
    type get_##prop##cate() \
    { \
        return prop##cate##_; \
    } \
    void set_##prop##cate(const type& x = {}, bool initialize = false) \
    { \
        if (prop##cate##_ != x) { \
            prop##cate##_ = x; \
            if (!initialize) { \
                auto confProps = lrcInstance_->getCurrAccConfig(); \
                confProps.cate.prop = x; \
                lrcInstance_->accountModel().setAccountConfig(lrcInstance_->get_currentAccountId(), \
                                                              confProps); \
            } \
            Q_EMIT prop##cate##Changed(); \
        } \
    }

#define QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(type, prop) \
private: \
    Q_PROPERTY(type prop READ get_##prop WRITE set_##prop NOTIFY prop##Changed); \
    ACCOUNT_CONFIG_SETTINGS_PROPERTY_BASE(type, prop)

#define QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(type, prop, cate) \
private: \
    Q_PROPERTY(type prop##_##cate READ get_##prop##cate WRITE set_##prop##cate NOTIFY \
                   prop##cate##Changed); \
    ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY_BASE(type, prop, cate)

#define QML_NEW_ACCOUNT_MODEL_SETTINGS_PROPERTY(type, prop, appSettingName) \
private: \
    Q_PROPERTY(type prop READ get_##prop WRITE set_##prop NOTIFY prop##Changed); \
    NEW_ACCOUNT_MODEL_SETTINGS_PROPERTY_BASE(type, prop, appSettingName)

class CurrentAccount final : public QObject
{
    Q_OBJECT
    // Basic settings
    Q_PROPERTY(bool enabled READ get_enabled WRITE set_enabled NOTIFY enabledChanged);

    QML_RO_PROPERTY(QString, id)
    QML_RO_PROPERTY(QString, uri)
    QML_RO_PROPERTY(QString, registeredName)
    QML_RO_PROPERTY(QString, alias)
    QML_RO_PROPERTY(QString, bestId)
    QML_RO_PROPERTY(QString, bestName)
    QML_RO_PROPERTY(QString, managerUri)
    QML_RO_PROPERTY(bool, hasAvatarSet)
    QML_RO_PROPERTY(lrc::api::account::Status, status)
    QML_RO_PROPERTY(lrc::api::profile::Type, type)

    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(bool, keepAliveEnabled)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(bool, peerDiscovery)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(bool, sendReadReceipt)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(bool, isRendezVous)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(bool, autoAnswer)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(bool, proxyEnabled)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(bool, upnpEnabled)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(bool, publishedSameAsLocal)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(bool, allowIPAutoRewrite)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(QString, proxyServer)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(QString, routeset)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(QString, username)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(QString, hostname)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(QString, password)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(QString, mailbox)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(QString, publishedAddress)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(int, localPort)
    QML_ACCOUNT_CONFIG_SETTINGS_PROPERTY(int, publishedPort)

    // Moderator settings
    Q_PROPERTY(bool isAllModeratorsEnabled READ get_isAllModeratorsEnabled WRITE
                   set_isAllModeratorsEnabled NOTIFY isAllModeratorsEnabledChanged)
    Q_PROPERTY(bool isLocalModeratorsEnabled READ get_isLocalModeratorsEnabled WRITE
                   set_isLocalModeratorsEnabled NOTIFY isLocalModeratorsEnabledChanged)

    // RingNS setting
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, uri, RingNS)

    // DHT settings
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, PublicInCalls, DHT)

    // TLS settings
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, enable, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, verifyServer, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, verifyClient, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, requireClientCertificate, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, certificateListFile, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, certificateFile, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, privateKeyFile, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, password, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, serverName, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(lrc::api::account::TlsMethod, method, TLS)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(int, negotiationTimeoutSec, TLS)

    // SRTP settings
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, enable, SRTP)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, rtpFallback, SRTP)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(lrc::api::account::KeyExchangeProtocol,
                                                  keyExchange,
                                                  SRTP)

    // TURN settings
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, enable, TURN)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, server, TURN)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, username, TURN)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, password, TURN)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, realm, TURN)

    // STUN settings
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, enable, STUN)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, server, STUN)

    // Video & Audio settings
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, videoEnabled, Video)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(int, videoPortMin, Video)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(int, videoPortMax, Video)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(int, audioPortMin, Audio)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(int, audioPortMax, Audio)

    // Ringtone settings
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(bool, ringtoneEnabled, Ringtone)
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(QString, ringtonePath, Ringtone)

    // Registration settings
    QML_ACCOUNT_CONFIG_CATEGORY_SETTINGS_PROPERTY(int, expire, Registration)

    // NewAccount model settings
    QML_NEW_ACCOUNT_MODEL_SETTINGS_PROPERTY(bool, autoTransferFromTrusted, AutoAcceptFiles)
    QML_NEW_ACCOUNT_MODEL_SETTINGS_PROPERTY(bool, autoTransferFromUntrusted, AllowFromUntrusted)
    QML_NEW_ACCOUNT_MODEL_SETTINGS_PROPERTY(int, autoTransferSizeThreshold, AcceptTransferBelow)

public:
    explicit CurrentAccount(LRCInstance* lrcInstance,
                            AppSettingsManager* settingsManager,
                            QObject* parent = nullptr);
    ~CurrentAccount() = default;

    void set_enabled(bool enabled = false, bool initialize = false);
    bool get_enabled();

    void set_isAllModeratorsEnabled(bool enabled, bool initialize = false);
    bool get_isAllModeratorsEnabled();

    void set_isLocalModeratorsEnabled(bool enabled, bool initialize = false);
    bool get_isLocalModeratorsEnabled();

Q_SIGNALS:
    void enabledChanged();
    void isAllModeratorsEnabledChanged();
    void isLocalModeratorsEnabledChanged();

private Q_SLOTS:
    void updateData();
    void onAccountUpdated(const QString& id);

private:
    bool enabled_;
    bool isAllModeratorsEnabled_;
    bool isLocalModeratorsEnabled_;

    AppSettingsManager* settingsManager_;
    LRCInstance* lrcInstance_;
};
