/****************************************************************************
 *   Copyright (C) 2012-2018 Savoir-faire Linux                          *
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

#include <QtCore/QAbstractListModel>

#include "typedefs.h"

//Qt
#include <QtCore/QString>
class QItemSelectionModel;

//Ring
#include <credential.h>

class CredentialModelPrivate;
class Account;

///CredentialModel: A model for account credentials
class LIB_EXPORT CredentialModel : public QAbstractItemModel {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:
   //Roles
   enum Role {
      NAME     = 100,
      PASSWORD = 101,
      REALM    = 102,
   };

   /// @enum CredentialModel::Action Manage a CredentialModel lifecycle
   enum class EditAction {
      SAVE   = 0, /*!< Save the model, if there is a conflict, use "ours"          */
      MODIFY = 1, /*!< Notify the state machine that the data has changed          */
      RELOAD = 2, /*!< Reload from the daemon, if there is a conflict, use "their" */
      CLEAR  = 3, /*!< Remove all credentials                                      */
      COUNT__
   };

   /// @enum CredentialModel::EditState track the changes from both clients and daemon
   enum class EditState {
      LOADING  = 0, /*!< The credentials are being loaded, they are not ready yet  */
      READY    = 1, /*!< Both side are synchronized                                */
      MODIFIED = 2, /*!< Our version differ from the remote one                    */
      OUTDATED = 3, /*!< The remote version differ from ours                       */
      COUNT__
   };

   //Constructor
   explicit CredentialModel(Account* acc);
   virtual ~CredentialModel();

   //Abstract model member
   virtual QVariant              data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int                   rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags         flags       ( const QModelIndex& index                                    ) const override;
   virtual bool                  setData     ( const QModelIndex& index, const QVariant &value, int role   )       override;
   virtual QModelIndex           parent      ( const QModelIndex& index                                    ) const override;
   virtual int                   columnCount ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual QModelIndex           index       ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QHash<int,QByteArray> roleNames(                                                                ) const override;

   //Mutator
   QModelIndex addCredentials(Credential::Type type);
   QModelIndex addCredentials();
   void removeCredentials(const QModelIndex& idx);
   bool performAction(CredentialModel::EditAction action);

   //Getter
   CredentialModel::EditState editState() const;
   QAbstractItemModel* availableTypeModel() const;
   QItemSelectionModel* availableTypeSelectionModel() const;
   Credential* primaryCredential(Credential::Type type = Credential::Type::SIP);

   //Operator
   CredentialModel* operator<<(CredentialModel::EditAction& action);

Q_SIGNALS:
   void primaryCredentialChanged(Credential::Type type, Credential* c);

private:
   QScopedPointer<CredentialModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(CredentialModel)
};
Q_DECLARE_METATYPE(CredentialModel*)

CredentialModel LIB_EXPORT *operator<<(CredentialModel* a, CredentialModel::EditAction action);
