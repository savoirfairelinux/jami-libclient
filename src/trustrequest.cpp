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
#include "trustrequest.h"

//Qt
#include <QtCore/QDateTime>

//Ring
#include <account.h>
#include <certificate.h>
#include <certificatemodel.h>

//DRing
#include "dbus/configurationmanager.h"

class TrustRequestPrivate
{
public:
   //Attributes
   QDateTime    m_Time        ;
   Certificate* m_pCertificate;
   Account*     m_pAccount    ;
};

TrustRequest::TrustRequest(Account* a, const QString& id, time_t time) : QObject(a), d_ptr(new TrustRequestPrivate)
{
   d_ptr->m_pAccount     = a;
   d_ptr->m_Time         = QDateTime::fromTime_t(time);
   d_ptr->m_pCertificate = CertificateModel::instance()->getCertificateFromId(id, a);
}

TrustRequest::~TrustRequest()
{
   delete d_ptr;
}

Certificate* TrustRequest::certificate() const
{
   return d_ptr->m_pCertificate;
}

QDateTime TrustRequest::date() const
{
   return d_ptr->m_Time;
}

Account* TrustRequest::account() const
{
   return d_ptr->m_pAccount;
}

bool TrustRequest::accept()
{
   if (DBus::ConfigurationManager::instance().acceptTrustRequest(d_ptr->m_pAccount->id(), d_ptr->m_pCertificate->remoteId())) {
      emit requestAccepted();
      return true;
   }
   return false;
}

bool TrustRequest::discard()
{
   if (DBus::ConfigurationManager::instance().discardTrustRequest(d_ptr->m_pAccount->id(), d_ptr->m_pCertificate->remoteId())) {
      emit requestDiscarded();
      return true;
   }
   return false;
}
