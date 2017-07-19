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

#include <QtCore/QAbstractTableModel>

#include <typedefs.h>

class Account;

class BootstrapModelPrivate;

class LIB_EXPORT BootstrapModel : public QAbstractTableModel
{
   Q_OBJECT
public:
   friend class Account;

   enum class Columns {
      HOSTNAME,
      PORT,
   };

   /// @enum BootstrapModel::EditAction Manage a BootstrapModel lifecycle
   enum class EditAction {
      SAVE   = 0, /*!< Save the model, if there is a conflict, use "ours"                    */
      MODIFY = 1, /*!< Notify the state machine that the data has changed                    */
      RELOAD = 2, /*!< Reload from the account hostname, if there is a conflict, use "their" */
      CLEAR  = 3, /*!< Remove all bootstrap servers                                          */
      RESET  = 4, /*!< Reset the model with default servers                                  */
      COUNT__
   };

   /// @enum BootstrapModel::EditState track the changes from both clients and daemon
   enum class EditState {
      LOADING   = 0, /*!< The bootstrap servers are being loaded, they are not ready yet */
      READY     = 1, /*!< Both side are synchronized                                     */
      MODIFIED  = 2, /*!< Our version differ from the remote one                         */
      OUTDATED  = 3, /*!< The remote version differ from ours                            */
      RELOADING = 4, /*!< During a reload                                                */
      RESETING =  5, /*!< During a reset                                                 */
      COUNT__
   };

   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role   )       override;
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual bool          removeRows  ( int row, int count, const QModelIndex& parent = QModelIndex())      override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation, int role = Qt::DisplayRole    ) const override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getter
   bool                      isCustom            () const;
   BootstrapModel::EditState editState           () const;

   //Mutator
   void reset          (                                   ); // DEPRECATED
   bool performAction  ( BootstrapModel::EditAction action );

   //Operator
   BootstrapModel* operator<<(BootstrapModel::EditAction& action);

   void reload();

private:
   explicit BootstrapModel(Account* a);
   virtual ~BootstrapModel();

   BootstrapModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(BootstrapModel)

};

BootstrapModel LIB_EXPORT *operator<<(BootstrapModel* a, BootstrapModel::EditAction action);
