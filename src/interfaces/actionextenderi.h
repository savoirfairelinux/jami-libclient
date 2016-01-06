/****************************************************************************
 *   Copyright (C) 2015-2016 by Emmanuel Lepage Vallee                           *
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

// Qt
#include <QtCore/QVariant>

#include <typedefs.h>

class Account;
class Person;
class ContactMethod;
class Call;

namespace Interfaces {

///Interface for extra User Action Model actions
class ActionExtenderI {
public:
   ///Hint the implementation how to sort/filter the persons
   enum class SelectPersonHint {
      NONE = 0x0 << 0
      // Will be used eventually, let's avoid breaking the API
   };

   ///Hint the implementation how to sort/filter the contact methods
   enum class SelectContactMethodHint {
      NONE            = 0x0 << 0,
      RECENT          = 0x1 << 0,
      WITHOUT_PERSON  = 0x1 << 1,
   };

   virtual ~ActionExtenderI() = default;

   virtual void editPerson(Person* p) = 0;
   virtual void viewChatHistory(ContactMethod* cm) = 0;
   virtual void viewChatHistory(Person* p) = 0;
   virtual void copyInformation(QMimeData* data) = 0;
   virtual bool warnDeletePerson(Person* p) = 0;
   virtual bool warnDeleteCall(Call* c) = 0;
   virtual Person* selectPerson(FlagPack<SelectPersonHint> hints = SelectPersonHint::NONE, const QVariant& hintVar = {}) const = 0;
   virtual ContactMethod* selectContactMethod(FlagPack<SelectContactMethodHint> hints = SelectContactMethodHint::NONE, const QVariant& hintVar = {}) const = 0;
};

} // namespace Interfaces

DECLARE_ENUM_FLAGS(Interfaces::ActionExtenderI::SelectContactMethodHint)
DECLARE_ENUM_FLAGS(Interfaces::ActionExtenderI::SelectPersonHint       )