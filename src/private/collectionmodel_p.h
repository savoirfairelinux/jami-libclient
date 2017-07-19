/****************************************************************************
 *   Copyright (C) 2014-2017 Savoir-faire Linux                          *
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

//Qt
#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QHash>
#include <QtCore/QMutex>
class QAbstractItemModel;

//Ring
class CollectionInterface;
class CollectionModel;
class CollectionExtensionInterface;
class CollectionConfigurationInterface;
class CollectionCreationInterface;

class CollectionModelPrivate final : public QObject
{
   Q_OBJECT
public:
   explicit CollectionModelPrivate(CollectionModel* parent);
   virtual ~CollectionModelPrivate();

   /*
    * This is not very efficient, it doesn't really have to be given the low
    * volume. If it ever have to scale, a better mapToSource using persistent
    * index have to be implemented.
    */
   struct ProxyItem {
      ProxyItem() : parent(nullptr),col(0),row(0),collection(nullptr),manageableCount(0){}
      virtual ~ProxyItem();
      int row;
      int col;
      CollectionInterface* collection;
      ProxyItem* parent;
      QVector<ProxyItem*> m_Children;
      QString m_AltName;
      int manageableCount;
   };

   QHash<CollectionInterface*,ProxyItem*>   m_hBackendsNodes;
   QVector<ProxyItem*>                      m_lTopLevelBackends;
   QVector<CollectionExtensionInterface*>   m_lExtensions;
   QHash<QString,ProxyItem*>                m_hCategories;
   QAbstractItemModel*                      m_pManageableProxy;
   QList<CollectionConfigurationInterface*> m_lConfigurator;
   QList<CollectionCreationInterface*>      m_lCreator;
   QMutex                                   m_NewCollectionMutex;
   QHash<CollectionInterface*,bool>         m_hPendingChanges;

   //Helper
   void registerNew(CollectionInterface* col);

private:
   CollectionModel* q_ptr;

private Q_SLOTS:
   void slotUpdate();
   void slotExtensionDataChanged(const QModelIndex& idx);
};

