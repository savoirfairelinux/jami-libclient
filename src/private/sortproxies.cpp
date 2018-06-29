/****************************************************************************
 *   Copyright (C) 2015-2018 Savoir-faire Linux                               *
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
#include "sortproxies.h"

//LibSTDC++
#include <functional>

//Qt
#include <QtCore/QAbstractListModel>
#include <QtCore/QItemSelectionModel>
#include <QtCore/QSortFilterProxyModel>

//Ring
#include "matrixutils.h"
#include <globalinstances.h>
#include <interfaces/pixmapmanipulatori.h>

namespace CategoryModelCommon {
   inline Qt::ItemFlags flags(const QModelIndex& idx) {
      return idx.isValid()?Qt::ItemIsEnabled|Qt::ItemIsSelectable : Qt::NoItemFlags;
   }

   bool setData( const QModelIndex& index, const QVariant &value, int role);
   bool setData( const QModelIndex& index, const QVariant &value, int role) {
      Q_UNUSED(index)
      Q_UNUSED(value)
      Q_UNUSED(role)
      return false;
   }
}

class RemoveDisabledProxy : public QSortFilterProxyModel
{
   Q_OBJECT
public:
   explicit RemoveDisabledProxy(QObject* parent) : QSortFilterProxyModel(parent) {
      setDynamicSortFilter(true);
   }
protected:
   virtual bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const override;
};

class ContactSortingCategoryModel : public QAbstractListModel
{
   Q_OBJECT
public:
   ContactSortingCategoryModel(QObject* parent = nullptr);
   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;
   virtual bool          setData  ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual ~ContactSortingCategoryModel();
};

class HistorySortingCategoryModel : public QAbstractListModel
{
   Q_OBJECT
public:
   HistorySortingCategoryModel(QObject* parent = nullptr);
   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;
   virtual bool          setData  ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual ~HistorySortingCategoryModel();
};

bool RemoveDisabledProxy::filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const
{
   const Qt::ItemFlags flags = sourceModel()->index(source_row,0,source_parent).flags();
   if (!(flags & Qt::ItemIsEnabled))
      return false;
   else if (!source_parent.isValid() || source_parent.parent().isValid())
      return true;

   return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

ContactSortingCategoryModel::ContactSortingCategoryModel(QObject* parent) : QAbstractListModel(parent)
{

}

ContactSortingCategoryModel::~ContactSortingCategoryModel()
{

}

void sortContact(QSortFilterProxyModel* p, int roleIdx);
void sortContact(QSortFilterProxyModel* p, int roleIdx)
{
}

QVariant ContactSortingCategoryModel::data( const QModelIndex& index, int role ) const
{
   return QVariant();
}

int ContactSortingCategoryModel::rowCount( const QModelIndex& parent) const
{
   return 0;
}

Qt::ItemFlags ContactSortingCategoryModel::flags( const QModelIndex& index ) const
{
   return CategoryModelCommon::flags(index);
}

bool ContactSortingCategoryModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   return CategoryModelCommon::setData(index,value,role);
}

void sortHistory(QSortFilterProxyModel* p, int role);
void sortHistory(QSortFilterProxyModel* p, int role)
{

}

HistorySortingCategoryModel::HistorySortingCategoryModel(QObject* parent) : QAbstractListModel(parent)
{

}

HistorySortingCategoryModel::~HistorySortingCategoryModel()
{

}

QVariant HistorySortingCategoryModel::data( const QModelIndex& index, int role) const
{
   return QVariant();
}

int HistorySortingCategoryModel::rowCount( const QModelIndex& parent) const
{
   return 0;
}

Qt::ItemFlags HistorySortingCategoryModel::flags( const QModelIndex& index ) const
{
   return CategoryModelCommon::flags(index);
}

bool HistorySortingCategoryModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   return CategoryModelCommon::setData(index,value,role);
}

template<typename T>
SortingCategory::ModelTuple* createModels(QAbstractItemModel* src, int filterRole, int sortRole, std::function<void(QSortFilterProxyModel*,const QModelIndex&)> callback)
{
   SortingCategory::ModelTuple* ret = new SortingCategory::ModelTuple;

   ret->categories = new T(src);

   QSortFilterProxyModel* proxy = new RemoveDisabledProxy(src);
   proxy->setSortRole              ( sortRole                  );
   proxy->setSortLocaleAware       ( true                      );
   proxy->setFilterRole            ( filterRole                );
   proxy->setSortCaseSensitivity   ( Qt::CaseInsensitive       );
   proxy->setFilterCaseSensitivity ( Qt::CaseInsensitive       );
   ret->model = proxy;
   ret->model->setSourceModel(src);

   ret->selectionModel = new QItemSelectionModel(ret->model);
   QObject::connect(ret->selectionModel, &QItemSelectionModel::currentChanged,[proxy,callback](const QModelIndex& idx) {
      callback(proxy,idx);
   });

   return ret;
}

SortingCategory::ModelTuple* SortingCategory::getContactProxy()
{
   return nullptr;
}

SortingCategory::ModelTuple* SortingCategory::getHistoryProxy()
{
   return nullptr;
}

#include <sortproxies.moc>
