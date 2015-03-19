/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
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
#include "categorizedhistorymodel.h"

//C include
#include <time.h>

//Qt include
#include <QMimeData>
#include <QCoreApplication>

//Ring lib
#include "mime.h"
#include "dbus/callmanager.h"
#include "dbus/configurationmanager.h"
#include "call.h"
#include "person.h"
#include "contactmethod.h"
#include "callmodel.h"
#include "collectioneditor.h"
#include "historytimecategorymodel.h"
#include "lastusednumbermodel.h"
#include "collectioninterface.h"
#include "delegates/itemmodelstateserializationdelegate.h"

/*****************************************************************************
 *                                                                           *
 *                             Private classes                               *
 *                                                                           *
 ****************************************************************************/

class HistoryTopLevelItem;

class CategorizedHistoryModelPrivate : public QObject
{
   Q_OBJECT
public:
   CategorizedHistoryModelPrivate(CategorizedHistoryModel* parent);

   //Model
   class HistoryItem : public CategorizedCompositeNode {
   public:
      explicit HistoryItem(Call* call);
      virtual ~HistoryItem();
      virtual QObject* getSelf() const;
      Call* call() const;
      int m_Index;
      HistoryTopLevelItem* m_pParent;
      HistoryItemNode* m_pNode;
   private:
      Call* m_pCall;
   };

   //Helpers
   HistoryTopLevelItem* getCategory(const Call* call);

   //Attributes
   static CallMap m_sHistoryCalls;

   //Model categories
   QVector<HistoryTopLevelItem*>       m_lCategoryCounter ;
   QHash<int,HistoryTopLevelItem*>     m_hCategories      ;
   QHash<QString,HistoryTopLevelItem*> m_hCategoryByName  ;
   int                          m_Role             ;
   QStringList                  m_lMimes           ;

private:
   CategorizedHistoryModel* q_ptr;

public Q_SLOTS:
   void add(Call* call);
   void reloadCategories();
   void slotChanged(const QModelIndex& idx);
};

class HistoryTopLevelItem : public CategorizedCompositeNode,public QObject {
   friend class CategorizedHistoryModel;
   friend class CategorizedHistoryModelPrivate;
public:
   virtual QObject* getSelf() const;
   virtual ~HistoryTopLevelItem();
   int m_Index;
   int m_AbsoluteIndex;
   QVector<CategorizedHistoryModelPrivate::HistoryItem*> m_lChildren;
private:
   explicit HistoryTopLevelItem(const QString& name, int index);
   QString m_NameStr;
   int modelRow;
};

class HistoryItemNode : public QObject //TODO remove this once Qt4 support is dropped
{
   Q_OBJECT
public:
   HistoryItemNode(CategorizedHistoryModel* m, Call* c, CategorizedHistoryModelPrivate::HistoryItem* backend);
   Call* m_pCall;
private:
   CategorizedHistoryModelPrivate::HistoryItem* m_pBackend;
   CategorizedHistoryModel* m_pModel;
private Q_SLOTS:
   void slotNumberChanged();
Q_SIGNALS:
   void changed(const QModelIndex& idx);
};

HistoryItemNode::HistoryItemNode(CategorizedHistoryModel* m, Call* c, CategorizedHistoryModelPrivate::HistoryItem* backend) :
m_pCall(c),m_pBackend(backend),m_pModel(m){
   connect(c,SIGNAL(changed()),this,SLOT(slotNumberChanged()));
}

void HistoryItemNode::slotNumberChanged()
{
   emit changed(m_pModel->index(m_pBackend->m_Index,0,m_pModel->index(m_pBackend->m_pParent->m_AbsoluteIndex,0)));
}

CategorizedHistoryModel* CategorizedHistoryModel::m_spInstance    = nullptr;
CallMap       CategorizedHistoryModelPrivate::m_sHistoryCalls          ;

HistoryTopLevelItem::HistoryTopLevelItem(const QString& name, int index) : 
   CategorizedCompositeNode(CategorizedCompositeNode::Type::TOP_LEVEL),QObject(nullptr),m_Index(index),m_NameStr(name),
   m_AbsoluteIndex(-1),modelRow(-1)
{}

HistoryTopLevelItem::~HistoryTopLevelItem() {
   const int idx = CategorizedHistoryModel::m_spInstance->d_ptr->m_lCategoryCounter.indexOf(this);
   if (idx != -1)
      CategorizedHistoryModel::m_spInstance->d_ptr->m_lCategoryCounter.remove(idx);
   while(m_lChildren.size()) {
      CategorizedHistoryModelPrivate::HistoryItem* item = m_lChildren[0];
      m_lChildren.remove(0);
      delete item;
   }
}

QObject* HistoryTopLevelItem::getSelf() const
{
   return const_cast<HistoryTopLevelItem*>(this);
}

CategorizedHistoryModelPrivate::HistoryItem::HistoryItem(Call* call) : CategorizedCompositeNode(CategorizedCompositeNode::Type::CALL),m_pCall(call),
m_Index(0),m_pParent(nullptr),m_pNode(nullptr)
{
   
}

CategorizedHistoryModelPrivate::HistoryItem::~HistoryItem()
{
   delete m_pNode;
}


QObject* CategorizedHistoryModelPrivate::HistoryItem::getSelf() const
{
   return const_cast<Call*>(m_pCall);
}

Call* CategorizedHistoryModelPrivate::HistoryItem::call() const
{
   return m_pCall;
}


/*****************************************************************************
 *                                                                           *
 *                                 Constructor                               *
 *                                                                           *
 ****************************************************************************/

CategorizedHistoryModelPrivate::CategorizedHistoryModelPrivate(CategorizedHistoryModel* parent) : QObject(parent), q_ptr(parent),
m_Role(static_cast<int>(Call::Role::FuzzyDate))
{
}

///Constructor
CategorizedHistoryModel::CategorizedHistoryModel():QAbstractItemModel(QCoreApplication::instance()),CollectionManagerInterface<Call>(this),
d_ptr(new CategorizedHistoryModelPrivate(this))
{
   m_spInstance  = this;
   d_ptr->m_lMimes << RingMimes::PLAIN_TEXT << RingMimes::PHONENUMBER << RingMimes::HISTORYID;
} //initHistory

///Destructor
CategorizedHistoryModel::~CategorizedHistoryModel()
{
   for (int i=0; i<d_ptr->m_lCategoryCounter.size();i++) {
      delete d_ptr->m_lCategoryCounter[i];
   }
   while(d_ptr->m_lCategoryCounter.size()) {
      HistoryTopLevelItem* item = d_ptr->m_lCategoryCounter[0];
      d_ptr->m_lCategoryCounter.remove(0);

      delete item;
   }
   m_spInstance = nullptr;
}

QHash<int,QByteArray> CategorizedHistoryModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles.insert(static_cast<int>(Call::Role::Name          ) ,QByteArray("name"          ));
      roles.insert(static_cast<int>(Call::Role::Number        ) ,QByteArray("number"        ));
      roles.insert(static_cast<int>(Call::Role::Direction     ) ,QByteArray("direction"     ));
      roles.insert(static_cast<int>(Call::Role::Date          ) ,QByteArray("date"          ));
      roles.insert(static_cast<int>(Call::Role::Length        ) ,QByteArray("length"        ));
      roles.insert(static_cast<int>(Call::Role::FormattedDate ) ,QByteArray("formattedDate" ));
      roles.insert(static_cast<int>(Call::Role::HasRecording  ) ,QByteArray("hasRecording"  ));
      roles.insert(static_cast<int>(Call::Role::Historystate  ) ,QByteArray("historyState"  ));
      roles.insert(static_cast<int>(Call::Role::Filter        ) ,QByteArray("filter"        ));
      roles.insert(static_cast<int>(Call::Role::FuzzyDate     ) ,QByteArray("fuzzyDate"     ));
      roles.insert(static_cast<int>(Call::Role::IsBookmark    ) ,QByteArray("isBookmark"    ));
      roles.insert(static_cast<int>(Call::Role::Security      ) ,QByteArray("security"      ));
      roles.insert(static_cast<int>(Call::Role::Department    ) ,QByteArray("department"    ));
      roles.insert(static_cast<int>(Call::Role::Email         ) ,QByteArray("email"         ));
      roles.insert(static_cast<int>(Call::Role::Organisation  ) ,QByteArray("organisation"  ));
      roles.insert(static_cast<int>(Call::Role::Object        ) ,QByteArray("object"        ));
      roles.insert(static_cast<int>(Call::Role::Photo         ) ,QByteArray("photo"         ));
      roles.insert(static_cast<int>(Call::Role::State         ) ,QByteArray("state"         ));
      roles.insert(static_cast<int>(Call::Role::StartTime     ) ,QByteArray("startTime"     ));
      roles.insert(static_cast<int>(Call::Role::StopTime      ) ,QByteArray("stopTime"      ));
      roles.insert(static_cast<int>(Call::Role::DropState     ) ,QByteArray("dropState"     ));
      roles.insert(static_cast<int>(Call::Role::DTMFAnimState ) ,QByteArray("dTMFAnimState" ));
      roles.insert(static_cast<int>(Call::Role::LastDTMFidx   ) ,QByteArray("lastDTMFidx"   ));
      roles.insert(static_cast<int>(Call::Role::IsRecording   ) ,QByteArray("isRecording"   ));
   }
   return roles;
}

///Singleton
CategorizedHistoryModel* CategorizedHistoryModel::instance()
{
   if (!m_spInstance)
      m_spInstance = new CategorizedHistoryModel();
   return m_spInstance;
}


/*****************************************************************************
 *                                                                           *
 *                           History related code                            *
 *                                                                           *
 ****************************************************************************/
///Get the top level item based on a call
HistoryTopLevelItem* CategorizedHistoryModelPrivate::getCategory(const Call* call)
{
   HistoryTopLevelItem* category = nullptr;
   static QString name;
   int index = -1;
   if (m_Role == static_cast<int>(Call::Role::FuzzyDate)) {
      index = call->roleData(Call::Role::FuzzyDate).toInt();
      name = HistoryTimeCategoryModel::indexToName(index);
      category = m_hCategories[index];
   }
   else {
      name = call->roleData(m_Role).toString();
      category = m_hCategoryByName[name];
   }
   if (!category) {
      category = new HistoryTopLevelItem(name,index);
      category->modelRow = m_lCategoryCounter.size();
      //emit layoutAboutToBeChanged(); //Not necessary
      CategorizedHistoryModel::instance()->beginInsertRows(QModelIndex(),m_lCategoryCounter.size(),m_lCategoryCounter.size());
      category->m_AbsoluteIndex = m_lCategoryCounter.size();
      m_lCategoryCounter << category;
      m_hCategories    [index] = category;
      m_hCategoryByName[name ] = category;
      CategorizedHistoryModel::instance()->endInsertRows();
      //emit layoutChanged();
   }
   return category;
}


const CallMap CategorizedHistoryModel::getHistoryCalls() const
{
   return d_ptr->m_sHistoryCalls;
}

///Add to history
void CategorizedHistoryModelPrivate::add(Call* call)
{
   if (!call || call->lifeCycleState() != Call::LifeCycleState::FINISHED || !call->startTimeStamp()) {
      return;
   }

//    if (!m_HavePersonModel && call->contactBackend()) {
//       connect(((QObject*)call->contactBackend()),SIGNAL(collectionChanged()),this,SLOT(reloadCategories()));
//       m_HavePersonModel = true;
//    }//TODO implement reordering

   emit q_ptr->newHistoryCall(call);
   emit q_ptr->layoutAboutToBeChanged();
   HistoryTopLevelItem* tl = getCategory(call);
   const QModelIndex& parentIdx = q_ptr->index(tl->modelRow,0);
   q_ptr->beginInsertRows(parentIdx,tl->m_lChildren.size(),tl->m_lChildren.size());
   CategorizedHistoryModelPrivate::HistoryItem* item = new CategorizedHistoryModelPrivate::HistoryItem(call);
   item->m_pParent = tl;
   item->m_pNode = new HistoryItemNode(q_ptr,call,item);
   connect(item->m_pNode,SIGNAL(changed(QModelIndex)),this,SLOT(slotChanged(QModelIndex)));
   item->m_Index = tl->m_lChildren.size();
   tl->m_lChildren << item;

   //Try to prevent startTimeStamp() collisions, it technically doesn't work as time_t are signed
   //we don't care
   m_sHistoryCalls[(call->startTimeStamp() << 10)+qrand()%1024] = call;
   q_ptr->endInsertRows();
   emit q_ptr->layoutChanged();
   LastUsedNumberModel::instance()->addCall(call);
   emit q_ptr->historyChanged();

   /*
   // Loop until it find a compatible backend
   //HACK only support a single active history backend
   if (!call->collection()) {
      foreach (CollectionInterface* backend, q_ptr->collections(CollectionInterface::ADD)) {
         if (backend->editor<Call>()->addNew(call)) {
            call->setCollection(backend);
            break;
         }
      }
   }*/
}

///Set if the history has a limit
void CategorizedHistoryModel::setHistoryLimited(bool isLimited)
{
   if (!isLimited)
      DBus::ConfigurationManager::instance().setHistoryLimit(0);
}

///Set the number of days before history items are discarded
void CategorizedHistoryModel::setHistoryLimit(int numberOfDays)
{
   DBus::ConfigurationManager::instance().setHistoryLimit(numberOfDays);
}

///Is history items are being deleted after "historyLimit()" days
bool CategorizedHistoryModel::isHistoryLimited() const
{
   return DBus::ConfigurationManager::instance().getHistoryLimit() != 0;
}

///Number of days before items are discarded (0 = never)
int CategorizedHistoryModel::historyLimit() const
{
   return DBus::ConfigurationManager::instance().getHistoryLimit();
}


/*****************************************************************************
 *                                                                           *
 *                              Model related                                *
 *                                                                           *
 ****************************************************************************/

void CategorizedHistoryModelPrivate::reloadCategories()
{
   q_ptr->beginResetModel();
   m_hCategories.clear();
   m_hCategoryByName.clear();
   foreach(HistoryTopLevelItem* item, m_lCategoryCounter) {
      delete item;
   }
   m_lCategoryCounter.clear();
   foreach(Call* call, m_sHistoryCalls) {
      HistoryTopLevelItem* category = getCategory(call);
      if (category) {
         HistoryItem* item = new HistoryItem(call);
         item->m_Index = category->m_lChildren.size();
         item->m_pNode = new HistoryItemNode(q_ptr,call,item);
         connect(item->m_pNode,SIGNAL(changed(QModelIndex)),this,SLOT(slotChanged(QModelIndex)));
         item->m_pParent = category;
         category->m_lChildren << item;
      }
      else
         qDebug() << "ERROR count";
   }
   q_ptr->endResetModel();
   emit q_ptr->layoutAboutToBeChanged();
   emit q_ptr->layoutChanged();
   emit q_ptr->dataChanged(q_ptr->index(0,0),q_ptr->index(q_ptr->rowCount()-1,0));
}

void CategorizedHistoryModelPrivate::slotChanged(const QModelIndex& idx)
{
   emit q_ptr->dataChanged(idx,idx);
}

bool CategorizedHistoryModel::setData( const QModelIndex& idx, const QVariant &value, int role)
{
   if (idx.isValid() && idx.parent().isValid()) {
      CategorizedCompositeNode* modelItem = (CategorizedCompositeNode*)idx.internalPointer();
      if (role == static_cast<int>(Call::Role::DropState)) {
         modelItem->setDropState(value.toInt());
         emit dataChanged(idx, idx);
      }
   }
   return false;
}

QVariant CategorizedHistoryModel::data( const QModelIndex& idx, int role) const
{
   if (!idx.isValid())
      return QVariant();

   CategorizedCompositeNode* modelItem = static_cast<CategorizedCompositeNode*>(idx.internalPointer());
   switch (modelItem->type()) {
      case CategorizedCompositeNode::Type::TOP_LEVEL:
      switch (role) {
         case Qt::DisplayRole:
            return static_cast<HistoryTopLevelItem*>(modelItem)->m_NameStr;
         case static_cast<int>(Call::Role::FuzzyDate):
         case static_cast<int>(Call::Role::Date):
            return d_ptr->m_lCategoryCounter.size() - static_cast<HistoryTopLevelItem*>(modelItem)->m_Index;
         default:
            break;
      }
      break;
   case CategorizedCompositeNode::Type::CALL:
      if (role == static_cast<int>(Call::Role::DropState))
         return QVariant(modelItem->dropState());
      else {
         const int parRow = idx.parent().row();
         const HistoryTopLevelItem* parTli = d_ptr->m_lCategoryCounter[parRow];
         if (d_ptr->m_lCategoryCounter.size() > parRow && parRow >= 0 && parTli && parTli->m_lChildren.size() > idx.row())
            return parTli->m_lChildren[idx.row()]->call()->roleData((Call::Role)role);
      }
      break;
   case CategorizedCompositeNode::Type::NUMBER:
   case CategorizedCompositeNode::Type::BOOKMARK:
   case CategorizedCompositeNode::Type::CONTACT:
   default:
      break;
   };
   return QVariant();
}

QVariant CategorizedHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return QVariant(tr("History"));
   if (role == Qt::InitialSortOrderRole)
      return QVariant(Qt::DescendingOrder);
   return QVariant();
}

int CategorizedHistoryModel::rowCount( const QModelIndex& parentIdx ) const
{
   if ((!parentIdx.isValid()) || (!parentIdx.internalPointer())) {
      return d_ptr->m_lCategoryCounter.size();
   }
   else {
      CategorizedCompositeNode* node = static_cast<CategorizedCompositeNode*>(parentIdx.internalPointer());
      switch(node->type()) {
         case CategorizedCompositeNode::Type::TOP_LEVEL:
            return ((HistoryTopLevelItem*)node)->m_lChildren.size();
         case CategorizedCompositeNode::Type::CALL:
         case CategorizedCompositeNode::Type::NUMBER:
         case CategorizedCompositeNode::Type::BOOKMARK:
         case CategorizedCompositeNode::Type::CONTACT:
         default:
            return 0;
      };
   }
}

Qt::ItemFlags CategorizedHistoryModel::flags( const QModelIndex& idx ) const
{
   if (!idx.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | (idx.parent().isValid()?Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled:Qt::ItemIsEnabled);
}

int CategorizedHistoryModel::columnCount ( const QModelIndex& parentIdx) const
{
   Q_UNUSED(parentIdx)
   return 1;
}

QModelIndex CategorizedHistoryModel::parent( const QModelIndex& idx) const
{
   if (!idx.isValid() || !idx.internalPointer()) {
      return QModelIndex();
   }
   CategorizedCompositeNode* modelItem = static_cast<CategorizedCompositeNode*>(idx.internalPointer());
   if (modelItem && modelItem->type() == CategorizedCompositeNode::Type::CALL) {
      const Call* call = (Call*)((CategorizedCompositeNode*)(idx.internalPointer()))->getSelf();
      //TODO this is called way to often to use getCategory, make sure getSelf return the node and cache this
      HistoryTopLevelItem* tli = d_ptr->getCategory(call);
      if (tli)
         return CategorizedHistoryModel::index(tli->modelRow,0);
   }
   return QModelIndex();
}

QModelIndex CategorizedHistoryModel::index( int row, int column, const QModelIndex& parentIdx) const
{
   if (!parentIdx.isValid()) {
      if (row >= 0 && d_ptr->m_lCategoryCounter.size() > row) {
         return createIndex(row,column,(void*)d_ptr->m_lCategoryCounter[row]);
      }
   }
   else {
      CategorizedCompositeNode* node = static_cast<CategorizedCompositeNode*>(parentIdx.internalPointer());
      switch(node->type()) {
         case CategorizedCompositeNode::Type::TOP_LEVEL:
            if (((HistoryTopLevelItem*)node)->m_lChildren.size() > row)
               return createIndex(row,column,(void*)static_cast<CategorizedCompositeNode*>(((HistoryTopLevelItem*)node)->m_lChildren[row]));
            break;
         case CategorizedCompositeNode::Type::CALL:
         case CategorizedCompositeNode::Type::NUMBER:
         case CategorizedCompositeNode::Type::BOOKMARK:
         case CategorizedCompositeNode::Type::CONTACT:
            break;
      };
   }
   return QModelIndex();
}

///Called when dynamically adding calls, otherwise the proxy filter will segfault
bool CategorizedHistoryModel::insertRows( int row, int count, const QModelIndex & parent)
{
   if (parent.isValid()) {
      beginInsertRows(parent,row,row+count-1);
      endInsertRows();
      return true;
   }
   return false;
}

QStringList CategorizedHistoryModel::mimeTypes() const
{
   return d_ptr->m_lMimes;
}

QMimeData* CategorizedHistoryModel::mimeData(const QModelIndexList &indexes) const
{
   QMimeData *mimeData2 = new QMimeData();
   foreach (const QModelIndex &idx, indexes) {
      if (idx.isValid()) {
         const QString text = data(idx, static_cast<int>(Call::Role::Number)).toString();
         mimeData2->setData(RingMimes::PLAIN_TEXT , text.toUtf8());
         const Call* call = (Call*)((CategorizedCompositeNode*)(idx.internalPointer()))->getSelf();
         mimeData2->setData(RingMimes::PHONENUMBER, call->peerContactMethod()->toHash().toUtf8());
         CategorizedCompositeNode* node = static_cast<CategorizedCompositeNode*>(idx.internalPointer());
         if (node->type() == CategorizedCompositeNode::Type::CALL)
            mimeData2->setData(RingMimes::HISTORYID  , static_cast<Call*>(node->getSelf())->dringId().toUtf8());
         return mimeData2;
      }
   }
   return mimeData2;
}

bool CategorizedHistoryModel::dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column, const QModelIndex &parentIdx)
{
   Q_UNUSED(row)
   Q_UNUSED(column)
   Q_UNUSED(action)
   setData(parentIdx,-1,static_cast<int>(Call::Role::DropState));
   QByteArray encodedContactMethod = mime->data( RingMimes::PHONENUMBER );
   QByteArray encodedPerson     = mime->data( RingMimes::CONTACT     );

   if (parentIdx.isValid() && mime->hasFormat( RingMimes::CALLID)) {
      QByteArray encodedCallId      = mime->data( RingMimes::CALLID      );
      Call* call = CallModel::instance()->fromMime(encodedCallId);
      if (call) {
         const QModelIndex& idx = index(row,column,parentIdx);
         if (idx.isValid()) {
            const Call* target = (Call*)((CategorizedCompositeNode*)(idx.internalPointer()))->getSelf();
            if (target) {
               CallModel::instance()->transfer(call,target->peerContactMethod());
               return true;
            }
         }
      }
   }
   return false;
}

void CategorizedHistoryModel::collectionAddedCallback(CollectionInterface* backend)
{
   Q_UNUSED(backend)
}

///Call all collections that support clearing
bool CategorizedHistoryModel::clearAllCollections() const
{
   foreach (CollectionInterface* backend, collections()) { //TODO use the filter API
      if (backend->supportedFeatures() & CollectionInterface::CLEAR) {
         backend->clear();
      }
   }
   return true;
}

bool CategorizedHistoryModel::addItemCallback(const Call* item)
{
   d_ptr->add(const_cast<Call*>(item));
   return true;
}

bool CategorizedHistoryModel::removeItemCallback(const Call* item)
{
   Q_UNUSED(item)
   return false;
}

///Return valid payload types
int CategorizedHistoryModel::acceptedPayloadTypes() const
{
   return CallModel::DropPayloadType::CALL;
}

void CategorizedHistoryModel::setCategoryRole(int role)
{
   if (d_ptr->m_Role != role) {
      d_ptr->m_Role = role;
      d_ptr->reloadCategories();
   }
}

#include <categorizedhistorymodel.moc>
