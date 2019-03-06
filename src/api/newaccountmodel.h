/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                             *
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

// std
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"
#include "api/account.h"

namespace lrc
{

class CallbacksHandler;
class Database;
class NewAccountModelPimpl;

namespace api
{

class Lrc;
class BehaviorController;

/**
  *  @brief Class that manages account information.
  */
class LIB_EXPORT NewAccountModel : public QObject {
    Q_OBJECT
public:
    NewAccountModel(Lrc& lrc,
                    const CallbacksHandler& callbackHandler,
                    const api::BehaviorController& behaviorController,
                    migrationCallback& willMigrateCb,
                    migrationCallback& didMigrateCb);

    ~NewAccountModel();
    /**
     * get a list of all acountId.
     * @return a std::vector<std::string>.
     */
    std::vector<std::string> getAccountList() const;
    /**
     * get account informations associated to an accountId.
     * @param accountId.
     * @return a const account::Info& structure.
     */
    const account::Info& getAccountInfo(const std::string& accountId) const;
    /**
     * flag account corresponding to passed id as freeable.
     */
    void flagFreeable(const std::string& accountId) const;

     /**
     * set account enable/disable, save config and do unregister for account
     * @param accountId.
     * @param enabled.
     */
    void setAccountEnabled(const std::string& accountID, bool enabled) const;

    /**
     * saves account config to .yml
     * @param accountId.
     * @param reference to the confProperties
     */
    void setAccountConfig(const std::string& accountID,
                          const account::ConfProperties_t& confProperties) const;
    /**
     * gets a copy of the accounts config
     * @param accountId.
     * @return an account::Info::ConfProperties_t structure.
     */
    account::ConfProperties_t getAccountConfig(const std::string& accountId) const;
    /**
     * Call exportToFile from the daemon
     * @param accountId
     * @param path destination
     * @param password
     * @return if the file is exported with success
     */
    bool exportToFile(const std::string& accountId, const std::string& path, const std::string& password = {}) const;
    /**
     * Call exportOnRing from the daemon
     * @param accountId
     * @param password
     * @return if the export is initialized
     */
    bool exportOnRing(const std::string& accountId, const std::string& password) const;
    /**
     * Call removeAccount from the daemon
     * @param accountId to remove
     * @note will emit accountRemoved
     */
    void removeAccount(const std::string& accountId) const;
    /**
     * Call changeAccountPassword from the daemon
     * @param accountId
     * @param currentPassword
     * @param newPassword
     * @return if the password has been changed
     */
    bool changeAccountPassword(const std::string& accountId,
                               const std::string& currentPassword,
                               const std::string& newPassword) const;
    /**
     * Change the avatar of an account
     * @param accountId
     * @param avatar
     * @throws out_of_range exception if account is not found
     */
    void setAvatar(const std::string& accountId, const std::string& avatar);
    /**
     * Change the alias of an account
     * @param accountId
     * @param alias
     * @throws out_of_range exception if account is not found
     */
    void setAlias(const std::string& accountId, const std::string& alias);
    /**
     * Try to register a name
     * @param accountId
     * @param password
     * @param username
     * @return if operation started
     */
    bool registerName(const std::string& accountId, const std::string& password, const std::string& username);

    /**
     * Create a new Ring or SIP account
     * @param type determine if the new account will be a Ring account or a SIP one
     * @param displayName
     * @param username
     * @param archivePath
     * @param password of the archive (unused for SIP)
     * @param pin of the archive (unused for SIP)
     * @param uri of the account (for SIP)
     * @return the created account
     */
    static std::string createNewAccount(profile::Type type,
                                        const std::string& displayName = "",
                                        const std::string& archivePath = "",
                                        const std::string& password = "",
                                        const std::string& pin = "",
                                        const std::string& uri = "");

    /**
     * Set an account to the first position
     */
    void setTopAccount(const std::string& accountId);

    /**
     * Get the vCard for an account
     * @param id
     * @return vcard of the account
     */
    std::string accountVCard(const std::string& accountId, bool compressImage = true) const;

Q_SIGNALS:
    /**
     * Connect this signal to know when an invalid account is here
     * @param accountID
     */
    void invalidAccountDetected(const std::string& accountID);
    /**
     * Connect this signal to know when the status of an account has changed.
     * @param accountID
     */
    void accountStatusChanged(const std::string& accountID);
    /**
     * Connect this signal to know when an account was added.
     * @param accountID
     */
    void accountAdded(const std::string& accountID);
    /**
     * Connect this signal to know when an account was removed.
     * @param accountID
     */
    void accountRemoved(const std::string& accountID);
    /**
     * Connect this signal to know when an account was updated.
     * @param accountID
     */
    void profileUpdated(const std::string& accountID);

    /**
     * Connect this signal to know when an account is exported on the DHT
     * @param accountID
     * @param status
     * @param pin
     */
    void exportOnRingEnded(const std::string& accountID, account::ExportOnRingStatus status, const std::string& pin);

    /**
     * Name registration has ended
     * @param accountId
     * @param status
     * @param name
     */
    void nameRegistrationEnded(const std::string& accountId, account::RegisterNameStatus status, const std::string& name);

    /**
     * Name registration has been found
     * @param accountId
     * @param status
     * @param name
     */
    void registeredNameFound(const std::string& accountId, account::LookupStatus status, const std::string& address, const std::string& name);

    /**
     * Migration has finished
     * @param accountId
     * @param ok
     */
    void migrationEnded(const std::string& accountId, bool ok);

private:
    std::unique_ptr<NewAccountModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
