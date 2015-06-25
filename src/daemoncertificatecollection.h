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
#ifndef DAEMONCERTIFICATECOLLECTION_H
#define DAEMONCERTIFICATECOLLECTION_H

#include <collectioninterface.h>
#include <typedefs.h>

class Call;
class Person;
class DaemonCertificateCollectionPrivate;
class Certificate;
class Account;

template<typename T> class CollectionMediator;

/**
 * This class is used when no other Person collections are available. It will
 * provide a basic collection to add and use Persons/Contacts. It is also
 * necessary as some metadata required for elements such as auto completion are
 * provided by the Person collections.
 */
class LIB_EXPORT DaemonCertificateCollection : public CollectionInterface
{

public:

   enum class Mode {
      ALLOWED,
      BANNED
   };

   explicit DaemonCertificateCollection(CollectionMediator<Certificate>* mediator, Account* a, Mode mode);
   virtual ~DaemonCertificateCollection();

   virtual bool load  () override;
   virtual bool reload() override;
   virtual bool clear () override;

   virtual QString    name     () const override;
   virtual QString    category () const override;
   virtual QVariant   icon     () const override;
   virtual bool       isEnabled() const override;
   virtual QByteArray id       () const override;

   virtual FlagPack<SupportedFeatures> supportedFeatures() const override;

private:
   DaemonCertificateCollectionPrivate* d_ptr;
   Q_DECLARE_PRIVATE(DaemonCertificateCollection)
};
Q_DECLARE_METATYPE(DaemonCertificateCollection*)

#endif
