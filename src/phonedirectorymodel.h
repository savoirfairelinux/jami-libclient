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

#include "typedefs.h"

// STD
#include <functional>

//Qt
#include <QtCore/QString>
#include <QtCore/QAbstractTableModel>

//Ring
#include "uri.h"
class ContactMethod         ;
class Person              ;
class Account             ;
class Call                ;
class TemporaryContactMethod;
class NumberTreeBackend;

//Private
class PhoneDirectoryModelPrivate;

///CredentialModel: A model for account credentials
class LIB_EXPORT PhoneDirectoryModel : public QAbstractTableModel {

   //NumberCompletionModel need direct access to the indexes
   friend class NumberCompletionModel;
   friend class NumberCompletionModelPrivate;
   friend class MostPopularNumberModel;
   friend class CategorizedBookmarkModel;

   //Friend unit test class
   friend class AutoCompletionTest;

   //Phone number need to update the indexes as they change
   friend class ContactMethod;

   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:
   Q_PROPERTY(int count READ count )

   enum class Role {
      Object = 100,
   };

   virtual ~PhoneDirectoryModel();

   //Abstract model members
   virtual QVariant      data       (const QModelIndex& index, int role = Qt::DisplayRole                 ) const override;
   virtual int           rowCount   (const QModelIndex& parent = QModelIndex()                            ) const override;
   virtual int           columnCount(const QModelIndex& parent = QModelIndex()                            ) const override;
   virtual Qt::ItemFlags flags      (const QModelIndex& index                                             ) const override;
   virtual bool          setData    (const QModelIndex& index, const QVariant &value, int role            )       override;
   virtual QVariant      headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Singleton
   static PhoneDirectoryModel& instance();

   //Factory
   Q_INVOKABLE ContactMethod* getNumber(const QString& uri, const QString& type = QString());
   Q_INVOKABLE ContactMethod* getNumber(const URI& uri,     const QString& type = QString());
   Q_INVOKABLE ContactMethod* getNumber(const QString& uri, Account* account, const QString& type = QString());
   Q_INVOKABLE ContactMethod* getNumber(const QString& uri, Person* contact, Account* account = nullptr, const QString& type = QString());
   Q_INVOKABLE ContactMethod* fromHash (const QString& hash);
   Q_INVOKABLE ContactMethod* fromTemporary(const TemporaryContactMethod* number);

   ContactMethod* getExistingNumberIf(const URI& uri, const std::function<bool(const ContactMethod*)>& pred) const;

   //Getter
   int count() const;
   bool callWithAccount() const;
   QAbstractListModel* mostPopularNumberModel() const;
   bool hasUnreadMessage() const;

   //Setters
   void setCallWithAccount(bool value);

   //Static
   QVector<ContactMethod*> getNumbersByPopularity() const;

private:
   //Constructor
   explicit PhoneDirectoryModel(QObject* parent = nullptr);

   //Attributes
   QScopedPointer<PhoneDirectoryModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(PhoneDirectoryModel)

Q_SIGNALS:
   void lastUsedChanged(ContactMethod* cm, time_t t);
   void contactChanged(ContactMethod* cm, Person* newContact, Person* oldContact);
   void contactMethodMerged(ContactMethod* cm, ContactMethod* into);
};
Q_DECLARE_METATYPE(PhoneDirectoryModel*)
