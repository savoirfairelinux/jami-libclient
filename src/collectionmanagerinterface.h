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
#ifndef COLLECTIONMANAGERINTERFACE_H
#define COLLECTIONMANAGERINTERFACE_H

#include "typedefs.h"

//Qt
#include <QtCore/QString>
#include <QtCore/QMutexLocker>

//libstdc++
#include <type_traits>
#include <functional>

//Ring
#include <collectioninterface.h>
#include <collectionmediator.h>

class QAbstractItemModel;

enum LoadOptions {
   NONE           = 0x0     ,
   FORCE_ENABLED  = 0x1 << 0,
   FORCE_DISABLED = 0x1 << 1,
};

class CollectionManagerInterfaceBasePrivate;
class CollectionCreationInterface;
class CollectionConfigurationInterface;

/**
 * Common elements for each CollectionManagerInterface
 */
class LIB_EXPORT CollectionManagerInterfaceBase {
public:
   virtual ~CollectionManagerInterfaceBase(){}

   virtual bool hasEnabledCollections (FlagPack<CollectionInterface::SupportedFeatures> features = CollectionInterface::SupportedFeatures::NONE) const = 0;
   virtual bool hasCollections        (FlagPack<CollectionInterface::SupportedFeatures> features = CollectionInterface::SupportedFeatures::NONE) const = 0;

   ///Enable / disable a collection
   virtual bool enableCollection( CollectionInterface*  collection, bool enabled) = 0;

   virtual bool clearAllCollections() const = 0;

protected:
   void registerToModel(CollectionInterface* col) const;
   void addCreatorToList(CollectionCreationInterface* creator);
   void addConfiguratorToList(CollectionConfigurationInterface* configurator);
   void setCollectionConfigurator(CollectionInterface* col, std::function<CollectionConfigurationInterface*()> getter);

private:
   CollectionManagerInterfaceBasePrivate* d_ptr;
   Q_DECLARE_PRIVATE(CollectionManagerInterfaceBase)
};

template <class T>
class CollectionManagerInterfacePrivate;

/**
 * This is the base for all models based on the itemcollection framework.
 *
 * This interface has to be implemented by each models. The abstract
 * private methods will be called when the managed collections need
 * to interact with the model.
 *
 * All implementation should define their item collection type in the
 * class declaration like:
 *
 * template <typename T > using CollectionMediator = CollectionMediator<Person>;
 *
 * And individual collections should extend that alias. For example:
 *
 * class MyPersonSourceBackend : public CollectionInterface {
 *     public:
 *        MyPersonSourceBackend(CollectionInterfaceMediator* mediator)
 * };
 *
 * The mediator is used to bridge the model and the item collections. The mediator
 * implement the common logic that should otherwise have been copy pasted in each
 * collections.
 */
template <class T> class LIB_EXPORT CollectionManagerInterface  : public CollectionManagerInterfaceBase {
   friend class CollectionMediator<T>;

public:
   /**
    * Extend a QAbstractItemModel to have the collection management interface.
    * This will add the addBackend and a few other methods.
    *
    * This interface need to be used on a QAbstractItemModel derived class
    *
    * @param self "this"
    */
   explicit CollectionManagerInterface(QAbstractItemModel* self);
   virtual ~CollectionManagerInterface();

   /**
    * This method is used to add a collection to a model. The LoadOptions
    * can be used to enforce some parameters. Please note this function is
    * a variadic template. If the collection require some arguments to be passed
    * to its constructor, they can be added as extra parameters.
    *
    * Please note that each collection need to take a CollectionMediator as first
    * argument.
    *
    * @return The newly created collection
    */
   template <class T2, typename ...Ts>
   T2* addCollection(Ts... args, const LoadOptions options = LoadOptions::NONE);

   /**
    * Set an object that will be used when the user wish to add a new collection
    * of that type.
    * 
    * That object can be a widget or anything. I will be passed when a creator
    * is requested for that type of collection.
    */
   template <class T2>
   CollectionCreationInterface* registerCreator(CollectionCreationInterface* creator = nullptr);

   /**
    * @see  template <class T2> void registerCreator(CollectionCreationInterface* creator);
    */
   template <class T2>
   CollectionConfigurationInterface* registerConfigarator(CollectionConfigurationInterface* creator = nullptr);


   /// Do this manager have active collections
   bool hasEnabledCollections (FlagPack<CollectionInterface::SupportedFeatures> features = CollectionInterface::SupportedFeatures::NONE) const;
   bool hasCollections        (FlagPack<CollectionInterface::SupportedFeatures> features = CollectionInterface::SupportedFeatures::NONE) const;

   /// List all Collections
   const QVector< CollectionInterface* > collections       (FlagPack<CollectionInterface::SupportedFeatures> features = CollectionInterface::SupportedFeatures::NONE) const;
   const QVector< CollectionInterface* > enabledCollections(FlagPack<CollectionInterface::SupportedFeatures> features = CollectionInterface::SupportedFeatures::NONE) const;

   ///Enable / disable a collection
   bool enableCollection( CollectionInterface*  collection, bool enabled);

   virtual bool clearAllCollections() const;

   /**
    * Delete the item from the model and from its collection. This
    * is permanent and cannot be undone.
    *
    * Please note that certain type of items, while removed from the view
    * will continue to exist after being removed. This include items part
    * of multiple Collections or items generated from runtime data.
    *
    * @return true if successful, false is the collection doesn't support removing items or the operation failed.
    */
   bool deleteItem(T* item);

private:
   /**
    * This method is called when a new collection is added. Some models
    * may need to act on such action, other don't.
    */
   virtual void collectionAddedCallback(CollectionInterface* collection);

   /**
    * This method implement the logic necessary to add the item to
    * the model.
    *
    * This method can be called with items already part of the model.
    * All implementation must handle that.
    * 
    * Please note that the constness is expected to be broken when using
    * setData(), but not otherwise.
    */
   virtual bool addItemCallback   (const T* item) = 0;

   /**
    * Remove an item from the model. Subclasses must implement the logic
    * necessary to remove an item from the QAbstractCollection.
    *
    * This function can be called with nullptr or with items not part
    * of the model. All implementations must handle that.
    */
   virtual bool removeItemCallback(const T* item) = 0;

   CollectionManagerInterfacePrivate<T>* d_ptr;
   QMutex m_InsertionMutex;
};

#include "collectionmanagerinterface.hpp"

#endif
