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
#include "sortproxies.h"

//LibSTDC++
#include <functional>

//Qt
#include <QtCore/QAbstractListModel>
#include <QtCore/QItemSelectionModel>
#include <QtCore/QSortFilterProxyModel>

//Ring
#include "matrixutils.h"
#include <categorizedcontactmodel.h>
#include <categorizedhistorymodel.h>
#include <interfaces/instances.h>
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

   static const Matrix1D<CategorizedContactModel::SortedProxy::Categories,QString> contactSortNames = {{
      QObject::tr("Name"         ),
      QObject::tr("Organisation" ),
      QObject::tr("Recently used"),
      QObject::tr("Group"        ),
      QObject::tr("Department"   ),
   }};

   static const Matrix1D<CategorizedHistoryModel::SortedProxy::Categories,QString> historySortNames = {{
      QObject::tr("Date"       ),
      QObject::tr("Name"       ),
      QObject::tr("Popularity" ),
      QObject::tr("Duration"   ),
      QObject::tr("Total time" ),
   }};

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
   static CategorizedContactModel* m = CategorizedContactModel::instance();
   switch(static_cast<CategorizedContactModel::SortedProxy::Categories>(roleIdx)) {
      case CategorizedContactModel::SortedProxy::Categories::NAME:
         m->setSortAlphabetical(true);
         m->setDefaultCategory(QObject::tr("Empty"));
         p->setSortRole(Qt::DisplayRole);
         m->setRole(Qt::DisplayRole);
         break;
      case CategorizedContactModel::SortedProxy::Categories::ORGANIZATION:
         m->setSortAlphabetical(false);
         m->setDefaultCategory(QObject::tr("Unknown"));
         p->setSortRole((int)Person::Role::Organization);
         m->setRole((int)Person::Role::Organization);
         break;
      case CategorizedContactModel::SortedProxy::Categories::RECENTLYUSED:
         m->setSortAlphabetical(false);
         m->setDefaultCategory(QObject::tr("Never"));
         p->setSortRole((int)Person::Role::IndexedLastUsed);
         m->setRole((int)Person::Role::FormattedLastUsed);
         break;
      case CategorizedContactModel::SortedProxy::Categories::GROUP:
         m->setSortAlphabetical(false);
         m->setDefaultCategory(QObject::tr("Other"));
         p->setSortRole((int)Person::Role::Group);
         m->setRole((int)Person::Role::Group);
         break;
      case CategorizedContactModel::SortedProxy::Categories::DEPARTMENT:
         m->setSortAlphabetical(false);
         m->setDefaultCategory(QObject::tr("Unknown"));
         p->setSortRole((int)Person::Role::Department);
         m->setRole((int)Person::Role::Department);
         break;
      case CategorizedContactModel::SortedProxy::Categories::COUNT__:
         break;
   };
}

QVariant ContactSortingCategoryModel::data( const QModelIndex& index, int role ) const
{
   if (index.isValid()) {
      switch (role) {
         case Qt::DisplayRole:
            return CategoryModelCommon::contactSortNames[static_cast<CategorizedContactModel::SortedProxy::Categories>(index.row())];
         case Qt::DecorationRole:
            return Interfaces::pixmapManipulator().contactSortingCategoryIcon(static_cast<CategorizedContactModel::SortedProxy::Categories>(index.row()));
      }
   }
   return QVariant();
}

int ContactSortingCategoryModel::rowCount( const QModelIndex& parent) const
{
   return parent.isValid()? 0 : enum_class_size<CategorizedContactModel::SortedProxy::Categories>();
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
   switch (static_cast<CategorizedHistoryModel::SortedProxy::Categories>(role)) {
      case CategorizedHistoryModel::SortedProxy::Categories::DATE:
         CategorizedHistoryModel::instance()->setCategoryRole(static_cast<int>(Call::Role::FuzzyDate));
         p->setSortRole(static_cast<int>(Call::Role::Date));
         break;
      case CategorizedHistoryModel::SortedProxy::Categories::NAME:
         CategorizedHistoryModel::instance()->setCategoryRole(static_cast<int>(Call::Role::Name));
         p->setSortRole(Qt::DisplayRole);
         break;
      case CategorizedHistoryModel::SortedProxy::Categories::POPULARITY:
         CategorizedHistoryModel::instance()->setCategoryRole(static_cast<int>(Call::Role::CallCount));
         p->setSortRole(static_cast<int>(Call::Role::CallCount));
         break;
      case CategorizedHistoryModel::SortedProxy::Categories::LENGTH:
         CategorizedHistoryModel::instance()->setCategoryRole(static_cast<int>(Call::Role::Length));
         p->setSortRole(static_cast<int>(Call::Role::Length));
         break;
      case CategorizedHistoryModel::SortedProxy::Categories::SPENT_TIME:
         CategorizedHistoryModel::instance()->setCategoryRole(static_cast<int>(Call::Role::TotalSpentTime));
         p->setSortRole(static_cast<int>(Call::Role::TotalSpentTime));
         break;
      case CategorizedHistoryModel::SortedProxy::Categories::COUNT__:
         break;
   }
}

HistorySortingCategoryModel::HistorySortingCategoryModel(QObject* parent) : QAbstractListModel(parent)
{

}

HistorySortingCategoryModel::~HistorySortingCategoryModel()
{

}

QVariant HistorySortingCategoryModel::data( const QModelIndex& index, int role) const
{
   if (index.isValid()) {
      switch(role) {
         case Qt::DisplayRole:
            return CategoryModelCommon::historySortNames[static_cast<CategorizedHistoryModel::SortedProxy::Categories>(index.row())];
         case Qt::DecorationRole:
            return Interfaces::pixmapManipulator().historySortingCategoryIcon(static_cast<CategorizedHistoryModel::SortedProxy::Categories>(index.row()));
      }
   }

   return QVariant();
}

int HistorySortingCategoryModel::rowCount( const QModelIndex& parent) const
{
   return parent.isValid()? 0 : enum_class_size<CategorizedHistoryModel::SortedProxy::Categories>();
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
SortingCategory::ModelTuple* createModels(QAbstractItemModel* src, int role, std::function<void(QSortFilterProxyModel*,const QModelIndex&)> callback)
{
   SortingCategory::ModelTuple* ret = new SortingCategory::ModelTuple;

   ret->categories = new T(src);

   QSortFilterProxyModel* proxy = new RemoveDisabledProxy(src);
   proxy->setSortRole              ( Qt::DisplayRole           );
   proxy->setSortLocaleAware       ( true                      );
   proxy->setFilterRole            ( role );
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
   return createModels<ContactSortingCategoryModel>(CategorizedContactModel::instance(),(int)Person::Role::Filter, [](QSortFilterProxyModel* proxy,const QModelIndex& idx) {
      if (idx.isValid()) {
         qDebug() << "Selection changed" << idx.row();
         sortContact(proxy,idx.row());
      }
   });
}

SortingCategory::ModelTuple* SortingCategory::getHistoryProxy()
{
   return createModels<HistorySortingCategoryModel>(CategorizedHistoryModel::instance(),static_cast<int>(Call::Role::Date), [](QSortFilterProxyModel* proxy,const QModelIndex& idx) {
     if (idx.isValid()) {
         qDebug() << "Selection changed" << idx.row();
         sortHistory(proxy,idx.row());
      }
   });
}

#include <sortproxies.moc>
