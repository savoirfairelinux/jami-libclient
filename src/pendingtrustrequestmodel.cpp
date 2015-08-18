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
#include "pendingtrustrequestmodel.h"

//Qt
#include <QtCore/QDateTime>

//Ring
#include <trustrequest.h>
#include <certificate.h>
#include <account.h>
#include "private/pendingtrustrequestmodel_p.h"

enum Columns {
   HASH,
   TIME
};

PendingTrustRequestModelPrivate::PendingTrustRequestModelPrivate(PendingTrustRequestModel* p) : q_ptr(p)
{}

PendingTrustRequestModel::PendingTrustRequestModel(Account* a) : QAbstractTableModel(a),
d_ptr(new PendingTrustRequestModelPrivate(this))
{
   d_ptr->m_pAccount = a;
}

PendingTrustRequestModel::~PendingTrustRequestModel()
{
   delete d_ptr;
}

QVariant PendingTrustRequestModel::data( const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   switch(index.column()) {
      case Columns::HASH:
         switch(role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRequests[index.row()]->certificate()->remoteId();
         }
         break;
      case Columns::TIME:
         switch(role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRequests[index.row()]->date();
         }
         break;
   }

   return QVariant();
}

int PendingTrustRequestModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid()? 0 : d_ptr->m_lRequests.size();
}

int PendingTrustRequestModel::columnCount( const QModelIndex& parent ) const
{
   return parent.isValid()? 0 : 2;
}

Qt::ItemFlags PendingTrustRequestModel::flags( const QModelIndex& index ) const
{
   Q_UNUSED(index);
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool PendingTrustRequestModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QHash<int,QByteArray> PendingTrustRequestModel::roleNames() const
{
   return {};
}

void PendingTrustRequestModelPrivate::addRequest(TrustRequest* r)
{
   q_ptr->beginInsertRows(QModelIndex(),m_lRequests.size(),m_lRequests.size());
   m_lRequests << r;
   q_ptr->endRemoveRows();

   QObject::connect(r, &TrustRequest::requestAccepted, [this,r]() {
      emit q_ptr->requestAccepted(r);
   });
   QObject::connect(r, &TrustRequest::requestDiscarded, [this,r]() {
      emit q_ptr->requestDiscarded(r);
   });
}

void PendingTrustRequestModelPrivate::removeRequest(TrustRequest* r)
{
   const int index = m_lRequests.indexOf(r);

   if (index == -1)
      return;

   q_ptr->beginRemoveRows(QModelIndex(), index, index);
   m_lRequests.removeAt(index);
   q_ptr->endRemoveRows();
}
