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
#include <QtCore/QSortFilterProxyModel>

//Ring
#include "collectionmanagerinterface.h"
#include "delegates/itemmodelstateserializationdelegate.h"
#include "collectionextensioninterface.h"
#include "private/collectionmodel_p.h"

CollectionModel* CollectionModelPrivate::m_spInstance = nullptr;

class ManageableCollectionProxy : public QSortFilterProxyModel
{
public:
   ManageableCollectionProxy(QAbstractItemModel* parent) : QSortFilterProxyModel(parent)
   {
      setSourceModel(parent);
   }
   virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};

CollectionModelPrivate::CollectionModelPrivate(CollectionModel* parent) : QObject(parent),q_ptr(parent),m_pManageableProxy(nullptr)
{}

CollectionModel::CollectionModel(QObject* parent) : QAbstractTableModel(parent), d_ptr(new CollectionModelPrivate(this))
{
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
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[static_cast<int>(Role::count                ) ] = "count"                 ;
      roles[static_cast<int>(Role::supportNone          ) ] = "supportNone"           ;
      roles[static_cast<int>(Role::supportLoad          ) ] = "supportLoad"           ;
      roles[static_cast<int>(Role::supportSave          ) ] = "supportSave"           ;
      roles[static_cast<int>(Role::supportEdit          ) ] = "supportEdit"           ;
      roles[static_cast<int>(Role::supportProbe         ) ] = "supportProbe"          ;
      roles[static_cast<int>(Role::supportAdd           ) ] = "supportAdd"            ;
      roles[static_cast<int>(Role::supportSave_all      ) ] = "supportSave_all"       ;
      roles[static_cast<int>(Role::supportClear         ) ] = "supportClear"          ;
      roles[static_cast<int>(Role::supportRemove        ) ] = "supportRemove"         ;
      roles[static_cast<int>(Role::supportExport        ) ] = "supportExport"         ;
      roles[static_cast<int>(Role::supportImport        ) ] = "supportImport"         ;
      roles[static_cast<int>(Role::isEnableable         ) ] = "isEnableable"          ;
      roles[static_cast<int>(Role::isDisableable        ) ] = "isDisableable"         ;
      roles[static_cast<int>(Role::isManageable         ) ] = "isManageable"          ;
      roles[static_cast<int>(Role::hasManageableChildren) ] = "hasManageableChildren" ;

   }

   return roles;
}

QVariant CollectionModel::data (const QModelIndex& idx, int role) const
{
   if (idx.isValid()) {
      CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(idx.internalPointer());

      if (idx.column() > 0 && item->collection)
         return d_ptr->m_lExtensions[idx.column()-1]->data(item->collection,idx,role);

      if (item->collection) {
         switch(role) {
            case Qt::DisplayRole:
               return item->collection->name();
               break;
            case Qt::DecorationRole:
               return item->collection->icon();
               break;
            case Qt::UserRole: // and Role::count
               return 'x'+QString::number(item->collection->size());
               break;
            case Qt::CheckStateRole: {
               if (ItemModelStateSerializationDelegate::instance()) //TODO do better than that
                  return item->collection->isEnabled()?Qt::Checked:Qt::Unchecked;
            }
            case static_cast<int>(Role::hasManageableChildren):
               return item->manageableCount ? true : false;
               break;
         };

         //Retro-map to the SupportedFeatures //WARNING if SupportedFeatures change, this need to be updated
         if (role > Qt::UserRole && role <= Qt::UserRole+14) {
            return (bool) (role == Qt::UserRole+1 ? true : bool(item->collection->supportedFeatures() & ((CollectionInterface::SupportedFeatures)(0x01 << (role - Qt::UserRole - 2)))));
         }
      }
      else {
         switch(role) {
            case Qt::DisplayRole:
               return item->m_AltName;
               break;
            case static_cast<int>(Role::hasManageableChildren):
               return item->manageableCount ? true : false;
               break;
         }
      }
   }
   return QVariant();
}

int CollectionModel::rowCount (const QModelIndex& parent) const
{
   if (!parent.isValid()) {
      return d_ptr->m_lTopLevelBackends.size();
   }
   else {
      CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(parent.internalPointer());
      return item->m_Children.size();
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

   //Categories can only be displayed
   if (!item->collection)
      return Qt::ItemIsEnabled;

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
      return (!item->collection)?false : d_ptr->m_lExtensions[idx.column()-1]->setData(item->collection,idx,value,role);
   }

   if (role == Qt::CheckStateRole && idx.column() == 0) {
      CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(idx.internalPointer());
      if (item && item->collection) {
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
   if (parent.isValid() && parent.model() == this && row < rowCount(parent)) {
      CollectionModelPrivate::ProxyItem* parentItem = static_cast<CollectionModelPrivate::ProxyItem*>(parent.internalPointer());
      CollectionModelPrivate::ProxyItem* item = nullptr;
      if (row < parentItem->m_Children.size())
         item = parentItem->m_Children[row];
      else {
         return QModelIndex();
      }
      item->row    = row; //FIXME dead code?
      item->col    = column;
      return createIndex(row,column,item);
   }
   else { //Top level
      CollectionModelPrivate::ProxyItem* item = nullptr;
      if (row < d_ptr->m_lTopLevelBackends.size())
         item = d_ptr->m_lTopLevelBackends[row];
      else {
         return QModelIndex();
      }
      item->row = row; //FIXME dead code?
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
         bool check,wasChecked;
         if (current) {
            check = ItemModelStateSerializationDelegate::instance()->isChecked(current);
            wasChecked = current->isEnabled();
            if (check && !wasChecked)
               current->enable(true);
            else if ((!check) && wasChecked)
               current->enable(false);
         }

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

   ProxyItem* cat = m_hCategories[col->category()];
   if (col->category().isEmpty()){
      //TODO implement a default category
   }

   if (!cat) {
      cat              = new ProxyItem();
      cat->parent      = nullptr;
      cat->row         = m_lTopLevelBackends.size();
      cat->col         = 0;
      cat->m_AltName   = col->category();
      cat->collection  = nullptr;

      m_hCategories[cat->m_AltName] = cat;
      m_hBackendsNodes[col] = cat;

      q_ptr->beginInsertRows(QModelIndex(),m_lTopLevelBackends.size(),m_lTopLevelBackends.size());
      m_lTopLevelBackends << cat;
      q_ptr->endInsertRows();
   }

   ProxyItem* item = new ProxyItem();
   const bool hasParent = col->parent();
   ProxyItem* par = hasParent?m_hBackendsNodes[col->parent()] : cat;

   item->parent = par;
   item->row    = par->m_Children.size();
   item->col    = 0;
   q_ptr->beginInsertRows(q_ptr->createIndex(par->row,par->col,par),par->m_Children.size(),par->m_Children.size());
   par->m_Children << item;
   q_ptr->endInsertRows();

   //Make sure the manageable proxy get noticed things changed
   if (col->supportedFeatures() & CollectionInterface::SupportedFeatures::MANAGEABLE) {
      item->manageableCount++;
      while(par) {
         par->manageableCount++;
         const QModelIndex& idx = q_ptr->createIndex(par->row,0,par);
         emit q_ptr->dataChanged(idx,idx);
         par = par->parent;
      }
   }

   item->collection      = col ;
   m_hBackendsNodes[col] = item;
}

bool ManageableCollectionProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
   CollectionModelPrivate::ProxyItem* item = static_cast<CollectionModelPrivate::ProxyItem*>(sourceModel()->index(source_row,0,source_parent).internalPointer());
   return item ? item->manageableCount : false;
}

/**
 * Filter the CollectionModel to only keep the manageable collections. Those
 * collections can be exposed and configured in the UI
 */
QAbstractItemModel* CollectionModel::manageableCollections() const
{
   if (!d_ptr->m_pManageableProxy)
      d_ptr->m_pManageableProxy = new ManageableCollectionProxy(const_cast<CollectionModel*>(this));
   return d_ptr->m_pManageableProxy;
}

#include <collectionmodel.moc>
