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

// Std
#include <memory>
#include <deque>
#include <map>

// Qt
#include <qobject.h>

// Data
#include "data/account.h"

// Lrc
#include "typedefs.h"

namespace lrc
{

class Database;
class Lrc;

class LIB_EXPORT NewAccountModel : public QObject {
    Q_OBJECT

    friend class Lrc;

public:
    ~NewAccountModel();
    /**
     * get account informations associated to accountId.
     * @param accountId.
     * @return a const account::Info& structure.
     */
    const account::Info& getAccountInfo(const std::string& accountId);

private:
    explicit NewAccountModel(const Database& database);
    const Database& database_;
    AccountsInfoMap accounts_;
};

using upNewAccountModel = std::unique_ptr<NewAccountModel>;

} // namespace lrc
