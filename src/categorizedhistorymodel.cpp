/****************************************************************************
 *   Copyright (C) 2012-2017 Savoir-faire Linux                          *
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
#include "private/sortproxies.h"
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

/*****************************************************************************
 *                                                                           *
 *                             Private classes                               *
 *                                                                           *
 ****************************************************************************/

struct HistoryNode;

class CategorizedHistoryModelPrivate final : public QObject
{
   Q_OBJECT
public:
   CategorizedHistoryModelPrivate(CategorizedHistoryModel* parent);

   //Helpers
   HistoryNode* getCategory(const Call* call);

   //Attributes
   static CallMap m_sHistoryCalls;

   //Model categories
   QVector<HistoryNode*>        m_lCategoryCounter ;
   QHash<int,HistoryNode*>      m_hCategories      ;
   QHash<QString,HistoryNode*>  m_hCategoryByName  ;
   SortingCategory::ModelTuple* m_pSortedProxy {nullptr};
   int                          m_Role             ;
   QStringList                  m_lMimes           ;
   CategorizedHistoryModel::SortedProxy m_pProxies;

private:
   CategorizedHistoryModel* q_ptr;

public Q_SLOTS:
   void add(Call* call);
   void reloadCategories();
   void slotChanged(const QModelIndex& idx);
};

struct HistoryNode final
{
   enum class Type {
      CAT ,
      CALL,
   };

   ~HistoryNode() {
      foreach (HistoryNode* n, m_lChildren)
         delete n;
   }

   //Attributes
   HistoryNode* m_pParent {  nullptr  };
   int          m_Index   {    -1     };
   Call*        m_pCall   {  nullptr  };
   Type         m_Type    { Type::CAT };
   QString      m_Name                ;
   int          m_AbsIdx  { 0         };
   QVector<HistoryNode*> m_lChildren  ;
};

CallMap CategorizedHistoryModelPrivate::m_sHistoryCalls;

/*****************************************************************************
 *                                                                           *
 *                                 Constructor                               *
 *                                                                           *
 ****************************************************************************/

CategorizedHistoryModelPrivate::CategorizedHistoryModelPrivate(CategorizedHistoryModel* parent) : QObject(parent), q_ptr(parent),
m_Role(static_cast<int>(Call::Role::FuzzyDate)),m_pSortedProxy(nullptr)
{
}

///Constructor
CategorizedHistoryModel::CategorizedHistoryModel():QAbstractItemModel(QCoreApplication::instance()),CollectionManagerInterface<Call>(this),
d_ptr(new CategorizedHistoryModelPrivate(this))
{
   d_ptr->m_lMimes << RingMimes::PLAIN_TEXT << RingMimes::PHONENUMBER << RingMimes::HISTORYID;
} //initHistory

///Destructor
CategorizedHistoryModel::~CategorizedHistoryModel()
{

   while(d_ptr->m_lCategoryCounter.size()) {
      HistoryNode* item = d_ptr->m_lCategoryCounter[0];
      d_ptr->m_lCategoryCounter.remove(0);

      delete item;
   }

   if (d_ptr->m_pSortedProxy)
      delete d_ptr->m_pSortedProxy;
}

QHash<int,QByteArray> CategorizedHistoryModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles.insert(static_cast<int>(Call::Role::Name            ) ,QByteArray("name"          ));
      roles.insert(static_cast<int>(Call::Role::Number          ) ,QByteArray("number"        ));
      roles.insert(static_cast<int>(Call::Role::Direction       ) ,QByteArray("direction"     ));
      roles.insert(static_cast<int>(Call::Role::Date            ) ,QByteArray("date"          ));
      roles.insert(static_cast<int>(Call::Role::Length          ) ,QByteArray("length"        ));
      roles.insert(static_cast<int>(Call::Role::FormattedDate   ) ,QByteArray("formattedDate" ));
      roles.insert(static_cast<int>(Call::Role::HasAVRecording  ) ,QByteArray("hasAVRecording"));
      roles.insert(static_cast<int>(Call::Role::Historystate    ) ,QByteArray("historyState"  ));
      roles.insert(static_cast<int>(Call::Role::Filter          ) ,QByteArray("filter"        ));
      roles.insert(static_cast<int>(Call::Role::FuzzyDate       ) ,QByteArray("fuzzyDate"     ));
      roles.insert(static_cast<int>(Call::Role::IsBookmark      ) ,QByteArray("isBookmark"    ));
      roles.insert(static_cast<int>(Call::Role::Security        ) ,QByteArray("security"      ));
      roles.insert(static_cast<int>(Call::Role::Department      ) ,QByteArray("department"    ));
      roles.insert(static_cast<int>(Call::Role::Email           ) ,QByteArray("email"         ));
      roles.insert(static_cast<int>(Call::Role::Organisation    ) ,QByteArray("organisation"  ));
      roles.insert(static_cast<int>(Call::Role::Object          ) ,QByteArray("object"        ));
      roles.insert(static_cast<int>(Call::Role::Photo           ) ,QByteArray("photo"         ));
      roles.insert(static_cast<int>(Call::Role::State           ) ,QByteArray("state"         ));
      roles.insert(static_cast<int>(Call::Role::StartTime       ) ,QByteArray("startTime"     ));
      roles.insert(static_cast<int>(Call::Role::StopTime        ) ,QByteArray("stopTime"      ));
      roles.insert(static_cast<int>(Call::Role::DropState       ) ,QByteArray("dropState"     ));
      roles.insert(static_cast<int>(Call::Role::DTMFAnimState   ) ,QByteArray("dTMFAnimState" ));
      roles.insert(static_cast<int>(Call::Role::LastDTMFidx     ) ,QByteArray("lastDTMFidx"   ));
      roles.insert(static_cast<int>(Call::Role::IsAVRecording   ) ,QByteArray("isAVRecording" ));
      roles.insert(static_cast<int>(Call::Role::DateOnly        ) ,QByteArray("dateOnly"      ));
      roles.insert(static_cast<int>(Call::Role::DateTime        ) ,QByteArray("dateTime"      ));
   }
   return roles;
}

///Singleton
CategorizedHistoryModel& CategorizedHistoryModel::instance()
{
    static auto instance = new CategorizedHistoryModel;
    return *instance;
}


/*****************************************************************************
 *                                                                           *
 *                           History related code                            *
 *                                                                           *
 ****************************************************************************/
///Get the top level item based on a call
HistoryNode* CategorizedHistoryModelPrivate::getCategory(const Call* call)
{
   HistoryNode* category = nullptr;
   static QString name;
   int index = -1;
   const QVariant var = call->roleData(m_Role);

   if (m_Role == static_cast<int>(Call::Role::FuzzyDate)) {
      index    = var.toInt();
      name     = HistoryTimeCategoryModel::indexToName(index);
      category = m_hCategories[index];
   }
   else if (var.type() == QVariant::Int || var.type() == QVariant::UInt) {
      index    = var.toInt();
      name     = var.toString();
      category = m_hCategories[index];
   }
   else {
      name     = var.toString();
      category = m_hCategoryByName[name];
   }

   if (!category) {
      category = new HistoryNode();
      category->m_Name   = name;
      category->m_AbsIdx = index;
      category->m_Index  = m_lCategoryCounter.size();

      CategorizedHistoryModel::instance().beginInsertRows(QModelIndex(),m_lCategoryCounter.size(),m_lCategoryCounter.size());

      m_lCategoryCounter << category;

      if (index != -1)
         m_hCategories    [index] = category;

      m_hCategoryByName   [name ] = category;
      CategorizedHistoryModel::instance().endInsertRows();

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

   emit q_ptr->newHistoryCall(call);

   HistoryNode* tl = getCategory(call);

   const QModelIndex& parentIdx = q_ptr->index(tl->m_Index, 0);

   q_ptr->beginInsertRows(parentIdx,tl->m_lChildren.size(),tl->m_lChildren.size());

   HistoryNode* item = new HistoryNode();

   item->m_Type = HistoryNode::Type::CALL;
   item->m_pCall = call;
   item->m_pParent = tl;

   connect(call, &Call::changed, [this, item]() {
      const QModelIndex idx = q_ptr->createIndex(item->m_Index, 0, item);
      emit q_ptr->dataChanged(idx, idx);
   });

   const int size = tl->m_lChildren.size();
   item->m_Index = size;
   tl->m_lChildren << item;

   //Try to prevent startTimeStamp() collisions, it technically doesn't work as time_t are signed
   //we don't care
   m_sHistoryCalls[(call->startTimeStamp() << 10)+qrand()%1024] = call;
   q_ptr->endInsertRows();

   LastUsedNumberModel::instance().addCall(call);
   emit q_ptr->historyChanged();

   //When the categories goes from 0 items to many, its conceptual state change
   //therefore the clients may want to act on this, notify them
   if (!size) {
      const QModelIndex idx = q_ptr->index(item->m_Index,0);
      emit q_ptr->dataChanged(idx,idx);
   }
}

///Set if the history has a limit
void CategorizedHistoryModel::setHistoryLimited(bool isLimited)
{
   if (!isLimited)
      ConfigurationManager::instance().setHistoryLimit(0);
}

///Set if the history is enabled
void CategorizedHistoryModel::setHistoryEnabled(bool isEnabled)
{
   if (!isEnabled)
      ConfigurationManager::instance().setHistoryLimit(-1);
}

///Set the number of days before history items are discarded
void CategorizedHistoryModel::setHistoryLimit(int numberOfDays)
{
   ConfigurationManager::instance().setHistoryLimit(numberOfDays);
}

///Is history items are being deleted after "historyLimit()" days
bool CategorizedHistoryModel::isHistoryLimited() const
{
   return ConfigurationManager::instance().getHistoryLimit() > 0;
}

///Number of days before items are discarded (0 = never)
int CategorizedHistoryModel::historyLimit() const
{
   return ConfigurationManager::instance().getHistoryLimit();
}

///Get if the history is enabled
bool CategorizedHistoryModel::isHistoryEnabled() const
{
   return ConfigurationManager::instance().getHistoryLimit() >= 0;
}


/*****************************************************************************
 *                                                                           *
 *                              Model related                                *
 *                                                                           *
 ****************************************************************************/

void CategorizedHistoryModelPrivate::reloadCategories()
{
   emit q_ptr->layoutAboutToBeChanged();
   m_hCategories.clear();
   m_hCategoryByName.clear();
   q_ptr->beginRemoveRows(QModelIndex(),0,m_lCategoryCounter.size()-1);
   foreach(HistoryNode* item, m_lCategoryCounter) {
      delete item;
   }
   q_ptr->endRemoveRows();
   m_lCategoryCounter.clear();

   foreach(Call* call, m_sHistoryCalls) {
      HistoryNode* category = getCategory(call);
      if (category) {
         HistoryNode* item = new HistoryNode();
         item->m_Type = HistoryNode::Type::CALL;
         item->m_pCall = call;
         item->m_Index = category->m_lChildren.size();

         connect(call, &Call::changed, [this, item]() {
            const QModelIndex idx = q_ptr->createIndex(item->m_Index, 0, item);
            emit q_ptr->dataChanged(idx, idx);
         });

         item->m_pParent = category;
         q_ptr->beginInsertRows(q_ptr->index(category->m_Index,0), item->m_Index, item->m_Index); {
            category->m_lChildren << item;
         } q_ptr->endInsertRows();
      }
      else
         qDebug() << "ERROR count";
   }

   emit q_ptr->layoutChanged();
   emit q_ptr->dataChanged(q_ptr->index(0,0),q_ptr->index(q_ptr->rowCount()-1,0));
}

void CategorizedHistoryModelPrivate::slotChanged(const QModelIndex& idx)
{
   emit q_ptr->dataChanged(idx,idx);
}

bool CategorizedHistoryModel::setData( const QModelIndex& idx, const QVariant &value, int role)
{
   Q_UNUSED(idx)
   Q_UNUSED(value)
   Q_UNUSED(role)

   return false;
}

QVariant CategorizedHistoryModel::data( const QModelIndex& idx, int role) const
{
   if (!idx.isValid())
      return QVariant();

   HistoryNode* modelItem = static_cast<HistoryNode*>(idx.internalPointer());

   switch (modelItem->m_Type) {
      case HistoryNode::Type::CAT:
         switch (role) {
            case Qt::DisplayRole:
               return modelItem->m_Name;
            case static_cast<int>(Call::Role::FuzzyDate):
            case static_cast<int>(Call::Role::Date):
            case static_cast<int>(Call::Role::CallCount):
               return modelItem->m_AbsIdx;
            default:
               break;
         }
         break;
      case HistoryNode::Type::CALL:
         switch (role) {
            // Dates need to be sorted from newest to oldest
            case static_cast<int>(Call::Role::FuzzyDate):
            case static_cast<int>(Call::Role::Date):
               return -modelItem->m_pCall->roleData(static_cast<int>(Call::Role::Date)).toInt();
            default:
               return modelItem->m_pCall->roleData(role);
         }
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
      HistoryNode* node = static_cast<HistoryNode*>(parentIdx.internalPointer());
      switch(node->m_Type) {
         case HistoryNode::Type::CAT:
            return node->m_lChildren.size();
         case HistoryNode::Type::CALL:
            return 0;
      };
   }
   return 0;
}

Qt::ItemFlags CategorizedHistoryModel::flags( const QModelIndex& idx ) const
{
   if (!idx.isValid())
      return Qt::NoItemFlags;

   HistoryNode* node = static_cast<HistoryNode*>(idx.internalPointer());
   const bool hasParent = node->m_Type != HistoryNode::Type::CAT;
   const bool isEnabled = node->m_Type == HistoryNode::Type::CALL && node->m_pCall->isActive();

   return (isEnabled?Qt::ItemIsEnabled:Qt::NoItemFlags) | Qt::ItemIsSelectable | (hasParent?Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled:Qt::ItemIsEnabled);
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

   HistoryNode* modelItem = static_cast<HistoryNode*>(idx.internalPointer());

   if (modelItem->m_Type == HistoryNode::Type::CALL) {
      HistoryNode* tli = modelItem->m_pParent;

      return createIndex(tli->m_Index, idx.column(), tli);
   }

   return QModelIndex();
}

QModelIndex CategorizedHistoryModel::index( int row, int column, const QModelIndex& parentIdx) const
{
   if (!parentIdx.isValid() && row >= 0 && row < d_ptr->m_lCategoryCounter.size())
      return createIndex(row,column,(void*)d_ptr->m_lCategoryCounter[row]);

   HistoryNode* node = static_cast<HistoryNode*>(parentIdx.internalPointer());

   switch(node->m_Type) {
      case HistoryNode::Type::CAT:
         if (row < node->m_lChildren.size())
            return createIndex(row,column, node->m_lChildren[row]);
         break;
      case HistoryNode::Type::CALL:
         break;
   };

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
         const HistoryNode* node = static_cast<HistoryNode*>(idx.internalPointer());
         const QString      text = data(idx, static_cast<int>(Call::Role::Number)).toString();
         const Call*        call = node->m_pCall;

         //TODO use RingMimes::payload once the multi selection is investigated
         mimeData2->setData(RingMimes::PLAIN_TEXT , text.toUtf8());

         mimeData2->setData(RingMimes::PHONENUMBER, call->peerContactMethod()->toHash().toUtf8());

         if (node->m_Type == HistoryNode::Type::CALL)
            mimeData2->setData(RingMimes::HISTORYID  , call->dringId().toUtf8());

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
      QByteArray encodedCallId = mime->data( RingMimes::CALLID );
      Call*      call          = CallModel::instance().fromMime(encodedCallId);

      if (call) {
         const QModelIndex& idx = index(row,column,parentIdx);

         if (idx.isValid()) {
            const Call* target = static_cast<HistoryNode*>(idx.internalPointer())->m_pCall;

            if (target) {
               CallModel::instance().transfer(call,target->peerContactMethod());
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
    foreach (CollectionInterface* backend, collections(CollectionInterface::SupportedFeatures::CLEAR)) {
        backend->clear();
    }
    return true;
}

///Delete all history and clear model
void CategorizedHistoryModel::clear()
{
    beginResetModel();
    clearAllCollections();
    endResetModel();
}

bool CategorizedHistoryModel::addItemCallback(const Call* item)
{
   d_ptr->add(const_cast<Call*>(item));
   return true;
}

bool CategorizedHistoryModel::removeItemCallback(const Call* item)
{
   emit const_cast<Call*>(item)->changed();
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

QSortFilterProxyModel* CategorizedHistoryModel::SortedProxy::model() const
{
   if (!CategorizedHistoryModel::instance().d_ptr->m_pSortedProxy)
      CategorizedHistoryModel::instance().d_ptr->m_pSortedProxy = SortingCategory::getHistoryProxy();

   return CategorizedHistoryModel::instance().d_ptr->m_pSortedProxy->model;
}

QAbstractItemModel* CategorizedHistoryModel::SortedProxy::categoryModel() const
{
   if (!CategorizedHistoryModel::instance().d_ptr->m_pSortedProxy)
      CategorizedHistoryModel::instance().d_ptr->m_pSortedProxy = SortingCategory::getHistoryProxy();

   return CategorizedHistoryModel::instance().d_ptr->m_pSortedProxy->categories;
}

QItemSelectionModel* CategorizedHistoryModel::SortedProxy::categorySelectionModel() const
{
   if (!CategorizedHistoryModel::instance().d_ptr->m_pSortedProxy)
      CategorizedHistoryModel::instance().d_ptr->m_pSortedProxy = SortingCategory::getHistoryProxy();

   return CategorizedHistoryModel::instance().d_ptr->m_pSortedProxy->selectionModel;
}

CategorizedHistoryModel::SortedProxy& CategorizedHistoryModel::SortedProxy::instance()
{
   return CategorizedHistoryModel::instance().d_ptr->m_pProxies;
}

#include <categorizedhistorymodel.moc>
