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

///Top level bookmark item
class BookmarkTopLevelItem : public CategorizedCompositeNode {
   friend class CategorizedBookmarkModel;
   public:
      virtual QObject* getSelf() const;
      int m_Row;
   private:
      explicit BookmarkTopLevelItem(QString name);
      QList<NumberTreeBackend*> m_lChildren;
      QString m_Name;
      bool m_MostPopular;
};

class CategorizedBookmarkModelPrivate : public QObject
{
   Q_OBJECT
public:
   CategorizedBookmarkModelPrivate(CategorizedBookmarkModel* parent);

//    QVector<CollectionInterface*> m_lBackends;

   //Attributes
   QList<BookmarkTopLevelItem*>         m_lCategoryCounter ;
   QHash<QString,BookmarkTopLevelItem*> m_hCategories      ;
   QStringList                          m_lMimes           ;

   //Helpers
   QVariant commonCallInfo(NumberTreeBackend* call, int role = Qt::DisplayRole) const;
   QString category(NumberTreeBackend* number) const;
   bool                  displayFrequentlyUsed() const;
   QVector<ContactMethod*>   bookmarkList         () const;
   static QVector<ContactMethod*> serialisedToList(const QStringList& list);

private Q_SLOTS:
   void slotRequest(const QString& uri);
   void slotIndexChanged(const QModelIndex& idx);

private:
   CategorizedBookmarkModel* q_ptr;
};

CategorizedBookmarkModel* CategorizedBookmarkModel::m_spInstance = nullptr;

class BookmarkItemNode;

static bool test = false;
//Model item/index
class NumberTreeBackend : public CategorizedCompositeNode
{
   friend class CategorizedBookmarkModel;
   public:
      NumberTreeBackend(ContactMethod* number);
      virtual ~NumberTreeBackend();
      virtual QObject* getSelf() const { return nullptr; }

      ContactMethod* m_pNumber;
      BookmarkTopLevelItem* m_pParent;
      int m_Index;
      BookmarkItemNode* m_pNode;
};

class BookmarkItemNode : public QObject //TODO remove this once Qt4 support is dropped
{
   Q_OBJECT
public:
   BookmarkItemNode(CategorizedBookmarkModel* m, ContactMethod* n, NumberTreeBackend* backend);
private:
   NumberTreeBackend* m_pBackend;
   CategorizedBookmarkModel* m_pModel;
private Q_SLOTS:
   void slotNumberChanged();
Q_SIGNALS:
   void changed(const QModelIndex& idx);
};

CategorizedBookmarkModelPrivate::CategorizedBookmarkModelPrivate(CategorizedBookmarkModel* parent) : QObject(parent), q_ptr(parent)
{
   
}

NumberTreeBackend::NumberTreeBackend(ContactMethod* number): CategorizedCompositeNode(CategorizedCompositeNode::Type::BOOKMARK),
   m_pNumber(number),m_pParent(nullptr),m_pNode(nullptr),m_Index(-1){
   Q_ASSERT(number != nullptr);
}

NumberTreeBackend::~NumberTreeBackend() {
   if (m_pNode) delete m_pNode;
}

BookmarkItemNode::BookmarkItemNode(CategorizedBookmarkModel* m, ContactMethod* n, NumberTreeBackend* backend) :
m_pBackend(backend),m_pModel(m){
   connect(n,SIGNAL(changed()),this,SLOT(slotNumberChanged()));
}

void BookmarkItemNode::slotNumberChanged()
{
   emit changed(m_pModel->index(m_pBackend->m_Index,0,m_pModel->index(m_pBackend->m_pParent->m_Row,0)));
}

QObject* BookmarkTopLevelItem::getSelf() const
{
   return nullptr;
}

CategorizedBookmarkModel::CategorizedBookmarkModel(QObject* parent) : QAbstractItemModel(parent), CollectionManagerInterface<ContactMethod>(this),
d_ptr(new CategorizedBookmarkModelPrivate(this))
{
   setObjectName("CategorizedBookmarkModel");
   reloadCategories();
   d_ptr->m_lMimes << RingMimes::PLAIN_TEXT << RingMimes::PHONENUMBER;

   //Connect
   connect(&DBus::PresenceManager::instance(),SIGNAL(newServerSubscriptionRequest(QString)),d_ptr,SLOT(slotRequest(QString)));
//    if (Call::contactBackend()) {
//       connect(Call::contactBackend(),SIGNAL(collectionChanged()),this,SLOT(reloadCategories()));
//    } //TODO implement reordering
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
   test = true;
   beginResetModel(); {
      d_ptr->m_hCategories.clear();

      //TODO this is not efficient, nor necessary
      foreach(BookmarkTopLevelItem* item, d_ptr->m_lCategoryCounter) {
         foreach (NumberTreeBackend* child, item->m_lChildren) {
            delete child;
         }
         delete item;
      }
      d_ptr->m_lCategoryCounter.clear();

      //Load most used contacts
      if (d_ptr->displayFrequentlyUsed()) {
         BookmarkTopLevelItem* item = new BookmarkTopLevelItem(tr("Most popular"));
         d_ptr->m_hCategories["mp"] = item;
         item->m_Row = d_ptr->m_lCategoryCounter.size();
         item->m_MostPopular = true;
         d_ptr->m_lCategoryCounter << item;
         const QVector<ContactMethod*> cl = PhoneDirectoryModel::instance()->getNumbersByPopularity();

         for (int i=0;i<((cl.size()>=10)?10:cl.size());i++) {
            ContactMethod* n = cl[i];
            NumberTreeBackend* bm = new NumberTreeBackend(n);
            bm->m_pParent = item;
            bm->m_Index = item->m_lChildren.size();
            bm->m_pNode = new BookmarkItemNode(this,n,bm);
            connect(bm->m_pNode,SIGNAL(changed(QModelIndex)),d_ptr,SLOT(slotIndexChanged(QModelIndex)));
            item->m_lChildren << bm;
         }

      }

      foreach(ContactMethod* bookmark, d_ptr->bookmarkList()) {
         NumberTreeBackend* bm = new NumberTreeBackend(bookmark);
         const QString val = d_ptr->category(bm);
         if (!d_ptr->m_hCategories[val]) {
            BookmarkTopLevelItem* item = new BookmarkTopLevelItem(val);
            d_ptr->m_hCategories[val] = item;
            item->m_Row = d_ptr->m_lCategoryCounter.size();
            d_ptr->m_lCategoryCounter << item;
         }
         BookmarkTopLevelItem* item = d_ptr->m_hCategories[val];
         if (item) {
            bookmark->setBookmarked(true);
            bm->m_pParent = item;
            bm->m_Index = item->m_lChildren.size();
            bm->m_pNode = new BookmarkItemNode(this,bookmark,bm);
            connect(bm->m_pNode,SIGNAL(changed(QModelIndex)),d_ptr,SLOT(slotIndexChanged(QModelIndex)));
            item->m_lChildren << bm;
         }
         else
            qDebug() << "ERROR count";
      }

   } endResetModel();

   emit layoutAboutToBeChanged();
   test = false;
   emit layoutChanged();
} //reloadCategories

//Do nothing
bool CategorizedBookmarkModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

///Get bookmark model data CategorizedCompositeNode::Type and Call::Role
QVariant CategorizedBookmarkModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid() || test)
      return QVariant();

   CategorizedCompositeNode* modelItem = static_cast<CategorizedCompositeNode*>(index.internalPointer());
   if (!modelItem)
      return QVariant();
   switch (modelItem->type()) {
      case CategorizedCompositeNode::Type::TOP_LEVEL:
         switch (role) {
            case Qt::DisplayRole:
               return static_cast<BookmarkTopLevelItem*>(modelItem)->m_Name;
            case static_cast<int>(Call::Role::Name):
               if (static_cast<BookmarkTopLevelItem*>(modelItem)->m_MostPopular) {
                  return "000000";
               }
               else {
                  return static_cast<BookmarkTopLevelItem*>(modelItem)->m_Name;
               }
         }
         break;
      case CategorizedCompositeNode::Type::BOOKMARK:
         return d_ptr->commonCallInfo(static_cast<NumberTreeBackend*>(modelItem),role);
         break;
      case CategorizedCompositeNode::Type::CALL:
      case CategorizedCompositeNode::Type::NUMBER:
      case CategorizedCompositeNode::Type::CONTACT:
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
   if (test) return 0; //HACK
   if (!parent.isValid())
      return d_ptr->m_lCategoryCounter.size();
   else if (!parent.parent().isValid() && parent.row() < d_ptr->m_lCategoryCounter.size()) {
      BookmarkTopLevelItem* item = static_cast<BookmarkTopLevelItem*>(parent.internalPointer());
      return item->m_lChildren.size();
   }
   return 0;
}

Qt::ItemFlags CategorizedBookmarkModel::flags( const QModelIndex& index ) const
{
   if (!index.isValid())
      return 0;
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | (index.parent().isValid()?Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled:Qt::ItemIsEnabled);
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
   const CategorizedCompositeNode* modelItem = static_cast<CategorizedCompositeNode*>(idx.internalPointer());
   if (modelItem->type() == CategorizedCompositeNode::Type::BOOKMARK) {
      BookmarkTopLevelItem* item = static_cast<const NumberTreeBackend*>(modelItem)->m_pParent;
      if (item) {
         return index(item->m_Row,0);
      }
   }
   return QModelIndex();
} //parent

///Get the index
QModelIndex CategorizedBookmarkModel::index(int row, int column, const QModelIndex& parent) const
{
   if (parent.isValid() && (!column) && d_ptr->m_lCategoryCounter.size() > parent.row() && d_ptr->m_lCategoryCounter[parent.row()]->m_lChildren.size() > row)
      return createIndex(row,column,(void*) static_cast<CategorizedCompositeNode*>(d_ptr->m_lCategoryCounter[parent.row()]->m_lChildren[row]));
   else if (row >= 0 && row < d_ptr->m_lCategoryCounter.size() && !column) {
      return createIndex(row,column,(void*) static_cast<CategorizedCompositeNode*>(d_ptr->m_lCategoryCounter[row]));
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

///Get call info TODO use Call:: one
QVariant CategorizedBookmarkModelPrivate::commonCallInfo(NumberTreeBackend* number, int role) const
{
   if (!number)
      return QVariant();
   QVariant cat;
   switch (role) {
      case Qt::DisplayRole:
      case static_cast<int>(Call::Role::Name):
         cat = number->m_pNumber->contact()?number->m_pNumber->contact()->formattedName():number->m_pNumber->primaryName();
         break;
      case Qt::ToolTipRole:
         cat = number->m_pNumber->presenceMessage();
         break;
      case static_cast<int>(Call::Role::Number):
         cat = number->m_pNumber->uri();//call->getPeerContactMethod();
         break;
      case static_cast<int>(Call::Role::Direction):
         cat = 4;//call->getHistoryState();
         break;
      case static_cast<int>(Call::Role::Date):
         cat = tr("N/A");//call->getStartTimeStamp();
         break;
      case static_cast<int>(Call::Role::Length):
         cat = tr("N/A");//call->getLength();
         break;
      case static_cast<int>(Call::Role::FormattedDate):
         cat = tr("N/A");//QDateTime::fromTime_t(call->getStartTimeStamp().toUInt()).toString();
         break;
      case static_cast<int>(Call::Role::HasAVRecording):
         cat = false;//call->hasRecording();
         break;
      case static_cast<int>(Call::Role::FuzzyDate):
         cat = "N/A";//timeToHistoryCategory(QDateTime::fromTime_t(call->getStartTimeStamp().toUInt()).date());
         break;
      case static_cast<int>(Call::Role::ContactMethod):
         return QVariant::fromValue(number->m_pNumber);
      case static_cast<int>(Call::Role::IsBookmark):
         return true;
      case static_cast<int>(Call::Role::Filter):
         return number->m_pNumber->uri()+number->m_pNumber->primaryName();
      case static_cast<int>(Call::Role::IsPresent):
         return number->m_pNumber->isPresent();
      case static_cast<int>(Call::Role::Photo):
         if (number->m_pNumber->contact())
            return number->m_pNumber->contact()->photo();
         cat = true;
         break;
   }
   return cat;
} //commonCallInfo

///Get category
QString CategorizedBookmarkModelPrivate::category(NumberTreeBackend* number) const
{
   QString cat = commonCallInfo(number).toString();
   if (cat.size())
      cat = cat[0].toUpper();
   return cat;
}

void CategorizedBookmarkModelPrivate::slotRequest(const QString& uri)
{
   Q_UNUSED(uri)
   qDebug() << "Presence Request" << uri << "denied";
   //DBus::PresenceManager::instance().answerServerRequest(uri,true); //FIXME turn on after 1.3.0
}



QVector<ContactMethod*> CategorizedBookmarkModelPrivate::serialisedToList(const QStringList& list)
{
   QVector<ContactMethod*> numbers;
   foreach(const QString& item,list) {
      ContactMethod* nb = PhoneDirectoryModel::instance()->fromHash(item);
      if (nb) {
         nb->setTracked(true);
         nb->setUid(item);
         numbers << nb;
      }
   }
   return numbers;
}

bool CategorizedBookmarkModelPrivate::displayFrequentlyUsed() const
{
   return true;
}

QVector<ContactMethod*> CategorizedBookmarkModelPrivate::bookmarkList() const
{
   return (q_ptr->collections().size() > 0) ? q_ptr->collections()[0]->items<ContactMethod>() : QVector<ContactMethod*>();
}

BookmarkTopLevelItem::BookmarkTopLevelItem(QString name) 
   : CategorizedCompositeNode(CategorizedCompositeNode::Type::TOP_LEVEL),m_Name(name),
      m_MostPopular(false),m_Row(-1)
{
}

bool CategorizedBookmarkModel::removeRows( int row, int count, const QModelIndex & parent)
{
   if (parent.isValid()) {
      const int parentRow = parent.row();
      beginRemoveRows(parent,row,row+count-1);
      for (int i=row;i<row+count;i++)
         d_ptr->m_lCategoryCounter[parent.row()]->m_lChildren.removeAt(i);
      endRemoveRows();
      if (!d_ptr->m_lCategoryCounter[parentRow]->m_lChildren.size()) {
         beginRemoveRows(QModelIndex(),parentRow,parentRow);
         d_ptr->m_hCategories.remove(d_ptr->m_hCategories.key(d_ptr->m_lCategoryCounter[parentRow]));
         d_ptr->m_lCategoryCounter.removeAt(parentRow);
         for (int i=0;i<d_ptr->m_lCategoryCounter.size();i++) {
            d_ptr->m_lCategoryCounter[i]->m_Row =i;
         }
         endRemoveRows();
      }
      return true;
   }
   return false;
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
   Q_UNUSED(idx)
//    ContactMethod* nb = getNumber(idx);
//    if (nb) {
//       removeRows(idx.row(),1,idx.parent());
//       removeBookmark(nb);
//       emit layoutAboutToBeChanged();
//       emit layoutChanged();
//    }
   collections()[0]->editor<ContactMethod>()->remove(getNumber(idx));
}

ContactMethod* CategorizedBookmarkModel::getNumber(const QModelIndex& idx)
{
   if (idx.isValid()) {
      if (idx.parent().isValid() && idx.parent().row() < d_ptr->m_lCategoryCounter.size()) {
         return d_ptr->m_lCategoryCounter[idx.parent().row()]->m_lChildren[idx.row()]->m_pNumber;
      }
   }
   return nullptr;
}

///Callback when an item change
void CategorizedBookmarkModelPrivate::slotIndexChanged(const QModelIndex& idx)
{
   emit q_ptr->dataChanged(idx,idx);
}


// bool CategorizedBookmarkModel::hasCollections() const
// {
//    return d_ptr->m_lBackends.size();
// }

// bool CategorizedBookmarkModel::hasEnabledCollections() const
// {
//    foreach(CollectionInterface* b, d_ptr->m_lBackends) {
//       if (b->isEnabled())
//          return true;
//    }
//    return false;
// }

// const QVector<CollectionInterface*> CategorizedBookmarkModel::collections() const
// {
//    return d_ptr->m_lBackends;
// }


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

// const QVector<CollectionInterface*> CategorizedBookmarkModel::enabledCollections() const
// {
//    return d_ptr->m_lBackends; //TODO filter them
// }

// CommonCollectionModel* CategorizedBookmarkModel::backendModel() const
// {
//    return nullptr; //TODO
// }

bool CategorizedBookmarkModel::clearAllCollections() const
{
   foreach (CollectionInterface* backend, collections()) {
      if (backend->supportedFeatures() & CollectionInterface::SupportedFeatures::ADD) {
         backend->clear();
      }
   }
   return true;
}

// bool CategorizedBookmarkModel::enableBackend(CollectionInterface* backend, bool enable)
// {
//    Q_UNUSED(backend)
//    Q_UNUSED(enable)
//    return false; //TODO
// }

// void CategorizedBookmarkModel::addBackend(CollectionInterface* backend, LoadOptions options)
// {
//    d_ptr->m_lBackends << backend;
//    connect(backend,SIGNAL(newBookmarkAdded(ContactMethod*)),this,SLOT(reloadCategories()));
//    if (options & LoadOptions::FORCE_ENABLED)
//       backend->load();
// }

void CategorizedBookmarkModel::collectionAddedCallback(CollectionInterface* backend)
{
   Q_UNUSED(backend)
   reloadCategories();
}

// QString CategorizedBookmarkModel::backendCategoryName() const
// {
//    return tr("Bookmarks");
// }

#include <categorizedbookmarkmodel.moc>
