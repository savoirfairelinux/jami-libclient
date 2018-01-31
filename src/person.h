/****************************************************************************
 *   Copyright (C) 2009-2018 Savoir-faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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
#include <QtCore/QVariant>
#include <time.h>
#include <itembase.h>
#include <media/media.h>

//Ring
#include "itemdataroles.h"
class ContactMethod;
class PersonPrivate;
class AddressPrivate;
class Account;
class CollectionInterface;
class PersonPlaceHolderPrivate;

#include "typedefs.h"

///Person: Abstract version of a contact
class LIB_EXPORT Person : public ItemBase
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

   friend class ContactMethod;

public:

   enum class Role {
      Organization      = static_cast<int>(Ring::Role::UserRole) + 100,
      Group             ,
      Department        ,
      PreferredEmail    ,
      FormattedLastUsed ,
      IndexedLastUsed   ,
      DatedLastUsed     ,
      IdOfLastCMUsed    ,
      Object            ,
      Filter            , //All roles, all at once
      DropState         = static_cast<int>(Ring::Role::DropState), //State for drag and drop
   };

   ///@enum Encoding How to decode the person content payload
   enum class Encoding {
      UID  , /*!< The bytearray only has an unique identifier      */
      vCard, /*!< The bytearray contain a RFC 6868 compliant vCard */
   };

   ///Represent the physical address of a contact
   class Address {
      public:
         explicit Address();
         virtual ~Address();

         //Getters
         QString addressLine() const;
         QString city       () const;
         QString zipCode    () const;
         QString state      () const;
         QString country    () const;
         QString type       () const;

         //Setters
         void setAddressLine(const QString& value);
         void setCity       (const QString& value);
         void setZipCode    (const QString& value);
         void setState      (const QString& value);
         void setCountry    (const QString& value);
         void setType       (const QString& value);

      private:
         AddressPrivate* d_ptr;
   };

   typedef QVector<ContactMethod*> ContactMethods;

   //Properties
   Q_PROPERTY( ContactMethods        phoneNumbers   READ phoneNumbers   WRITE setContactMethods )
   Q_PROPERTY( QString               nickName       READ nickName       WRITE setNickName       )
   Q_PROPERTY( QString               firstName      READ firstName      WRITE setFirstName      )
   Q_PROPERTY( QString               secondName     READ secondName     WRITE setFamilyName     )
   Q_PROPERTY( QString               formattedName  READ formattedName  WRITE setFormattedName  )
   Q_PROPERTY( QString               organization   READ organization   WRITE setOrganization   )
   Q_PROPERTY( QByteArray            uid            READ uid            WRITE setUid            )
   Q_PROPERTY( QString               preferredEmail READ preferredEmail WRITE setPreferredEmail )
   Q_PROPERTY( QVariant              photo          READ photo          WRITE setPhoto          )
   Q_PROPERTY( QString               group          READ group          WRITE setGroup          )
   Q_PROPERTY( QString               department     READ department     WRITE setDepartment     )
   Q_PROPERTY( time_t                lastUsedTime   READ lastUsedTime                           )
   Q_PROPERTY( bool                  hasBeenCalled  READ hasBeenCalled                          )

   //Mutator
   Q_INVOKABLE void addAddress(const Address& addr);
   Q_INVOKABLE void addCustomField(const QString& key, const QString& value);
   Q_INVOKABLE const QByteArray toVCard(QList<Account*> accounts = {}) const;

protected:
   //The D-Pointer can be shared if a PlaceHolderPerson is merged with a real one
   PersonPrivate* d_ptr;
   Q_DECLARE_PRIVATE(Person)

   void replaceDPointer(Person* other);

public:
   //Constructors & Destructors
   explicit Person(CollectionInterface* parent = nullptr);
   Person(const QByteArray& content, Person::Encoding encoding = Encoding::UID, CollectionInterface* parent = nullptr);
   Person(const Person& other) noexcept;
   virtual ~Person();

   //Getters
   const  ContactMethods& phoneNumbers() const;
   const  QString& nickName         () const;
   const  QString& firstName        () const;
   const  QString& secondName       () const;
   const  QString& formattedName    () const;
   const  QString& organization     () const;
   const  QByteArray& uid           () const;
   const  QString& preferredEmail   () const;
   const  QVariant photo            () const;
   const  QString& group            () const;
   const  QString& department       () const;
   time_t lastUsedTime              () const;
   ContactMethod* lastUsedContactMethod() const;

   Q_INVOKABLE QVariant   roleData   (int role) const;
   Q_INVOKABLE QMimeData* mimePayload(        ) const;

   //Cache
   QString filterString            () const;

   //Number related getters (proxies)
   bool isPresent                  () const;
   bool isTracked                  () const;
   bool supportPresence            () const;
   bool isReachable                () const;
   bool isPlaceHolder              () const;
   bool hasBeenCalled              () const;

   bool hasRecording(Media::Media::Type type, Media::Media::Direction direction) const;

   //Setters
   void setContactMethods ( ContactMethods           );
   void setFormattedName  ( const QString&    name   );
   void setNickName       ( const QString&    name   );
   void setFirstName      ( const QString&    name   );
   void setFamilyName     ( const QString&    name   );
   void setOrganization   ( const QString&    name   );
   void setPreferredEmail ( const QString&    name   );
   void setGroup          ( const QString&    name   );
   void setDepartment     ( const QString&    name   );
   void setUid            ( const QByteArray& id     );
   void setPhoto          ( const QVariant&   photo  );
   void ensureUid         (                          );

   //Updates an existing contact from vCard info
   void updateFromVCard(const QByteArray& content);

   //Operator
   bool operator==(const Person* other) const;
   bool operator==(const Person& other) const;

private Q_SLOTS:
   void slotPresenceChanged(); //TODO remove

Q_SIGNALS:
   ///The presence status of a contact method changed
   void presenceChanged           ( ContactMethod* );
   ///The person presence status changed
   void statusChanged             ( bool           );
   ///The person properties changed
   void changed                   (                );
   ///The number of and/or the contact methods themselves have changed
   void phoneNumbersChanged       (                );
   ///The number of and/or the contact methods themselvesd are about to change
   void phoneNumbersAboutToChange (                );
   ///The person data were merged from another source
   void rebased                   ( Person*        );
   ///The last time there was an interaction with this person changed
   void lastUsedTimeChanged       ( long long      ) const;
   ///A new call used a ContactMethod associated with this Person
   void callAdded                 ( Call*          );

protected:
   //Presence secret methods
   void updatePresenceInformations(const QString& uri, bool status, const QString& message);
};

class LIB_EXPORT PersonPlaceHolder : public Person {
   Q_OBJECT
public:
   explicit PersonPlaceHolder(const QByteArray& uid);
   bool merge(Person* contact);
private:
   PersonPlaceHolderPrivate* d_ptr;
   Q_DECLARE_PRIVATE(PersonPlaceHolder)
};

Q_DECLARE_METATYPE(Person*)
