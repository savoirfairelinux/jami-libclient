/************************************************************************************
 *   Copyright (C) 2015 by Savoir-faire Linux                                       *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>         *
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

//Qt
#include <QtCore/QCoreApplication>

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
        Person*        m_pPerson       ;
        ContactMethod* m_pContactMethod;
        Call*          m_pCall         ;
        CallGroup*     m_pCallGroup    ;
        //ImConversationIterator; //TODO
        //ImConversationIterator;
    } m_uContent;

    //Helpers
    inline time_t lastUsed() const;
};

RecentModel* RecentModel::m_spInstance = nullptr;

class RecentModelPrivate : public QObject
{
public:
    RecentModelPrivate(RecentModel* p);

    /*
    * m_lTopLevelReverted hold the elements in the reverse order of
    * QAbstractItemModel::index. This cause most of the energy to be
    * in the bottom half of the vector, preventing std::move every time
    * someone is contacted
    */
    QList<RecentViewNode*>                m_lTopLevelReverted;
    QHash<Person*,RecentViewNode*>        m_hPersonsToNodes  ;
    QHash<ContactMethod*,RecentViewNode*> m_hCMsToNodes      ;

    //Helper
    void insertNode(RecentViewNode* n, time_t t, bool isNew);
    void removeNode(RecentViewNode* n                      );

private:
    RecentModel* q_ptr;

public Q_SLOTS:
    void slotLastUsedTimeChanged(Person*        p , time_t t              );
    void slotLastUsedChanged    (ContactMethod* cm, time_t t              );
    void slotContactChanged     (ContactMethod* cm, Person* np, Person* op);
    void slotCallAdded          (Call* call       , Call* parent          );
};

RecentModelPrivate::RecentModelPrivate(RecentModel* p) : q_ptr(p)
{
}

RecentModel::RecentModel(QObject* parent) : QAbstractItemModel(parent), d_ptr(new RecentModelPrivate(this))
{
    connect(PersonModel::instance()        , &PersonModel::lastUsedTimeChanged    , d_ptr, &RecentModelPrivate::slotLastUsedTimeChanged);
    connect(PhoneDirectoryModel::instance(), &PhoneDirectoryModel::lastUsedChanged, d_ptr, &RecentModelPrivate::slotLastUsedChanged    );
    connect(PhoneDirectoryModel::instance(), &PhoneDirectoryModel::contactChanged , d_ptr, &RecentModelPrivate::slotContactChanged     );
    connect(CallModel::instance()          , &CallModel::callAdded                , d_ptr, &RecentModelPrivate::slotCallAdded          );

    //Fill the contacts
    for (int i=0; i < PersonModel::instance()->rowCount(); i++) {
        Person* p = qvariant_cast<Person*>(PersonModel::instance()->data(
                                               PersonModel::instance()->index(i,0),
                                               static_cast<int>(Person::Role::Object)
                                               ));

        if (p && p->lastUsedTime())
            d_ptr->slotLastUsedTimeChanged(p, p->lastUsedTime());
    }

    //Fill the "orphan" contact methods
    for (int i=0; i < PhoneDirectoryModel::instance()->rowCount(); i++) {
        ContactMethod* cm = qvariant_cast<ContactMethod*>(PhoneDirectoryModel::instance()->data(
                                                              PhoneDirectoryModel::instance()->index(i,0),
                                                              static_cast<int>(PhoneDirectoryModel::Role::Object)
                                                              ));

        if (cm && cm->lastUsed() && !cm->contact())
            d_ptr->slotLastUsedChanged(cm, cm->lastUsed());
    }

    //Fill node with history data
    CallMap callMap = CategorizedHistoryModel::instance()->getHistoryCalls();
    for (auto call : callMap) {
        d_ptr->slotCallAdded(call, nullptr);
    }
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

RecentModel* RecentModel::instance()
{
    if(!m_spInstance)
        m_spInstance = new RecentModel(QCoreApplication::instance());
    return m_spInstance;
}

bool RecentModel::hasActiveCall(const QModelIndex &parent)
{
    if (not parent.isValid())
        return false;

    RecentViewNode* node = static_cast<RecentViewNode*>(parent.internalPointer());
    for (int i = node->m_lChildren.size()-1; i >= 0; i--) {
        if (node->m_lChildren.at(i)->m_Type == RecentViewNode::Type::CALL)
            return  !(node->m_lChildren.at(i)->m_uContent.m_pCall->isHistory());
    }
    return false;
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
        //return node->m_uContent.m_pCallGroup->m_lCalls[0]->roleData(role);
        return QVariant(QString(QString::number(node->m_uContent.m_pCallGroup->m_lCalls.size())
                                + (node->m_uContent.m_pCallGroup->m_Direction == Call::Direction::INCOMING ? " Incoming" : " Outgoing")
                                +" Missed Calls"));
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

        newPos = (*lower)->m_Index+1;
    }

    //Begin the transaction
    if (!isNew) {
        if (newPos == n->m_Index)
            return; //Nothing to do

        q_ptr->beginMoveRows(QModelIndex(), n->m_Index, n->m_Index, QModelIndex(), newPos ? newPos+1 : newPos );
        m_lTopLevelReverted.removeAt(m_lTopLevelReverted.size()-1-n->m_Index);
    }
    else
        q_ptr->beginInsertRows(QModelIndex(),newPos,newPos);

    //Apply the transaction
    m_lTopLevelReverted.insert(m_lTopLevelReverted.size()-newPos,n);
    for (int i = m_lTopLevelReverted.size()-1; i >= 0; i--) {
        m_lTopLevelReverted[m_lTopLevelReverted.size()-1-i]->m_Index = i;
    }

    //Notify that the transaction is complete
    if (!isNew)
        q_ptr->endMoveRows();
    else
        q_ptr->endInsertRows();

    //Uncomment if there is issues
//        qDebug() << "\n\nList:" << m_lTopLevelReverted.size() << isNew;
//        for (int i = 0; i<m_lTopLevelReverted.size();i++) {
//            qDebug() << "|||" << m_lTopLevelReverted[i]->lastUsed() << m_lTopLevelReverted[i]->m_Index << q_ptr->data(q_ptr->index(m_lTopLevelReverted.size()-1-i,0),Qt::DisplayRole);
//            for (auto child : m_lTopLevelReverted[i]->m_lChildren) {
//                qDebug() << "|||" << "|||" << child << (child->m_Type == RecentViewNode::Type::CALL ? "Call" : "CallGroup : " + QString::number(child->m_uContent.m_pCallGroup->m_lCalls.size()));
//            }
//        }
}

void RecentModelPrivate::removeNode(RecentViewNode* n)
{
    const int idx  = n->m_Index;

    q_ptr->beginRemoveRows(QModelIndex(), idx, idx);

    m_lTopLevelReverted.removeOne(n);

    if (idx < m_lTopLevelReverted.size()) {
        for (int i = 0; i <= idx; i++) {
            m_lTopLevelReverted[i]->m_Index--;
        }
    }
    q_ptr->endRemoveRows();
}

void RecentModelPrivate::slotLastUsedTimeChanged(Person* p, time_t t)
{
    RecentViewNode* n = m_hPersonsToNodes[p];
    const bool isNew = !n;

    if (isNew) {
        n = new RecentViewNode();
        n->m_Type               = RecentViewNode::Type::PERSON;
        n->m_uContent.m_pPerson = p                           ;
        n->m_pParent            = nullptr                     ;
        n->m_Index              = 0                           ;
        m_hPersonsToNodes[p]    = n                           ;
        for(auto cm : p->phoneNumbers()) {
            RecentViewNode* cmNode = m_hCMsToNodes[cm];
            if (cmNode)
                n->m_lChildren.append(cmNode->m_lChildren);
        }
    }
    insertNode(n, t, isNew);
}

void RecentModelPrivate::slotLastUsedChanged(ContactMethod* cm, time_t t)
{
    //ContactMethod with a Person are handled elsewhere
    if (!cm->contact()) {
        RecentViewNode* n = m_hCMsToNodes[cm];
        const bool isNew = !n;

        if (isNew) {
            n = new RecentViewNode();
            n->m_Type                      = RecentViewNode::Type::CONTACT_METHOD;
            n->m_uContent.m_pContactMethod = cm                                  ;
            n->m_pParent                   = nullptr                             ;
            n->m_Index                     = 0                                   ;
            m_hCMsToNodes[cm]              = n                                   ;
        }
        insertNode(n, t, isNew);
    }
}

///Remove the contact method once they are associated with a contact
void RecentModelPrivate::slotContactChanged(ContactMethod* cm, Person* np, Person* op)
{
    Q_UNUSED(np)
    if (op)
        return;
    RecentViewNode* n = m_hCMsToNodes[cm];

    if (n)
        removeNode(n);
}

void RecentModelPrivate::slotCallAdded(Call* call, Call* parent)
{
    Q_UNUSED(parent)

    RecentViewNode* n = nullptr;

    if (auto p = call->peerContactMethod()->contact()) {
        n = m_hPersonsToNodes[p];
    } else {
        n = m_hCMsToNodes[call->peerContactMethod()];
    }
    if (!n)
        return;

    auto node = new RecentViewNode();
    node->m_pParent = n;
    node->m_Index = n->m_lChildren.size();

    //for new call we'll update their node when it's over
    if (not call->isHistory())
        connect(call, &Call::isOver, [=]() {
            auto node = n->m_lChildren.takeLast();
            slotCallAdded(node->m_uContent.m_pCall, nullptr);
            delete node;
            disconnect(call);
        });

    //if the last node inserted is a group of call
    //check if we should append to it
    //else check if we should make a new one
    if (n->m_lChildren.size() > 0) {
        if (n->m_lChildren.last()->m_Type == RecentViewNode::Type::CALL_GROUP) {
            auto callGroup = n->m_lChildren.last()->m_uContent.m_pCallGroup;
            if (call->isMissed() && callGroup->m_Missed && call->direction() == callGroup->m_Direction) {
                callGroup->m_lCalls.append(call);
                delete node;
                return;
            }
        } else if (n->m_lChildren.last()->m_Type == RecentViewNode::Type::CALL) {
            auto lastCall = n->m_lChildren.last()->m_uContent.m_pCall;
            if (lastCall->isMissed() && call->isMissed() && call->direction() == lastCall->direction()) {
                node->m_Type = RecentViewNode::Type::CALL_GROUP;
                auto callGroup = new CallGroup();
                callGroup->m_Direction = lastCall->direction();
                callGroup->m_Missed = true;
                callGroup->m_lCalls.append(lastCall);
                callGroup->m_lCalls.append(call);
                delete n->m_lChildren.takeLast();
                node->m_Index = n->m_lChildren.size();
                node->m_uContent.m_pCallGroup = callGroup;
                n->m_lChildren.append(node);
                return;
            }
        }
    }
    node->m_Type = RecentViewNode::Type::CALL;
    node->m_uContent.m_pCall = call;
    n->m_lChildren.append(node);
}
