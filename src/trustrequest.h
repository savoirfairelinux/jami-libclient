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
#ifndef TRUSTREQUEST_H
#define TRUSTREQUEST_H

#include <QtCore/QObject>
#include <typedefs.h>

class TrustRequestPrivate;
class AccountModel;
class AccountModelPrivate;
class Certificate;
class Account;
class AccountPrivate;

class LIB_EXPORT TrustRequest : public QObject
{
   Q_OBJECT

friend class AccountModel;
friend class AccountModelPrivate;
friend class AccountPrivate;

public:

   //Getter
   Certificate* certificate() const;
   QDateTime    date       () const;
   Account*     account    () const;

   //Mutator
   Q_INVOKABLE bool accept ();
   Q_INVOKABLE bool discard();

private:
   explicit TrustRequest(Account* a, const QString& id, time_t time);
   virtual ~TrustRequest();

   TrustRequestPrivate* d_ptr;
   Q_DECLARE_PRIVATE(TrustRequest)

Q_SIGNALS:
   void requestAccepted ();
   void requestDiscarded();
};

#endif
