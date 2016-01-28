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
#include "collectionextensionmodel.h"

#include "collectionextensioninterface.h"

class CollectionExtensionModelPrivate
{
public:
   static CollectionExtensionModel* m_spInstance;
};

CollectionExtensionModel::~CollectionExtensionModel()
{
   delete d_ptr;
}

QList<CollectionExtensionInterface*>& CollectionExtensionModelSpecific::entries()
{
   static QList<CollectionExtensionInterface*> m_slEntries;

   return m_slEntries;
}

CollectionExtensionModel* CollectionExtensionModelPrivate::m_spInstance = nullptr;

CollectionExtensionModel* CollectionExtensionModel::instance()
{
   if (!CollectionExtensionModelPrivate::m_spInstance)
      CollectionExtensionModelPrivate::m_spInstance = new CollectionExtensionModel();

   return CollectionExtensionModelPrivate::m_spInstance;
}

QVariant CollectionExtensionModel::data( const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   return CollectionExtensionModelSpecific::entries()[index.row()]->data(role);
}

int CollectionExtensionModel::rowCount( const QModelIndex& parent) const
{
   return parent.isValid() ? 0 : CollectionExtensionModelSpecific::entries().size();
}

Qt::ItemFlags CollectionExtensionModel::flags( const QModelIndex& index) const
{
   return index.isValid() ? Qt::ItemIsEnabled : Qt::NoItemFlags;
}

bool CollectionExtensionModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QHash<int,QByteArray> CollectionExtensionModel::roleNames() const
{
   return {};
}
