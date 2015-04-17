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
#ifndef COLLECTIONEXTENSIONMODEL_H
#define COLLECTIONEXTENSIONMODEL_H

#include <QtCore/QAbstractListModel>

#include <typedefs.h>

class CollectionExtensionInterface;

class CollectionExtensionModelPrivate;

/**
 * This model expose the collections available to the managers
 */
class LIB_EXPORT CollectionExtensionModel : public QAbstractListModel
{
   Q_OBJECT

public:

   //Model functions
   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;
   virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual bool          setData  ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   /**
    * Register an extension available to all type of collections
    *
    * This does **NOT** activate the extension
    */
   template<class T>
   static int registerExtension();

   /**
    * Return an unique identifier for that extension type
    */
   template<class T>
   static int getExtensionId();

   /**
    * Get the instance of a particular extension
    * 
    * This function will register the type if it isn't already registered
    */
   template<class T>
   static T* getExtension();

   //Singleton
   static CollectionExtensionModel* instance();


private:
   CollectionExtensionModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(CollectionExtensionModel)
};

#include <collectionextensionmodel.hpp>

#endif