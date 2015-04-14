/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
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
#ifndef CREDENTIAL_MODEL_H
#define CREDENTIAL_MODEL_H

#include <QtCore/QString>
#include <QtCore/QAbstractListModel>
#include "typedefs.h"


class CredentialModelPrivate;
class Account;

///CredentialModel: A model for account credentials
class LIB_EXPORT CredentialModel : public QAbstractListModel {
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
      READY    = 1, /*!< Both side are synchronised                                */
      MODIFIED = 2, /*!< Our version differ from the remote one                    */
      OUTDATED = 3, /*!< The remote version differ from ours                       */
      COUNT__
   };

   //Constructor
   explicit CredentialModel(Account* acc);
   virtual ~CredentialModel();

   //Abstract model member
   QVariant                      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   int                           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   Qt::ItemFlags                 flags    ( const QModelIndex& index                                 ) const override;
   virtual bool                  setData  ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames(                                                          ) const override;

   //Mutator
   QModelIndex addCredentials();
   void removeCredentials(const QModelIndex& idx);
   bool performAction(CredentialModel::EditAction action);

   //Getter
   CredentialModel::EditState editState() const;

   //Operator
   CredentialModel* operator<<(CredentialModel::EditAction& action);

private:
   QScopedPointer<CredentialModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(CredentialModel)
};
Q_DECLARE_METATYPE(CredentialModel*)

CredentialModel* operator<<(CredentialModel* a, CredentialModel::EditAction action);

#endif
