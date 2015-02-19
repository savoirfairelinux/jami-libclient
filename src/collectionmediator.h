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
#ifndef ITEM_BACKEND_MEDIATOR_H
#define ITEM_BACKEND_MEDIATOR_H

#include <typedefs.h>

template<class T>
class CollectionManagerInterface;

template<class T>
class CollectionMediatorPrivate;

/**
 * This is the base class for each BackendMediator. A backend mediator
 * is a intermediary object between the backend and the model responsible
 * to manage the backends objects. The purpose of this layer are:
 *
 *  * Isolate the item gestion away from the manager public API
 *  * Work around the lack of polymorphic generics for template objects
 *
 * The later objective make it easier to later implement the decorator pattern.
 */
template<typename T>
class LIB_EXPORT CollectionMediator {
public:
   CollectionMediator(CollectionManagerInterface<T>* parentManager, QAbstractItemModel* m);
   bool addItem   (T* item);
   bool removeItem(T* item);

   QAbstractItemModel* model() const;

private:
   CollectionMediatorPrivate<T>* d_ptr;
};

#include <collectionmediator.hpp>

#endif
