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

// Lrc
#include "typedefs.h"

// Data
#include "api/account.h"

namespace lrc
{

class CallbacksHandler;
class Database;

namespace api
{

class NewAccountModelPimpl;

using AccountInfoMap = std::map<std::string, account::Info>;

class LIB_EXPORT NewAccountModel : public QObject {
    Q_OBJECT
public:
    NewAccountModel(const Database& database, const CallbacksHandler& callbackHandler);
    ~NewAccountModel();

    /**
     * get a list with all acountId.
     * @return a std::vector<std::string>.
     */
    const std::vector<std::string> getAccountList() const;
    /**
     * get account informations associated to accountId.
     * @param accountId.
     * @return a const account::Info& structure.
     */
    const api::account::Info& getAccountInfo(const std::string& accountId) const;

private:
    std::unique_ptr<NewAccountModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
