/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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
#ifndef PHONENUMBER_H
#define PHONENUMBER_H

#include "typedefs.h"
#include <time.h>
#include <itembase.h>

//Qt
#include <QStringList>
#include <QtCore/QSize>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>

//Ring
#include "uri.h"
class Account;
class Person;
class Call;
class ContactMethodPrivate;
class TemporaryContactMethod;
class NumberCategory;


///ContactMethod: represent a phone number
class LIB_EXPORT ContactMethod : public ItemBase<QObject>
{
   Q_OBJECT
public:
   friend class PhoneDirectoryModel;
   friend class PhoneDirectoryModelPrivate;

   enum class Role {
      Uri          = 1000,
      Object       = 1001,
      CategoryIcon = 1002,
      //TODO implement all others
   };

   //Properties
   Q_PROPERTY(Account*      account          READ account           WRITE setAccount              )
   Q_PROPERTY(Person*       person           READ contact           WRITE setPerson               )
   Q_PROPERTY(int           lastUsed         READ lastUsed                                        )
   Q_PROPERTY(QString       uri              READ uri                                             )
   Q_PROPERTY(int           callCount        READ callCount                                       )
   Q_PROPERTY(QList<Call*>  calls            READ calls                                           )
   Q_PROPERTY(int           popularityIndex  READ popularityIndex                                 )
   Q_PROPERTY(bool          bookmarked       READ isBookmarked                                    )
   Q_PROPERTY(QString       uid              READ uid               WRITE setUid                  )
   Q_PROPERTY(bool          isTracked        READ isTracked         NOTIFY trackedChanged         )
   Q_PROPERTY(bool          isPresent        READ isPresent         NOTIFY presentChanged         )
   Q_PROPERTY(bool          supportPresence  READ supportPresence                                 )
   Q_PROPERTY(QString       presenceMessage  READ presenceMessage   NOTIFY presenceMessageChanged )
   Q_PROPERTY(uint          weekCount        READ weekCount                                       )
   Q_PROPERTY(uint          trimCount        READ trimCount                                       )
   Q_PROPERTY(bool          haveCalled       READ haveCalled                                      )
   Q_PROPERTY(QString       primaryName      READ primaryName                                     )
   Q_PROPERTY(bool          isBookmarked     READ isBookmarked                                    )
   Q_PROPERTY(QVariant      icon             READ icon                                            )
   Q_PROPERTY(int           totalSpentTime   READ totalSpentTime                                  )

//    Q_PROPERTY(QHash<QString,int> alternativeNames READ alternativeNames         )

   ///@enum Type: Is this temporary, blank, used or unused
   enum class Type {
      BLANK     = 0, /*!< This number represent no number                                  */
      TEMPORARY = 1, /*!< This number is not yet complete                                  */
      USED      = 2, /*!< This number have been called before                              */
      UNUSED    = 3, /*!< This number have never been called, but is in the address book   */
      ACCOUNT   = 4, /*!< This number correspond to the URI of a SIP account               */
   };
   Q_ENUMS(Type)

   //Getters
   URI                 uri             () const;
   NumberCategory*     category        () const;
   bool                isTracked       () const;
   bool                isPresent       () const;
   QString             presenceMessage () const;
   Account*            account         () const;
   Person*             contact         () const;
   time_t              lastUsed        () const;
   ContactMethod::Type type            () const;
   int                 callCount       () const;
   uint                weekCount       () const;
   uint                trimCount       () const;
   bool                haveCalled      () const;
   QList<Call*>        calls           () const;
   int                 popularityIndex () const;
   QHash<QString,int>  alternativeNames() const;
   QString             primaryName     () const;
   bool                isBookmarked    () const;
   bool                supportPresence () const;
   QVariant            icon            () const;
   int                 totalSpentTime  () const;
   QString             uid             () const;

   QVariant roleData(int role) const;

   //Setters
   Q_INVOKABLE void setAccount   (Account*            account   );
   Q_INVOKABLE void setPerson    (Person*             contact   );
   Q_INVOKABLE void setTracked   (bool                track     );
   void             setCategory  (NumberCategory*     cat       );
   void             setBookmarked(bool                bookmarked);
   void             setUid       (const QString&      uri       );
   bool             setType      (ContactMethod::Type t         );

   //Mutator
   Q_INVOKABLE void addCall(Call* call);
   Q_INVOKABLE void incrementAlternativeName(const QString& name);

   //Static
   static const ContactMethod* BLANK();

   //Helper
   QString toHash() const;

   //Operator
   bool operator==(ContactMethod* other);
   bool operator==(const ContactMethod* other) const;
   bool operator==(ContactMethod& other);
   bool operator==(const ContactMethod& other) const;

protected:
   //Constructor
   ContactMethod(const URI& uri, NumberCategory* cat, Type st = Type::UNUSED);
   virtual ~ContactMethod();

   //Private setters
   void setPresent(bool present);
   void setPresenceMessage(const QString& message);

   //PhoneDirectoryModel mutator
   bool merge(ContactMethod* other);

   //Getter
   bool hasType() const;
   int  index() const;

   //Setter
   void setHasType(bool value);
   void setIndex(int value);
   void setPopularityIndex(int value);

   //Many phone numbers can have the same "d" if they were merged
   QSharedPointer<ContactMethodPrivate> d_ptr;

private:
   friend class ContactMethodPrivate;

   //Static attributes
   static QHash<int,Call*> m_shMostUsed  ;
   static const ContactMethod* m_spBlank   ;

private Q_SLOTS:
   void accountDestroyed(QObject* o);
   void contactRebased(Person* other);

Q_SIGNALS:
   ///A new call have used this ContactMethod
   void callAdded             ( Call* call          );
   ///A property associated with this number has changed
   void changed               (                     );
   ///The presence status of this phone number has changed
   void presentChanged        ( bool                );
   ///The presence status message associated with this number
   void presenceMessageChanged( const QString&      );
   ///This number track presence
   void trackedChanged        ( bool                );
   /**
    * The name used to be represent this number has changed
    * It is important for user of this object to track this
    * as the name will change over time as new contact
    * sources are added
    */
   void primaryNameChanged    ( const QString& name );
   /**
    * Two previously independent number have been merged
    * this happen when new information cues prove that number
    * with previously ambiguous data
    */
   void rebased               ( ContactMethod* other  );
};

Q_DECLARE_METATYPE(ContactMethod*)

///@class TemporaryContactMethod: An incomplete phone number
class LIB_EXPORT TemporaryContactMethod : public ContactMethod {
   Q_OBJECT
public:
   explicit TemporaryContactMethod(const ContactMethod* number = nullptr);
   void setUri(const QString& uri);
};

#endif
