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
#include "collectioninterface.h"
#include "collectioneditor.h"
#include <QtCore/QObject>
#include <QtCore/QMetaObject>
#include <QtCore/QCoreApplication>

class ItemBasePrivate
{
public:
   ItemBasePrivate() : m_pBackend(nullptr),m_isActive(true){}
   CollectionInterface* m_pBackend;
   bool m_isActive;
};

template<typename T2>
bool ItemBase::hasExtenstion() const
{
   if (!d_ptr->m_pBackend)
      return false;

   return d_ptr->m_pBackend->isExtensionActive<T2>();
}

template<typename T2>
T2* ItemBase::extension() const
{
   if (!d_ptr->m_pBackend)
      return CollectionExtensionModel::getExtension<T2>();;

   return d_ptr->m_pBackend->extension<T2>();
}
