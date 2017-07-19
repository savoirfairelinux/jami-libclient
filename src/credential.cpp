/****************************************************************************
 *   Copyright (C) 2015-2017 Savoir-faire Linux                               *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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
#include <credential.h>

struct CredentialPrivate
{
   QString m_Realm;
   QString m_Username;
   QString m_Password;
   FlagPack<Credential::Type> m_Type {FlagPack<Credential::Type>()};
};

Credential::Credential(const FlagPack<Credential::Type>& t) : d_ptr(new CredentialPrivate())
{
   d_ptr->m_Type = t;
}

Credential::~Credential()
{
   delete d_ptr;
}

QString Credential::realm() const
{
   return d_ptr->m_Realm;
}

QString Credential::username() const
{
   return d_ptr->m_Username;
}

QString Credential::password() const
{
   return d_ptr->m_Password;
}

FlagPack<Credential::Type> Credential::type() const
{
   return d_ptr->m_Type;
}

void Credential::setRealm(const QString& value)
{
   d_ptr->m_Realm = value;
   emit realmChanged(d_ptr->m_Realm);
   emit changed();
}

void Credential::setUsername(const QString& value)
{
   d_ptr->m_Username = value;
   emit usernameChanged(d_ptr->m_Username);
   emit changed();
}

void Credential::setPassword(const QString& value)
{
   d_ptr->m_Password = value;
   emit passwordChanged(d_ptr->m_Password);
   emit changed();
}

void Credential::setType(const FlagPack<Type>& value)
{
   d_ptr->m_Type = value;
}
