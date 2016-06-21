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
#pragma once

//Qt
#include <QtCore/QObject>

//Ring
#include "account.h"

class PendingTrustRequestModel;
class TrustRequest;
class CollectionInterface;
class Person;

class PendingTrustRequestModelPrivate final : public QObject
{
    Q_OBJECT
public:
   //Constructor
   explicit PendingTrustRequestModelPrivate(PendingTrustRequestModel* parent);
   ~PendingTrustRequestModelPrivate();

   //Attributes
   QVector<TrustRequest*>        m_lRequests;
   Account*                      m_pAccount ;
   QHash<const Person*, TrustRequest*> m_hPersonsToTrustRequests;

   //Pending person collection from which the trust requests will be reconstructed/tied to
   // is this a hack? I don't know...
   CollectionInterface*          m_pPendingCollection;

   //Helper
   void newTrustRequest                  (Account *a, const QString& hash, const QByteArray& payload, time_t time);
   void addRequest                       (TrustRequest* r);
   void removeRequest                    (TrustRequest* r);
   CollectionInterface* pendingCollection();

private:
   PendingTrustRequestModel* q_ptr;

public Q_SLOTS:
    void requestAccepted();
    void requestDiscarded();
};
