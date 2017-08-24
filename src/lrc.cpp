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
#include "lrc.h"
#include "newaccountmodel.h"
#include "databasemanager.h"

namespace lrc
{

Lrc::Lrc()
: QObject(nullptr)
{
    // create the database manager
    if (not databaseManager_) {
        DatabaseManager* ptr = new DatabaseManager();
        databaseManager_ = std::unique_ptr<DatabaseManager>(ptr);
    }

    // create the account model
    if (not accountModel_) {
        NewAccountModel* ptr = new NewAccountModel(databaseManager_);
        accountModel_ = std::unique_ptr<NewAccountModel>(ptr);
    }

}

Lrc::~Lrc()
{
}

}
