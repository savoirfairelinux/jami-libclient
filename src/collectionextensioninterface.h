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
#ifndef COLLECTIONEXTENSIONINTERFACE_H
#define COLLECTIONEXTENSIONINTERFACE_H

#include "typedefs.h"

#include <QtCore/QVariant>
#include <QtCore/QModelIndex>

class CollectionInterface;

/**
 * This interface can be used to extend the collection system.
 *
 * It is a business logic container. The interface will eventually be extended
 * to allow various callbacks at key ItemBase lifecycle moment.
 * 
 * Subclasses need to call DECLARE_COLLECTION_EXTENSION in the .cpp and
 * include collectionextensionmodel.h
 */
class LIB_EXPORT CollectionExtensionInterface : public QObject
{
   Q_OBJECT

   friend class CollectionExtensionModel;

public:

   explicit CollectionExtensionInterface(QObject* parent);

   virtual QVariant data(int role) const = 0;

Q_SIGNALS:
   void dataChanged(const QModelIndex& idx);
};

#define DECLARE_COLLECTION_EXTENSION_M1(x, y) x ## y
#define DECLARE_COLLECTION_EXTENSION_M2(x, y) DECLARE_COLLECTION_EXTENSION_M1(x, y)

#define DECLARE_COLLECTION_EXTENSION(T) \
static auto DECLARE_COLLECTION_EXTENSION_M2(val,__COUNTER__) = []{\
   CollectionExtensionModel::registerExtension<T>();\
   return 0;\
}();

#endif
