/****************************************************************************
 *   Copyright (C) 2015-2016 by Savoir-faire Linux                               *
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
#include "pendingcontactrequestmodel.h"

//Qt
#include <QtCore/QDateTime>

//Ring
#include <contactrequest.h>
#include <certificate.h>
#include <account.h>
#include "private/pendingcontactrequestmodel_p.h"
#include "person.h"
#include "contactmethod.h"

PendingContactRequestModelPrivate::PendingContactRequestModelPrivate(PendingContactRequestModel* p) : q_ptr(p)
{}

PendingContactRequestModel::PendingContactRequestModel(Account* a) : QAbstractTableModel(a),
d_ptr(new PendingContactRequestModelPrivate(this))
{
   d_ptr->m_pAccount = a;
}

PendingContactRequestModel::~PendingContactRequestModel()
{
   delete d_ptr;
}

QVariant PendingContactRequestModel::data( const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   switch(index.column()) {
      case Columns::PEER_ID:
         switch(role) {
            case Qt::DisplayRole:
            return d_ptr->m_lRequests[index.row()]->peer()->phoneNumbers()[0]->bestId();
         }
         break;
      case Columns::TIME:
         switch(role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRequests[index.row()]->date();
         }
         break;
      case Columns::FORMATTED_NAME:
         switch(role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRequests[index.row()]->peer()->formattedName();
         }
         break;
      case Columns::COUNT__:
         switch(role) {
             case Qt::DisplayRole:
                return static_cast<int>(PendingContactRequestModel::Columns::COUNT__);
         }
         break;
   }

   return QVariant::fromValue(d_ptr->m_lRequests[index.row()]->roleData(role));
}

int PendingContactRequestModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid()? 0 : d_ptr->m_lRequests.size();
}

int PendingContactRequestModel::columnCount( const QModelIndex& parent ) const
{
   return parent.isValid()? 0 : static_cast<int>(PendingContactRequestModel::Columns::COUNT__);
}

Qt::ItemFlags PendingContactRequestModel::flags( const QModelIndex& index ) const
{
   Q_UNUSED(index);
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool PendingContactRequestModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QHash<int,QByteArray> PendingContactRequestModel::roleNames() const
{
   return {};
}

void PendingContactRequestModelPrivate::addRequest(ContactRequest* r)
{
   // do not add the same contact request several time
   if(std::any_of(m_lRequests.begin(), m_lRequests.end(),
      [&](ContactRequest* r_){ return *r_ == *r ;})) {
      return;
   }

   // update (remove old add new) contact request if the remoteIds match.
   auto iter = std::find_if(m_lRequests.begin(), m_lRequests.end(), [&](ContactRequest* r_) {
      return (r_->certificate()->remoteId() == r->certificate()->remoteId());
   });

    if(iter)
        removeRequest(*iter);

   q_ptr->beginInsertRows(QModelIndex(),m_lRequests.size(),m_lRequests.size());
   m_lRequests << r;
   q_ptr->endInsertRows();

    QObject::connect(r, &ContactRequest::requestAccepted, [this,r]() {
        // the request was handled so it can be removed, from the pending list
        removeRequest(r);

        // it's important to emit after the request was removed.
        emit q_ptr->requestAccepted(r);
    });

    QObject::connect(r, &ContactRequest::requestDiscarded, [this,r]() {
        // the request was handled so it can be removed, from the pending list
        removeRequest(r);

        // it's important to emit after the request was removed.
        emit q_ptr->requestDiscarded(r);
    });

    QObject::connect(r, &ContactRequest::requestBlocked, [this,r]() {
        // the request was handled so it can be removed, from the pending list
        removeRequest(r);
    });

    emit q_ptr->requestAdded(r);
}

void PendingContactRequestModelPrivate::removeRequest(ContactRequest* r)
{
   const int index = m_lRequests.indexOf(r);

   if (index == -1)
      return;

   q_ptr->beginRemoveRows(QModelIndex(), index, index);
   m_lRequests.removeAt(index);
   q_ptr->endRemoveRows();
}
