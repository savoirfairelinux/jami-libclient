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
#include "collectionmodel.h"

//Qt
#include <QtCore/QCoreApplication>

//Ring
#include "collectionmanagerinterface.h"
#include "delegates/itemmodelstateserializationdelegate.h"
#include "collectionextensioninterface.h"
#include "private/collectionmodel_p.h"

CollectionModel* CollectionModelPrivate::m_spInstance = nullptr;

CollectionModelPrivate::CollectionModelPrivate(CollectionModel* parent) : QObject(parent),q_ptr(parent)
{}

CollectionModel::CollectionModel(QObject* parent) : QAbstractTableModel(parent), d_ptr(new CollectionModelPrivate(this))
{
   connect(PersonModel::instance(),SIGNAL(newBackendAdded(CollectionInterface*)),d_ptr.data(),SLOT(slotUpdate()));
   load();
}

CollectionModel::~CollectionModel()
{
   while (d_ptr->m_lTopLevelBackends.size()) {
      CollectionModelPrivate::ProxyItem* item = d_ptr->m_lTopLevelBackends[0];
      d_ptr->m_lTopLevelBackends.remove(0);
      while (item->m_Children.size()) {
         //FIXME I don't think it can currently happen, but there may be
         //more than 2 levels.
         CollectionModelPrivate::ProxyItem* item2 = item->m_Children[0];
         item->m_Children.remove(0);
         delete item2;
      }
      delete item;
   }
}

CollectionModel* CollectionModel::instance()
{
   if (!CollectionModelPrivate::m_spInstance)
      CollectionModelPrivate::m_spInstance = new CollectionModel(QCoreApplication::instance());
   return CollectionModelPrivate::m_spInstance;
}

QHash<int,QByteArray> CollectionModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

QVariant CollectionModel::data (const QModelIndex& idx, int role) const
{
   if (idx.isValid()) {
      CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(idx.internalPointer());

      if (idx.column() > 0)
         return d_ptr->m_lExtensions[idx.column()-1]->data(item->collection,idx,role);

      switch(role) {
         case Qt::DisplayRole:
            return item->collection->name();
            break;
         case Qt::DecorationRole:
            return item->collection->icon();
            break;
//          case Qt::CheckStateRole:
//             return item->collection->isEnabled()?Qt::Checked:Qt::Unchecked;
         case Qt::CheckStateRole: {
            if (ItemModelStateSerializationDelegate::instance())
               return ItemModelStateSerializationDelegate::instance()->isChecked(item->collection)?Qt::Checked:Qt::Unchecked;
         }
      };
   }
   //else {
//       CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(idx.internalPointer());
//       return item->model->data(item->model->index(item->row,item->col));
   //}
   return QVariant();
}

int CollectionModel::rowCount (const QModelIndex& parent) const
{
   if (!parent.isValid()) {
      return d_ptr->m_lTopLevelBackends.size();
   }
   else {
      CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(parent.internalPointer());
      return item->collection->children().size();
   }
}

int CollectionModel::columnCount (const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return 1+d_ptr->m_lExtensions.size();
}

Qt::ItemFlags CollectionModel::flags(const QModelIndex& idx) const
{
   if (!idx.isValid())
      return 0;
   CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(idx.internalPointer());
   if (idx.column() > 0) {
      //Make sure the cell is disabled if the row is
      Qt::ItemFlags f = d_ptr->m_lExtensions[idx.column()-1]->flags(item->collection,idx);
      return  (((f&Qt::ItemIsEnabled)&&(!item->collection->isEnabled()))?f^Qt::ItemIsEnabled:f);
   }
   const bool checkable = item->collection->supportedFeatures() & (CollectionInterface::SupportedFeatures::ENABLEABLE |
   CollectionInterface::SupportedFeatures::DISABLEABLE | CollectionInterface::SupportedFeatures::MANAGEABLE  );
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | (checkable?Qt::ItemIsUserCheckable:Qt::NoItemFlags);
}

bool CollectionModel::setData (const QModelIndex& idx, const QVariant &value, int role )
{
   Q_UNUSED(idx)
   Q_UNUSED(value)
   Q_UNUSED(role)
   if (idx.isValid() && idx.column() > 0) {
      CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(idx.internalPointer());
      return d_ptr->m_lExtensions[idx.column()-1]->setData(item->collection,idx,value,role);
   }

   if (role == Qt::CheckStateRole && idx.column() == 0) {
      CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(idx.internalPointer());
      if (item) {
         const bool old = item->collection->isEnabled();
         ItemModelStateSerializationDelegate::instance()->setChecked(item->collection,value==Qt::Checked);
         emit dataChanged(index(idx.row(),0),index(idx.row(),columnCount()-1));
         if (old != (value==Qt::Checked)) {
            emit checkStateChanged();
         }
         return true;
      }
   }
   return false;
}

QModelIndex CollectionModel::parent( const QModelIndex& idx ) const
{
   if (idx.isValid()) {
      CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(idx.internalPointer());
      if (!item->parent)
         return QModelIndex();
      return createIndex(item->row,item->col,item->parent);
   }
   return QModelIndex();
}

QModelIndex CollectionModel::index( int row, int column, const QModelIndex& parent ) const
{
   if (parent.isValid()) {
      CollectionModelPrivate::ProxyItem* parentItem = static_cast<CollectionModelPrivate::ProxyItem*>(parent.internalPointer());
      CollectionModelPrivate::ProxyItem* item = nullptr;
      if (row < parentItem->m_Children.size())
         item = parentItem->m_Children[row];
      else {
         item = new CollectionModelPrivate::ProxyItem();
         item->parent = parentItem;
         item->collection = parentItem->collection->children()[row];
         parentItem->m_Children << item;
      }
      item->row    = row;
      item->col    = column;
      return createIndex(row,column,item);
   }
   else { //Top level
      CollectionModelPrivate::ProxyItem* item = nullptr;
      if (row < d_ptr->m_lTopLevelBackends.size())
         item = d_ptr->m_lTopLevelBackends[row];
      else {

         if (row >= PersonModel::instance()->collections().size())
            return QModelIndex();

         item = new CollectionModelPrivate::ProxyItem();
         item->collection = PersonModel::instance()->collections()[row];
         d_ptr->m_lTopLevelBackends << item;
      }
      item->row = row;
      item->col = column;
      return createIndex(item->row,item->col,item);
   }
}

void CollectionModelPrivate::slotUpdate()
{
   emit q_ptr->layoutChanged();
}

QVariant CollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      if (section > 0)
         return d_ptr->m_lExtensions[section-1]->headerName();
      return QVariant(tr("Name"));
   }
   return QVariant();
}

bool CollectionModel::save()
{
   if (ItemModelStateSerializationDelegate::instance()) {

      //Load newly enabled collections
      foreach(CollectionModelPrivate::ProxyItem* top, d_ptr->m_lTopLevelBackends) {
         CollectionInterface* current = top->collection;
         bool check = ItemModelStateSerializationDelegate::instance()->isChecked(current);
         bool wasChecked = current->isEnabled();
         if (check && !wasChecked)
            current->enable(true);
         else if ((!check) && wasChecked)
            current->enable(false);

         //TODO implement real tree digging
         foreach(CollectionModelPrivate::ProxyItem* leaf ,top->m_Children) {
            current = leaf->collection;
            check = ItemModelStateSerializationDelegate::instance()->isChecked(current);
            wasChecked = current->isEnabled();
            if (check && !wasChecked)
               current->enable(true);
            else if ((!check) && wasChecked)
               current->enable(false);
            //else: do nothing
         }
      }
      return ItemModelStateSerializationDelegate::instance()->save();
   }
   return false;
}

bool CollectionModel::load()
{
   if (ItemModelStateSerializationDelegate::instance()) {
      return ItemModelStateSerializationDelegate::instance()->load();
   }
   return false;
}

///Return the collection at a given index
CollectionInterface* CollectionModel::collectionAt(const QModelIndex& index)
{
   if (!index.isValid())
      return nullptr;
   return static_cast<CollectionModelPrivate::ProxyItem*>(index.internalPointer())->collection;
}

void CollectionModel::addExtension(CollectionExtensionInterface* extension)
{
   emit layoutAboutToBeChanged();
   d_ptr->m_lExtensions << extension;
   connect(extension,SIGNAL(dataChanged(QModelIndex)),d_ptr.data(),SLOT(slotExtensionDataChanged(QModelIndex)));
   emit layoutChanged();
}

void CollectionModelPrivate::slotExtensionDataChanged(const QModelIndex& idx)
{
   emit q_ptr->dataChanged(idx,idx);
}

void CollectionModelPrivate::registerNew(CollectionInterface* col)
{
   if (!col)
      return;

   ProxyItem* item = new ProxyItem();
   if (col->parent()) {
      ProxyItem* p = m_hBackendsNodes[col->parent()];
      item->parent = p;
      item->row    = p->m_Children.size();
      item->col    = 0;
      q_ptr->beginInsertRows(q_ptr->createIndex(p->row,p->col,p),p->m_Children.size(),p->m_Children.size());
      p->m_Children << item;
      q_ptr->endInsertRows();
   }
   else {
      item->parent = nullptr;
      item->row    = m_lTopLevelBackends.size();
      item->col    = 0;

      q_ptr->beginInsertRows(QModelIndex(),m_lTopLevelBackends.size(),m_lTopLevelBackends.size());
      m_lTopLevelBackends << item;
      q_ptr->endInsertRows();
   }


   item->collection      = col ;
   m_hBackendsNodes[col] = item;
}

#include <collectionmodel.moc>
