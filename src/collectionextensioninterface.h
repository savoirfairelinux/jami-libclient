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
#ifndef ABSTRACTITEMBACKENDMODELEXTENSION_H
#define ABSTRACTITEMBACKENDMODELEXTENSION_H

#include "typedefs.h"

#include <QtCore/QVariant>
#include <QtCore/QModelIndex>

class CollectionInterface;

class LIB_EXPORT CollectionExtensionInterface : public QObject
{
   Q_OBJECT

public:
   explicit CollectionExtensionInterface(QObject* parent);

   virtual QVariant      data    (CollectionInterface* backend, const QModelIndex& index, int role = Qt::DisplayRole      ) const = 0;
   virtual Qt::ItemFlags flags   (CollectionInterface* backend, const QModelIndex& index                                  ) const = 0;
   virtual bool          setData (CollectionInterface* backend, const QModelIndex& index, const QVariant &value, int role ) = 0;
   virtual QString       headerName() const = 0;

Q_SIGNALS:
   void dataChanged(const QModelIndex& idx);
};

#endif
