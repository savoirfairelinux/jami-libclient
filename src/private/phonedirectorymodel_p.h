/****************************************************************************
 *   Copyright (C) 2013-2018 Savoir-faire Linux                          *
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

//Ring
class PhoneDirectoryModel;
#include "contactmethod.h"
#include "namedirectory.h"

//Internal data structures
///@struct NumberWrapper Wrap phone numbers to prevent collisions
struct NumberWrapper {
   QVector<ContactMethod*> numbers;
};

class MostPopularNumberModel final : public QAbstractListModel
{
   Q_OBJECT
public:
   explicit MostPopularNumberModel();

   //Model functions
   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;
   virtual bool          setData  ( const QModelIndex& index, const QVariant &value, int role)       override;

   void addRow();
   void reload();
};

class PhoneDirectoryModelPrivate final : public QObject
{
   Q_OBJECT
public:
   explicit PhoneDirectoryModelPrivate(PhoneDirectoryModel* parent);


   //Model columns
   enum class Columns {
      URI              = 0,
      TYPE             = 1,
      CONTACT          = 2,
      ACCOUNT          = 3,
      STATE            = 4,
      CALL_COUNT       = 5,
      WEEK_COUNT       = 6,
      TRIM_COUNT       = 7,
      HAVE_CALLED      = 8,
      LAST_USED        = 9,
      NAME_COUNT       = 10,
      TOTAL_SECONDS    = 11,
      POPULARITY_INDEX = 12,
      BOOKMARED        = 13,
      TRACKED          = 14,
      HAS_CERTIFICATE  = 15,
      PRESENT          = 16,
      PRESENCE_MESSAGE = 17,
      UID              = 18,
      REGISTERED_NAME  = 19,
   };


   //Helpers
   void indexNumber(ContactMethod* number, const QStringList& names   );
   void setAccount (ContactMethod* number,       Account*     account );
   ContactMethod* fillDetails(NumberWrapper* wrap, const URI& strippedUri, Account* account, Person* contact, const QString& type);

   //Attributes
   QVector<ContactMethod*>         m_lNumbers         ;
   QHash<QString,NumberWrapper*> m_hDirectory       ;
   QVector<ContactMethod*>         m_lPopularityIndex ;
   QMap<QString,NumberWrapper*>  m_lSortedNames     ;
   QMap<QString,NumberWrapper*>  m_hSortedNumbers   ;
   QHash<QString,NumberWrapper*> m_hNumbersByNames  ;
   bool                          m_CallWithAccount  ;
   MostPopularNumberModel*       m_pPopularModel    ;

   Q_DECLARE_PUBLIC(PhoneDirectoryModel)

private:
   PhoneDirectoryModel* q_ptr;

private Q_SLOTS:
   void slotCallAdded(Call* call);
   void slotChanged();
   void slotLastUsedChanged(time_t t);
   void slotContactChanged(Person* newContact, Person* oldContact);
   void slotRegisteredNameFound(const Account* account, NameDirectory::LookupStatus status, const QString& address, const QString& name);
   void slotContactMethodMerged(ContactMethod* other);

   //From DBus
   void slotNewBuddySubscription(const QString& uri, const QString& accountId, bool status, const QString& message);
};
