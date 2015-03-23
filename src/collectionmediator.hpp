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

#include <collectionmanagerinterface.h>

template<class T>
class CollectionMediatorPrivate
{
public:
   CollectionManagerInterface<T>* m_pParent;
   QAbstractItemModel* m_pModel;
};

template<typename T>
CollectionMediator<T>::CollectionMediator(CollectionManagerInterface<T>* parentManager, QAbstractItemModel* m) :
   d_ptr(new CollectionMediatorPrivate<T>())
{
   d_ptr->m_pParent = parentManager;
   d_ptr->m_pModel  = m;
}

template<typename T>
bool CollectionMediator<T>::addItem(const T* item)
{
   return d_ptr->m_pParent->addItemCallback(item);
}

template<typename T>
bool CollectionMediator<T>::removeItem(const T* item)
{
   return d_ptr->m_pParent->removeItemCallback(item);
}

template<typename T>
QAbstractItemModel* CollectionMediator<T>::model() const
{
   return d_ptr->m_pModel;
}
