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
#ifndef USERACTIONMODEL_H
#define USERACTIONMODEL_H

#include <QtCore/QString>
#include <QtCore/QAbstractItemModel>
#include "typedefs.h"

//Qt
class QSortFilterProxyModel;

//Ring
#include "call.h"
class Call;
class CallModel;
class UserActionModelPrivate;

/**
 * @class UserActionModel Hold available actions for a given call state
 *
 **/
class LIB_EXPORT UserActionModel : public QAbstractListModel {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:

   //Roles
   enum Role {
      VISIBLE       = 100,
      RELATIVEINDEX = 101,
   };

   ///If options are checkable or not
   enum class ActionStatfulnessLevel {
      UNISTATE  = 0, /*!< The action has no state beside being available or not                              */
      CHECKABLE = 1, /*!< The action can be (un)available and "checked"                                      */
      TRISTATE  = 2, /*!< The action can be (un)available and unchecked, partially checked and fully checked */
      COUNT__,
   };

   ///(End)user action, all possibility, not only state aware ones like "Action"
   enum class Action {
      //Uni selection
      ACCEPT          , /*!< Pickup incoming call(s) or send                        */
      HOLD            , /*!< [Stateful] Hold (check) or Unhold (uncheck) call(s)    */
      MUTE_AUDIO      , /*!< [Stateful] Stop sending audio to call(s)               */
      MUTE_VIDEO      , /*!< [Stateful] Stop sending video to call(s)               */
      SERVER_TRANSFER , /*!< [Stateful] Perform an unattended transfer              */
      RECORD          , /*!< [Stateful] Record the call(s) to .wav file(s)          */
      HANGUP          , /*!< Resuse an incomming call or hang up an in progress one */

      //Multi selection
      JOIN            , /*!< [Stateful] Join all seclect calls into a conference    */
      COUNT__,
   };
   Q_ENUMS(Action)

   Q_PROPERTY(QSortFilterProxyModel* activeActionModel READ activeActionModel);

   //Constructor
   explicit UserActionModel(Call* parent);
   UserActionModel(CallModel* parent);
   virtual ~UserActionModel();

   //Abstract model members
   virtual QVariant      data       (const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount   (const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags      (const QModelIndex& index                                 ) const override;
   virtual bool          setData    (const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getters
   Q_INVOKABLE bool isActionEnabled ( UserActionModel::Action action ) const;
   Q_INVOKABLE uint relativeIndex   ( UserActionModel::Action action ) const;
   QSortFilterProxyModel* activeActionModel() const;

   //Mutators
   bool execute( const Action action    ) const;
   bool execute( const QModelIndex& idx ) const;

private:
   const QScopedPointer<UserActionModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(UserActionModel)

Q_SIGNALS:
   ///The list of currently available actions has changed
   void actionStateChanged();
};
Q_DECLARE_METATYPE(UserActionModel*)


/**
 * "Java bean" used to avoid having a 5+ parameter pixmap delegate
 */
struct UserActionElement {
   UserActionModel::Action action     ;
   QList<Call*>            calls      ;
   Qt::CheckState          checkState ;
};

#endif
