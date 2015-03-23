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
#ifndef ITEMBACKENDINTERFACE_H
#define ITEMBACKENDINTERFACE_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QVariant>
#include <QtCore/QAbstractItemModel>

#include "typedefs.h"

//Libstdc++
#include <functional>

//Ring
class CollectionInterfacePrivate;
class CollectionEditorBase;
class CollectionConfigurationInterface;
template<typename T> class CollectionEditor;
template<typename T> class CollectionMediator;
template<typename T> class ItemBase;

/**
 * This is the interface that must be implemented by each item collections to
 * be used by a CollectionManager.
 *
 * The class need to be extended with a template constructor:
 *
 * MyBackend::MyBackend<Person>(CollectionMediator<Person>* mediator, CollectionInterface* parent = nullptr) :
 *    CollectionMediator<Person>(mediator,parent) {}
 *
 * Each collections also need to implement that constructor or they wont load.
 */
class LIB_EXPORT CollectionInterface
{
   template<typename T> friend class CollectionMediator;
   template<typename T> friend class CollectionManagerInterface;
   friend class CollectionManagerInterfaceBase;
   friend class ItemBase<QObject>;
public:

   /**
    * Each backend can have a serie of feaures that are expected to work. While most editor mutators
    * are implemented anyway, each backend should list what is officially supposed to work and what is
    * not.
    */
   enum SupportedFeatures {
      NONE        = 0x0      ,
      LOAD        = 0x1 <<  0, /*!< Load this backend, DO NOT load anything before "load" is called         */
      SAVE        = 0x1 <<  1, /*!< Save an item                                                            */
      EDIT        = 0x1 <<  2, /*!< Edit, but **DOT NOT**, save an item)                                    */
      PROBE       = 0x1 <<  3, /*!< Check if the backend has new items (some collections do this automagically)*/
      ADD         = 0x1 <<  4, /*!< Add (and save) a new item to the backend                                */
      SAVE_ALL    = 0x1 <<  5, /*!< Save all items at once, this may or may not be faster than "add"        */
      CLEAR       = 0x1 <<  6, /*!< Clear all items from this backend                                       */
      REMOVE      = 0x1 <<  7, /*!< Remove a single item                                                    */
      EXPORT      = 0x1 <<  8, /*!< Export all items, format and output need to be defined by each collections */
      IMPORT      = 0x1 <<  9, /*!< Import items from an external source, details defined by each collections  */
      ENABLEABLE  = 0x1 << 10, /*!< Can be enabled, I know, it is not a word, but Java use it too           */
      DISABLEABLE = 0x1 << 11, /*!< Can be disabled, I know, it is not a word, but Java use it too          */
      MANAGEABLE  = 0x1 << 12, /*!< Can be managed the config GUI                                           */
   };

   //Constructor
   template<typename T>
   explicit CollectionInterface(CollectionEditor<T>* editor, CollectionInterface* parent = nullptr);
   virtual ~CollectionInterface();

   //Generic information getters

   /**
    * This method must return an human readable, translatable string.
    * This will be used to display the backend in the backend manager
    * is SupportedFeatures::MANAGEABLE is set.
    */
   virtual QString    name     () const = 0;

   /**
    * Each MANAGEABLE collections can be part of a meta category. This category
    * will be the top level element of the BackendManagerModel. This name
    * must never change once it is set.
    */
   virtual QString   category  () const = 0;

   /**
    * This method must return an optinal icon to be used in the
    * backend manager is SupportedFeatures::MANAGEABLE is set.
    */
   virtual QVariant   icon     () const   ;

   /**
    * Return if the backend is currently enabled. An enabled backend
    * is one where items are loaded and operations such as save()
    * work (when supported)
    */
   virtual bool       isEnabled() const = 0;

   /**
    * This is the identifier for this backend. This id will not
    * be displayed to the user, so it doesn't have to be human
    * readable. It is usually useful when linking to remote
    * data sources.
    */
   virtual QByteArray id       () const = 0;

   /**
    * Return the features supported by this backend.
    *
    * This method mush always return the same set of flags.
    *
    * @see SupportedFeatures
    */
   virtual SupportedFeatures supportedFeatures() const = 0;

   //Management methods

   /**
    * Enable this backend, this may or may not imply load()
    */
   virtual bool enable (bool);

   /**
    * Load a backend. This is used to fetch the elements to
    * be added to the model. This function can (and should)
    * start external workers to fetch the elements and add
    * them asynchroniously.
    *
    * @see BackendManagerInterface::addItemCallback
    * @see BackendManagerInterface::removeItemCallback
    */
   virtual bool load   (    ) = 0;

   /**
    * Reload this backend. In the best case, this should
    * update the existing elements instead of deleting them,
    * as this will cause a (massive) memory leak. Reloaded
    * can be necessary when a file change on disk or by user
    * actions.
    */
   virtual bool reload (    );

   /**
    * Clear this backend. Please note that the elements themselves
    * should *NOT* be deleted as the pointer has been shared with
    * upstream. Unless reference counting is implemented everywhere,
    * will will cause a crash.
    */
   virtual bool clear  (    );

   /**
    * Return the number of elements tracked by this collection.
    * This can usually be extracted from editor<T>()->items().size();
    */
   virtual int size() const;

   /**
    * Return a pointer to the model implementing the CollectionManager.
    */
   QAbstractItemModel* model() const;

   /**
    * Return the items stored in the backend for a given type. The type
    * usually is the one managed the the CollectionManager.
    */
   template<typename T>
   QVector<T*> items() const;

   /**
    * Some collections can be hierarchical, for example, a email backend
    * can have multiple "folders" where mails are stored, a contact backend
    * can have multiple contact groups and an history one can have an archived
    * section for previous years. This method return the parent when applicable.
    */
   CollectionInterface*           parent  () const;

   /**
    * As explained in the "parent()" method, this method return the backend children
    * collections. This can be used by a client to implement different behaviour depending
    * on the backend at a finer level.
    */
   QVector<CollectionInterface*>  children() const;

   /** Get the concrete editor associated with this backend. The template arguments
    *  must match the one used by the model.
    */
   template<typename T>
   CollectionEditor<T>* editor() const;

   /**
    * Add a new element to the backend
    */
   bool add   (ItemBase<QObject>* base);

   /**
    * Return an object that has been associated with this collection type
    * 
    * It can be set using registerConfigarator() available in every collection
    * manager objects such as PersonModel, CategorizedHistoryModel and others.
    */
   CollectionConfigurationInterface* configurator() const;

protected:
   void setConfigurator(std::function<CollectionConfigurationInterface*()> getter);
   void addChildren(CollectionInterface* c);

   bool save      (ItemBase<QObject>* base);
   bool edit      (ItemBase<QObject>* base);
   bool remove    (ItemBase<QObject>* base);
   void activate  (ItemBase<QObject>* base);
   void deactivate(ItemBase<QObject>* base);
   QMetaObject metaObject();

private:
   CollectionInterfacePrivate* d_ptr;
};

#include <collectioninterface.hpp>

#endif
