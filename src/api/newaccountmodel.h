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

namespace lrc
{

class CallbacksHandler;
class Database;
class NewAccountModelPimpl;

namespace api
{

class Lrc;
class BehaviorController;

namespace account {
    struct ConfProperties_t;
    struct Info;
}

/**
  *  @brief Class that manages account information.
  */
class LIB_EXPORT NewAccountModel : public QObject {
    Q_OBJECT
public:
    using AccountInfoMap = std::map<std::string, account::Info>;

    NewAccountModel(Lrc& lrc,
                    Database& database,
                    const CallbacksHandler& callbackHandler,
                    const api::BehaviorController& behaviorController);

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
     * @return if the file is exported with success
     */
    bool exportToFile(const std::string& accountId, const std::string& path) const;
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

Q_SIGNALS:
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

private:
    std::unique_ptr<NewAccountModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
