/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
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
#ifndef BACKENDMANAGERINTERFACE_H
#define BACKENDMANAGERINTERFACE_H

#include "typedefs.h"

//Qt
#include <QtCore/QString>

//libstdc++
#include <type_traits>

//Ring
#include <collectioninterface.h>
#include <collectionmediator.h>

class CommonCollectionModel;

enum LoadOptions {
   NONE           = 0x0     ,
   FORCE_ENABLED  = 0x1 << 0,
   FORCE_DISABLED = 0x1 << 1,
};

template <class T>
class CollectionManagerInterfacePrivate;

/**
 * This is the base for all models based on the itembackend framework.
 *
 * This interface has to be implemented by each models. The abstract
 * private methods will be called when the managed backends need
 * to interact with the model.
 *
 * All implementation should define their item backend type in the
 * class declaration like:
 *
 * template <typename T > using CollectionMediator = CollectionMediator<Person>;
 *
 * And individual backends should extend that alias. For example:
 *
 * class MyPersonSourceBackend : public CollectionInterface {
 *     public:
 *        MyPersonSourceBackend(CollectionInterfaceMediator* mediator)
 * };
 *
 * The mediator is used to bridge the model and the item backends. The mediator
 * implement the common logic that should otherwise have been copy pasted in each
 * backends.
 */
template <class T> class LIB_EXPORT CollectionManagerInterface {
   friend class CollectionMediator<T>;

public:
   CollectionManagerInterface();
   virtual ~CollectionManagerInterface() {};

   /**
    * This method is used to add a backend to a model. The LoadOptions
    * can be used to enforce some parameters. Please note this function is
    * a variadic template. If the backend require some arguments to be passed
    * to its constructor, they can be added as extra parameters.
    *
    * Please note that each backend need to take a CollectionMediator as first
    * argument.
    *
    * @return The newly created backend
    */
   template <class T2, typename ...Ts>
   T2* addBackend(Ts... args, const LoadOptions options = LoadOptions::NONE);

   /// Do this manager have active backends
   virtual bool hasEnabledBackends () const final;
   virtual bool hasBackends        () const final;

   /// List all backends
   virtual const QVector< CollectionInterface* > backends       () const final;
   virtual const QVector< CollectionInterface* > enabledBackends() const final;

   ///Enable / disable a backend
   virtual bool enableBackend( CollectionInterface*  backend, bool enabled) final;

   virtual bool clearAllBackends() const;

   /**
    * Delete the item from the model and from its backend. This
    * is permanent and cannot be undone.
    *
    * Please note that certain type of items, while removed from the view
    * will continue to exist after being removed. This include items part
    * of multiple backends or items generated from runtime data.
    *
    * @return true if successful, false is the backend doesn't support removing items or the operation failed.
    */
   bool deleteItem(T* item);

private:
   /**
    * This method is called when a new backend is added. Some models
    * may need to act on such action, other don't.
    */
   virtual void backendAddedCallback(CollectionInterface* backend);

   /**
    * This method implement the logic necessary to add the item to
    * the model.
    *
    * This method can be called with items already part of the model.
    * All implementation must handle that.
    */
   virtual bool addItemCallback   (T* item) = 0;

   /**
    * Remove an item from the model. Subclasses must implement the logic
    * necessary to remove an item from the QAbstractCollection.
    *
    * This function can be called with nullptr or with items not part
    * of the model. All implementations must handle that.
    */
   virtual bool removeItemCallback(T* item) = 0;

   CollectionManagerInterfacePrivate<T>* d_ptr;
};

#include "collectionmanagerinterface.hpp"

#endif
