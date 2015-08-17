/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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
#ifndef CREDENTIAL_H
#define CREDENTIAL_H

#include <typedefs.h>

class CredentialPrivate;

///A set of credential to be used by an account
class LIB_EXPORT Credential final
{
public:
   enum class Type {
      SIP ,
      STUN,
      TURN //Does this exist?
   };

   //Constructor
   explicit Credential(const FlagPack<Credential::Type>& t);
   ~Credential();

   //Getter
   QString        realm   () const;
   QString        username() const;
   QString        password() const;
   FlagPack<Type> type    () const;

   //Setter
   void setRealm   (const QString&        value );
   void setUsername(const QString&        value );
   void setPassword(const QString&        value );
   void setType    (const FlagPack<Type>& value );

private:
   CredentialPrivate* d_ptr;
};
Q_DECLARE_METATYPE(Credential*)
DECLARE_ENUM_FLAGS(Credential::Type)

#endif
