/****************************************************************************
 *   Copyright (C) 2015 by Emmanuel Lepage Vallee                           *
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
#pragma once

#include <typedefs.h>

class Account;
class Person;
class ContactMethod;
class Call;

namespace Interfaces {

///Interface for getting the account color and icon
class ActionExtenderI {
public:
   virtual ~ActionExtenderI() = default;

   virtual void editPerson(Person* p) = 0;
   virtual void viewChatHistory(ContactMethod* cm) = 0;
   virtual void copyInformation(QMimeData* data) = 0;
   virtual bool warnDeletePerson(Person* p) = 0;
   virtual bool warnDeleteCall(Call* c) = 0;
};

} // namespace Interfaces
