/****************************************************************************
 *   Copyright (C) 2015-2017 Savoir-faire Linux                               *
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
#pragma once

#include <QtCore/QObject>
#include <typedefs.h>

class ContactRequestPrivate;
class AccountModel;
class AccountModelPrivate;
class Certificate;
class Account;
class AccountPrivate;
class Person;

class LIB_EXPORT ContactRequest : public QObject
{
   Q_OBJECT

friend class Account;
friend class AccountModelPrivate;
friend class AccountPrivate;

public:

   //Getter
   Certificate* certificate() const;
   QDateTime    date       () const;
   Account*     account    () const;
   Q_INVOKABLE QVariant roleData (int role) const;
   Person* peer() const;

   // Setter
   void setPeer(Person* person);

   //Mutator
   Q_INVOKABLE bool accept ();
   Q_INVOKABLE bool discard();
   Q_INVOKABLE void block();

   // Operator
   bool operator==(const ContactRequest& another) const;

private:
   explicit ContactRequest(Account* a, Person* p, const QString& id = QString(), time_t time = -1);
   virtual ~ContactRequest();

   ContactRequestPrivate* d_ptr;
   Q_DECLARE_PRIVATE(ContactRequest)

Q_SIGNALS:
   void requestAccepted ();
   void requestDiscarded();
   void requestBlocked();
};
