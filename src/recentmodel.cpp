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
#include <QtCore/QDateTime>

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
   RecentViewNode(Call* c, RecentModelPrivate* model);
   RecentViewNode(const Person *p, RecentModelPrivate* model);
   RecentViewNode(ContactMethod *cm, RecentModelPrivate* model);
   virtual ~RecentViewNode();

   //Attributes
   RecentModelPrivate*    m_pModel   ;
   long int               m_Index    ;
   Type                   m_Type     ;
   RecentViewNode*        m_pParent  ;
   QList<RecentViewNode*> m_lChildren;
   QMetaObject::Connection m_ConnectionChanged;
   QMetaObject::Connection m_ConnectionLastUsedChanged;
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
   void            slotChanged(        );
};

class PeopleProxy : public QSortFilterProxyModel
{
   Q_OBJECT
public:
    PeopleProxy(RecentModel* source_model);

    virtual QVariant data(const QModelIndex& index, int role) const override;
protected:
    virtual bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const override;
};

class RecentModelPrivate : public QObject
{
    Q_OBJECT
public:
   RecentModelPrivate(RecentModel* p);

   /*
   * m_lTopLevel holds the elements in the order they are added to the
   * model. The PeoplProxy QSortFilterProxyModel is then used to sort this
   * list by last used time (most recent at the top)
   */
   QList<RecentViewNode*>                m_lTopLevel        ;
   QHash<const Person*,RecentViewNode*>  m_hPersonsToNodes  ;
   QHash<ContactMethod*,RecentViewNode*> m_hCMsToNodes      ;
   QHash<Call*,RecentViewNode*>          m_hCallsToNodes    ;

   QItemSelectionModel*                  m_pSelectionModel  ;

   //Helper
   void            insertTopNode (RecentViewNode* n                      );
   void            removeTopNode (RecentViewNode* n                      );
   RecentViewNode* parentNode    (Call *call                             )  const;
   void            insertCallNode(RecentViewNode *parent, RecentViewNode* callNode);
   void            removeCall    (RecentViewNode *callNode               );
   void            selectNode    (RecentViewNode* node                   )  const;

private:
   RecentModel* q_ptr;

public Q_SLOTS:
   void slotPersonAdded        (const Person*  p                         );
   void slotContactMethodAdded (ContactMethod* cm                        );
   void slotContactChanged     (ContactMethod* cm, Person* np, Person* op);
   void slotCallAdded          (Call* call       , Call* parent          );
   void slotChanged            (RecentViewNode* node                     );
   void slotCallStateChanged   (Call* call       , Call::State previousState);
   void slotPhoneDirectoryRowsInserted(const QModelIndex& parent, int first, int last);
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
   connect(&PersonModel::instance()        , &PersonModel::newPersonAdded         , d_ptr, &RecentModelPrivate::slotPersonAdded        );
   connect(&PhoneDirectoryModel::instance(), &QAbstractItemModel::rowsInserted    , d_ptr, &RecentModelPrivate::slotPhoneDirectoryRowsInserted);
   connect(&PhoneDirectoryModel::instance(), &PhoneDirectoryModel::contactChanged , d_ptr, &RecentModelPrivate::slotContactChanged     );
   connect(&CallModel::instance()          , &CallModel::callAdded                , d_ptr, &RecentModelPrivate::slotCallAdded          );
   connect(&CallModel::instance()          , &CallModel::callStateChanged         , d_ptr, &RecentModelPrivate::slotCallStateChanged   );

   //Fill the contacts
   for (int i=0; i < PersonModel::instance().rowCount(); i++) {
      auto person = qvariant_cast<Person*>(PersonModel::instance().data(
         PersonModel::instance().index(i,0),
         static_cast<int>(Person::Role::Object)
      ));

      d_ptr->slotPersonAdded(person);
   }

   //Fill the "orphan" contact methods
   for (int i = 0; i < PhoneDirectoryModel::instance().rowCount(); i++) {
      auto cm = qvariant_cast<ContactMethod*>(PhoneDirectoryModel::instance().data(
         PhoneDirectoryModel::instance().index(i,0),
         static_cast<int>(PhoneDirectoryModel::Role::Object)
      ));

      d_ptr->slotContactMethodAdded(cm);
   }

   //Fill node with history data
   //const CallMap callMap = CategorizedHistoryModel::instance().getHistoryCalls();
   //Q_FOREACH(auto const &call , callMap) {
   //    d_ptr->slotCallAdded(call, nullptr);
   //}
}

RecentModel::~RecentModel()
{
   for (RecentViewNode* n : d_ptr->m_lTopLevel)
      delete n;

   delete d_ptr;
}

RecentViewNode::RecentViewNode()
{

}

RecentViewNode::RecentViewNode(Call* c, RecentModelPrivate *model)
{
    m_pModel            = model                     ;
    m_Type              = RecentViewNode::Type::CALL;
    m_uContent.m_pCall  = c                         ;
    m_pParent           = nullptr                   ;
    m_Index             = -1                         ;
    m_ConnectionChanged = QObject::connect(c, &Call::changed, [this](){this->slotChanged();});
}

RecentViewNode::RecentViewNode(const Person* p, RecentModelPrivate *model)
{
    m_pModel             = model                       ;
    m_Type               = RecentViewNode::Type::PERSON;
    m_uContent.m_pPerson = p                           ;
    m_pParent            = nullptr                     ;
    m_Index              = -1                           ;
    m_ConnectionChanged  = QObject::connect(p, &Person::changed, [this](){this->slotChanged();});
    m_ConnectionLastUsedChanged = QObject::connect(p, &Person::lastUsedTimeChanged, [this](){this->slotChanged();});
}

RecentViewNode::RecentViewNode(ContactMethod *cm, RecentModelPrivate *model)
{
    m_pModel                    = model                               ;
    m_Type                      = RecentViewNode::Type::CONTACT_METHOD;
    m_uContent.m_pContactMethod = cm                                  ;
    m_pParent                   = nullptr                             ;
    m_Index                     = -1                                   ;
    m_ConnectionChanged         = QObject::connect(cm, &ContactMethod::changed, [this](){this->slotChanged();});
    m_ConnectionLastUsedChanged = QObject::connect(cm, &ContactMethod::lastUsedChanged, [this](){this->slotChanged();});
}

RecentViewNode::~RecentViewNode()
{
    QObject::disconnect(m_ConnectionChanged);
    QObject::disconnect(m_ConnectionLastUsedChanged);
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

void
RecentViewNode::slotChanged()
{
    m_pModel->slotChanged(this);
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
    if (auto callNode = d_ptr->m_hCallsToNodes.value(call)) {
        if (callNode->m_pParent)
            return index(callNode->m_Index, 0, index(callNode->m_pParent->m_Index, 0));
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
      return d_ptr->m_lTopLevel.size();

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
   if (!parent.isValid() && row >= 0 && row < d_ptr->m_lTopLevel.size() && !column)
      return createIndex(row, 0, d_ptr->m_lTopLevel[row]);

   if (!parent.isValid())
      return QModelIndex();

   auto node = static_cast<RecentViewNode*>(parent.internalPointer());

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

void RecentModelPrivate::insertTopNode(RecentViewNode* node)
{
    if (!node) {
        qWarning() << "trying to insert null node";
        return;
    }

    // make sure this is a top node
    if (node->m_pParent) {
        qWarning() << "trying to insert top node, but it has a parent";
        return;
    }

    const auto newIndex = m_lTopLevel.size();

    q_ptr->beginInsertRows(QModelIndex(), newIndex, newIndex);
    m_lTopLevel.insert(newIndex, node);
    q_ptr->endInsertRows();
    node->m_Index = newIndex;
}

void RecentModelPrivate::removeTopNode(RecentViewNode* n)
{
   if (!n) {
      qWarning() << "trying to remove null node";
      return;
   }

   // make sure this is a top node
   if (n->m_pParent) {
      qWarning() << "trying to remove top node, but it has a parent";
      return;
   }

   const int idx = n->m_Index;

   q_ptr->beginRemoveRows(QModelIndex(), idx, idx);

   m_lTopLevel.removeAt(idx);

   delete n;

   for (int row = idx; row < m_lTopLevel.size(); ++row) {
       m_lTopLevel[row]->m_Index = row;
  }


   q_ptr->endRemoveRows();
}

void RecentModelPrivate::slotPersonAdded(const Person* p)
{
    if (p) {
        if (m_hPersonsToNodes.contains(p)) {
            qWarning() << "Person is already in the RecentModel";
        } else {
            auto n = new RecentViewNode(p, this);
            m_hPersonsToNodes[p] = n;
            insertTopNode(n);
        }
    }
}

void RecentModelPrivate::slotContactMethodAdded(ContactMethod *cm)
{
    // only add CM if it doesn't have a Person or the Person is placeholder
    if (cm && cm->lastUsed() && (!cm->contact() || cm->contact()->isPlaceHolder())) {
        if (m_hCMsToNodes.contains(cm)) {
            qWarning() << "ContactMethod is already in the RecentModel";
        } else {
            auto n = new RecentViewNode(cm, this);
            m_hCMsToNodes[cm] = n;
            insertTopNode(n);
        }
    }
}

void RecentModelPrivate::slotPhoneDirectoryRowsInserted(const QModelIndex& parent, int first, int last)
{
    for (int row = first; row < last; ++row) {
        auto idx = PhoneDirectoryModel::instance().index(row, 0, parent);
        if (idx.isValid()) {
            auto cm = qvariant_cast<ContactMethod*>(idx.data(static_cast<int>(PhoneDirectoryModel::Role::Object)));
            slotContactMethodAdded(cm);
        } else {
            qWarning() << "PhoneDirectoryModel inserting invlid row";
        }
    }
}

///Remove the contact method once they are associated with a contact
void RecentModelPrivate::slotContactChanged(ContactMethod* cm, Person* np, Person* op)
{
    Q_UNUSED(np)
    // m_hCMsToNodes contains RecentViewNode pointers, take will return a default
    // constructed ptr (e.g nullptr) if key is not in the QHash
    if (auto n = m_hCMsToNodes.take(cm)) {
        // remove its child calls from the list first, they will be destroyed when the call is over
        Q_FOREACH(auto cmNode, n->m_lChildren) {
            cmNode->m_pParent = nullptr;
        }
        n->m_lChildren.clear();
        removeTopNode(n);
    }
}

void
RecentModelPrivate::insertCallNode(RecentViewNode *parent, RecentViewNode* callNode)
{
    if (!parent) {
        qWarning() << "parent node is null while trying to insert a call node";
        return;
    }

    if (!callNode) {
        qWarning() << "call node is null when trying to insert a call node";
        return;
    }

    // make sure the parent is a Person or CM
    // TODO: allow conference parent
    if ( ! (parent->m_Type == RecentViewNode::Type::PERSON
            || parent->m_Type == RecentViewNode::Type::CONTACT_METHOD) ) {
        qWarning() << "parent of Call Node must be a Person or Contact Method";
        return;
    }

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
RecentModelPrivate::removeCall(RecentViewNode* callNode)
{
    if (callNode->m_Type != RecentViewNode::Type::CALL) {
        qWarning() << "cannot remove node which is not of type call" << callNode;
        return;
    }

    m_hCallsToNodes.remove(callNode->m_uContent.m_pCall);

    // if it was in the RecentModel, then we need to emit rowsRemoved
    if (auto parentNode = callNode->m_pParent) {
        auto parent = q_ptr->index(parentNode->m_Index, 0);
        const auto removedIndex = callNode->m_Index;

        q_ptr->beginRemoveRows(parent, removedIndex, removedIndex);

        parentNode->m_lChildren.removeAt(removedIndex);

        // update the indices of the remaining children
        for (int i = removedIndex; i < parentNode->m_lChildren.size(); ++i) {
            if (auto child = parentNode->m_lChildren.at(i))
                --child->m_Index;
        }
        q_ptr->endRemoveRows();

        if (parentNode->m_lChildren.size() == 1) {
            // there is now only one call, emit dataChanged on it so it becomes hidden in the PeopleProxy
            auto firstChild = q_ptr->index(0, 0, parent);
            emit q_ptr->dataChanged(firstChild, firstChild);
        }
        // emit dataChanted on the parent since the number of children has changed
        emit q_ptr->dataChanged(parent, parent);
    }
    delete callNode;
}

void
RecentModelPrivate::slotCallAdded(Call* call, Call* parent)
{
    Q_UNUSED(parent)
    if(!call) {
        qWarning() << "callAdded on nullptr";
        return;
    }

    // new call, create Call node and try to find its parent
    // if we find a parent, insert the Call node into the model, otherwise it will be done in slotChanged
    auto callNode = new RecentViewNode(call, this);
    m_hCallsToNodes[call] = callNode;

    // check if call is associated with a Person or CM yet
    if (auto parent = parentNode(call))
        insertCallNode(parent, callNode);

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
RecentModelPrivate::slotChanged(RecentViewNode *node)
{
    if(!node) {
        qWarning() << "changed called on null RecentViewNode";
        return;
    }
    switch (node->m_Type) {
        case RecentViewNode::Type::PERSON:
        case RecentViewNode::Type::CONTACT_METHOD:
        {
            auto idx = q_ptr->index(node->m_Index, 0);
            if (idx.isValid())
                emit q_ptr->dataChanged(idx, idx);
        }
        break;
        case RecentViewNode::Type::CALL:
        {
            // make sure the Call has a parent, else try to find one
            if (node->m_pParent) {
                auto parent = q_ptr->index(node->m_pParent->m_Index, 0);
                auto idx = q_ptr->index(node->m_Index, 0, parent);
                if (parent.isValid()) {
                    emit q_ptr->dataChanged(parent, parent);
                    if (idx.isValid())
                        emit q_ptr->dataChanged(idx, idx);
                }
            } else {
                if (auto parent = parentNode(node->m_uContent.m_pCall)) {
                    insertCallNode(parent, node);
                }
            }
        }
        break;
        case RecentViewNode::Type::CALL_GROUP:
        case RecentViewNode::Type::TEXT_MESSAGE:
        case RecentViewNode::Type::TEXT_MESSAGE_GROUP:
            // nothing to do for now
            break;
    }
}

void
RecentModelPrivate::slotCallStateChanged(Call* call, Call::State previousState)
{
    Q_UNUSED(previousState)

    if (auto callNode = m_hCallsToNodes.value(call)) {
        switch(call->state()) {
           case Call::State::COUNT__:
           case Call::State::ERROR:
           case Call::State::ABORTED:
           case Call::State::OVER:
              removeCall(callNode);
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
              break;
        };
    }
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
    setSortRole(static_cast<int>(Ring::Role::LastUsed));
    sort(0, Qt::DescendingOrder);
}

bool
PeopleProxy::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const
{
    // show all top nodes
    if (!source_parent.isValid()) {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    // show children if there is more than one
    } else if (sourceModel()->rowCount(source_parent) > 1 )
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

#include <recentmodel.moc>
