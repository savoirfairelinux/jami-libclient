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
      ACTION        = 102,
   };

   /**
    * All assets currently available for the item. An item can have multiple
    * assets. For example, a Call as a call, a contact method and probably a
    * contact.
    */
   enum class Asset {
      NONE           = 0x0 << 0, /*!< No assets                                    */
      CALL           = 0x1 << 0, /*!< A single live or history call                */
      CALL_MODEL     = 0x1 << 1, /*!< The current selected call from the CallModel */
      PERSON         = 0x1 << 2, /*!< A person, contact or profile                 */
      CONTACT_METHOD = 0x1 << 3, /*!< A contact method or a bookmark               */
      COUNT__
   };
   Q_FLAGS(Asset)

   ///If options are checkable or not
   enum class ActionStatfulnessLevel {
      UNISTATE  = 0, /*!< The action has no state beside being available or not                              */
      CHECKABLE = 1, /*!< The action can be (un)available and "checked"                                      */
      TRISTATE  = 2, /*!< The action can be (un)available and unchecked, partially checked and fully checked */
      COUNT__,
   };

   ///(End)user action, all possibility, not only state aware ones like "Action"
   enum class Action {
      //Call
      ACCEPT            , /*!< Pickup incoming call(s) or send                        */
      HOLD              , /*!< [Stateful] Hold (check) or Unhold (uncheck) call(s)    */
      MUTE_AUDIO        , /*!< [Stateful] Stop sending audio to call(s)               */
      MUTE_VIDEO        , /*!< [Stateful] Stop sending video to call(s)               */
      SERVER_TRANSFER   , /*!< [Stateful] Perform an unattended transfer              */
      RECORD            , /*!< [Stateful] Record the call(s) to .wav file(s)          */
      HANGUP            , /*!< Refuse an incoming call or hang up an in progress one  */
      JOIN              , /*!< [Stateful] Join all seclect calls into a conference    */
      TOGGLE_VIDEO      , /*!< Toggle the video media on that asset                   */

      //Contact
      ADD_CONTACT       , /*!< Add a new contact for that asset contact method        */
      ADD_TO_CONTACT    , /*!< Add the asset contact method to an existing  contact   */
      DELETE_CONTACT    , /*!< Delete the contact attached to the asset               */
      EMAIL_CONTACT     , /*!< Email the contact attached to the asset                */
      COPY_CONTACT      , /*!< Copy the vCard/HTML/Plain text contact data            */
      BOOKMARK          , /*!< Toogle the bookmarked state of that contact [method]   */
      VIEW_CHAT_HISTORY , /*!< View the text recording associated with the CM         */
      ADD_CONTACT_METHOD, /*!< Add a contact method to a contact                      */
      CALL_CONTACT      , /*!< Call this contact [method]                             */

      //Call model
      ADD_NEW           , /*!< Add a new call                                         */

      //History
      REMOVE_HISTORY    , /*!< Remove this asset from the history                     */

      //Multi selection

      //No selection
      COUNT__,
   };
   Q_ENUMS(Action)

   enum class Context {
      NONE          = 0x0 << 0, /*!< Nothing                                                    */
      MINIMAL       = 0x1 << 0, /*!< The bare minimum required to work with the asset           */
      RECOMMANDED   = 0x1 << 1, /*!< Commonly useful actions related to an asset                */
      ADVANCED      = 0x1 << 2, /*!< Uncommon actions that can be performed on the asset        */
      MANAGEMENT    = 0x1 << 3, /*!< Manage the data related to this (bookmark, add contact...) */
      CONTACT       = 0x1 << 4, /*!< Actions related to contacting this person (email, call...) */
      TREE_ELEMENTS = 0x1 << 5, /*!< All actions that require a second dimension to manage      */
   };
   Q_FLAGS(Context)

   Q_PROPERTY(QSortFilterProxyModel* activeActionModel READ activeActionModel)

   //Constructor
   explicit UserActionModel(Call* parent, const FlagPack<Context> c = FlagPack<Context>(Context::MINIMAL)| Context::RECOMMANDED);
   UserActionModel(CallModel* parent    , const FlagPack<Context> c = FlagPack<Context>(Context::MINIMAL)| Context::RECOMMANDED);
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

   //Operators
   UserActionModel* operator<<(UserActionModel::Action& action);

private:
   const QScopedPointer<UserActionModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(UserActionModel)

Q_SIGNALS:
   ///The list of currently available actions has changed
   void actionStateChanged();
};
Q_DECLARE_METATYPE(UserActionModel*)
Q_DECLARE_METATYPE(UserActionModel::Action)
DECLARE_ENUM_FLAGS(UserActionModel::Context)


UserActionModel* operator<<(UserActionModel* m,UserActionModel::Action action);

/**
 * "Java bean" used to avoid having a 5+ parameter pixmap delegate
 */
struct UserActionElement {
   UserActionModel::Action action     ;
   QList<Call*>            calls      ;
   Qt::CheckState          checkState ;
};

#endif
