/****************************************************************************
 *   Copyright (C) 2016-2017 Savoir-faire Linux                               *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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
#include "profile.h"

//Qt
#include <QtCore/QUrl>
#include <QtCore/QDir>

//Ring
#include <account.h>
#include <person.h>

class ProfilePrivate
{
public:
   ProfilePrivate();
   Profile::Accounts m_Accounts;
   Person* m_Person;

};

ProfilePrivate::ProfilePrivate()
{

}

Profile::Profile(CollectionInterface* parent, Person* p) : ItemBase(nullptr),
d_ptr(new ProfilePrivate())
{
    if (parent)
        setCollection(parent);
    d_ptr->m_Person = p;
}

Profile::~Profile()
{
   delete d_ptr;
}

const QByteArray Profile::id() const
{
    return person()->uid();
}

const Profile::Accounts& Profile::accounts() const
{
   return d_ptr->m_Accounts;
}

Person* Profile::person() const
{
   return d_ptr->m_Person;
}

void Profile::setAccounts(const Profile::Accounts& accounts)
{
   d_ptr->m_Accounts = accounts;
}

bool Profile::addAccount(Account* acc)
{
    if (d_ptr->m_Accounts.indexOf(acc) == -1) {
       d_ptr->m_Accounts << acc;

       // The Account::setProfile logic should take care of removing the old
       // profile.
       acc->setProfile(this);

       return true;
    }

    return false;
}

bool Profile::removeAccount(Account* acc)
{
    for (int i = 0 ; i < d_ptr->m_Accounts.size() ; ++i) {
        if (d_ptr->m_Accounts[i]->id() == acc->id()) {
            d_ptr->m_Accounts.remove(i);
            d_ptr->m_Accounts.squeeze();
            return true;
        }
    }
    return false;
}
