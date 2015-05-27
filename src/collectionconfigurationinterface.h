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
#ifndef COLLECTIONCONFIGURATIONINTERFACE_H
#define COLLECTIONCONFIGURATIONINTERFACE_H

#include <QtCore/QObject>

#include "typedefs.h"

class CollectionInterface;

class LIB_EXPORT CollectionConfigurationInterface : public QObject
{
   Q_OBJECT
public:

   explicit CollectionConfigurationInterface(QObject* parent = nullptr) : QObject(parent) {}

   //Getter
   virtual QByteArray id  () const = 0;
   virtual QString    name() const = 0;
   virtual QVariant   icon() const = 0;

   //Mutator

   /**
    * This function will be called when a collection request to be configured
    * 
    * @param col The collection to be edited. It can casted
    * @param parent can be used for layout information.
    */
   virtual void loadCollection(CollectionInterface* col, QObject* parent = nullptr) =0;

   virtual void save(){}
   virtual bool hasChanged() {return false;}

Q_SIGNALS:
   void changed();

};
Q_DECLARE_METATYPE(CollectionConfigurationInterface*)

#endif
