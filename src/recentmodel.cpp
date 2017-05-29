/************************************************************************************
 *   Copyright (C) 2015-2016 by Savoir-faire Linux                                       *
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
#include "accountmodel.h"
#include "contactrequest.h"
#include "certificate.h"
#include "availableaccountmodel.h"
#include "pendingcontactrequestmodel.h"

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
      CONFERENCE        ,
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
    virtual bool filterAcceptsRow ( int sourceRow, const QModelIndex & sourceParent ) const override;

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
   QHash<Call*,RecentViewNode*>          m_hCallsToNodes    ;
   QHash<Call*,RecentViewNode*>          m_hConfToNodes     ;

   QItemSelectionModel*                  m_pSelectionModel  ;

   //Helper
   void            insertNode    (RecentViewNode* n, time_t t, bool isNew);
   void            removeNode    (RecentViewNode* n                      );
   RecentViewNode* parentNode    (Call *call                             )  const;
   void            insertCallNode(RecentViewNode* parent, RecentViewNode* callNode);
   void            moveCallNode  (RecentViewNode* destination, RecentViewNode* callNode);
   void            removeCall    (RecentViewNode* callNode               );
   void            selectNode    (RecentViewNode* node                   )  const;

private:
   RecentModel* q_ptr;

public Q_SLOTS:
   void slotLastUsedTimeChanged(const Person*  p , time_t t              );
   void slotPersonAdded        (const Person*  p                         );
   void slotPersonRemoved      (const Person*  p                         );
   void slotLastUsedChanged    (ContactMethod* cm, time_t t              );
   void slotContactChanged     (ContactMethod* cm, const Person* np, const Person* op);
   void slotCallAdded          (Call* call       , Call* parent          );
   void slotChanged            (RecentViewNode* node                     );
   void slotCallStateChanged   (Call* call       , Call::State previousState);
   void slotConferenceAdded    (Call* conf                               );
   void slotConferenceRemoved  (Call* conf                               );
   void slotConferenceChanged  (Call* conf                               );
   void slotCurrentCallChanged (const QModelIndex &current, const QModelIndex &previous);
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

QStringList
RecentModel::getParticipantName(Call* call) const
{
    if (not call || call->type() != Call::Type::CONFERENCE) {
        qWarning() << "Invalid use of getParticipantName";
        return QStringList();
    }
    if (auto node = d_ptr->m_hConfToNodes[call]) {
        QStringList participants;
        Q_FOREACH(auto child, node->m_lChildren) {
            participants << data(getIndex(child->m_uContent.m_pCall), static_cast<int>(Ring::Role::Name)).toString();
        }
        return participants;
    }
    return QStringList();
}

int
RecentModel::getParticipantNumber(Call *call) const
{
    if (not call || call->type() != Call::Type::CONFERENCE) {
        qWarning() << "Invalid use of getParticipantNumber";
        return 0;
    }
    if (auto node = d_ptr->m_hConfToNodes[call]) {
        return node->m_lChildren.size();
    }
    return 0;
}

void RecentModelPrivate::selectNode(RecentViewNode* node) const
{
   const auto idx = q_ptr->createIndex(node->m_Index, 0, node);

   q_ptr->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
}


RecentModel::RecentModel(QObject* parent) : QAbstractItemModel(parent), d_ptr(new RecentModelPrivate(this))
{
    connect(&PersonModel::instance()        , &PersonModel::lastUsedTimeChanged    , d_ptr, &RecentModelPrivate::slotLastUsedTimeChanged);
    connect(&PersonModel::instance()        , &PersonModel::newPersonAdded         , d_ptr, &RecentModelPrivate::slotPersonAdded        );
    connect(&PersonModel::instance()        , &PersonModel::personRemoved          , d_ptr, &RecentModelPrivate::slotPersonRemoved      );
    connect(&PhoneDirectoryModel::instance(), &PhoneDirectoryModel::lastUsedChanged, d_ptr, &RecentModelPrivate::slotLastUsedChanged    );
    connect(&PhoneDirectoryModel::instance(), &PhoneDirectoryModel::contactChanged , d_ptr, &RecentModelPrivate::slotContactChanged     );
    connect(&CallModel::instance()          , &CallModel::callAdded                , d_ptr, &RecentModelPrivate::slotCallAdded          );
    connect(&CallModel::instance()          , &CallModel::callStateChanged         , d_ptr, &RecentModelPrivate::slotCallStateChanged   );
    connect(&CallModel::instance()          , &CallModel::conferenceCreated        , d_ptr, &RecentModelPrivate::slotConferenceAdded    );
    connect(&CallModel::instance()          , &CallModel::conferenceRemoved        , d_ptr, &RecentModelPrivate::slotConferenceRemoved  );
    connect(&CallModel::instance()          , &CallModel::conferenceChanged        , d_ptr, &RecentModelPrivate::slotConferenceChanged  );
    connect(CallModel::instance().selectionModel(), &QItemSelectionModel::currentChanged, d_ptr, &RecentModelPrivate::slotCurrentCallChanged);

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

    //Get any ongoing calls (except conferences)
    auto callList = CallModel::instance().getActiveCalls();
    for (int i = 0; i < callList.size(); ++i) {
        auto call = callList.at(i);
        if (call->type() != Call::Type::CONFERENCE)
            d_ptr->slotCallAdded(call, nullptr);
    }

    //Add any ongoing conferences
    auto confList = CallModel::instance().getActiveConferences();
    for (int i = 0; i < confList.size(); ++i) {
        auto conf = confList.at(i);
        d_ptr->slotConferenceAdded(conf);
    }

    // sync initial selection with the CallModel in case there are any ongoing Calls
    d_ptr->slotCurrentCallChanged(CallModel::instance().selectionModel()->currentIndex(), QModelIndex());
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

RecentViewNode::RecentViewNode(Call* c, RecentModelPrivate *model)
{
    m_pModel            = model                     ;
    m_Type = c->type() == Call::Type::CONFERENCE ? RecentViewNode::Type::CONFERENCE : RecentViewNode::Type::CALL;
    m_uContent.m_pCall  = c                         ;
    m_pParent           = nullptr                   ;
    m_Index             = 0                         ;
    m_ConnectionChanged = QObject::connect(c, &Call::changed, [this](){this->slotChanged();});
}

RecentViewNode::RecentViewNode(const Person* p, RecentModelPrivate *model)
{
    m_pModel             = model                       ;
    m_Type               = RecentViewNode::Type::PERSON;
    m_uContent.m_pPerson = p                           ;
    m_pParent            = nullptr                     ;
    m_Index              = 0                           ;
    m_ConnectionChanged  = QObject::connect(p, &Person::changed, [this](){this->slotChanged();});
}

RecentViewNode::RecentViewNode(ContactMethod *cm, RecentModelPrivate *model)
{
    m_pModel                    = model                               ;
    m_Type                      = RecentViewNode::Type::CONTACT_METHOD;
    m_uContent.m_pContactMethod = cm                                  ;
    m_pParent                   = nullptr                             ;
    m_Index                     = 0                                   ;
    m_ConnectionChanged         = QObject::connect(cm, &ContactMethod::changed, [this](){this->slotChanged();});
}

RecentViewNode::~RecentViewNode()
{
    QObject::disconnect(m_ConnectionChanged);
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
      case Type::CONFERENCE:
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
    // first check if it is a conference
    if (auto confNode = d_ptr->m_hConfToNodes.value(call))
        return index(confNode->m_Index, 0);

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
    // check if the CM is an item the RecentModel
    if (d_ptr->m_hCMsToNodes.contains(cm)) {
        if (auto node = d_ptr->m_hCMsToNodes.value(cm))
            return index(node->m_Index, 0);
    }

    // otherwise, its possible the CM is contained within a Person item
    if (auto person = cm->contact())
        return getIndex(person);

    return {};
}


/**
 * Returns if the given index corresponds to an item in the RecentModel which is a conference
 */
bool
RecentModel::isConference(const QModelIndex& idx) const
{
    if (idx.isValid()) {
        auto object = idx.data(static_cast<int>(Ring::Role::Object));
        if (auto call = object.value<Call *>())
            return call->type() == Call::Type::CONFERENCE;
    }
    return false;
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
 * Helper function to extract all ContactMethod from a given index
 * The index must be of type ContactMethod, Person or Call
 */
QVector<ContactMethod*>
RecentModel::getContactMethods(const QModelIndex& idx) const
{
    auto result = QVector<ContactMethod*>();

    RecentViewNode* node = static_cast<RecentViewNode*>(idx.internalPointer());
    if (!idx.isValid() || !node) {
        return result;
    }

    switch(node->m_Type) {
        case RecentViewNode::Type::PERSON            :
            result << node->m_uContent.m_pPerson->phoneNumbers();
            return result;
        case RecentViewNode::Type::CONTACT_METHOD    :
            result << node->m_uContent.m_pContactMethod;
            return result;
        case RecentViewNode::Type::CALL              :
            result << node->m_uContent.m_pCall->peerContactMethod();
            return result;
        case RecentViewNode::Type::CALL_GROUP        :
        case RecentViewNode::Type::TEXT_MESSAGE      :
        case RecentViewNode::Type::TEXT_MESSAGE_GROUP:
        case RecentViewNode::Type::CONFERENCE        :
            break;
    }
    return result;
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

   if (node->m_Type == RecentViewNode::Type::CALL
           || node->m_Type == RecentViewNode::Type::CONFERENCE ) {
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
      case RecentViewNode::Type::CONFERENCE        :
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

   // update all indices after this one
   for (int i = m_lTopLevelReverted.size() - 1 - idx; i >= 0; --i) {
      --m_lTopLevelReverted[i]->m_Index;
   }

   q_ptr->endRemoveRows();
}

void RecentModelPrivate::slotPersonAdded(const Person* p)
{
    if (!p) return;

    // make sure all the CMs associated with this Person are moved into this Person's node
    for ( const auto contactMethod : p->phoneNumbers() ) {
        slotContactChanged(contactMethod, p, nullptr);
    }
}

void RecentModelPrivate::slotPersonRemoved(const Person* p)
{
    // delete p from contacts
    RecentViewNode* n = m_hPersonsToNodes.value(p);
    const bool isNewContact = !n;

    if ( isNewContact )
       return;

    removeNode(n);

    // but keep the conversation
    for ( const auto cmToAdd : p->phoneNumbers() ) {
        const bool isNewCm = !m_hCMsToNodes.value(cmToAdd);
        n = new RecentViewNode(cmToAdd, this);
        m_hCMsToNodes[cmToAdd] = n;
        insertNode(n, cmToAdd->lastUsed(), isNewCm);
    }
}

void RecentModelPrivate::slotLastUsedTimeChanged(const Person* p, time_t t)
{
   RecentViewNode* n = m_hPersonsToNodes.value(p);
   const bool isNew = !n;

   if (isNew) {
      n = new RecentViewNode(p, this);
      n->m_pParent = nullptr;
      m_hPersonsToNodes[p] = n;
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
         n = new RecentViewNode(cm, this);
         m_hCMsToNodes[cm] = n;
      }
      insertNode(n, t, isNew);
   }
}

///(Re)move the ContactMethod node once they are associated with a (new) Person
void RecentModelPrivate::slotContactChanged(ContactMethod* contactMethod, const Person* newPerson, const Person* oldPerson)
{
    // TODO: implement for when the Person of the CM changes, ie: oldPerson != nullptr
    Q_UNUSED(oldPerson)

    if (not newPerson
        or /* avoid to add en entry in recent model when a person was built for a contact request */
        (contactMethod->account()
         and contactMethod->account()->pendingContactRequestModel()->findContactRequestFrom(contactMethod)))
        return;

    // make sure the Person node exists first, then move any children of the CM nodes
    slotLastUsedTimeChanged(newPerson, newPerson->lastUsedTime());

    if (!newPerson->phoneNumbers().contains(contactMethod))
        qWarning() << "CM has new Person parent, but is not contained in its list of CMs";

    // m_hCMsToNodes contains RecentViewNode pointers, take will return a default
    // constructed ptr (e.g nullptr) if key is not in the QHash
    if (auto oldParentNode = m_hCMsToNodes.take(contactMethod)) {

        // move any child nodes (Calls) to new Person
        if (auto newParentNode = m_hPersonsToNodes.value(newPerson)) {
            // we need to make a copy of the container since we're modifying it
            const auto callListCopy = oldParentNode->m_lChildren;
            for (const auto &callNode : callListCopy) {
                moveCallNode(newParentNode, callNode);
            }

            // check if the node we will remove is the selected node and select the new node
            auto selectedIdx = q_ptr->selectionModel()->currentIndex();
            auto oldIdx = q_ptr->createIndex(oldParentNode->m_Index, 0, oldParentNode);
            if (selectedIdx == oldIdx)
                selectNode(newParentNode);

            removeNode(oldParentNode);
        } else {
            qWarning("RecentModel: ContactMethod has new Person, but corresponding Person node doesn't exist");
        }
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

    if (parent->m_lChildren.size() > 1) {
        // emit a dataChanged on the first call so that the PeopleProxy
        // now shows the first call (otherwise it will only show the 2nd +)
        auto firstChild = q_ptr->index(0, 0, parentIdx);
        emit q_ptr->dataChanged(firstChild, firstChild);
    }

    /* in the case of a conference, we select the call;
     * in case the parent only has one call, we select the parent;
     * in case the parent has multiple calls, we select the call;
     */
    auto callIdx = q_ptr->index(callNode->m_Index, 0, parentIdx);
    if (q_ptr->isConference(callIdx) || q_ptr->isConference(parentIdx) || q_ptr->rowCount(parentIdx) > 1)
        q_ptr->selectionModel()->setCurrentIndex(callIdx, QItemSelectionModel::ClearAndSelect);
    else
        q_ptr->selectionModel()->setCurrentIndex(callIdx.parent(), QItemSelectionModel::ClearAndSelect);
}

void
RecentModelPrivate::moveCallNode(RecentViewNode* destination, RecentViewNode* callNode)
{
    if (not callNode->m_pParent) {
        qWarning() << "Trying to move call node with invalid parent";
        return;
    }
    if (callNode->m_Type != RecentViewNode::Type::CALL) {
        qWarning() << "cannot move node which is not of type call" << callNode;
        return;
    }

    auto parentNode = callNode->m_pParent;
    auto parent = q_ptr->index(parentNode->m_Index, 0);
    const auto removedIndex = callNode->m_Index;

    // check if this call was selected, so that we can re-select the same one after its moved
    auto selected = false;
    auto selectedIdx = q_ptr->selectionModel()->currentIndex();
    auto oldIdx = q_ptr->index(removedIndex, 0, parent);
    if (selectedIdx == oldIdx)
        selected = true;

    q_ptr->beginRemoveRows(parent, removedIndex, removedIndex);

    parentNode->m_lChildren.removeAt(removedIndex);

    // update the indices of the remaining children
    for (int i = removedIndex; i < parentNode->m_lChildren.size(); ++i) {
        if (auto child = parentNode->m_lChildren.at(i))
            --child->m_Index;
    }
    q_ptr->endRemoveRows();

    callNode->m_pParent = destination;
    callNode->m_Index = destination->m_lChildren.size();
    auto destIdx = q_ptr->index(destination->m_Index, 0);
    q_ptr->beginInsertRows(destIdx, callNode->m_Index, callNode->m_Index);
    destination->m_lChildren.append(callNode);
    q_ptr->endInsertRows();

    if (parentNode->m_lChildren.size() == 1) {
        // there is now only one call, emit dataChanged on it so it becomes hidden in the PeopleProxy
        auto firstChild = q_ptr->index(0, 0, parent);
        emit q_ptr->dataChanged(firstChild, firstChild);
    }
    // emit dataChanged on the parent since the number of children has changed
    emit q_ptr->dataChanged(parent, parent);

    if (selected)
        selectNode(callNode);
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
    if (auto parent = parentNode(call)) {
        insertCallNode(parent, callNode);
    }
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
        case RecentViewNode::Type::CONFERENCE:
        {
            auto idx = q_ptr->index(node->m_Index, 0);
            emit q_ptr->dataChanged(idx, idx);
        }
        break;
        case RecentViewNode::Type::CALL:
        {
            // make sure the Call has a parent, else try to find one
            if (node->m_pParent) {
                auto parent = q_ptr->index(node->m_pParent->m_Index, 0);
                auto idx = q_ptr->index(node->m_Index, 0, parent);
                emit q_ptr->dataChanged(parent, parent);
                emit q_ptr->dataChanged(idx, idx);
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

void
RecentModelPrivate::slotConferenceRemoved(Call* conf)
{
    RecentViewNode* n = m_hConfToNodes[conf];

    if (n) {
        foreach (RecentViewNode* node, n->m_lChildren) {
            if (node->m_uContent.m_pCall->lifeCycleState() != Call::LifeCycleState::PROGRESS)
                removeCall(node);
            else {
                moveCallNode(parentNode(node->m_uContent.m_pCall), node);
            }
        }
        removeNode(n);
        m_hConfToNodes.remove(conf);
    }
}

void
RecentModelPrivate::slotConferenceAdded(Call* conf)
{
    RecentViewNode* n = m_hConfToNodes[conf];
    const bool isNew = !n;

    if (isNew) {
        n = new RecentViewNode(conf, this);
        m_hConfToNodes[conf] = n;
    }
    insertNode(n, conf->startTimeStamp(), isNew);

    // move the participants after inserting the node
    auto pList = CallModel::instance().getConferenceParticipants(conf);
    foreach (Call* p, pList) {
        moveCallNode(n, m_hCallsToNodes.value(p));
    }

    // Select the newly created conference
    q_ptr->selectionModel()->setCurrentIndex(q_ptr->getIndex(conf), QItemSelectionModel::ClearAndSelect);
}

void RecentModelPrivate::slotConferenceChanged(Call* conf)
{
    if (auto confNode = m_hConfToNodes.value(conf)) {
        auto pSet = QSet<Call*>::fromList(CallModel::instance().getConferenceParticipants(conf));
        if (pSet.isEmpty()) {
            slotConferenceRemoved(conf);
            return;
        }
        if (confNode->m_lChildren.size() == pSet.size())
            return;
        QSet<Call*> confPSet;
        foreach(const RecentViewNode* node, confNode->m_lChildren) {
            confPSet.insert(node->m_uContent.m_pCall);
        }
        if (confPSet.size() > pSet.size()) {
            confPSet = confPSet.subtract(pSet);
            foreach(Call* call, confPSet) {
                removeCall(m_hCallsToNodes.value(call));
            }
        } else {
            pSet = pSet.subtract(confPSet);
            foreach(Call* call, pSet) {
                moveCallNode(confNode, m_hCallsToNodes.value(call));
            }
            // Reselect the conference
            q_ptr->selectionModel()->setCurrentIndex(q_ptr->getIndex(conf), QItemSelectionModel::ClearAndSelect);

        }
    }
}

void
RecentModelPrivate::slotCurrentCallChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    auto callIdx = q_ptr->getIndex(CallModel::instance().getCall(current));
    if (callIdx.isValid()) {
        /* in the case of a conference, we select the call;
         * in case the parent only has one call, we select the parent;
         * in case the parent has multiple calls, we select the call;
         */
        auto parentIdx = callIdx.parent();
        if (q_ptr->isConference(callIdx) || q_ptr->isConference(parentIdx) || q_ptr->rowCount(parentIdx) > 1)
            q_ptr->selectionModel()->setCurrentIndex(callIdx, QItemSelectionModel::ClearAndSelect);
        else
            q_ptr->selectionModel()->setCurrentIndex(callIdx.parent(), QItemSelectionModel::ClearAndSelect);
    } else {
        /* nothing is selected in the CallModel; however, if we still have a call selected in the
         * RecentModel, we need to select it in the CallModel, or else all the actions of the call
         * will be invalid, since the UserActionModel is based on the selection of the CallModel */
         auto recentIdx = q_ptr->selectionModel()->currentIndex();
         auto recentCall = q_ptr->getActiveCall(recentIdx);
         if (recentIdx.isValid() && recentCall) {
             CallModel::instance().selectCall(recentCall);
         }
         /* otherwise do not update the selection in the RecentModel, eg: if a Person was selected
          * and the Call is over, we still want the Person to be selected */
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

    /* since the filterAcceptsRow depends on the selected account, we need to re-run the filtering
     * when the selected account changes automatically */
    connect( AvailableAccountModel::instance().selectionModel(),
        &QItemSelectionModel::currentChanged, [this]() {this->invalidateFilter();});
}



/**
 * This is the filtering function of the PeopleProxy. The PeopleProxy is essentially the "smart list" in
 * the clients that implement one. The desired behaviour is to group ContactMethods which belong to the
 * same contact (Person) under one item. Furthermore, we want to filter out items which do not belong to the
 * currentDefaultAccount() of the AvailableAccountModel. The only exception are items which currently have
 * an active Call since we always want the user to see any ongoing calls. We don't want the user to have
 * to take any action to see ongoing Calls.
 *
 * We also implement a fitler based on the filterRegExp() string of the model. It will try to find any
 * name or URI associated with the item which contains the string (case insensitive) and filter out
 * eveything else. Again, the only exception is Calls, in case a user forgets to clear the search entry.
 */
bool
PeopleProxy::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
    auto chosenAccount = AvailableAccountModel::instance().currentDefaultAccount();

    //we filter only on top nodes
    if (!sourceParent.isValid()) {
        auto idx = sourceModel()->index(sourceRow, 0);
        auto type = idx.data(static_cast<int>(Ring::Role::ObjectType)).value<Ring::ObjectType>();
        auto object = idx.data(static_cast<int>(Ring::Role::Object));

        // get set of CMs with active calls
        auto activeCalls = CallModel::instance().getActiveCalls();
        QSet<const ContactMethod *> activeCallCMs;
        for (auto call : activeCalls) {
            activeCallCMs << call->peerContactMethod();
        }

        Person *person = nullptr;
        auto filterFunction = [&person, chosenAccount, this, activeCallCMs] (const ContactMethod* cm) {
            auto passesFilter = false;

            // never filter out items with active calls
            if (activeCallCMs.contains(cm)) return true;

            // filter everything out if there is no account chosen
            if (not chosenAccount) return false;

            // only proceed if there is no account set yet, or if it matches the chosen account
            if ( !cm->account() or (cm->account() == chosenAccount)) {
                /* we need to check the Person name as well as any identifier of the
                 * ContactMethod.
                 * note: QString::contains() will return true for an empty param string
                 */
                passesFilter =
                    (person and person->formattedName().contains(filterRegExp())) or
                    cm->uri().full().contains(filterRegExp()) or
                    cm->registeredName().contains(filterRegExp()) or
                    cm->primaryName().contains(filterRegExp());
            }
            return passesFilter;
        };

        //we want to filter on name and number; note that Person object may have many numbers
        switch (type) {
            case Ring::ObjectType::Person:
            {
                person = object.value<Person *>();
                const auto personCMs = person->phoneNumbers();

                return std::any_of(std::begin(personCMs), std::end(personCMs), filterFunction);
            }
            case Ring::ObjectType::ContactMethod:
            {
                auto cm = object.value<ContactMethod *>();

                return filterFunction(cm);
            }
            case Ring::ObjectType::Call:
                return true;

            // top nodes are only of type Person, ContactMethod or Call
            case Ring::ObjectType::Media:
            case Ring::ObjectType::Certificate:
            case Ring::ObjectType::ContactRequest:
            case Ring::ObjectType::COUNT__:
            break;
        }

        return false; // no matches
    }
    //in the case of children, only show if there is more than one unless it is a conference
    if (static_cast<RecentModel *>(sourceModel())->isConference(sourceParent)
        || sourceModel()->rowCount(sourceParent) > 1 )
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
