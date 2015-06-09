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

template<typename Base>
ItemBase<Base>::ItemBase(Base* parent) :Base(nullptr), d_ptr(new ItemBasePrivate())
{
   QObject::moveToThread(QCoreApplication::instance()->thread());
   QObject::setParent(parent);
}

template<typename Base>
ItemBase<Base>::~ItemBase()
{
   delete d_ptr;
}

template<typename Base>
CollectionInterface* ItemBase<Base>::collection() const
{
   return d_ptr->m_pBackend;
}

template<typename Base>
void ItemBase<Base>::setCollection(CollectionInterface* backend)
{
   d_ptr->m_pBackend = backend;
}

///Save the contact
template<typename Base>
bool ItemBase<Base>::save() const
{
   if (!d_ptr->m_pBackend)
      return false;

   return d_ptr->m_pBackend->save(this);
}

///Show an implementation dependant dialog to edit the contact
template<typename Base>
bool ItemBase<Base>::edit()
{
   if (!d_ptr->m_pBackend)
      return false;

   return d_ptr->m_pBackend->edit(this);
}

///Remove the contact from the backend
template<typename Base>
bool ItemBase<Base>::remove()
{
   if (!d_ptr->m_pBackend)
      return false;

   return d_ptr->m_pBackend->remove(this);
}

template<typename Base>
bool ItemBase<Base>::isActive() const
{
   return d_ptr->m_pBackend->isEnabled() && d_ptr->m_isActive;
}

template<typename Base>
template<typename T2>
bool ItemBase<Base>::hasExtenstion() const
{
   if (!d_ptr->m_pBackend)
      return false;

   return d_ptr->m_pBackend->isExtensionActive<T2>();
}

template<typename Base>
template<typename T2>
T2* ItemBase<Base>::extenstion() const
{
   if (!d_ptr->m_pBackend)
      return nullptr;

   return d_ptr->m_pBackend->extension<T2>();
}
