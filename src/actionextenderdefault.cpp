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
#include "actionextenderdefault.h"

#include "macro.h"

namespace Interfaces {

void ActionExtenderDefault::editPerson(Person* p)
{
   Q_UNUSED(p)
}

void ActionExtenderDefault::viewChatHistory(ContactMethod* cm)
{
   Q_UNUSED(cm)
}

void ActionExtenderDefault::viewChatHistory(Person* p)
{
   Q_UNUSED(p)
}

void ActionExtenderDefault::copyInformation(QMimeData* data)
{
   Q_UNUSED(data)
}

bool ActionExtenderDefault::warnDeletePerson(Person* p)
{
   Q_UNUSED(p)
   return true;
}

bool ActionExtenderDefault::warnDeleteCall(Call* c)
{
   Q_UNUSED(c)
   return true;
}

Person* ActionExtenderDefault::selectPerson(FlagPack<SelectPersonHint> hints, const QVariant& hintVar) const
{
   Q_UNUSED(hintVar)
   Q_UNUSED(hints)
   return nullptr;
}

ContactMethod* ActionExtenderDefault::selectContactMethod(FlagPack<SelectContactMethodHint> hints, const QVariant& hintVar) const
{
   Q_UNUSED(hintVar)
   Q_UNUSED(hints)
   return nullptr;
}

} // namespace Interfaces
