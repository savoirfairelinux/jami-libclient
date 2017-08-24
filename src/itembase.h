/****************************************************************************
 *   Copyright (C) 2015-2017 Savoir-faire Linux                               *
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
#pragma once

#include <typedefs.h>

#include <collectioninterface.h>

class ItemBasePrivate;

/**
 * Base class for all items to be managed in CollectionInterface
 */
class LIB_EXPORT ItemBase : public QObject
{
   Q_OBJECT

   friend class CollectionInterface;
public:
   //Constructor
   explicit ItemBase(QObject* parent = nullptr);
   virtual ~ItemBase();
   virtual CollectionInterface* collection() const final;

   //Extension system
   template<typename T2>
   bool hasExtenstion() const;

   template<typename T2>
   T2* extension() const;

   //Mutator methods
   Q_INVOKABLE bool save    () const;
   Q_INVOKABLE bool edit    ()      ;
   Q_INVOKABLE bool remove  ()      ;
   Q_INVOKABLE bool isActive() const;

   //Setter
   void setCollection(CollectionInterface* backend);

private:
   ItemBasePrivate* d_ptr;
};

#include <itembase.hpp>
