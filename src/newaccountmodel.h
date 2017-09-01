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

// Interface
#include "api/newaccountmodeli.h"

// Lrc
#include "typedefs.h"

namespace lrc
{

class Database;

using AccountInfoMap = std::map<std::string, api::account::Info>;

class LIB_EXPORT NewAccountModel : public api::NewAccountModelI {
public:
    NewAccountModel(const Database& database);
    ~NewAccountModel();

    const api::account::Info& getAccountInfo(const std::string& accountId) override;

private:
    const Database& database_;
    AccountInfoMap accounts_;
};

} // namespace lrc
