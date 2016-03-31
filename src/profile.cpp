/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
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
   QString m_Name;
   Profile::Accounts m_Accounts;
   Person* m_Person;

};

ProfilePrivate::ProfilePrivate()
{

}

Profile::Profile(CollectionInterface* parent, Person* p) : ItemBase(nullptr),
d_ptr(new ProfilePrivate())
{
    setCollection(parent);
    d_ptr->m_Person = p;
}

Profile::~Profile()
{
   delete d_ptr;
}

const Profile::Accounts& Profile::accounts() const
{
   return d_ptr->m_Accounts;
}

const QString Profile::name() const
{
   return d_ptr->m_Name;
}

const Person* Profile::person() const
{
   return d_ptr->m_Person;
}

void Profile::setAccounts(Profile::Accounts accounts)
{
   d_ptr->m_Accounts = accounts;
}
