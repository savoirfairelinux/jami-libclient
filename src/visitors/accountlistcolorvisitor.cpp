/****************************************************************************
 *   Copyright (C) 2012 by Savoir-Faire Linux                               *
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
#include "accountlistcolorvisitor.h"

AccountListColorVisitor* AccountListColorVisitor::m_spInstance = new AccountListColorVisitor();

QVariant AccountListColorVisitor::getColor(const Account* a)
{
   Q_UNUSED(a)
   //The default implementation does nothing
   return QVariant();
}

QVariant AccountListColorVisitor::getIcon(const Account* a)
{
   Q_UNUSED(a)
   //The default implementation does nothing
   return QVariant();
}

AccountListColorVisitor::~AccountListColorVisitor()
{

}

AccountListColorVisitor* AccountListColorVisitor::instance()
{
   return m_spInstance;
}

void AccountListColorVisitor::setInstance(AccountListColorVisitor* visitor)
{
   m_spInstance = visitor;
}
