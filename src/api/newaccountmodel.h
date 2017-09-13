/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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

namespace account { struct Info; }

/**
  *  @brief Class that manages account information.
  */
class LIB_EXPORT NewAccountModel : public QObject {
    Q_OBJECT
public:
    using AccountInfoMap = std::map<std::string, account::Info>;

    NewAccountModel(Database& database, const CallbacksHandler& callbackHandler);
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
    const account::Info& getAccountInfo(const std::string& accountId) const; // [jn] we should not send a ref but a copy, the client has no authority to edit anything

Q_SIGNALS: // [jn] docs!
    void incomingCall(const std::string& accountId, const std::string& contactUri);
    void accountStatusChanged(const std::string& accountID);

private:
    std::unique_ptr<NewAccountModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
