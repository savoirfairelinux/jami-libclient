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
#ifndef ITEMBASE2_H
#define ITEMBASE2_H

#include <typedefs.h>

#include <collectioninterface.h>

class ItemBasePrivate;

/**
 * Base class for all items to be managed in CollectionInterface
 */
template<typename T>
class LIB_EXPORT ItemBase : public T {
   friend class CollectionInterface;
public:
   //Constructor
   explicit ItemBase(T* parent = nullptr);
   virtual CollectionInterface* backend() final;

   //Mutator methods
   bool save() const;
   bool edit()      ;
   bool remove()    ;

   //Setter
   void setBackend(CollectionInterface* backend);

protected:
private:
   ItemBasePrivate* d_ptr;
};

#include <itembase.hpp>

#endif
