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
#include "api/newaccountmodel.h"


// Models and database
#include "database.h"
#include "api/newcallmodel.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"

namespace lrc
{

namespace api
{

class NewAccountModelPimpl
{
public:
    NewAccountModelPimpl(const Database& database);
    ~NewAccountModelPimpl();

    const Database& database;
    AccountInfoMap accounts;
};

NewAccountModel::NewAccountModel(const Database& database)
: pimpl_(std::make_unique<NewAccountModelPimpl>(database))
{
}

NewAccountModel::~NewAccountModel()
{
}

const std::vector<std::string>
NewAccountModel::getAccountList() const
{
    return {};
}

const account::Info&
NewAccountModel::getAccountInfo(const std::string& accountId)
{
    return pimpl_->accounts[accountId];
}

NewAccountModelPimpl::NewAccountModelPimpl(const Database& database)
: database(database)
{

}

NewAccountModelPimpl::~NewAccountModelPimpl()
{

}

} // namespace api
} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
