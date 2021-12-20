/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                             *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>       *
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

#include "typedefs.h"
#include "api/account.h"

#include <QObject>

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>

namespace lrc {

class CallbacksHandler;
class Database;
class NewAccountModelPimpl;

namespace api {

class Lrc;
class BehaviorController;

/**
 *  @brief Class that manages account information.
 */
class LIB_EXPORT NewAccountModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString downloadDirectory_qml MEMBER downloadDirectory)
public:
    NewAccountModel(Lrc& lrc,
                    const CallbacksHandler& callbackHandler,
                    const api::BehaviorController& behaviorController,
                    MigrationCb& willMigrateCb,
                    MigrationCb& didMigrateCb);

    ~NewAccountModel();
    /**
     * @return a list of all acountId.
     */
    Q_INVOKABLE QStringList getAccountList() const;
    /**
     * get account informations associated to an accountId.
     * @param accountId.
     * @return a const account::Info& structure.
     */
    const account::Info& getAccountInfo(const QString& accountId) const;
    /**
     * flag account corresponding to passed id as freeable.
     */
    void flagFreeable(const QString& accountId) const;

    /**
     * Used when images < 20 Mb are automatically accepted and downloaded
     * Should contains the full directory with the end marker (/ on linux for example)
     */
    QString downloadDirectory;
    /**
     * Accept transfer from untrusted contacts
     */
    bool autoTransferFromUntrusted {false};
    /**
     * Accept transfer from trusted contacts
     */
    bool autoTransferFromTrusted {true};
    /**
     * Automatically accept transfer under
     */
    unsigned autoTransferSizeThreshold {20} /* Mb */;
    /**
     * set account enable/disable, save config and do unregister for account
     * @param accountId.
     * @param enabled.
     */
    Q_INVOKABLE void setAccountEnabled(const QString& accountID, bool enabled) const;
    /**
     * saves account config to .yml
     * @param accountId.
     * @param reference to the confProperties
     */
    void setAccountConfig(const QString& accountID,
                          const account::ConfProperties_t& confProperties) const;
    /**
     * gets a copy of the accounts config
     * @param accountId.
     * @return an account::Info::ConfProperties_t structure.
     */
    account::ConfProperties_t getAccountConfig(const QString& accountId) const;
    /**
     * Call exportToFile from the daemon
     * @param accountId
     * @param path destination
     * @param password
     * @return if the file is exported with success
     */
    Q_INVOKABLE bool exportToFile(const QString& accountId,
                                  const QString& path,
                                  const QString& password = {}) const;
    /**
     * Call exportOnRing from the daemon
     * @param accountId
     * @param password
     * @return if the export is initialized
     */
    Q_INVOKABLE bool exportOnRing(const QString& accountId, const QString& password) const;
    /**
     * Call removeAccount from the daemon
     * @param accountId to remove
     * @note will emit accountRemoved
     */
    Q_INVOKABLE void removeAccount(const QString& accountId) const;
    /**
     * Call changeAccountPassword from the daemon
     * @param accountId
     * @param currentPassword
     * @param newPassword
     * @return if the password has been changed
     */
    bool changeAccountPassword(const QString& accountId,
                               const QString& currentPassword,
                               const QString& newPassword) const;
    /**
     * Change the avatar of an account
     * @param accountId
     * @param avatar
     * @throws out_of_range exception if account is not found
     */
    void setAvatar(const QString& accountId, const QString& avatar);
    /**
     * Change the alias of an account
     * @param accountId
     * @param alias
     * @throws out_of_range exception if account is not found
     */
    void setAlias(const QString& accountId, const QString& alias);
    /**
     * Try to register a name
     * @param accountId
     * @param password
     * @param username
     * @return if operation started
     */
    Q_INVOKABLE bool registerName(const QString& accountId,
                                  const QString& password,
                                  const QString& username);
    /**
     * Connect to JAMS to retrieve the account
     * @param username
     * @param password
     * @param serverUri
     * @param config
     * @return the account id
     */
    static QString connectToAccountManager(const QString& username,
                                           const QString& password,
                                           const QString& serverUri,
                                           const MapStringString& config = MapStringString());
    /**
     * Create a new Ring or SIP account
     * @param type determine if the new account will be a Ring account or a SIP one
     * @param displayName
     * @param username
     * @param archivePath
     * @param password of the archive (unused for SIP)
     * @param pin of the archive (unused for SIP)
     * @param uri of the account (for SIP)
     * @param config
     * @return the created account
     */
    static QString createNewAccount(profile::Type type,
                                    const QString& displayName = "",
                                    const QString& archivePath = "",
                                    const QString& password = "",
                                    const QString& pin = "",
                                    const QString& uri = "",
                                    const MapStringString& config = MapStringString());
    /**
     * Set an account to the first position
     */
    void setTopAccount(const QString& accountId);
    /**
     * Get the vCard for an account
     * @param id
     * @return vcard of the account
     */
    QString accountVCard(const QString& accountId, bool compressImage = true) const;
    /**
     * Get the best name for an account
     * @param id
     * @return best name of the account
     */
    const QString bestNameForAccount(const QString& accountID);
    /**
     * Get the best id for an account
     * @param id
     * @return best id of the account
     */
    const QString bestIdForAccount(const QString& accountID);
    /**
     * Add/remove default moderator
     * @param accountID
     * @param peerURI
     * @param state
     */
    void setDefaultModerator(const QString& accountID, const QString& peerURI, const bool& state);
    /**
     * Get default moderators for an account
     * @param accountID
     * @return default moderators for the account
     */
    QStringList getDefaultModerators(const QString& accountID);
    /**
     * Enable/disable local moderators
     * @param accountID
     * @param isModEnabled
     */
    void enableLocalModerators(const QString& accountID, const bool& isModEnabled);
    /**
     * Get local moderators state
     * @param accountID
     * @return if local moderator is enabled
     */
    bool isLocalModeratorsEnabled(const QString& accountID);
    /**
     * Enable/disable all moderators
     * @param accountID
     * @param allModerators
     */
    void setAllModerators(const QString& accountID, const bool& allModerators);
    /**
     * Get all moderators state
     * @param accountID
     * @return if all moderator is enabled
     */
    bool isAllModerators(const QString& accountID);
    /**
     * Get notifications count across accounts
     */
    int notificationsCount() const;
    /**
     * Retrieve account's avatar
     */
    QString avatar(const QString& accountId) const;

Q_SIGNALS:
    /**
     * Connect this signal to know when an invalid account is here
     * @param accountID
     */
    void invalidAccountDetected(const QString& accountID);
    /**
     * Connect this signal to know when the status of an account has changed.
     * @param accountID
     */
    void accountStatusChanged(const QString& accountID);
    /**
     * Connect this signal to know when the details of an account has changed.
     * @param accountID
     */
    void accountDetailsChanged(const QString& accountID);
    /**
     * Connect this signal to know when an account was added.
     * @param accountID
     */
    void accountAdded(const QString& accountID);
    /**
     * Connect this signal to know when an account was removed.
     * @param accountID
     */
    void accountRemoved(const QString& accountID);
    /**
     * Connect this signal to know when an account was updated.
     * @param accountID
     */
    void profileUpdated(const QString& accountID);

    /**
     * Connect this signal to know when an account is exported on the DHT
     * @param accountID
     * @param status
     * @param pin
     */
    void exportOnRingEnded(const QString& accountID,
                           account::ExportOnRingStatus status,
                           const QString& pin);

    /**
     * Name registration has ended
     * @param accountId
     * @param status
     * @param name
     */
    void nameRegistrationEnded(const QString& accountId,
                               account::RegisterNameStatus status,
                               const QString& name);

    /**
     * Name registration has been found
     * @param accountId
     * @param status
     * @param name
     */
    void registeredNameFound(const QString& accountId,
                             account::LookupStatus status,
                             const QString& address,
                             const QString& name);

    /**
     * Migration has finished
     * @param accountId
     * @param ok
     */
    void migrationEnded(const QString& accountId, bool ok);

private:
    std::unique_ptr<NewAccountModelPimpl> pimpl_;
};
} // namespace api
} // namespace lrc
Q_DECLARE_METATYPE(lrc::api::NewAccountModel*)
