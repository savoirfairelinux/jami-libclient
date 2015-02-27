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
#include <QtCore/QSortFilterProxyModel>

//Ring
#include "call.h"
#include "callmodel.h"
#include "account.h"
#include "accountmodel.h"
#include "delegates/pixmapmanipulationdelegate.h"
#include "private/useractions.h"

class ActiveUserActionModel : public QSortFilterProxyModel
{
public:
   ActiveUserActionModel(QAbstractItemModel* parent) : QSortFilterProxyModel(parent)
   {
      setSourceModel(parent);
   }
   virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};

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
   static const TypedStateMachine< TypedStateMachine< bool                                    , Call::State                > , UserActionModel::Action > availableActionMap        ;
   static const TypedStateMachine< TypedStateMachine< bool                                    , Account::RegistrationState > , UserActionModel::Action > availableAccountActionMap ;
   static const TypedStateMachine< TypedStateMachine< bool                                    , SelectionState             > , UserActionModel::Action > multi_call_options        ;
   static const TypedStateMachine< bool                                                                                      , UserActionModel::Action > heterogenous_call_options ;
   static const TypedStateMachine< TypedStateMachine< bool                                    , Account::Protocol          > , UserActionModel::Action > availableProtocolActions  ;
   static const TypedStateMachine< TypedStateMachine< UserActionModel::ActionStatfulnessLevel , SelectionState             > , UserActionModel::Action > actionStatefulness        ;

   //Helpers
   bool updateAction   (UserActionModel::Action action                          );
   bool updateByCall   (UserActionModel::Action action, const Call* c           );
   void updateCheckMask(int& ret, UserActionModel::Action action, const Call* c );

   //Attribues
   Call*                                                             m_pCall              ;
   UserActionModelMode                                               m_Mode               ;
   SelectionState                                                    m_SelectionState     ;
   TypedStateMachine< bool, UserActionModel::Action>                 m_CurrentActions     ;
   TypedStateMachine< Qt::CheckState, UserActionModel::Action>       m_CurrentActionsState;
   static const TypedStateMachine< QString, UserActionModel::Action> m_ActionNames        ;
   ActiveUserActionModel*                                            m_pActiveModel       ;

   //The mute per call, per media is not merged upstream yet, faking it for now
   //BEGIN HACK
   bool m_MuteAudio_TO_REMOVE;
   bool m_MuteVideo_TO_REMOVE;
   //END HACK

private:
   UserActionModel* q_ptr;

public Q_SLOTS:
   //Single call mode
   void slotStateChanged(); //TODO

   //CallModel mode
   void updateActions();
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

/**
 * This matrix define if an option is available depending on the number of selection elements
 */
const TypedStateMachine< TypedStateMachine< bool , UserActionModelPrivate::SelectionState > , UserActionModel::Action > UserActionModelPrivate::multi_call_options = {{
   /*                       NONE   UNIQUE   MULTI  */
   /* ACCEPT          */ {{ false,  true ,  true  }},
   /* HOLD            */ {{ false,  true ,  false }},
   /* MUTE_AUDIO      */ {{ false,  true ,  true  }},
   /* MUTE_VIDEO      */ {{ false,  true ,  true  }},
   /* SERVER_TRANSFER */ {{ false,  true ,  true  }},
   /* RECORD          */ {{ false,  true ,  true  }},
   /* HANGUP          */ {{ false,  true ,  true  }},

   /* JOIN            */ {{ false,  false,  true  }},
}};

/**
 * This matrix define if an option is available when multiple elements with mismatching CheckState are selected
 */
const TypedStateMachine< bool, UserActionModel::Action > UserActionModelPrivate::heterogenous_call_options = {{
   /* ACCEPT          */ true  , /* N/A                                       */
   /* HOLD            */ false , /* Do not allow to set a state               */
   /* MUTE_AUDIO      */ true  , /* Force mute on all calls                   */
   /* MUTE_VIDEO      */ true  , /* Mute all calls                            */
   /* SERVER_TRANSFER */ false , /* It make no sense to let the user set this */
   /* RECORD          */ true  , /* Force "record" on all calls               */
   /* HANGUP          */ true  , /* N/A                                       */

   /* JOIN            */ true  , /* N/A                                       */
}};

/**
 * This matrix allow to enable/disable actions depending on the call protocol
 */
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

/**
 * This matrix define if an action is stateless or stateful. The only state
 * supported is "checked", but when multiple items are selected, this can
 * cause a heterogenous bunch of checked and unchecked elements, this is
 * called "TRISTATE".
 */
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
   /* ACCEPT          */ QObject::tr("ACCEPT"          ), //TODO use better (and stateful) names
   /* HOLD            */ QObject::tr("HOLD"            ),
   /* MUTE_AUDIO      */ QObject::tr("MUTE_AUDIO"      ),
   /* MUTE_VIDEO      */ QObject::tr("MUTE_VIDEO"      ),
   /* SERVER_TRANSFER */ QObject::tr("SERVER_TRANSFER" ),
   /* RECORD          */ QObject::tr("RECORD"          ),
   /* HANGUP          */ QObject::tr("HANGUP"          ),

   /* JOIN            */ QObject::tr("JOIN"            ),
}};

UserActionModelPrivate::UserActionModelPrivate(UserActionModel* parent) : QObject(parent),q_ptr(parent),
m_pCall(nullptr), m_pActiveModel(nullptr)
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

   connect(AccountModel::instance(), SIGNAL(accountStateChanged(Account*,Account::RegistrationState)), d_ptr.data(), SLOT(slotStateChanged()));
}

/**
 * Create an UserActionModel around the CallModel selected call(s)
 */
UserActionModel::UserActionModel(CallModel* parent) : QAbstractListModel(parent),d_ptr(new UserActionModelPrivate(this))
{
   Q_ASSERT(parent != nullptr);
   d_ptr->m_Mode = UserActionModelPrivate::UserActionModelMode::CALLMODEL;
   d_ptr->m_SelectionState = UserActionModelPrivate::SelectionState::UNIQUE;

   connect(parent->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex))             , d_ptr.data(), SLOT(updateActions()));
   connect(parent->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection))        , d_ptr.data(), SLOT(updateActions()));
   connect(parent,                   SIGNAL(callStateChanged(Call*,Call::State))                    , d_ptr.data(), SLOT(updateActions()));
   connect(AccountModel::instance(), SIGNAL(accountStateChanged(Account*,Account::RegistrationState)), d_ptr.data(), SLOT(updateActions()));
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
         if (d_ptr->actionStatefulness[action][d_ptr->m_SelectionState] != UserActionModel::ActionStatfulnessLevel::UNISTATE)
            return d_ptr->m_CurrentActionsState[action];
         break;
      case Qt::DecorationRole: {
         UserActionElement state;
         state.action     = action       ;
         state.calls      = {}           ;
         state.checkState = Qt::Unchecked;
         return PixmapManipulationDelegate::instance()->userActionIcon(state);
      }
      case UserActionModel::Role::ACTION:
         return QVariant::fromValue(static_cast<Action>(idx.row()));
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
      | (d_ptr->actionStatefulness[action][d_ptr->m_SelectionState] != UserActionModel::ActionStatfulnessLevel::UNISTATE
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

void UserActionModelPrivate::updateCheckMask(int& ret, UserActionModel::Action action, const Call* c)
{
   switch(action) {
      case UserActionModel::Action::ACCEPT          :
         ret += 0;
         break;
      case UserActionModel::Action::HOLD            :
         ret += c->state() == Call::State::HOLD? 100 : 1;
         break;
      case UserActionModel::Action::MUTE_AUDIO      :
         ret += m_MuteAudio_TO_REMOVE ? 100 : 1;
         break;
      case UserActionModel::Action::MUTE_VIDEO      :
         ret += m_MuteVideo_TO_REMOVE ? 100 : 1;
         break;
      case UserActionModel::Action::SERVER_TRANSFER :
         ret += c->state() == Call::State::TRANSFERRED? 100 : 1;
         break;
      case UserActionModel::Action::RECORD          :
         ret += c->isRecording() ? 100 : 1;
         break;
      case UserActionModel::Action::HANGUP          :
         ret += 0;
         break;
      case UserActionModel::Action::JOIN            :
         ret += 0;
         break;
      case UserActionModel::Action::COUNT__:
         break;
   };
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
   int state = 0;
   switch(m_Mode) {
      case UserActionModelMode::CALL:
         updateCheckMask(state,action,m_pCall);
         m_CurrentActionsState[action] = state / 100 ? Qt::Checked : Qt::Unchecked;

         return updateByCall(action, m_pCall);
      case UserActionModelMode::CALLMODEL: {
         bool ret = true;

         m_SelectionState = CallModel::instance()->selectionModel()->selectedRows().size() > 1 ? SelectionState::MULTI : SelectionState::UNIQUE;

         //Aggregate and reduce the action state for each selected calls
         for (const QModelIndex& idx : CallModel::instance()->selectionModel()->selectedRows()) {
            const Call* c = qvariant_cast<Call*>(idx.data(Call::Role::Object));
            updateCheckMask    ( state ,action, c );
            ret &= updateByCall( action       , c );
         }

         //Detect if the multiple selection has mismatching item states, disable it if necessary
         m_CurrentActionsState[action] = (state % 100 && state / 100) ? Qt::PartiallyChecked : (state / 100 ? Qt::Checked : Qt::Unchecked);
         return ret && (m_CurrentActionsState[action] != Qt::PartiallyChecked || heterogenous_call_options[action]);
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

uint UserActionModel::relativeIndex( UserActionModel::Action action ) const
{
   int i(0),ret(0);
   while (i != static_cast<int>(action) && i < enum_class_size<UserActionModel::Action>()) {
      ret += isActionEnabled(static_cast<UserActionModel::Action>(i))?1:0;
      i++;
   }
   return ret;
}

bool UserActionModel::execute(const UserActionModel::Action action) const
{
   /*
    * The rational behind not expanding Call::Actions to handle this are:
    *
    * 1) There is some actions that apply _only_ to multiple calls
    * 2) There is still a need to abstract the multi call use case
    */

   //1) Build the list of all selected calls
   QList<Call*> selected;
   switch(d_ptr->m_Mode) {
      case UserActionModelPrivate::UserActionModelMode::CALL:
         selected << d_ptr->m_pCall;
         break;
      case UserActionModelPrivate::UserActionModelMode::CALLMODEL: {
         for (const QModelIndex& idx : CallModel::instance()->selectionModel()->selectedRows()) {
            Call* c = qvariant_cast<Call*>(idx.data(Call::Role::Object));
            if (c)
               selected << c;
         }
         break;
      }
   };

   //2) Perform the actions
   switch(action) {
      case UserActionModel::Action::ACCEPT          :
         if (UserActions::accept(selected))
            d_ptr->updateActions();
         break;
      case UserActionModel::Action::HOLD            :
         switch(d_ptr->m_CurrentActionsState[UserActionModel::Action::HOLD]) {
            case Qt::Checked:
               if (UserActions::unhold(selected))
                  d_ptr->updateActions();
               break;
            case Qt::Unchecked:
               if (UserActions::hold(selected))
                  d_ptr->updateActions();
               break;
            case Qt::PartiallyChecked:
               //Invalid
               break;
         };
         break;
      case UserActionModel::Action::MUTE_AUDIO      :
         d_ptr->m_MuteAudio_TO_REMOVE != d_ptr->m_MuteAudio_TO_REMOVE; //FIXME evil fake property
         d_ptr->updateActions();
         break;
      case UserActionModel::Action::MUTE_VIDEO      :
         d_ptr->m_MuteVideo_TO_REMOVE != d_ptr->m_MuteVideo_TO_REMOVE; //FIXME evil fake property
         d_ptr->updateActions();
         UserActions::accept(selected);
         break;
      case UserActionModel::Action::SERVER_TRANSFER :
         UserActions::transfer(selected);
         break;
      case UserActionModel::Action::RECORD          :
         if (UserActions::record(selected))
            d_ptr->updateActions();
         break;
      case UserActionModel::Action::HANGUP          :
         if (UserActions::accept(selected))
            d_ptr->updateActions();
         break;
      case UserActionModel::Action::JOIN            :
         //TODO unimplemented
         break;
      case UserActionModel::Action::COUNT__:
         break;
   };

   return true; //TODO handle errors
}

/**
 * Execute an action
 * @param idx A model index. It can be from proxies.
 */
bool UserActionModel::execute(const QModelIndex& idx) const
{
   QModelIndex idx2 = idx;

   //Make this API a little more user friendly and unwind the proxies
   QAbstractProxyModel* m = nullptr;
   while (idx2.model() != this && idx2.isValid()) {
      m = qobject_cast<QAbstractProxyModel*>(const_cast<QAbstractItemModel*>(idx2.model()));
      if (m)
         idx2 = m->mapToSource(idx2);
      else
         break;
   }

   if (!idx2.isValid())
      return false;

   UserActionModel::Action action = static_cast<UserActionModel::Action>(idx2.row());
   return execute(action);
}

///Get a model filter with only the available actions
QSortFilterProxyModel* UserActionModel::activeActionModel() const
{
   if (!d_ptr->m_pActiveModel)
      d_ptr->m_pActiveModel = new ActiveUserActionModel(const_cast<UserActionModel*>(this));
   return d_ptr->m_pActiveModel;
}

//Do not display disabled account
bool ActiveUserActionModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
   return sourceModel()->index(source_row,0,source_parent).flags() & Qt::ItemIsEnabled;
}

#include <useractionmodel.moc>
