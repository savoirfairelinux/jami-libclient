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
#include "itembackendinterface.h"
#include "itembackendeditor.h"
#include <QtCore/QObject>
#include <QtCore/QMetaObject>

class ItemBasePrivate
{
public:
   ItemBackendInterface* m_pBackend;
};

template<typename Base>
ItemBase<Base>::ItemBase(Base* parent) : Base(parent), d_ptr(new ItemBasePrivate())
{
}

template<typename Base>
ItemBackendInterface* ItemBase<Base>::backend()
{
   return d_ptr->m_pBackend;
}

template<typename Base>
void ItemBase<Base>::setBackend(ItemBackendInterface* backend)
{
   d_ptr->m_pBackend = backend;
}

///Save the contact
template<typename Base>
bool ItemBase<Base>::save() const
{
//    if (((QObject*) this)->staticMetaObject() == d_ptr->m_pBackend->metaObject()){
      return d_ptr->m_pBackend->save(this);
//    }
//    else
//       qDebug() << "Cannot save, invalid item type";
}

///Show an implementation dependant dialog to edit the contact
template<typename Base>
bool ItemBase<Base>::edit()
{
//    if (((QObject*) this)->staticMetaObject() == d_ptr->m_pBackend->metaObject()){
      return d_ptr->m_pBackend->edit(this);
//    }
//    else
//       qDebug() << "Cannot save, invalid item type";
}

///Remove the contact from the backend
template<typename Base>
bool ItemBase<Base>::remove()
{
//    if (((QObject*) this)->staticMetaObject() == d_ptr->m_pBackend->metaObject()){
      return d_ptr->m_pBackend->remove(this);
//    }
//    else
//       qDebug() << "Cannot save, invalid item type";
}