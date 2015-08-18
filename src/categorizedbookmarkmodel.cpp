/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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
#include "categorizedbookmarkmodel.h"

//Qt
#include <QtCore/QMimeData>
#include <QtCore/QCoreApplication>
#include <QtCore/QAbstractItemModel>

//Ring
#include "categorizedhistorymodel.h"
#include "dbus/presencemanager.h"
#include "phonedirectorymodel.h"
#include "contactmethod.h"
#include "callmodel.h"
#include "call.h"
#include "person.h"
#include "uri.h"
#include "mime.h"
#include "collectioneditor.h"
#include "collectioninterface.h"
#include "private/phonedirectorymodel_p.h"

class NumberTreeBackend;

class CategorizedBookmarkModelPrivate final : public QObject
{
   Q_OBJECT
public:
   CategorizedBookmarkModelPrivate(CategorizedBookmarkModel* parent);

   //Attributes
   QList<NumberTreeBackend*>                     m_lCategoryCounter ;
   QHash<QString,NumberTreeBackend*>             m_hCategories      ;
   QStringList                                   m_lMimes           ;
   QHash<ContactMethod*,QMetaObject::Connection> m_Tracked          ;

   //Helpers
   QString                 category             ( NumberTreeBackend* number ) const;
   bool                    displayFrequentlyUsed(                           ) const;
   QVector<ContactMethod*> bookmarkList         (                           ) const;

public Q_SLOTS:
   void slotIndexChanged( const QModelIndex& idx );

private:
   CategorizedBookmarkModel* q_ptr;
};

CategorizedBookmarkModel* CategorizedBookmarkModel::m_spInstance = nullptr;

//Model item/index
class NumberTreeBackend final
{
   friend class CategorizedBookmarkModel;
public:
   enum class Type {
      BOOKMARK,
      CATEGORY,
   };

   //Constructor
   explicit NumberTreeBackend(const QString& name);
   NumberTreeBackend(ContactMethod* number);
   virtual ~NumberTreeBackend();

   //Attributes
   ContactMethod*            m_pNumber    ;
   NumberTreeBackend*        m_pParent    ;
   int                       m_Index      ;
   Type                      m_Type       ;
   QString                   m_Name       ;
   bool                      m_MostPopular;
   QList<NumberTreeBackend*> m_lChildren  ;
   QMetaObject::Connection   m_Conn       ;
};

CategorizedBookmarkModelPrivate::CategorizedBookmarkModelPrivate(CategorizedBookmarkModel* parent) :
QObject(parent), q_ptr(parent)
{}

NumberTreeBackend::NumberTreeBackend(ContactMethod* number):
m_pNumber(number),m_pParent(nullptr),m_Index(-1), m_Type(NumberTreeBackend::Type::BOOKMARK),
m_MostPopular(false)
{
   Q_ASSERT(number != nullptr);
}

NumberTreeBackend::NumberTreeBackend(const QString& name)
   : m_Type(NumberTreeBackend::Type::CATEGORY),m_Name(name),
     m_MostPopular(false),m_Index(-1), m_pNumber(nullptr),m_pParent(nullptr)
{}

NumberTreeBackend::~NumberTreeBackend()
{
   QObject::disconnect(m_Conn);
}

CategorizedBookmarkModel::CategorizedBookmarkModel(QObject* parent) : QAbstractItemModel(parent), CollectionManagerInterface<ContactMethod>(this),
d_ptr(new CategorizedBookmarkModelPrivate(this))
{
   setObjectName("CategorizedBookmarkModel");
   reloadCategories();
   d_ptr->m_lMimes << RingMimes::PLAIN_TEXT << RingMimes::PHONENUMBER;

   if (d_ptr->displayFrequentlyUsed()) {
      connect(PhoneDirectoryModel::instance()->mostPopularNumberModel(),&QAbstractItemModel::rowsInserted,this,&CategorizedBookmarkModel::reloadCategories);
   }
}

CategorizedBookmarkModel::~CategorizedBookmarkModel()
{
   foreach(NumberTreeBackend* item, d_ptr->m_lCategoryCounter) {
      foreach (NumberTreeBackend* child, item->m_lChildren) {
         auto l = d_ptr->m_Tracked[child->m_pNumber];
         if (l) {
            disconnect(l);
         }
         delete child;
      }
      delete item;
   }
   delete d_ptr;
}

CategorizedBookmarkModel* CategorizedBookmarkModel::instance()
{
   if (! m_spInstance )
      m_spInstance = new CategorizedBookmarkModel(QCoreApplication::instance());
   return m_spInstance;
}

QHash<int,QByteArray> CategorizedBookmarkModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[static_cast<int>(Call::Role::Name)] = CallModel::instance()->roleNames()[static_cast<int>(Call::Role::Name)];
   }
   return roles;
}

///Reload bookmark cateogries
void CategorizedBookmarkModel::reloadCategories()
{
   d_ptr->m_hCategories.clear();

   beginRemoveRows(QModelIndex(), 0, d_ptr->m_lCategoryCounter.size()-1);

   //TODO this is not efficient, nor necessary
   foreach(NumberTreeBackend* item, d_ptr->m_lCategoryCounter) {
      foreach (NumberTreeBackend* child, item->m_lChildren) {
         auto l = d_ptr->m_Tracked[child->m_pNumber];
         if (l) {
            disconnect(l);
         }
         delete child;
      }
      delete item;
   }
   d_ptr->m_Tracked.clear();
   d_ptr->m_lCategoryCounter.clear();
   endRemoveRows();

   //Load most used contacts
   if (d_ptr->displayFrequentlyUsed()) {
      NumberTreeBackend* item = new NumberTreeBackend(tr("Most popular"));
      d_ptr->m_hCategories["mp"] = item;
      item->m_Index = d_ptr->m_lCategoryCounter.size();
      item->m_MostPopular = true;
      beginInsertRows(QModelIndex(), d_ptr->m_lCategoryCounter.size(),d_ptr->m_lCategoryCounter.size());
      d_ptr->m_lCategoryCounter << item;
      endInsertRows();

      //This a proxy, so the items already exist elsewhere
      beginInsertRows(index(item->m_Index,0), item->m_lChildren.size(), item->m_lChildren.size());
      endInsertRows();

      const QVector<ContactMethod*> cl = PhoneDirectoryModel::instance()->getNumbersByPopularity();
   }

   foreach(ContactMethod* bookmark, d_ptr->bookmarkList()) {
      NumberTreeBackend* bm = new NumberTreeBackend(bookmark);
      const QString val = d_ptr->category(bm);
      if (!d_ptr->m_hCategories[val]) {
         NumberTreeBackend* item = new NumberTreeBackend(val);
         d_ptr->m_hCategories[val] = item;
         item->m_Index = d_ptr->m_lCategoryCounter.size();
         beginInsertRows(QModelIndex(), d_ptr->m_lCategoryCounter.size(),d_ptr->m_lCategoryCounter.size());
         d_ptr->m_lCategoryCounter << item;
         endInsertRows();
      }
      NumberTreeBackend* item = d_ptr->m_hCategories[val];
      if (item) {
         bookmark->setBookmarked(true);
         bm->m_pParent = item;
         bm->m_Index = item->m_lChildren.size();
         bm->m_Conn = connect(bookmark, &ContactMethod::changed, [this,bm]() {
            d_ptr->slotIndexChanged(index(bm->m_Index,0,index(bm->m_pParent->m_Index,0)));
         });

         beginInsertRows(index(item->m_Index,0), item->m_lChildren.size(), item->m_lChildren.size());
         item->m_lChildren << bm;
         endInsertRows();

         if (!d_ptr->m_Tracked[bookmark]) {
            const QString displayName = bm->m_pNumber->roleData(Qt::DisplayRole).toString();

            QMetaObject::Connection conn = connect(bookmark, &ContactMethod::primaryNameChanged, [this,displayName,bm]() {
               //If a contact arrive later, reload
               if (displayName != bm->m_pNumber->roleData(Qt::DisplayRole)) {
                  reloadCategories();
               }
            });

            d_ptr->m_Tracked[bookmark] = conn;
         }
      }
      else
         qDebug() << "ERROR count";
   }


   emit layoutAboutToBeChanged();
   emit layoutChanged();
} //reloadCategories

//Do nothing
bool CategorizedBookmarkModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

///Get bookmark model data CategorizedCompositeNode::Type and Call::Role
QVariant CategorizedBookmarkModel::data( const QModelIndex& index, int role) const
{
   if ((!index.isValid()))
      return QVariant();

   if (index.parent().isValid()) {
      NumberTreeBackend* parentItem = static_cast<NumberTreeBackend*>(index.parent().internalPointer());
      if (parentItem->m_MostPopular) {
         return PhoneDirectoryModel::instance()->mostPopularNumberModel()->data(
            PhoneDirectoryModel::instance()->mostPopularNumberModel()->index(index.row(),0),
            role
         );
      }
   }

   NumberTreeBackend* modelItem = static_cast<NumberTreeBackend*>(index.internalPointer());

   if (!modelItem)
      return QVariant();

   switch (modelItem->m_Type) {
      case NumberTreeBackend::Type::CATEGORY:
         switch (role) {
            case Qt::DisplayRole:
               return modelItem->m_Name;
            case static_cast<int>(Call::Role::Name):
               //Make sure it is at the top of the bookmarks when sorted
               if (modelItem->m_MostPopular) {
                  return "000000";
               }
               else {
                  return modelItem->m_Name;
               }
         }
         break;
      case NumberTreeBackend::Type::BOOKMARK:
         return modelItem->m_pNumber->roleData(role == Qt::DisplayRole ? (int)Call::Role::Name : role);
         break;
   };

   return QVariant();
} //Data

///Get header data
QVariant CategorizedBookmarkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return QVariant(tr("Contacts"));
   return QVariant();
}

///Get the number of child of "parent"
int CategorizedBookmarkModel::rowCount( const QModelIndex& parent ) const
{
   if (!parent.isValid())
      return d_ptr->m_lCategoryCounter.size();

   NumberTreeBackend* modelItem = static_cast<NumberTreeBackend*>(parent.internalPointer());

   //Is from MostPopularModel
   if (!modelItem)
      return 0;

   switch (modelItem->m_Type) {
      case NumberTreeBackend::Type::CATEGORY:
         if (modelItem->m_MostPopular) {
            static PhoneDirectoryModel* m = PhoneDirectoryModel::instance();
            return m->d_ptr->m_lPopularityIndex.size();
         }
         else
            return modelItem->m_lChildren.size();
      case NumberTreeBackend::Type::BOOKMARK:
         return 0;
   }
   return 0;
}

Qt::ItemFlags CategorizedBookmarkModel::flags( const QModelIndex& index ) const
{
   if (!index.isValid())
      return 0;
   return index.isValid() ? (
      Qt::ItemIsEnabled    |
      Qt::ItemIsSelectable |
      (index.parent().isValid()?Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled:Qt::ItemIsEnabled)
   ) : Qt::NoItemFlags;
}

///There is only 1 column
int CategorizedBookmarkModel::columnCount ( const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return 1;
}

///Get the bookmark parent
QModelIndex CategorizedBookmarkModel::parent( const QModelIndex& idx) const
{
   if (!idx.isValid()) {
      return QModelIndex();
   }
   const NumberTreeBackend* modelItem = static_cast<NumberTreeBackend*>(idx.internalPointer());

   if (!modelItem)
      return index(d_ptr->m_hCategories["mp"]->m_Index,0);

   switch(modelItem->m_Type) {
      case NumberTreeBackend::Type::BOOKMARK:
         return index(modelItem->m_pParent->m_Index,0);
      case NumberTreeBackend::Type::CATEGORY:
         return QModelIndex();
   }
   return QModelIndex();
} //parent

///Get the index
QModelIndex CategorizedBookmarkModel::index(int row, int column, const QModelIndex& parent) const
{
   if (column != 0)
      return QModelIndex();

   if (parent.isValid() && d_ptr->m_lCategoryCounter.size() > parent.row()) {
      const NumberTreeBackend* modelItem = static_cast<NumberTreeBackend*>(parent.internalPointer());

      if (modelItem->m_MostPopular)
         return createIndex(row, column, nullptr);

      if (modelItem->m_lChildren.size() > row)
         return createIndex(row,column,(void*) static_cast<NumberTreeBackend*>(modelItem->m_lChildren[row]));
   }
   else if (row >= 0 && row < d_ptr->m_lCategoryCounter.size()) {
      return createIndex(row,column,(void*) static_cast<NumberTreeBackend*>(d_ptr->m_lCategoryCounter[row]));
   }
   return QModelIndex();
}

///Get bookmarks mime types
QStringList CategorizedBookmarkModel::mimeTypes() const
{
   return d_ptr->m_lMimes;
}

///Generate mime data
QMimeData* CategorizedBookmarkModel::mimeData(const QModelIndexList &indexes) const
{
   QMimeData *mimeData = new QMimeData();
   foreach (const QModelIndex &index, indexes) {
      if (index.isValid()) {
         QString text = data(index, static_cast<int>(Call::Role::Number)).toString();
         mimeData->setData(RingMimes::PLAIN_TEXT , text.toUtf8());
         mimeData->setData(RingMimes::PHONENUMBER, text.toUtf8());
         return mimeData;
      }
   }
   return mimeData;
} //mimeData

///Return valid payload types
int CategorizedBookmarkModel::acceptedPayloadTypes()
{
   return CallModel::DropPayloadType::CALL;
}

///Get category
QString CategorizedBookmarkModelPrivate::category(NumberTreeBackend* number) const
{
   if (number->m_Name.size())
      return number->m_Name;

   QString cat = number->m_pNumber->roleData(Qt::DisplayRole).toString();

   if (cat.size())
      cat = cat[0].toUpper();
   return cat;
}

bool CategorizedBookmarkModelPrivate::displayFrequentlyUsed() const
{
   return true;
}

QVector<ContactMethod*> CategorizedBookmarkModelPrivate::bookmarkList() const
{
   return (q_ptr->collections().size() > 0) ? q_ptr->collections()[0]->items<ContactMethod>() : QVector<ContactMethod*>();
}

void CategorizedBookmarkModel::addBookmark(ContactMethod* number)
{
   Q_UNUSED(number)
   if (collections().size())
      collections()[0]->editor<ContactMethod>()->addNew(number);
   else
      qWarning() << "No bookmark backend is set";
}

void CategorizedBookmarkModel::removeBookmark(ContactMethod* number)
{
   collections()[0]->editor<ContactMethod>()->remove(number);
}

void CategorizedBookmarkModel::remove(const QModelIndex& idx)
{
   collections()[0]->editor<ContactMethod>()->remove(getNumber(idx));
}

ContactMethod* CategorizedBookmarkModel::getNumber(const QModelIndex& idx)
{
   if (idx.isValid()) {
      if (idx.parent().isValid() && idx.parent().row() < d_ptr->m_lCategoryCounter.size()) {
         NumberTreeBackend* bm = d_ptr->m_lCategoryCounter[idx.parent().row()]->m_lChildren[idx.row()];
         return bm->m_pNumber;
      }
   }
   return nullptr;
}

///Callback when an item change
void CategorizedBookmarkModelPrivate::slotIndexChanged(const QModelIndex& idx)
{
   emit q_ptr->dataChanged(idx,idx);
}

bool CategorizedBookmarkModel::addItemCallback(const ContactMethod* item)
{
   Q_UNUSED(item)
   reloadCategories(); //TODO this is far from optimal
   return true;
}

bool CategorizedBookmarkModel::removeItemCallback(const ContactMethod* item)
{
   Q_UNUSED(item)
   return false;
}

void CategorizedBookmarkModel::collectionAddedCallback(CollectionInterface* backend)
{
   Q_UNUSED(backend)
   reloadCategories();
}

#include <categorizedbookmarkmodel.moc>
