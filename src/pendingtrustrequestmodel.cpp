/****************************************************************************
 *   Copyright (C) 2015-2016 by Savoir-faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *   Author : Stepan Salenikovich <stepan.salenikovich@savoirfairelinux.com>*
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
#include "trustrequest.h"
#include "certificate.h"
#include "account.h"
#include "private/pendingtrustrequestmodel_p.h"
#include "personmodel.h"
#include "person.h"
#include "private/vcardutils.h"
#include "phonedirectorymodel.h"
#include "contactmethod.h"

enum Columns {
   HASH,
   TIME
};

PendingTrustRequestModelPrivate::PendingTrustRequestModelPrivate(PendingTrustRequestModel* p) : q_ptr(p)
{
    // try to get the PendingPeerProfileCollection, if it exists
    pendingCollection();
}

PendingTrustRequestModelPrivate::~PendingTrustRequestModelPrivate()
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

TrustRequest* PendingTrustRequestModel::getByPerson(const Person* p) const
{
    return d_ptr->m_hPersonsToTrustRequests.value(p);
}

CollectionInterface* PendingTrustRequestModelPrivate::pendingCollection()
{
    if (!m_pPendingCollection) {
        for (auto col : PersonModel::instance().collections()) {
            if (col->id() == "pppc") {
                m_pPendingCollection = col;
                break;
            }
        }
    }
    return m_pPendingCollection;
}

void PendingTrustRequestModelPrivate::newTrustRequest(Account *a, const QString& hash, const QByteArray& payload, time_t time)
{
    // TODO handle the case that the person is already trusted and we receive another request
    // TODO handle the case that the vCard cointains ringIDs, make sure they don't get trusted automatically (see setContactMethods() )

    // create CM from hash (ringID) and possible person
    //TODO: maybe we should always create a new Person? so that "attackers" don't try to spoof existing contacts...
    auto p = VCardUtils::mapToPerson(VCardUtils::toHashMap(payload));
    auto cm = PhoneDirectoryModel::instance().getNumber(hash, p, a);

    if (p) {
        // we need to set the collection of the person before setting the contact methods so they don't
        // get trusted automatically
        auto numbers = p->phoneNumbers();
        if (!numbers.contains(cm)) {
            // TODO: do we want to do this, or instead replace any existing CMs with this one?
            // what happens if you want to trust one CM with this vCard, but not another?
            numbers << cm;
            p->setContactMethods(numbers);
        }
    }

    cm->setLastUsed(time);

    auto request = new TrustRequest(a, hash, time, p);
    addRequest(request);
}

void PendingTrustRequestModelPrivate::addRequest(TrustRequest* r)
{
    //TODO: check if we already have a request from the same ID, or do we simply display all of them?
   q_ptr->beginInsertRows(QModelIndex(),m_lRequests.size(),m_lRequests.size());
   m_lRequests << r;
   q_ptr->endInsertRows();

   QObject::connect(r, &TrustRequest::requestAccepted, this, &PendingTrustRequestModelPrivate::requestAccepted);
   QObject::connect(r, &TrustRequest::requestDiscarded, this, &PendingTrustRequestModelPrivate::requestDiscarded);

   // add to collection so that the peer appears in the list and for persistence
   // if the colleciton is not instantiated the trust request will only appear in this model
   if (auto col = pendingCollection()) {
       if (auto p = r->person()) {
           p->setCollection(col);
           col->add(p);
           // make the mapping
           m_hPersonsToTrustRequests[p] = r;
       }
   }
}

void PendingTrustRequestModelPrivate::removeRequest(TrustRequest* r)
{
   const int index = m_lRequests.indexOf(r);

   if (index == -1)
      return;

   q_ptr->beginRemoveRows(QModelIndex(), index, index);
   m_lRequests.removeAt(index);
   if (auto p = r->person())
      m_hPersonsToTrustRequests.remove(p);
   q_ptr->endRemoveRows();
   r->deleteLater();
}

void PendingTrustRequestModelPrivate::requestAccepted()
{
    TrustRequest* r = qobject_cast<TrustRequest*>(sender());
    if (r) {
        // needs to be remove from pending requests and added to peers
        if (auto p = r->person()) {
            // TODO: will this cause the Person to disappear and reappear in the recent model view?
            p->remove();
            PersonModel::instance().addPeerProfile(p);
        }

        // TODO is this needed? we will delete the request after
        q_ptr->emit requestAccepted(r);

        removeRequest(r);
    }
}

void PendingTrustRequestModelPrivate::requestDiscarded()
{
    TrustRequest* r = qobject_cast<TrustRequest*>(sender());
    if (r) {
        // needs to be remove from pending requests
        if (auto p = r->person()) {
            p->remove();
        }

        // TODO is this needed? we will delete the request after
        q_ptr->emit requestDiscarded(r);

        removeRequest(r);
    }
}
