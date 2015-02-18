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
#include "useractionmodel.h"

//Qt
#include <QtCore/QItemSelection>

//Ring
#include "call.h"
#include "callmodel.h"
#include "account.h"
#include "accountmodel.h"

class UserActionModelPrivate : public QObject
{
   Q_OBJECT
public:
   enum class SelectionState {
      NONE  , /*!< No Item is selected         */
      UNIQUE, /*!< A single item is selected   */
      MULTI , /*!< Multiple items are selected */
      COUNT__,
   };

   enum class UserActionModelMode {
      CALL     , /*!< Model the react on a single call element    */
      CALLMODEL, /*!< Model that react on the callmodel selection */
   };

   //Availability matrices
   UserActionModelPrivate(UserActionModel* parent);
   static const TypedStateMachine< TypedStateMachine< bool                                    , Call::State                > , UserActionModel::Action > availableActionMap;
   static const TypedStateMachine< TypedStateMachine< bool                                    , Account::RegistrationState > , UserActionModel::Action > availableAccountActionMap;
   static const TypedStateMachine< TypedStateMachine< bool                                    , SelectionState             > , UserActionModel::Action > multi_call_options;
   static const TypedStateMachine< TypedStateMachine< bool                                    , Account::Protocol          > , UserActionModel::Action > availableProtocolActions;
   static const TypedStateMachine< TypedStateMachine< UserActionModel::ActionStatfulnessLevel , SelectionState             > , UserActionModel::Action > actionStatefulness;

   //Helpers
   void updateActions();
   bool updateAction(UserActionModel::Action action);
   bool updateByCall(UserActionModel::Action action, const Call* c);

   //Attribues
   Call*               m_pCall;
   UserActionModelMode m_Mode ;
   SelectionState      m_SelectionState;
   TypedStateMachine< bool, UserActionModel::Action> m_CurrentActions;
   static const TypedStateMachine< QString, UserActionModel::Action> m_ActionNames;

private:
   UserActionModel* q_ptr;

public Q_SLOTS:
   //Single call mode
   void slotStateChanged();

   //CallModel mode
   void slotCurrentChanged      (const QModelIndex&    current , const QModelIndex&    previous  );
   void slotSelectionChanged    (const QItemSelection& selected, const QItemSelection& deselected);
   void slotCallStateChanged    ( Call* call, Call::State previousState                          );

   //Common
   void slotAccountStateChanged ( Account* account, const Account::RegistrationState state       );
};


/*
 * This algorithm implement a matrix multiplication of the 4 sources of relevant states.
 * The result of the multiplication indicate is the option is enabled in a given scenario.
 */

//Enabled actions
const TypedStateMachine< TypedStateMachine< bool , Call::State > , UserActionModel::Action > UserActionModelPrivate::availableActionMap = {{
 /*                   INCOMING  RINGING CURRENT DIALING  HOLD  FAILURE BUSY  TRANSFERRED TRANSF_HOLD  OVER  ERROR CONFERENCE CONFERENCE_HOLD:*/
 /*ACCEPT          */ {{ true   , true ,  false,  true , false, false, false,   false,     false,    false, false,  false,      false    }},
 /*HOLD            */ {{ false  , false,  true ,  false, true , false, false,   false,     false,    false, false,  true ,      false    }},
 /*MUTE_AUDIO      */ {{ false  , true ,  true ,  false, false, false, false,   false,     false,    false, false,  false,      false    }},
 /*MUTE_VIDEO      */ {{ false  , true ,  true ,  false, false, false, false,   false,     false,    false, false,  false,      false    }},
 /*SERVER_TRANSFER */ {{ false  , false,  true ,  false, true , false, false,   false,     false,    false, false,  false,      false    }},
 /*RECORD          */ {{ false  , true ,  true ,  false, true , false, false,   true ,     true ,    false, false,  true ,      true     }},
 /*HANGUP          */ {{ true   , true ,  true ,  true , true , true , true ,   true ,     true ,    false, true ,  true ,      true     }},

 /*JOIN            */ {{ false  , true ,  true ,  false, true , false, false,   true ,     true ,    false, false,  true ,      true     }},
}};

/**
 * Assuming a call is in progress, the communication can still be valid if the account is down, however,
 * this will impact the available actions
 */
const TypedStateMachine< TypedStateMachine< bool , Account::RegistrationState > , UserActionModel::Action > UserActionModelPrivate::availableAccountActionMap = {{
   /*                       READY  UNREGISTERED  TRYING    ERROR  */
   /* ACCEPT          */ {{ true ,    false,     false,    false  }},
   /* HOLD            */ {{ true ,    false,     false,    false  }},
   /* MUTE_AUDIO      */ {{ true ,    true ,     true ,    true   }},
   /* MUTE_VIDEO      */ {{ true ,    true ,     true ,    true   }},
   /* SERVER_TRANSFER */ {{ true ,    false,     false,    false  }},
   /* RECORD          */ {{ true ,    true ,     true ,    true   }},
   /* HANGUP          */ {{ true ,    true ,     true ,    true   }},

   /* JOIN            */ {{ true ,    true ,     true ,    true   }},
}};

const TypedStateMachine< TypedStateMachine< bool , UserActionModelPrivate::SelectionState > , UserActionModel::Action > UserActionModelPrivate::multi_call_options = {{
   /*                       NONE   UNIQUE   MULTI  */
   /* ACCEPT          */ {{ false,  true ,  true  }},
   /* HOLD            */ {{ false,  true ,  true  }},
   /* MUTE_AUDIO      */ {{ false,  true ,  true  }},
   /* MUTE_VIDEO      */ {{ false,  true ,  true  }},
   /* SERVER_TRANSFER */ {{ false,  true ,  true  }},
   /* RECORD          */ {{ false,  true ,  true  }},
   /* HANGUP          */ {{ false,  true ,  true  }},

   /* JOIN            */ {{ false,  false,  true  }},
}};

const TypedStateMachine< TypedStateMachine< bool , Account::Protocol > , UserActionModel::Action > UserActionModelPrivate::availableProtocolActions = {{
   /*                        SIP   IAX    DHT    */
   /* ACCEPT          */ {{ true , true , true  }},
   /* HOLD            */ {{ true , true , true  }},
   /* MUTE_AUDIO      */ {{ true , true , true  }},
   /* MUTE_VIDEO      */ {{ true , true , true  }},
   /* SERVER_TRANSFER */ {{ true , false, false }},
   /* RECORD          */ {{ true , true , true  }},
   /* HANGUP          */ {{ true , true , true  }},

   /* JOIN            */ {{ true , true , true  }},
}};

#define ST UserActionModel::ActionStatfulnessLevel::
const TypedStateMachine< TypedStateMachine< UserActionModel::ActionStatfulnessLevel , UserActionModelPrivate::SelectionState > , UserActionModel::Action > UserActionModelPrivate::actionStatefulness = {{
   /*                           NONE         UNIQUE             MULTI     */
   /* ACCEPT          */ {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }},
   /* HOLD            */ {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }},
   /* MUTE_AUDIO      */ {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }},
   /* MUTE_VIDEO      */ {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }},
   /* SERVER_TRANSFER */ {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }},
   /* RECORD          */ {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }},
   /* HANGUP          */ {{ ST UNISTATE,  ST UNISTATE     ,  ST TRISTATE  }},
   /* JOIN            */ {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }},
}};
#undef ST

const TypedStateMachine< QString, UserActionModel::Action> UserActionModelPrivate::m_ActionNames = {{
   /* ACCEPT          */ QObject::tr("ACCEPT"            ), //TODO use better (and stateful) names
   /* HOLD            */ QObject::tr("HOLD"              ),
   /* MUTE_AUDIO      */ QObject::tr("MUTE_AUDIO"        ),
   /* MUTE_VIDEO      */ QObject::tr("MUTE_VIDEO"        ),
   /* SERVER_TRANSFER */ QObject::tr("SERVER_TRANSFER"   ),
   /* RECORD          */ QObject::tr("RECORD"            ),
   /* HANGUP          */ QObject::tr("HANGUP"            ),
   /* JOIN            */ QObject::tr("JOIN"              ),
}};

UserActionModelPrivate::UserActionModelPrivate(UserActionModel* parent) : QObject(parent),q_ptr(parent),m_pCall(nullptr)
{
}

/**
 * Create an UserActionModel around a single call. This wont take advantage
 * of the multiselection feature.
 */
UserActionModel::UserActionModel(Call* parent) : QAbstractListModel(parent),d_ptr(new UserActionModelPrivate(this))
{
   Q_ASSERT(parent != nullptr);
   Q_ASSERT(parent->state() != Call::State::OVER);
   d_ptr->m_SelectionState = UserActionModelPrivate::SelectionState::UNIQUE;
   d_ptr->m_Mode = UserActionModelPrivate::UserActionModelMode::CALL;
   d_ptr->m_pCall = parent;
   connect(AccountModel::instance(), &AccountModel::accountStateChanged     , d_ptr.data(), &UserActionModelPrivate::slotAccountStateChanged);
}

/**
 * Create an UserActionModel around the CallModel selected call(s)
 */
UserActionModel::UserActionModel(CallModel* parent) : QAbstractListModel(parent),d_ptr(new UserActionModelPrivate(this))
{
   Q_ASSERT(parent != nullptr);
   d_ptr->m_Mode = UserActionModelPrivate::UserActionModelMode::CALLMODEL;
   d_ptr->m_SelectionState = UserActionModelPrivate::SelectionState::UNIQUE;
   connect(parent->selectionModel(), &QItemSelectionModel::currentRowChanged, d_ptr.data(), &UserActionModelPrivate::slotCurrentChanged     );
   connect(parent->selectionModel(), &QItemSelectionModel::selectionChanged , d_ptr.data(), &UserActionModelPrivate::slotSelectionChanged   );
   connect(parent,                   &CallModel::callStateChanged           , d_ptr.data(), &UserActionModelPrivate::slotCallStateChanged   );

   connect(AccountModel::instance(), &AccountModel::accountStateChanged     , d_ptr.data(), &UserActionModelPrivate::slotAccountStateChanged);
}

UserActionModel::~UserActionModel()
{

}

QHash<int,QByteArray> UserActionModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

QVariant UserActionModel::data(const QModelIndex& idx, int role ) const
{
   if (!idx.isValid() && (idx.row()>=0 && idx.row() < enum_class_size<UserActionModel::Action>()))
      return QVariant();

   UserActionModel::Action action = static_cast<UserActionModel::Action>(idx.row());

   switch(role) {
      case Qt::DisplayRole:
         return UserActionModelPrivate::m_ActionNames[action];
      case Qt::CheckStateRole:
         return Qt::Unchecked;
   };

   return QVariant();
}

int UserActionModel::rowCount(const QModelIndex& parent ) const
{
   return parent.isValid() ? 0 : static_cast<int>(Action::COUNT__);
}

///For now, this model probably wont be used that way
Qt::ItemFlags UserActionModel::flags(const QModelIndex& idx ) const
{
   if (!idx.isValid() && (idx.row()>=0 && idx.row() < enum_class_size<UserActionModel::Action>()))
      return Qt::NoItemFlags;

   UserActionModel::Action action = static_cast<UserActionModel::Action>(idx.row());

   return (d_ptr->m_CurrentActions[action] ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable) : Qt::NoItemFlags)
      | (d_ptr->actionStatefulness[action][d_ptr->m_SelectionState] == UserActionModel::ActionStatfulnessLevel::CHECKABLE
      ? Qt::ItemIsUserCheckable : Qt::NoItemFlags);
}

// ///This model is read only
bool UserActionModel::setData(const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED( index )
   Q_UNUSED( value )
   Q_UNUSED( role  )
   return false;
}

bool UserActionModel::isActionEnabled( UserActionModel::Action action ) const
{
   return d_ptr->availableActionMap[action][d_ptr->m_pCall->state()];
}

void UserActionModelPrivate::slotStateChanged()
{
   emit q_ptr->actionStateChanged();
}

bool UserActionModelPrivate::updateByCall(UserActionModel::Action action, const Call* c)
{
   return (!c) ? false : (
      availableActionMap        [action] [c->state()                       ] &&
      availableAccountActionMap [action] [c->account()->registrationState()] &&
      multi_call_options        [action] [m_SelectionState                 ] &&
      availableProtocolActions  [action] [c->account()->protocol()         ] //
   );
}

bool UserActionModelPrivate::updateAction(UserActionModel::Action action)
{
   switch(m_Mode) {
      case UserActionModelMode::CALL:
         return updateByCall(action, m_pCall);
      case UserActionModelMode::CALLMODEL: {
         bool ret = true;
         m_SelectionState = CallModel::instance()->selectionModel()->selectedRows().size() > 1 ? SelectionState::MULTI : SelectionState::UNIQUE;
         for (const QModelIndex& idx : CallModel::instance()->selectionModel()->selectedRows())
            ret &= updateByCall(action, qvariant_cast<Call*>(idx.data(Call::Role::Object)));
         return ret;
      }
   };
   return false;
}

void UserActionModelPrivate::updateActions()
{
   for (UserActionModel::Action action : EnumIterator<UserActionModel::Action>())
      m_CurrentActions[action] = updateAction(action);
   emit q_ptr->dataChanged(q_ptr->index(0,0),q_ptr->index(enum_class_size<UserActionModel::Action>()-1,0));
}

void UserActionModelPrivate::slotCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
   Q_UNUSED(current)
   Q_UNUSED(previous)
   updateActions();
}

void UserActionModelPrivate::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
   Q_UNUSED(selected)
   Q_UNUSED(deselected)
   updateActions();
}

void UserActionModelPrivate::slotCallStateChanged(Call* call, Call::State previousState)
{
   Q_UNUSED(call)
   Q_UNUSED(previousState)
   updateActions();
}

void UserActionModelPrivate::slotAccountStateChanged(Account* account, const Account::RegistrationState state)
{
   Q_UNUSED(account)
   Q_UNUSED(state)
   updateActions();
}

uint UserActionModel::relativeIndex( UserActionModel::Action action ) const
{
   int i(0),ret(0);
   while (i != static_cast<int>(action) && i < enum_class_size<UserActionModel::Action>()) {
      ret += isActionEnabled(static_cast<UserActionModel::Action>(i))?1:0;
      i++;
   }
   return ret;
}

#include <useractionmodel.moc>
