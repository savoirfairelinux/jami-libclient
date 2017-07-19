/****************************************************************************
 *   Copyright (C) 2015-2017 Savoir-faire Linux                               *
 *   Author : Emmanuel Lepage <emmanuel.lepage@savoirfairelinux.com>        *
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

//Qt
class QMimeData;

//Ring
#include "interfaces/actionextenderi.h"
class Person;
class ContactMethod;
class Call;

namespace Interfaces {

///Default implementation of the ActionExtender interfaces; does nothing
class LIB_EXPORT ActionExtenderDefault final : public ActionExtenderI
{
public:

   virtual void editPerson(Person* p) override;
   virtual void viewChatHistory(ContactMethod* cm) override;
   virtual void viewChatHistory(Person* p) override;
   virtual void copyInformation(QMimeData* data) override;
   virtual bool warnDeletePerson(Person* p) override;
   virtual bool warnDeleteCall(Call* c) override;
   virtual Person* selectPerson(FlagPack<SelectPersonHint> hints, const QVariant& hintVar) const override;
   virtual ContactMethod* selectContactMethod(FlagPack<SelectContactMethodHint> hints, const QVariant& hintVar) const override;
};

} // namespace Interfaces
