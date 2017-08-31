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


// LRC
#include "api/newcallmodel.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"
#include "api/account.h"

#include "database.h"

namespace lrc
{

using namespace api;

class NewAccountModelPimpl
{
    //~ friend class Dm;
    //~ friend class Db;

public:
    NewAccountModelPimpl(const Database& database);
    ~NewAccountModelPimpl();

    const Database& database;
    NewAccountModel::AccountInfoMap accounts;
};

/**
 * The following namespace is used to separate visualy the sources used to update the data for the model.
 * We should NEVER have some information stored in two differents sources.
 */

namespace source
{
class Dm{ // daemon
    public:
    Dm(){};
    virtual ~Dm() = 0;
    static void getRingAccountsInfo(){ qDebug() << "!1!";};
};

class Db{ // database
    public:
    Db(){};
    virtual ~Db() = 0;
    static void getAccountsInfo(const Database& database);
};

} // namespace source

using namespace source;

void
Db::getAccountsInfo(const Database& database)
{
    qDebug() << "!2!";

    // insert into :
    std::map<std::string, std::string> bindCol1 = {{":col1" , "col1"}, {":col2", "col2"}};
    std::map<std::string, std::string> bindsSet1 = {{":col1" , "alphonse"}, {":col2", "nono"}};
    auto n = database.insertInto("tests", bindCol1, bindsSet1);
    qDebug() << "my last insert : " << n;
    
    // update :
    std::string set2 = "col1=:newvalue";
    std::map<std::string, std::string> bindSet2 = {{":newvalue" , "ATA"}};
    std::string where2 = "col1=:oldvalue";
    std::map<std::string, std::string> bindWhere2 = {{":oldvalue" , "qwert"}};
    database.update("tests", set2, bindSet2, where2, bindWhere2);

    // select :
    std::string select3 = "*";
    std::string where3 = "col1=:name";
    std::map<std::string, std::string> binds3 = {{":name", "TOTO"}};
    auto result = database.select(select3, "tests", where3, binds3);
    qDebug() << " voici les resultats";
    int j=0;
    for (auto&e : result.payloads){
        qDebug() << j << e.c_str();
        j++;
    }
    qDebug() << "fin";
    
    // delete :
    std::string where4 = "col2=:location";
    std::map<std::string, std::string> binds4 = {{":location", "TITI"}};
    database.deleteFrom("tests", where4, binds4);

}

using namespace source;


NewAccountModel::NewAccountModel(const Database& database)
: QObject()
, pimpl_(std::make_unique<NewAccountModelPimpl>(database))
{
    qDebug() << "@YO3@";

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
    qDebug() << "@YO4@";
    Db::getAccountsInfo(database);
    Dm::getRingAccountsInfo();
}

NewAccountModelPimpl::~NewAccountModelPimpl()
{

}

} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
