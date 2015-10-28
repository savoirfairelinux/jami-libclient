/************************************************************************************
 *   Copyright (C) 2015 by Savoir-faire Linux                                       *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>         *
 *            Alexandre Lision <alexandre.lision@savoirfairelinux.com>              *
 *            Stepan Salenikovich <stepan.salenikovich@savoirfairelinux.com>        *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/
#include "recentmodel.h"

//std
#include <algorithm>

//Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QSortFilterProxyModel>

//Ring
#include <call.h>
#include <person.h>
#include <personmodel.h>
#include <contactmethod.h>
#include <phonedirectorymodel.h>
#include <callmodel.h>
#include <categorizedhistorymodel.h>
#include <media/recordingmodel.h>
#include <media/textrecording.h>

struct CallGroup
{
   QVector<Call*>  m_lCalls   ;
   Call::Direction m_Direction;
   bool            m_Missed   ;
   time_t          m_LastUsed ;
};

struct RecentViewNode
{
   //Types
   enum class Type {
      PERSON            ,
      CONTACT_METHOD    ,
      CALL              ,
      CALL_GROUP        ,
      TEXT_MESSAGE      ,
      TEXT_MESSAGE_GROUP,
   };

   //Constructor
   explicit RecentViewNode();
   virtual ~RecentViewNode();

   //Attributes
   long int               m_Index    ;
   Type                   m_Type     ;
   RecentViewNode*        m_pParent  ;
   QList<RecentViewNode*> m_lChildren;
   union {
      const Person*  m_pPerson       ;
      ContactMethod* m_pContactMethod;
      Call*          m_pCall         ;
      CallGroup*     m_pCallGroup    ;
      //ImConversationIterator; //TODO
      //ImConversationIterator;
   } m_uContent;

   //Helpers
   inline time_t   lastUsed (          ) const;
   RecentViewNode* childNode(Call *call) const;
};

class PeopleProxy : public QSortFilterProxyModel
{
   Q_OBJECT
public:
    PeopleProxy(RecentModel* source_model);

    virtual QVariant data(const QModelIndex& index, int role) const override;
protected:
    virtual bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const override;

public Q_SLOTS:
    void slotRowsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd,
                                const QModelIndex & destinationParent, int destinationRow);
    void slotRowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd,
                       const QModelIndex & destinationParent, int destinationRow);
};

class RecentModelPrivate : public QObject
{
    Q_OBJECT
public:
   RecentModelPrivate(RecentModel* p);

   /*
   * m_lTopLevelReverted hold the elements in the reverse order of
   * QAbstractItemModel::index. This cause most of the energy to be
   * in the bottom half of the vector, preventing std::move every time
   * someone is contacted
   */
   QList<RecentViewNode*>                m_lTopLevelReverted;
   QHash<const Person*,RecentViewNode*>  m_hPersonsToNodes  ;
   QHash<ContactMethod*,RecentViewNode*> m_hCMsToNodes      ;

   QItemSelectionModel*                  m_pSelectionModel  ;

   //Helper
   void            insertNode    (RecentViewNode* n, time_t t, bool isNew);
   void            removeNode    (RecentViewNode* n                      );
   RecentViewNode* parentNode    (Call *call                             )  const;
   void            insertCallNode(RecentViewNode *parent, Call* call     );
   void            removeCall    (Call *call                             );
   void            updateCall    (Call *call                             );
   void            selectNode    (RecentViewNode* node                   )  const;

private:
   RecentModel* q_ptr;

public Q_SLOTS:
   void slotLastUsedTimeChanged(const Person*  p , time_t t              );
   void slotPersonAdded        (const Person*  p                         );
   void slotLastUsedChanged    (ContactMethod* cm, time_t t              );
   void slotContactChanged     (ContactMethod* cm, Person* np, Person* op);
   void slotCallAdded          (Call* call       , Call* parent          );
   void slotChanged            (                                         );
};

RecentModelPrivate::RecentModelPrivate(RecentModel* p) : q_ptr(p)
{
    m_pSelectionModel = nullptr;
}

QItemSelectionModel* RecentModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<RecentModel*>(this));
   }
   return d_ptr->m_pSelectionModel;
}

void RecentModelPrivate::selectNode(RecentViewNode* node) const
{
   const auto idx = q_ptr->createIndex(node->m_Index, 0, node->m_pParent);

   q_ptr->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
}


RecentModel::RecentModel(QObject* parent) : QAbstractItemModel(parent), d_ptr(new RecentModelPrivate(this))
{
   connect(&PersonModel::instance()        , &PersonModel::lastUsedTimeChanged    , d_ptr, &RecentModelPrivate::slotLastUsedTimeChanged);
   connect(&PersonModel::instance()        , &PersonModel::newPersonAdded         , d_ptr, &RecentModelPrivate::slotPersonAdded        );
   connect(&PhoneDirectoryModel::instance(), &PhoneDirectoryModel::lastUsedChanged, d_ptr, &RecentModelPrivate::slotLastUsedChanged    );
   connect(&PhoneDirectoryModel::instance(), &PhoneDirectoryModel::contactChanged , d_ptr, &RecentModelPrivate::slotContactChanged     );
   connect(&CallModel::instance()          , &CallModel::callAdded                , d_ptr, &RecentModelPrivate::slotCallAdded          );

   //Fill the contacts
   for (int i=0; i < PersonModel::instance().rowCount(); i++) {
      auto person = qvariant_cast<Person*>(PersonModel::instance().data(
         PersonModel::instance().index(i,0),
         static_cast<int>(Person::Role::Object)
      ));

      if (person && person->lastUsedTime())
         d_ptr->slotLastUsedTimeChanged(person, person->lastUsedTime());
   }

   //Fill the "orphan" contact methods
   for (int i = 0; i < PhoneDirectoryModel::instance().rowCount(); i++) {
      auto cm = qvariant_cast<ContactMethod*>(PhoneDirectoryModel::instance().data(
         PhoneDirectoryModel::instance().index(i,0),
         static_cast<int>(PhoneDirectoryModel::Role::Object)
      ));

      if (cm && cm->lastUsed() && (!cm->contact() || cm->contact()->isPlaceHolder()))
         d_ptr->slotLastUsedChanged(cm, cm->lastUsed());
   }

   //Fill node with history data
   //const CallMap callMap = CategorizedHistoryModel::instance().getHistoryCalls();
   //Q_FOREACH(auto const &call , callMap) {
   //    d_ptr->slotCallAdded(call, nullptr);
   //}
}

RecentModel::~RecentModel()
{
   for (RecentViewNode* n : d_ptr->m_lTopLevelReverted)
      delete n;

   delete d_ptr;
}

RecentViewNode::RecentViewNode()
{

}

RecentViewNode::~RecentViewNode()
{
   for (RecentViewNode* n : m_lChildren) {
      delete n;
   }
}

time_t RecentViewNode::lastUsed() const
{
   switch(m_Type) {
      case Type::PERSON            :
         return m_uContent.m_pPerson->lastUsedTime();
      case Type::CONTACT_METHOD    :
         return m_uContent.m_pContactMethod->lastUsed();
      case Type::CALL              :
         return m_uContent.m_pCall->stopTimeStamp();
      case Type::CALL_GROUP        :
         return m_uContent.m_pCallGroup->m_LastUsed;
      case Type::TEXT_MESSAGE      :
      case Type::TEXT_MESSAGE_GROUP:
         //TODO
         break;
   }
   return {};
}

// returns the child node containing the given call, if it exists
RecentViewNode*
RecentViewNode::childNode(Call *call) const
{
    if (!call)
        return {};

    // only Person and CM nodes contains calls as children for now, so no need to check other types
    if ( !(m_Type == RecentViewNode::Type::PERSON || m_Type == RecentViewNode::Type::CONTACT_METHOD) )
        return {};

    auto itEnd = m_lChildren.cend();
    auto it = std::find_if (m_lChildren.cbegin(),
               itEnd, [call] (RecentViewNode* child) {
                     return child->m_uContent.m_pCall == call;
               });

    if (it == itEnd)
        return {};

    return *it;
}

RecentModel& RecentModel::instance()
{
    static auto instance = new RecentModel(QCoreApplication::instance());
    return *instance;
}

/**
 * Tries to find the given call in the RecentModel and return
 * the corresponding index
 */
QModelIndex
RecentModel::getIndex(Call *call) const
{
    if (auto parent = d_ptr->parentNode(call)) {
        if (auto callNode = parent->childNode(call))
            return index(callNode->m_Index, 0, index(parent->m_Index, 0));
    }

    return {};
}

/**
 * Tries to find the given Person in the RecentModel and return
 * the corresponding index
 */
QModelIndex
RecentModel::getIndex(Person *p) const
{
    if (d_ptr->m_hPersonsToNodes.contains(p)) {
        if (auto node = d_ptr->m_hPersonsToNodes.value(p))
            return index(node->m_Index, 0);
    }

    return {};
}

/**
 * Tries to find the given CM in the RecentModel and return
 * the corresponding index
 */
QModelIndex
RecentModel::getIndex(ContactMethod *cm) const
{
    if (d_ptr->m_hCMsToNodes.contains(cm)) {
        if (auto node = d_ptr->m_hCMsToNodes.value(cm))
            return index(node->m_Index, 0);
    }

    return {};
}


/**
 * Check if given index has an ongoing call
 * returns true if one of its child is also in the CallModel
 */
bool RecentModel::hasActiveCall(const QModelIndex &idx)
{
   if (not idx.isValid())
      return false;

   auto node = static_cast<RecentViewNode*>(idx.internalPointer());
   QListIterator<RecentViewNode*> lIterator(node->m_lChildren);
   lIterator.toBack();
   while (lIterator.hasPrevious()) {
      auto child = lIterator.previous();
      if (child->m_Type == RecentViewNode::Type::CALL) {
         return  CallModel::instance().getIndex(child->m_uContent.m_pCall).isValid();
      }
   }
   return false;
}

/**
 * Return the first found ongoing call of the given index
 * Index can be the call itself or the associated parent
 */
Call* RecentModel::getActiveCall(const QModelIndex &idx)
{
   if (not idx.isValid())
      return nullptr;

   RecentViewNode* node = static_cast<RecentViewNode*>(idx.internalPointer());

   if (node->m_Type == RecentViewNode::Type::CALL) {
      return node->m_uContent.m_pCall;
   }

   QListIterator<RecentViewNode*> lIterator(node->m_lChildren);
   lIterator.toBack();
   while (lIterator.hasPrevious()) {
      auto child = lIterator.previous();
      if (child->m_Type == RecentViewNode::Type::CALL) {
         return child->m_uContent.m_pCall;
      }
   }
   return nullptr;
}

QHash<int,QByteArray> RecentModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
   }*/

   return roles;
}

bool RecentModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QVariant RecentModel::data( const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   RecentViewNode* node = static_cast<RecentViewNode*>(index.internalPointer());

   switch(node->m_Type) {
      case RecentViewNode::Type::PERSON            :
         return node->m_uContent.m_pPerson->roleData(role);
      case RecentViewNode::Type::CONTACT_METHOD    :
         return node->m_uContent.m_pContactMethod->roleData(role);
      case RecentViewNode::Type::CALL              :
         return node->m_uContent.m_pCall->roleData(role);
      case RecentViewNode::Type::CALL_GROUP        :
         return node->m_uContent.m_pCallGroup->m_lCalls[0]->roleData(role);
      case RecentViewNode::Type::TEXT_MESSAGE      :
      case RecentViewNode::Type::TEXT_MESSAGE_GROUP:
         //TODO
         break;
   }

   return QVariant();
}

int RecentModel::rowCount( const QModelIndex& parent ) const
{
   if (!parent.isValid())
      return d_ptr->m_lTopLevelReverted.size();

   RecentViewNode* node = static_cast<RecentViewNode*>(parent.internalPointer());
   return node->m_lChildren.size();
}

Qt::ItemFlags RecentModel::flags( const QModelIndex& index ) const
{
   return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
}

int RecentModel::columnCount( const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return 1;
}

QModelIndex RecentModel::parent( const QModelIndex& index ) const
{
   if (!index.isValid())
      return QModelIndex();

   RecentViewNode* node = static_cast<RecentViewNode*>(index.internalPointer());

   if (!node->m_pParent)
      return QModelIndex();

   return createIndex(node->m_pParent->m_Index, 0, node->m_pParent);
}

QModelIndex RecentModel::index( int row, int column, const QModelIndex& parent) const
{
   if (!parent.isValid() && row >= 0 && row < d_ptr->m_lTopLevelReverted.size() && !column)
      return createIndex(row, 0, d_ptr->m_lTopLevelReverted[d_ptr->m_lTopLevelReverted.size() - 1 - row]);

   if (!parent.isValid())
      return QModelIndex();

   RecentViewNode* node = static_cast<RecentViewNode*>(parent.internalPointer());

   if (row >= 0 && row < node->m_lChildren.size())
      return createIndex(row, 0, node->m_lChildren[row]);

   return QModelIndex();
}

QVariant RecentModel::headerData( int section, Qt::Orientation orientation, int role) const
{
   if (!section && role == Qt::DisplayRole && orientation == Qt::Horizontal)
      return tr("Recent persons");

   return QVariant();
}

/*
 * Move rows around to keep the person/contactmethods ordered
 *
 * Further optimization:
 *  * Invert m_Index to avoid the O(N) loop when adding an item
 */
void RecentModelPrivate::insertNode(RecentViewNode* n, time_t t, bool isNew)
{
   //Don't bother with the sorted insertion and indexes housekeeping
   if (m_lTopLevelReverted.isEmpty()) {
      q_ptr->beginInsertRows(QModelIndex(),0,0);
      m_lTopLevelReverted << n;
      q_ptr->endInsertRows();
      return;
   }

   //Compute the bounds, this is needed to use beginMoveRows
   int newPos = 0;

   if (m_lTopLevelReverted.last()->lastUsed() > t) {
      //NOTE std::lower_bound need the "value" argument to be the same type as the iterator
      //this use the m_Index field to hold the time_t
      static RecentViewNode fake;
      fake.m_Index = static_cast<long int>(t);

      //NOTE Using std::lower_bound is officially supported on QList on all platforms
      auto lower = std::lower_bound(m_lTopLevelReverted.begin(), m_lTopLevelReverted.end(), &fake,
      [t](const RecentViewNode* a, const RecentViewNode* t2) -> bool {
         return a->lastUsed() < t2->m_Index;
      });

      // the value pointed by the iterator returned by this function may also
      // be equivalent to val, and not only greater
      if (!isNew && n->m_Index == (*lower)->m_Index) {
         newPos = (*lower)->m_Index;
      } else
         newPos = (*lower)->m_Index+1;
   }

   //Begin the transaction
   if (!isNew) {
      if (newPos == n->m_Index)
         return; //Nothing to do
      if (not q_ptr->beginMoveRows(QModelIndex(), n->m_Index, n->m_Index, QModelIndex(), newPos)) {
          qWarning() << "RecentModel: Invalid move detected index : " << n->m_Index
                     << "newPos: " << newPos << "size: " << m_lTopLevelReverted.size();
          return;
      }
      m_lTopLevelReverted.removeAt(m_lTopLevelReverted.size()-1-n->m_Index);
   }
   else
      q_ptr->beginInsertRows(QModelIndex(),newPos,newPos);

   //Apply the transaction
   m_lTopLevelReverted.insert(m_lTopLevelReverted.size() - newPos,n);
   for (int i = 0 ; i < m_lTopLevelReverted.size(); ++i) {
      m_lTopLevelReverted[i]->m_Index = m_lTopLevelReverted.size() - 1 - i;
   }

   //Notify that the transaction is complete
   if (!isNew)
      q_ptr->endMoveRows();
   else
      q_ptr->endInsertRows();
   q_ptr->selectionModel()->select(q_ptr->index(n->m_Index, 0), QItemSelectionModel::ClearAndSelect);

#if 0
    //Uncomment if there is issues
    qDebug() << "\n\nList:" << m_lTopLevelReverted.size() << isNew;
    for (int i = 0; i<m_lTopLevelReverted.size();i++) {
        qDebug() << "|||" << m_lTopLevelReverted[i]->lastUsed() << m_lTopLevelReverted[i]->m_Index << q_ptr->data(q_ptr->index(m_lTopLevelReverted.size()-1-i,0),Qt::DisplayRole);
        for (auto child : m_lTopLevelReverted[i]->m_lChildren) {
            qDebug() << "|||" << "|||" << child << child->m_uContent.m_pCall->formattedName();
        }
     }
#endif
}

void RecentModelPrivate::removeNode(RecentViewNode* n)
{
   const int idx  = n->m_Index;

   q_ptr->beginRemoveRows(QModelIndex(), idx, idx);

   m_lTopLevelReverted.removeOne(n);

   delete n;

   if (idx < m_lTopLevelReverted.size()) {
      for (int i = 0; i <= idx; i++) {
         m_lTopLevelReverted[i]->m_Index--;
      }
   }
   q_ptr->endRemoveRows();
}

void RecentModelPrivate::slotPersonAdded(const Person* p)
{
   if (p)
      slotLastUsedTimeChanged(p, p->lastUsedTime());
}

void RecentModelPrivate::slotLastUsedTimeChanged(const Person* p, time_t t)
{
   RecentViewNode* n = m_hPersonsToNodes.value(p);
   const bool isNew = !n;

   if (isNew) {
      n = new RecentViewNode();
      n->m_Type               = RecentViewNode::Type::PERSON;
      n->m_uContent.m_pPerson = p                           ;
      n->m_pParent            = nullptr                     ;
      n->m_Index              = 0                           ;
      m_hPersonsToNodes[p]    = n                           ;
      connect(p, &Person::changed, this, &RecentModelPrivate::slotChanged);
      Q_FOREACH(auto cm, p->phoneNumbers()) {
         if (auto cmNode = m_hCMsToNodes.value(cm))
            n->m_lChildren.append(cmNode->m_lChildren);
      }

   }

   insertNode(n, t, isNew);
}

void RecentModelPrivate::slotLastUsedChanged(ContactMethod* cm, time_t t)
{
   //ContactMethod with a Person are handled elsewhere
   if (!cm->contact() || cm->contact()->isPlaceHolder()) {
      RecentViewNode* n = m_hCMsToNodes.value(cm);
      const bool isNew = !n;

      if (isNew) {
         n = new RecentViewNode();
         n->m_Type                      = RecentViewNode::Type::CONTACT_METHOD;
         n->m_uContent.m_pContactMethod = cm                                  ;
         n->m_pParent                   = nullptr                             ;
         n->m_Index                     = 0                                   ;
         m_hCMsToNodes[cm]              = n                                   ;
         connect(cm, &ContactMethod::changed, this, &RecentModelPrivate::slotChanged);
      }
      insertNode(n, t, isNew);
   }
}

///Remove the contact method once they are associated with a contact
void RecentModelPrivate::slotContactChanged(ContactMethod* cm, Person* np, Person* op)
{
   Q_UNUSED(np)
   // m_hCMsToNodes contains RecentViewNode pointers, take will return a default
   // constructed ptr (e.g nullptr) if key is not in the QHash
   if (auto n = m_hCMsToNodes.take(cm))
      removeNode(n);
}

void
RecentModelPrivate::insertCallNode(RecentViewNode *parent, Call* call)
{
    Q_ASSERT(parent && call);

    auto callNode = new RecentViewNode();
    callNode->m_Type = RecentViewNode::Type::CALL;
    callNode->m_uContent.m_pCall = call;
    callNode->m_pParent = parent;
    callNode->m_Index = parent->m_lChildren.size();

    auto parentIdx = q_ptr->index(parent->m_Index,0);

    q_ptr->beginInsertRows(parentIdx, callNode->m_Index, callNode->m_Index);
    parent->m_lChildren.append(callNode);
    q_ptr->endInsertRows();

    // emit dataChanged on parent, since number of children has changed
    emit q_ptr->dataChanged(parentIdx, parentIdx);

    if (callNode->m_Index == 2) {
        // we went from 1 to 2 calls, emit a dataChanged on the first call so that the PeopleProxy
        // now shows the first call (otherwise it will only show the 2nd +)
        auto firstChild = q_ptr->index(0, 0, parentIdx);
        emit q_ptr->dataChanged(firstChild, firstChild);
    }
}

void
RecentModelPrivate::removeCall(Call *call)
{
    if (auto parent = parentNode(call)) {
        if (auto callNode = parent->childNode(call)) {
            const auto removedIndex = callNode->m_Index;
            q_ptr->beginRemoveRows(q_ptr->index(parent->m_Index,0), removedIndex, removedIndex);
            parent->m_lChildren.removeAt(removedIndex);
            delete callNode;

            // update the indices of the remaining children
            for (int i = removedIndex; i < parent->m_lChildren.size(); ++i) {
                if (auto child = parent->m_lChildren.at(i))
                    --child->m_Index;
            }
            q_ptr->endRemoveRows();

            auto parentIdx = q_ptr->index(parent->m_Index, 0);
            if (parent->m_lChildren.size() == 1) {
                // there is now only one call, emit dataChanged on it so it becomes hidden in the PeopleProxy
                auto firstChild = q_ptr->index(0, 0, parentIdx);
                emit q_ptr->dataChanged(firstChild, firstChild);
            }
            // emit dataChanted on the parent since the number of children has changed
            emit q_ptr->dataChanged(parentIdx,parentIdx);
        }
    }
}

void
RecentModelPrivate::slotCallAdded(Call* call, Call* parent)
{
   Q_UNUSED(parent)
   Q_ASSERT(call);

   // new call, connect to its changed signal to react to all changes to the call
   connect(call, &Call::changed, this, &RecentModelPrivate::slotChanged);

   // check if call is associated with a Person or CM yet
   if (auto parent = parentNode(call))
       insertCallNode(parent, call);
}

// helper method to find parent node of a call, if it exists
RecentViewNode*
RecentModelPrivate::parentNode(Call *call) const
{
    if (!call)
        return {};

    if (auto p = call->peerContactMethod()->contact()) {
        // if we don't find the Person in our list of nodes, then we want to check the list of CMs
        if (m_hPersonsToNodes.contains(p))
            return m_hPersonsToNodes.value(p);
    }
    return m_hCMsToNodes.value(call->peerContactMethod());
}

void
RecentModelPrivate::slotChanged()
{
    QModelIndex idx;
    if (auto call = qobject_cast<Call*>(sender())) {
        updateCall(call);
        idx = q_ptr->getIndex(call);
    } else if (auto person = qobject_cast<Person*>(sender())) {
        idx = q_ptr->getIndex(person);
    } else if (auto cm = qobject_cast<ContactMethod*>(sender())) {
        idx = q_ptr->getIndex(cm);
    }
    if (idx.parent().isValid())
        emit q_ptr->dataChanged(idx.parent(), idx.parent());
    if (idx.isValid())
        emit q_ptr->dataChanged(idx, idx);
}

void
RecentModelPrivate::updateCall(Call* call)
{
    if (!call)
        return;

    // if the call is ERROR or OVER, make sure to remove it
    // otherwise make sure it is in the RecentModel
    // (it may not have been added if was not yet assocaited with a CM/Person
    switch(call->state()) {
        case Call::State::COUNT__:
        case Call::State::ERROR:
        case Call::State::ABORTED:
        case Call::State::OVER:
            removeCall(call);
        break;
        case Call::State::TRANSFERRED:
        case Call::State::INCOMING:
        case Call::State::RINGING:
        case Call::State::INITIALIZATION:
        case Call::State::CONNECTED:
        case Call::State::CURRENT:
        case Call::State::DIALING:
        case Call::State::NEW:
        case Call::State::HOLD:
        case Call::State::FAILURE:
        case Call::State::BUSY:
        case Call::State::TRANSF_HOLD:
        case Call::State::CONFERENCE:
        case Call::State::CONFERENCE_HOLD:
        {
            if (auto parent = parentNode(call)) {
                if (!parent->childNode(call))
                    insertCallNode(parent, call);
            }
        }
        break;
    };
}

///Filter out every data relevant to a person
QSortFilterProxyModel*
RecentModel::peopleProxy() const
{
   static PeopleProxy* p = new PeopleProxy(const_cast<RecentModel*>(this));
   return p;
}

PeopleProxy::PeopleProxy(RecentModel* sourceModel)
{
    setSourceModel(sourceModel);

    // forward the rowsAboutToBeMoved and rowsMoved signals from the source model
    // this is needed for the OSX and GNOME clients because they do not handle the layoutChanged
    // signal which is emited by the QSortFilterProxyModel when rows are moved in the source model
    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeMoved, this, &PeopleProxy::slotRowsAboutToBeMoved);
    connect(sourceModel, &QAbstractItemModel::rowsMoved, this, &PeopleProxy::slotRowsMoved);
}

bool
PeopleProxy::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const
{
    //Always show the top nodes; in the case of childre, only show if there is more than one
    if (!source_parent.isValid())
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    else if (sourceModel()->rowCount(source_parent) > 1 )
        return true;
    return false;
}

QVariant
PeopleProxy::data(const QModelIndex& index, int role) const
{
    auto indexSource = this->mapToSource(index);

    if (!indexSource.isValid())
        return QVariant();

    //This proxy model filters out single calls, so in this case we want to forward certain data
    //from the call to its parent
    RecentViewNode* node = static_cast<RecentViewNode*>(indexSource.internalPointer());
    bool topNode = node->m_Type == RecentViewNode::Type::PERSON         ||
                   node->m_Type == RecentViewNode::Type::CONTACT_METHOD;
    bool forwardRole = role == static_cast<int>(Ring::Role::State)          ||
                       role == static_cast<int>(Ring::Role::FormattedState) ||
                       role == static_cast<int>(Ring::Role::Length);
    if ( topNode && forwardRole ) {
        if (sourceModel()->rowCount(indexSource) == 1) {
            auto child = sourceModel()->index(0, 0, indexSource);
            return sourceModel()->data(child, role);
        }
    }

    return sourceModel()->data(indexSource, role);
}

void
PeopleProxy::slotRowsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd,
                                    const QModelIndex & destinationParent, int destinationRow)
{
    const auto proxySourceParent = mapFromSource(sourceParent);
    const auto proxyDestinationParent = mapFromSource(destinationParent);

    // since the PeopleProxy doesn't do any sorting or filtering on the top nodes and they are the
    // only ones being moved, we can assume the row numbers stay the same
    beginMoveRows(proxySourceParent, sourceStart, sourceEnd, proxyDestinationParent, destinationRow);
}

void
PeopleProxy::slotRowsMoved(const QModelIndex & parent, int start, int end,
                           const QModelIndex & destination, int row)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    Q_UNUSED(destination)
    Q_UNUSED(row)
    endMoveRows();
}

#include <recentmodel.moc>
