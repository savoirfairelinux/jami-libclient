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
#include "contactrequest.h"

//Qt
#include <QtCore/QDateTime>

//Ring
#include <account.h>
#include <certificate.h>
#include <certificatemodel.h>
#include "itembase.h"
#include "personmodel.h"

//DRing
#include "dbus/configurationmanager.h"

class ContactRequestPrivate
{
public:
   //Attributes
   QDateTime    m_Time        ;
   Certificate* m_pCertificate;
   Account*     m_pAccount    ;
   Person*      m_pPeer       ;
};

ContactRequest::ContactRequest(Account* a, Person* p, const QString& id, time_t time) : QObject(a), d_ptr(new ContactRequestPrivate)
{
   d_ptr->m_pAccount     = a;
   d_ptr->m_pPeer        = p;
   d_ptr->m_Time         = QDateTime::fromTime_t(time);
   d_ptr->m_pCertificate = CertificateModel::instance().getCertificateFromId(id, a);
}

ContactRequest::~ContactRequest()
{
   delete d_ptr;
}

Certificate* ContactRequest::certificate() const
{
   return d_ptr->m_pCertificate;
}

QDateTime ContactRequest::date() const
{
   return d_ptr->m_Time;
}

Account* ContactRequest::account() const
{
   return d_ptr->m_pAccount;
}

/**
 * get the person associated to the contact request.
 */
Person*
ContactRequest::peer() const
{
   return d_ptr->m_pPeer;
}

/**
 * set the person associated to the contact request.
 */
void
ContactRequest::setPeer(Person* person)
{
   d_ptr->m_pPeer = person;
}

bool ContactRequest::accept()
{
   if (ConfigurationManager::instance().acceptTrustRequest(d_ptr->m_pAccount->id(), d_ptr->m_pCertificate->remoteId())) {
      PersonModel::instance().addPeerProfile(peer());
      emit requestAccepted();
      return true;
   }
   return false;
}

bool ContactRequest::discard()
{
   if (ConfigurationManager::instance().discardTrustRequest(d_ptr->m_pAccount->id(), d_ptr->m_pCertificate->remoteId())) {
      emit requestDiscarded();
      return true;
   }
   return false;
}

/**
 * block the contact request.
 */
void
ContactRequest::block()
{
   ConfigurationManager::instance().removeContact(d_ptr->m_pAccount->id(), d_ptr->m_pCertificate->remoteId());
   emit requestBlocked();
}

/**
 * get the data by role selection
 * @param role define the role to select
 * @return a QVariant object, wich contains the selection
 */
QVariant
ContactRequest::roleData(int role) const
{
    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return certificate()->remoteId();
        case static_cast<int>(Ring::Role::Object):
            return QVariant::fromValue(const_cast<ContactRequest*>(this));
        case static_cast<int>(Ring::Role::ObjectType):
            return QVariant::fromValue(Ring::ObjectType::ContactRequest);
    }

    /* unknown role */
    return QVariant();
}

/**
 * defines the comparaison operator
 */
bool
ContactRequest::operator==(const ContactRequest& another) const
{
   bool certificate = d_ptr->m_pCertificate == another.d_ptr->m_pCertificate;
   bool account = d_ptr->m_pAccount == another.d_ptr->m_pAccount;
   bool time = d_ptr->m_Time == another.d_ptr->m_Time;

   return account && certificate && time;
}
