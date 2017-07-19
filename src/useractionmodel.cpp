/****************************************************************************
 *   Copyright (C) 2012-2017 Savoir-faire Linux                          *
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
#include "mime.h"
#include "callmodel.h"
#include "account.h"
#include "accountmodel.h"
#include "contactmethod.h"
#include "availableaccountmodel.h"
#include "phonedirectorymodel.h"
#include "globalinstances.h"
#include "interfaces/pixmapmanipulatori.h"
#include "interfaces/actionextenderi.h"
#include "private/useractions.h"
#include "private/matrixutils.h"
#include "media/textrecording.h"

#define UAM UserActionModel

class ActiveUserActionModel : public QSortFilterProxyModel
{
public:
   ActiveUserActionModel(QAbstractItemModel* parent) : QSortFilterProxyModel(parent)
   {
      setSourceModel(parent);
   }
   virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};

class UserActionModelPrivate final : public QObject
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
      CALL    , /*!< Model the react on a single call element    */
      GENERIC , /*!< Model that react on a model selection       */
   };

   //Availability matrices
   UserActionModelPrivate(UserActionModel* parent, const FlagPack<UAM::Context>& c);
   static const Matrix1D< UAM::Action                            , bool  > heterogenous_call_options ;
   static const Matrix2D< UAM::Action, Call::State               , bool  > availableActionMap        ;
   static const Matrix2D< UAM::Action, Account::RegistrationState, bool  > availableAccountActionMap ;
   static const Matrix2D< UAM::Action, SelectionState            , bool  > multi_call_options        ;
   static const Matrix2D< UAM::Action, Account::Protocol         , bool  > availableProtocolActions  ;
   static const Matrix1D< UAM::Action, FlagPack<UAM::Context>            > actionContext             ;
   static const Matrix1D< UAM::Action, FlagPack<UAM::Asset>              > availableByAsset          ;
   static const Matrix2D< UAM::Action, Ring::ObjectType          , bool  > availableObjectActions    ;
   static const Matrix1D< UAM::Action, bool(*)(const Person*       )     > personActionAvailability  ;
   static const Matrix1D< UAM::Action, bool(*)(const ContactMethod*)     > cmActionAvailability      ;

   static const Matrix2D< UAM::Action, SelectionState, UAM::ActionStatfulnessLevel > actionStatefulness;


   //Helpers
   bool updateAction         (UAM::Action action                          );
   bool updateByCall         (UAM::Action action, const Call* c           );
   bool updateByContactMethod(UAM::Action action, const ContactMethod* cm );
   bool updateByAccount      (UAM::Action action, const Account* a        );
   bool updateByPerson       (UAM::Action action, const Person* p         );
   void updateCheckMask      (int& ret, UAM::Action action, const Call* c );

   //Attributes
   Call*                                  m_pCall              ;
   UserActionModelMode                    m_Mode               ;
   SelectionState                         m_SelectionState     ;
   TypedStateMachine< bool, UAM::Action>  m_CurrentActions     ;
   Matrix1D< UAM::Action, Qt::CheckState> m_CurrentActionsState;
   Matrix1D< UAM::Action, QString>        m_ActionNames        ;
   ActiveUserActionModel*                 m_pActiveModel       ;
   FlagPack<UAM::Context>                 m_fContext           ;
   QItemSelectionModel*                   m_pSelectionModel {nullptr};
   QAbstractItemModel*                    m_pSourceModel    {nullptr};

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

#define UAMA UserActionModel::Action
#define CS Call::State
//Enabled actions
static EnumClassReordering<Call::State> co = {
   CS::NEW            , /* >----------|                                                                                                                       */
   CS::INCOMING       , /* -----------|-------|                                                                                                               */
   CS::RINGING        , /* -----------|-------|-------|                                                                                                       */
   CS::CURRENT        , /* -----------|-------|-------|------|                                                                                                */
   CS::DIALING        , /* -----------|-------|-------|------|-----|                                                                                          */
   CS::HOLD           , /* -----------|-------|-------|------|-----|-------|                                                                                  */
   CS::FAILURE        , /* -----------|-------|-------|------|-----|-------|------|                                                                           */
   CS::BUSY           , /* -----------|-------|-------|------|-----|-------|------|------|                                                                    */
   CS::TRANSFERRED    , /* -----------|-------|-------|------|-----|-------|------|------|-----|                                                              */
   CS::TRANSF_HOLD    , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|                                                      */
   CS::OVER           , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|                                               */
   CS::ERROR          , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|                                         */
   CS::CONFERENCE     , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|                                  */
   CS::CONFERENCE_HOLD, /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|------|                           */
   CS::INITIALIZATION , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|------|-------|                   */
   CS::ABORTED        , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|------|-------|------|            */
   CS::CONNECTED      , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|------|-------|------|-----|      */
};                      /*            |       |       |      |     |       |      |      |     |       |      |     |      |      |       |      |     |      */
                        /*            \/      \/      \/     \/    \/      \/     \/     \/    \/      \/     \/    \/     \/     \/      \/     \/    \/     */
const Matrix2D< UAM::Action, Call::State, bool > UserActionModelPrivate::availableActionMap = {
 { UAMA::ACCEPT            , {{co, { false, true  , false, false, true , false, false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::HOLD              , {{co, { false, false , false, true , false, true , false, false, false, false, false, false, true , false, false, false, false }}}},
 { UAMA::MUTE_AUDIO        , {{co, { false, false , true , true , false, false, false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::MUTE_VIDEO        , {{co, { false, false , true , true , false, false, false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::SERVER_TRANSFER   , {{co, { false, false , false, true , false, true , false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::RECORD            , {{co, { false, false , true , true , false, true , false, false, true , true , false, false, true , true , false, false, false }}}},
 { UAMA::HANGUP            , {{co, { true , true  , true , true , true , true , true , true , true , true , false, true , true , true , true , false, true  }}}},
 { UAMA::JOIN              , {{co, { false, false , true , true , false, true , false, false, true , true , false, false, true , true , false, false, false }}}},
 { UAMA::ADD_NEW           , {{co, { false, false , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::TOGGLE_VIDEO      , {{co, { false, false , false, true , false, true , false, false, false, false, false, false, true , false, false, false, false }}}},
 { UAMA::ADD_CONTACT       , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::ADD_TO_CONTACT    , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::DELETE_CONTACT    , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::EMAIL_CONTACT     , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::COPY_CONTACT      , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::BOOKMARK          , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::VIEW_CHAT_HISTORY , {{co, { false, true  , true , true , true , true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::ADD_CONTACT_METHOD, {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::CALL_CONTACT      , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::EDIT_CONTACT      , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::REMOVE_HISTORY    , {{co, { false, false , false, false, false, false, true , true , false, false, true , false, false, false, false, false, false }}}},
};
#undef CS

/**
 * Assuming a call is in progress, the communication can still be valid if the account is down, however,
 * this will impact the available actions
 */
const Matrix2D< UAMA, Account::RegistrationState, bool > UserActionModelPrivate::availableAccountActionMap = {
   /*                             READY  UNREGISTERED  INITIALIZING  TRYING  ERROR   */
   { UAMA::ACCEPT            , {{ true ,    false,     false,        false,  false  }}},
   { UAMA::HOLD              , {{ true ,    false,     false,        false,  false  }}},
   { UAMA::MUTE_AUDIO        , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::MUTE_VIDEO        , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::SERVER_TRANSFER   , {{ true ,    false,     false,        false,  false  }}},
   { UAMA::RECORD            , {{ true ,    true ,     false,        true,   true   }}},
   { UAMA::HANGUP            , {{ true ,    true ,     false,        true,   true   }}},

   { UAMA::JOIN              , {{ true ,    true ,     false,        true ,  true   }}},

   { UAMA::ADD_NEW           , {{ true ,    false,     false,        true ,  true   }}},
   { UAMA::TOGGLE_VIDEO      , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::ADD_CONTACT       , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::ADD_TO_CONTACT    , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::DELETE_CONTACT    , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::EMAIL_CONTACT     , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::COPY_CONTACT      , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::BOOKMARK          , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::VIEW_CHAT_HISTORY , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::ADD_CONTACT_METHOD, {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::CALL_CONTACT      , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::EDIT_CONTACT      , {{ true ,    true ,     false,        true ,  true   }}},
   { UAMA::REMOVE_HISTORY    , {{ true ,    true ,     false,        true ,  true   }}},
};

/**
 * This matrix define if an option is available depending on the number of selection elements
 */
const Matrix2D< UAMA, UserActionModelPrivate::SelectionState, bool > UserActionModelPrivate::multi_call_options = {
   /*                             NONE   UNIQUE   MULTI  */
   { UAMA::ACCEPT            , {{ false,  true ,  true  }}},
   { UAMA::HOLD              , {{ false,  true ,  false }}},
   { UAMA::MUTE_AUDIO        , {{ false,  true ,  true  }}},
   { UAMA::MUTE_VIDEO        , {{ false,  true ,  true  }}},
   { UAMA::SERVER_TRANSFER   , {{ false,  true ,  true  }}},
   { UAMA::RECORD            , {{ false,  true ,  true  }}},
   { UAMA::HANGUP            , {{ false,  true ,  true  }}},

   { UAMA::JOIN              , {{ false,  false,  true  }}},

   { UAMA::ADD_NEW           , {{ true ,  false,  false }}},
   { UAMA::TOGGLE_VIDEO      , {{ false,  true ,  false }}},
   { UAMA::ADD_CONTACT       , {{ false,  true ,  false }}},
   { UAMA::ADD_TO_CONTACT    , {{ false,  true ,  false }}},
   { UAMA::DELETE_CONTACT    , {{ false,  true ,  false }}},
   { UAMA::EMAIL_CONTACT     , {{ false,  true ,  false }}},
   { UAMA::COPY_CONTACT      , {{ false,  true ,  false }}},
   { UAMA::BOOKMARK          , {{ false,  true ,  false }}},
   { UAMA::VIEW_CHAT_HISTORY , {{ false,  true ,  false }}},
   { UAMA::ADD_CONTACT_METHOD, {{ false,  true ,  false }}},
   { UAMA::CALL_CONTACT      , {{ false,  true ,  false }}},
   { UAMA::EDIT_CONTACT      , {{ false,  true ,  false }}},
   { UAMA::REMOVE_HISTORY    , {{ false,  true ,  true  }}},
};

/**
 * This matrix define if an option is available when multiple elements with mismatching CheckState are selected
 */
const Matrix1D< UAMA, bool > UserActionModelPrivate::heterogenous_call_options = {
   { UAMA::ACCEPT            , true  }, /* N/A                                       */
   { UAMA::HOLD              , false }, /* Do not allow to set a state               */
   { UAMA::MUTE_AUDIO        , true  }, /* Force mute on all calls                   */
   { UAMA::MUTE_VIDEO        , true  }, /* Mute all calls                            */
   { UAMA::SERVER_TRANSFER   , false }, /* It make no sense to let the user set this */
   { UAMA::RECORD            , true  }, /* Force "record" on all calls               */
   { UAMA::HANGUP            , true  }, /* N/A                                       */

   { UAMA::JOIN              , true  }, /* N/A                                       */

   { UAMA::ADD_NEW           , false }, /* N/A                                       */
   { UAMA::TOGGLE_VIDEO      , false },
   { UAMA::ADD_CONTACT       , false },
   { UAMA::ADD_TO_CONTACT    , false },
   { UAMA::DELETE_CONTACT    , false },
   { UAMA::EMAIL_CONTACT     , false },
   { UAMA::COPY_CONTACT      , false },
   { UAMA::BOOKMARK          , false },
   { UAMA::VIEW_CHAT_HISTORY , false },
   { UAMA::ADD_CONTACT_METHOD, false },
   { UAMA::CALL_CONTACT      , false },
   { UAMA::EDIT_CONTACT      , false },
   { UAMA::REMOVE_HISTORY    , false },
};

/**
 * This matrix allow to enable/disable actions depending on the call protocol
 */
const Matrix2D< UAMA, Account::Protocol, bool > UserActionModelPrivate::availableProtocolActions = {
   /*                              SIP   DHT   */
   { UAMA::ACCEPT            , {{ true , true  }}},
   { UAMA::HOLD              , {{ true , true  }}},
   { UAMA::MUTE_AUDIO        , {{ true , true  }}},
   { UAMA::MUTE_VIDEO        , {{ true , true  }}},
   { UAMA::SERVER_TRANSFER   , {{ true , false }}},
   { UAMA::RECORD            , {{ true , true  }}},
   { UAMA::HANGUP            , {{ true , true  }}},

   { UAMA::JOIN              , {{ true , true  }}},

   { UAMA::ADD_NEW           , {{ true , true  }}},

   { UAMA::TOGGLE_VIDEO      , {{ true,  true  }}},
   { UAMA::ADD_CONTACT       , {{ true,  true  }}},
   { UAMA::ADD_TO_CONTACT    , {{ true,  true  }}},
   { UAMA::DELETE_CONTACT    , {{ true,  true  }}},
   { UAMA::EMAIL_CONTACT     , {{ true,  true  }}},
   { UAMA::COPY_CONTACT      , {{ true,  true  }}},
   { UAMA::BOOKMARK          , {{ true,  true  }}},
   { UAMA::VIEW_CHAT_HISTORY , {{ true,  true  }}},
   { UAMA::ADD_CONTACT_METHOD, {{ true,  true  }}},
   { UAMA::CALL_CONTACT      , {{ true,  true  }}},
   { UAMA::EDIT_CONTACT      , {{ true,  true  }}},
   { UAMA::REMOVE_HISTORY    , {{ true,  true  }}},
};

/**
 * This matrix define if an action is stateless or stateful. The only state
 * supported is "checked", but when multiple items are selected, this can
 * cause a heterogenous bunch of checked and unchecked elements, this is
 * called "TRISTATE".
 */
#define ST UserActionModel::ActionStatfulnessLevel::
const Matrix2D< UAMA, UserActionModelPrivate::SelectionState, UserActionModel::ActionStatfulnessLevel > UserActionModelPrivate::actionStatefulness = {
   /*                                NONE          UNIQUE             MULTI       */
   { UAMA::ACCEPT            , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::HOLD              , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::MUTE_AUDIO        , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::MUTE_VIDEO        , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::SERVER_TRANSFER   , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::RECORD            , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::HANGUP            , {{ ST UNISTATE,  ST UNISTATE     ,  ST TRISTATE  }}},

   { UAMA::JOIN              , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},

   { UAMA::ADD_NEW           , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::TOGGLE_VIDEO      , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::ADD_CONTACT       , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::ADD_TO_CONTACT    , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::DELETE_CONTACT    , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::EMAIL_CONTACT     , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::COPY_CONTACT      , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::BOOKMARK          , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::VIEW_CHAT_HISTORY , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::ADD_CONTACT_METHOD, {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::CALL_CONTACT      , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::EDIT_CONTACT      , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::REMOVE_HISTORY    , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
};
#undef ST

/**
 * This matrix categorize each actions. Action can be enabled in a given context
 * while being hidden in another. This allow, for example, to use the
 * UserActionModel to fill context menu.
 */
const Matrix1D< UAMA, FlagPack<UAM::Context>> UserActionModelPrivate::actionContext = {
   { UAMA::ACCEPT            , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMENDED },

   { UAMA::HOLD              , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMENDED },

   { UAMA::MUTE_AUDIO        , UAM::Context::RECOMMENDED },

   { UAMA::MUTE_VIDEO        , UAM::Context::RECOMMENDED },

   { UAMA::SERVER_TRANSFER   , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMENDED },

   { UAMA::RECORD            , UAM::Context::RECOMMENDED },

   { UAMA::HANGUP            , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMENDED },

   { UAMA::JOIN              , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMENDED },

   { UAMA::ADD_NEW           , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMENDED },

   { UAMA::TOGGLE_VIDEO      , UAM::Context::ADVANCED    },
   { UAMA::ADD_CONTACT       , UAM::Context::MANAGEMENT  },
   { UAMA::ADD_TO_CONTACT    , UAM::Context::MANAGEMENT  },
   { UAMA::DELETE_CONTACT    , UAM::Context::MANAGEMENT  },
   { UAMA::EMAIL_CONTACT     , UAM::Context::CONTACT     },
   { UAMA::COPY_CONTACT      , UAM::Context::MANAGEMENT  },
   { UAMA::BOOKMARK          , UAM::Context::MANAGEMENT  },
   { UAMA::VIEW_CHAT_HISTORY , UAM::Context::RECOMMENDED },
   { UAMA::ADD_CONTACT_METHOD, UAM::Context::MANAGEMENT  },
   { UAMA::CALL_CONTACT      , UAM::Context::CONTACT     },
   { UAMA::EDIT_CONTACT      , UAM::Context::CONTACT     },
   { UAMA::REMOVE_HISTORY    , UAM::Context::MANAGEMENT  },
};

const Matrix1D< UAMA, FlagPack<UAM::Asset>> UserActionModelPrivate::availableByAsset = {
   { UAMA::ACCEPT            , UAM::Asset::CALL           },
   { UAMA::HOLD              , UAM::Asset::CALL           },
   { UAMA::MUTE_AUDIO        , UAM::Asset::CALL           },
   { UAMA::MUTE_VIDEO        , UAM::Asset::CALL           },
   { UAMA::SERVER_TRANSFER   , UAM::Asset::CALL           },
   { UAMA::RECORD            , UAM::Asset::CALL           },
   { UAMA::HANGUP            , UAM::Asset::CALL           },
   { UAMA::JOIN              , UAM::Asset::CALL           },
   { UAMA::ADD_NEW           , UAM::Asset::CALL           },
   { UAMA::TOGGLE_VIDEO      , UAM::Asset::CALL           },
   { UAMA::ADD_CONTACT       , UAM::Asset::PERSON         },
   { UAMA::ADD_TO_CONTACT    , UAM::Asset::CONTACT_METHOD },
   { UAMA::DELETE_CONTACT    , UAM::Asset::PERSON         },
   { UAMA::EMAIL_CONTACT     , UAM::Asset::PERSON         },
   { UAMA::COPY_CONTACT      , UAM::Asset::PERSON         },
   { UAMA::BOOKMARK          , UAM::Asset::CONTACT_METHOD },
   { UAMA::VIEW_CHAT_HISTORY , UAM::Asset::CONTACT_METHOD },
   { UAMA::ADD_CONTACT_METHOD, UAM::Asset::PERSON         },
   { UAMA::CALL_CONTACT      , UAM::Asset::PERSON         },
   { UAMA::EDIT_CONTACT      , UAM::Asset::PERSON         },
   { UAMA::REMOVE_HISTORY    , UAM::Asset::CALL           },
};

/**
 * Different objects type have access to a different subset of actions
 */
const Matrix2D< UAMA, Ring::ObjectType , bool  > UserActionModelPrivate::availableObjectActions = {
   /*                            Person ContactMethod  Call    Media  Certificate   ContactRequest*/
   { UAMA::ACCEPT            , {{ false,    false,     true ,  false,    false,        false     }}},
   { UAMA::HOLD              , {{ false,    false,     true ,  true ,    false,        false     }}},
   { UAMA::MUTE_AUDIO        , {{ false,    false,     true ,  true ,    false,        false     }}},
   { UAMA::MUTE_VIDEO        , {{ false,    false,     true ,  true ,    false,        false     }}},
   { UAMA::SERVER_TRANSFER   , {{ false,    false,     true ,  false,    false,        false     }}},
   { UAMA::RECORD            , {{ false,    false,     true ,  true ,    false,        false     }}},
   { UAMA::HANGUP            , {{ false,    false,     true ,  false,    false,        false     }}},
   { UAMA::JOIN              , {{ false,    false,     true ,  false,    false,        false     }}},
   { UAMA::ADD_NEW           , {{ false,    false,     true ,  false,    false,        false     }}},
   { UAMA::TOGGLE_VIDEO      , {{ false,    false,     true ,  true ,    false,        false     }}},
   { UAMA::ADD_CONTACT       , {{ false,    true ,     true ,  false,    true ,        false     }}},
   { UAMA::ADD_TO_CONTACT    , {{ false,    true ,     true ,  false,    true ,        false     }}},
   { UAMA::DELETE_CONTACT    , {{ true ,    true ,     true ,  false,    true ,        false     }}},
   { UAMA::EMAIL_CONTACT     , {{ true ,    true ,     true ,  false,    true ,        false     }}},
   { UAMA::COPY_CONTACT      , {{ true ,    true ,     true ,  false,    true ,        false     }}},
   { UAMA::BOOKMARK          , {{ true ,    true ,     true ,  false,    true ,        false     }}},
   { UAMA::VIEW_CHAT_HISTORY , {{ true ,    true ,     true ,  true ,    true ,        true      }}},
   { UAMA::ADD_CONTACT_METHOD, {{ true ,    true ,     true ,  false,    true ,        false     }}},
   { UAMA::CALL_CONTACT      , {{ true ,    true ,     true ,  false,    true ,        false     }}},
   { UAMA::EDIT_CONTACT      , {{ true ,    true ,     true ,  false,    true ,        false     }}},
   { UAMA::REMOVE_HISTORY    , {{ true ,    true ,     true ,  false,    true ,        false     }}},
};

#define P_CB [](const Person* p) -> bool

/**
 * Persons aren't stateful but rather have a series of properties. Therefore a
 * series of state machines cannot properly represent that state of every
 * actions.
 */
const Matrix1D< UAM::Action, bool(*)(const Person*)> UserActionModelPrivate::personActionAvailability = {
   { UAMA::ACCEPT            , nullptr                                         },
   { UAMA::HOLD              , nullptr                                         },
   { UAMA::MUTE_AUDIO        , nullptr                                         },
   { UAMA::MUTE_VIDEO        , nullptr                                         },
   { UAMA::SERVER_TRANSFER   , nullptr                                         },
   { UAMA::RECORD            , nullptr                                         },
   { UAMA::HANGUP            , nullptr                                         },
   { UAMA::JOIN              , nullptr                                         },
   { UAMA::ADD_NEW           , nullptr                                         },
   { UAMA::TOGGLE_VIDEO      , nullptr                                         },
   { UAMA::ADD_CONTACT       , nullptr                                         },
   { UAMA::ADD_TO_CONTACT    , nullptr                                         },
   { UAMA::DELETE_CONTACT    , P_CB { return p->collection() &&
      p->collection()->supportedFeatures() &
         CollectionInterface::SupportedFeatures::REMOVE;
   }},
   { UAMA::EMAIL_CONTACT     , P_CB { return ! p->preferredEmail().isEmpty(); }},
   { UAMA::COPY_CONTACT      , nullptr                                         },
   { UAMA::BOOKMARK          , nullptr                                         },
   { UAMA::VIEW_CHAT_HISTORY , P_CB {
      return p->hasRecording(
        Media::Media::Type::TEXT,
        Media::Media::Direction::OUT
      );
   }},
   { UAMA::ADD_CONTACT_METHOD, nullptr                                         },
   { UAMA::CALL_CONTACT      , P_CB { return p->isReachable();                 }},
   { UAMA::EDIT_CONTACT    , P_CB { return p->collection() &&
      p->collection()->supportedFeatures() &
         CollectionInterface::SupportedFeatures::EDIT;
   }},
   { UAMA::REMOVE_HISTORY    , P_CB { return p->hasBeenCalled();               }},
};
#undef P_C

#define CM_CB [](const ContactMethod* cm) -> bool

/**
 * ContactMethods aren't stateful but rather have a series of properties.
 * Therefore a series of state machines cannot properly represent that state of
 * every actions.
 */
const Matrix1D< UAM::Action, bool(*)(const ContactMethod*)> UserActionModelPrivate::cmActionAvailability = {
   { UAMA::ACCEPT            , nullptr                                         },
   { UAMA::HOLD              , nullptr                                         },
   { UAMA::MUTE_AUDIO        , nullptr                                         },
   { UAMA::MUTE_VIDEO        , nullptr                                         },
   { UAMA::SERVER_TRANSFER   , nullptr                                         },
   { UAMA::RECORD            , nullptr                                         },
   { UAMA::HANGUP            , nullptr                                         },
   { UAMA::JOIN              , nullptr                                         },
   { UAMA::ADD_NEW           , nullptr                                         },
   { UAMA::TOGGLE_VIDEO      , nullptr                                         },
   { UAMA::ADD_CONTACT       , CM_CB { return (!cm) || (!cm->contact())
      || cm->contact()->isPlaceHolder();
   }},
   { UAMA::ADD_TO_CONTACT    , CM_CB { return (!cm) || !cm->contact();        }},
   { UAMA::DELETE_CONTACT    , CM_CB { return cm && cm->contact() &&
      cm->contact()->collection() &&
      cm->contact()->collection()->supportedFeatures() &
         CollectionInterface::SupportedFeatures::REMOVE;
   }},
   { UAMA::EMAIL_CONTACT     , CM_CB { return cm && cm->contact() &&
      !cm->contact()->preferredEmail().isEmpty();
   }},
   { UAMA::COPY_CONTACT      , CM_CB { return cm && cm->contact();            }},
   { UAMA::BOOKMARK          , nullptr                                         },
   { UAMA::VIEW_CHAT_HISTORY , CM_CB {
       return cm && ((
          cm->textRecording()
          && !cm->textRecording()->isEmpty()
       ) || (
           cm->protocolHint() == URI::ProtocolHint::RING_USERNAME ||
           cm->protocolHint() == URI::ProtocolHint::RING
       ));
   }},
   { UAMA::ADD_CONTACT_METHOD, CM_CB { return cm && cm->contact();            }},
   { UAMA::CALL_CONTACT      , CM_CB { return (!cm) || cm->isReachable();     }},
   { UAMA::EDIT_CONTACT      , CM_CB { return cm && cm->contact();            }},
   { UAMA::REMOVE_HISTORY    , CM_CB { return cm && cm->callCount();          }},
};
#undef CM_CB

UserActionModelPrivate::UserActionModelPrivate(UserActionModel* parent, const FlagPack<UAM::Context>& c) : QObject(parent),q_ptr(parent),
m_pCall(nullptr), m_pActiveModel(nullptr), m_fContext(c)
{
   //Init the default names
   m_ActionNames = {
      { UAMA::ACCEPT            , QObject::tr("Accept"                 )},
      { UAMA::HOLD              , QObject::tr("Hold"                   )},
      { UAMA::MUTE_AUDIO        , QObject::tr("Mute audio"             )},
      { UAMA::MUTE_VIDEO        , QObject::tr("Mute video"             )},
      { UAMA::SERVER_TRANSFER   , QObject::tr("Server transfer"        )},
      { UAMA::RECORD            , QObject::tr("Record"                 )},
      { UAMA::HANGUP            , QObject::tr("Hangup"                 )},
      { UAMA::JOIN              , QObject::tr("Join"                   )},
      { UAMA::ADD_NEW           , QObject::tr("Add new"                )},
      { UAMA::TOGGLE_VIDEO      , QObject::tr("Toggle video"           )},
      { UAMA::ADD_CONTACT       , QObject::tr("Add a contact"          )},
      { UAMA::ADD_TO_CONTACT    , QObject::tr("Add to existing contact")},
      { UAMA::DELETE_CONTACT    , QObject::tr("Delete contact"         )},
      { UAMA::EMAIL_CONTACT     , QObject::tr("Email contact"          )},
      { UAMA::COPY_CONTACT      , QObject::tr("Copy contact"           )},
      { UAMA::BOOKMARK          , QObject::tr("Bookmark"               )},
      { UAMA::VIEW_CHAT_HISTORY , QObject::tr("Open chat"              )},
      { UAMA::ADD_CONTACT_METHOD, QObject::tr("Add phone number"       )},
      { UAMA::CALL_CONTACT      , QObject::tr("Call again"             )},
      { UAMA::EDIT_CONTACT      , QObject::tr("Edit contact details"   )},
      { UAMA::REMOVE_HISTORY    , QObject::tr("Remove from history"    )},
   };

   m_CurrentActionsState = {
      { UAMA::ACCEPT            , Qt::Unchecked},
      { UAMA::HOLD              , Qt::Unchecked},
      { UAMA::MUTE_AUDIO        , Qt::Unchecked},
      { UAMA::MUTE_VIDEO        , Qt::Unchecked},
      { UAMA::SERVER_TRANSFER   , Qt::Unchecked},
      { UAMA::RECORD            , Qt::Unchecked},
      { UAMA::HANGUP            , Qt::Unchecked},
      { UAMA::JOIN              , Qt::Unchecked},
      { UAMA::ADD_NEW           , Qt::Unchecked},
      { UAMA::TOGGLE_VIDEO      , Qt::Unchecked},
      { UAMA::ADD_CONTACT       , Qt::Unchecked},
      { UAMA::ADD_TO_CONTACT    , Qt::Unchecked},
      { UAMA::DELETE_CONTACT    , Qt::Unchecked},
      { UAMA::EMAIL_CONTACT     , Qt::Unchecked},
      { UAMA::COPY_CONTACT      , Qt::Unchecked},
      { UAMA::BOOKMARK          , Qt::Unchecked},
      { UAMA::VIEW_CHAT_HISTORY , Qt::Unchecked},
      { UAMA::ADD_CONTACT_METHOD, Qt::Unchecked},
      { UAMA::CALL_CONTACT      , Qt::Unchecked},
      { UAMA::EDIT_CONTACT      , Qt::Unchecked},
      { UAMA::REMOVE_HISTORY    , Qt::Unchecked},
   };
}

#undef UAMA
#undef UAM

/**
 * Create an UserActionModel around a single call. This won't take advantage
 * of the multiselection feature.
 */
UserActionModel::UserActionModel(Call* parent, const FlagPack<UserActionModel::Context> c) : QAbstractListModel(parent),d_ptr(new UserActionModelPrivate(this,c))
{
   Q_ASSERT(parent != nullptr);
   Q_ASSERT(parent->state() != Call::State::OVER);
   d_ptr->m_SelectionState = UserActionModelPrivate::SelectionState::UNIQUE;
   d_ptr->m_Mode = UserActionModelPrivate::UserActionModelMode::CALL;
   d_ptr->m_pCall = parent;

   connect(&AccountModel::instance(), SIGNAL(accountStateChanged(Account*,Account::RegistrationState)), d_ptr.data(), SLOT(slotStateChanged()));
   d_ptr->updateActions();
}

/**
 * Create an UserActionModel around the CallModel selected call(s)
 */
UserActionModel::UserActionModel(QAbstractItemModel* parent, const FlagPack<UserActionModel::Context> c) : QAbstractListModel(parent),d_ptr(new UserActionModelPrivate(this,c))
{
   Q_ASSERT(parent != nullptr);
   d_ptr->m_Mode = UserActionModelPrivate::UserActionModelMode::GENERIC;
   d_ptr->m_SelectionState = UserActionModelPrivate::SelectionState::UNIQUE;
   d_ptr->m_pSourceModel = parent;

   connect(&AccountModel::instance(), &AccountModel::accountStateChanged      , d_ptr.data(), &UserActionModelPrivate::updateActions);

   if (auto callmodel = qobject_cast<CallModel*>(parent)) {
      setSelectionModel(callmodel->selectionModel());
      connect(callmodel, &CallModel::callStateChanged , d_ptr.data(), &UserActionModelPrivate::updateActions);
      connect(callmodel, &CallModel::mediaStateChanged, d_ptr.data(), &UserActionModelPrivate::updateActions);
      connect(callmodel, &CallModel::dialNumberChanged, d_ptr.data(), &UserActionModelPrivate::updateActions);
   }
   //TODO add other relevant models here Categorized*, RecentModel, etc

   d_ptr->updateActions();
}

UserActionModel::~UserActionModel()
{

}

QHash<int,QByteArray> UserActionModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[(int)Role::ACTION] = "action";
   }
   return roles;
}

QVariant UserActionModel::data(const QModelIndex& idx, int role ) const
{
   if ((!idx.isValid()) || !(idx.row()>=0 && idx.row() < enum_class_size<UserActionModel::Action>()))
      return QVariant();

   UserActionModel::Action action = static_cast<UserActionModel::Action>(idx.row());

   switch(role) {
      case Qt::DisplayRole:
         return d_ptr->m_ActionNames[action];
      case Qt::CheckStateRole:
         if (d_ptr->actionStatefulness[action][d_ptr->m_SelectionState] != UserActionModel::ActionStatfulnessLevel::UNISTATE)
            return d_ptr->m_CurrentActionsState[action];
         break;
      case Qt::DecorationRole: {
         UserActionElement state;
         state.action     = action       ;
         state.calls      = {}           ;
         state.checkState = Qt::Unchecked;
         return GlobalInstances::pixmapManipulator().userActionIcon(state);
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

///For now, this model probably won't be used that way
Qt::ItemFlags UserActionModel::flags(const QModelIndex& idx ) const
{
   if ((!idx.isValid()) || !(idx.row()>=0 && idx.row() < enum_class_size<UserActionModel::Action>()))
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
   if (!d_ptr->m_pCall)
      return false;

   return d_ptr->availableActionMap[action][d_ptr->m_pCall->state()];
}

void UserActionModelPrivate::slotStateChanged()
{
   emit q_ptr->actionStateChanged();
}

void UserActionModelPrivate::updateCheckMask(int& ret, UserActionModel::Action action, const Call* c)
{
   //TODO c will be nullptr if the selection is a person or a contact method
   //there is still a need to update the check mask, but it is less relevant
   //so it can wait for later. This will cause some weird issues with the
   //recent model
   if (!c)
      return;

   switch(action) {
      case UserActionModel::Action::ACCEPT          :
         ret += 0;
         break;
      case UserActionModel::Action::HOLD            :
         ret += c->state() == Call::State::HOLD? 100 : 1;
         break;
      case UserActionModel::Action::MUTE_AUDIO      : {
         auto a = c->firstMedia<Media::Audio>(Media::Media::Direction::OUT);
         ret += a && a->state() == Media::Media::State::MUTED ? 100 : 1;
         }
         break;
      case UserActionModel::Action::MUTE_VIDEO      : {
         auto v = c->firstMedia<Media::Video>(Media::Media::Direction::OUT);
         ret += v && v->state() == Media::Media::State::MUTED ? 100 : 1;
         }
         break;
      case UserActionModel::Action::SERVER_TRANSFER :
         ret += c->state() == Call::State::TRANSFERRED? 100 : 1;
         break;
      case UserActionModel::Action::RECORD          :
         ret += c->isRecording(Media::Media::Type::AUDIO,Media::Media::Direction::OUT) ? 100 : 1;
         break;
      case UserActionModel::Action::HANGUP          :
         ret += 0;
         break;
      case UserActionModel::Action::JOIN            :
         ret += 0;
         break;
      case UserActionModel::Action::ADD_NEW         :
         ret += 0;
         break;
      case UserActionModel::Action::TOGGLE_VIDEO      :
      case UserActionModel::Action::ADD_CONTACT       :
      case UserActionModel::Action::ADD_TO_CONTACT    :
      case UserActionModel::Action::DELETE_CONTACT    :
      case UserActionModel::Action::EMAIL_CONTACT     :
      case UserActionModel::Action::COPY_CONTACT      :
      case UserActionModel::Action::BOOKMARK          :
      case UserActionModel::Action::VIEW_CHAT_HISTORY :
      case UserActionModel::Action::ADD_CONTACT_METHOD:
      case UserActionModel::Action::CALL_CONTACT      :
      case UserActionModel::Action::EDIT_CONTACT      :
      case UserActionModel::Action::REMOVE_HISTORY    :
      case UserActionModel::Action::COUNT__:
         break;
   };

   //Avoid the noise
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wswitch-enum"
   //Update the labels
   switch (action) {
      case UserActionModel::Action::ACCEPT          :
         switch(c->state()) {
            case Call::State::DIALING        :
               m_ActionNames.setAt(UserActionModel::Action::ACCEPT, QObject::tr("Call"));
               break;
            default:
               m_ActionNames.setAt(UserActionModel::Action::ACCEPT,  QObject::tr("Accept"));
               break;
         }
         break;
      case UserActionModel::Action::HOLD            :
         switch(c->state()) {
            case Call::State::HOLD           :
            case Call::State::CONFERENCE_HOLD:
            case Call::State::TRANSF_HOLD    :
               m_ActionNames.setAt(UserActionModel::Action::HOLD, QObject::tr("Unhold"));
               break;
            default:
               m_ActionNames.setAt(UserActionModel::Action::HOLD, QObject::tr("Hold"));
               break;
         }
         break;
      case UserActionModel::Action::HANGUP          :
         switch(c->state()) {
            case Call::State::DIALING        :
            case Call::State::NEW            :
               m_ActionNames.setAt(UserActionModel::Action::HANGUP, QObject::tr("Cancel"));
               break;
            case Call::State::FAILURE        :
            case Call::State::ERROR          :
            case Call::State::COUNT__        :
            case Call::State::INITIALIZATION :
            case Call::State::BUSY           :
               m_ActionNames.setAt(UserActionModel::Action::HANGUP, QObject::tr("Remove"));
               break;
            default:
               m_ActionNames.setAt(UserActionModel::Action::HANGUP, QObject::tr("Hangup"));
               break;
         }
         break;
      case UserActionModel::Action::JOIN            :
      case UserActionModel::Action::ADD_NEW         :
      case UserActionModel::Action::COUNT__         :
      case UserActionModel::Action::MUTE_AUDIO      :
      case UserActionModel::Action::MUTE_VIDEO      :
      case UserActionModel::Action::SERVER_TRANSFER :
      case UserActionModel::Action::RECORD          :
      case UserActionModel::Action::TOGGLE_VIDEO      :
      case UserActionModel::Action::ADD_CONTACT       :
      case UserActionModel::Action::ADD_TO_CONTACT    :
      case UserActionModel::Action::DELETE_CONTACT    :
      case UserActionModel::Action::EMAIL_CONTACT     :
      case UserActionModel::Action::COPY_CONTACT      :
      case UserActionModel::Action::BOOKMARK          :
      case UserActionModel::Action::VIEW_CHAT_HISTORY :
      case UserActionModel::Action::ADD_CONTACT_METHOD:
      case UserActionModel::Action::CALL_CONTACT      :
      case UserActionModel::Action::EDIT_CONTACT      :
      case UserActionModel::Action::REMOVE_HISTORY    :
         break;
   }
   #pragma GCC diagnostic pop
}

bool UserActionModelPrivate::updateByCall(UserActionModel::Action action, const Call* c)
{
   if (!c)
      return false;
   Account* a = c->account() ? c->account() : AvailableAccountModel::instance().currentDefaultAccount();

   return (
      availableActionMap        [action] [c->state()             ] &&
      multi_call_options        [action] [m_SelectionState       ] &&
      actionContext             [action] & m_fContext              &&
      updateByAccount(action, a)
   );
}

bool UserActionModelPrivate::updateByAccount(UserActionModel::Action action, const Account* a)
{
   if (!a)
      return false;
   return (
      availableAccountActionMap [action] [a->registrationState() ] &&
      availableProtocolActions  [action] [a->protocol()          ] //
   );
}

bool UserActionModelPrivate::updateByContactMethod(UserActionModel::Action action, const ContactMethod* cm)
{
   // Some actions have a conditional CM
   if (!cm)
      return cmActionAvailability[action] ?
         cmActionAvailability[action](nullptr) : true;

   Account* a = cm->account() ? cm->account() : AvailableAccountModel::instance().currentDefaultAccount();

   return updateByAccount(action, a) && (
      (!cmActionAvailability[action]) || cmActionAvailability[action](cm)
   );
}

bool UserActionModelPrivate::updateByPerson(UserActionModel::Action action, const Person* p)
{
   return (!personActionAvailability[action]) || personActionAvailability[action](p);
}

bool UserActionModelPrivate::updateAction(UserActionModel::Action action)
{
   int state = 0;
   switch(m_Mode) {
      case UserActionModelMode::CALL:
         updateCheckMask(state,action,m_pCall);
         m_CurrentActionsState.setAt(action,  state / 100 ? Qt::Checked : Qt::Unchecked);

         return updateByCall(action, m_pCall);
      case UserActionModelMode::GENERIC: {
         bool ret = true;

         m_SelectionState = m_pSelectionModel ? (
            m_pSelectionModel->selectedRows().size() > 1 ?
               SelectionState::MULTI :
               SelectionState::UNIQUE
         ) : SelectionState::NONE ;

         //Aggregate and reduce the action state for each selected calls
         if (m_pSelectionModel && (m_pSelectionModel->selectedRows().size() || m_pSelectionModel->currentIndex().isValid())) {

            auto selected = m_pSelectionModel->selectedRows();

            if (selected.isEmpty() && m_pSelectionModel->currentIndex().isValid())
               selected << m_pSelectionModel->currentIndex();

            foreach (const QModelIndex& idx, selected) {

               const QVariant objTv = idx.data(static_cast<int>(Ring::Role::ObjectType));

               //Be sure the model support the UAM abstraction
               if (!objTv.canConvert<Ring::ObjectType>()) {
                  qWarning() << "Cannot determine object type";
                  continue;
               }

               const auto objT =  qvariant_cast<Ring::ObjectType>(objTv);

               ret &= availableObjectActions[action][objT];

               //There is no point in doing further checks
               if (!ret) {
                  continue;
               }

               switch(objT) {
                  case Ring::ObjectType::Person         : {

                     const auto p = qvariant_cast<Person*>(idx.data(static_cast<int>(Ring::Role::Object)));

                     ret &= updateByPerson( action, p );

                     break;
                  }
                  case Ring::ObjectType::ContactMethod  : {

                     const auto cm = qvariant_cast<ContactMethod*>(idx.data(static_cast<int>(Ring::Role::Object)));

                     ret &= cm ? updateByContactMethod( action, cm ) : false;

                     break;
                  }
                  case Ring::ObjectType::Call           : {

                     const auto c = qvariant_cast<Call*>(idx.data(static_cast<int>(Ring::Role::Object)));

                     ret &= updateByCall( action, c );

                     // Dialing (search field) calls have a new URI with every
                     // keystroke. Check is such URI match an existing one. This
                     // changes the availability of some actions. For example,
                     // the offline chat only works for Ring CM *or* SIP CM with
                     // an existing chat history.
                     if (c->state() == Call::State::DIALING) {
                        ret &= updateByContactMethod(
                          action, PhoneDirectoryModel::instance().getExistingNumberIf(
                             c->peerContactMethod()->uri(),
                             [](const ContactMethod* cm) -> bool { return cm->account();}
                          )
                        );
                     }

                     updateCheckMask( state ,action, c ); //TODO abstract this out

                     break;
                  }
                  case Ring::ObjectType::Media          : //TODO
                  case Ring::ObjectType::Certificate    : //TODO
                  case Ring::ObjectType::ContactRequest   : //TODO
                  case Ring::ObjectType::COUNT__        :
                     break;
               }
            }
         }
         else {
            Account* a = AvailableAccountModel::instance().currentDefaultAccount();
            ret = multi_call_options[action][UserActionModelPrivate::SelectionState::NONE]
               && (a?availableAccountActionMap[action][a->registrationState()]:false);
         }

         //Detect if the multiple selection has mismatching item states, disable it if necessary
         m_CurrentActionsState.setAt(action, (state % 100 && state / 100) ? Qt::PartiallyChecked : (state / 100 ? Qt::Checked : Qt::Unchecked));
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

   // For now, only handle single selection, multi-selection could be
   // re-enabled later

   // TODO This will be cleaned up later
   if (! d_ptr->m_pSelectionModel->hasSelection () && action == UserActionModel::Action::ADD_NEW) {
      if (UserActions::addNew()) {
         d_ptr->updateActions();
         return true;
      }
   }

   foreach (const QModelIndex& idx, d_ptr->m_pSelectionModel->selectedRows()) {
      const QVariant objTv = idx.data(static_cast<int>(Ring::Role::ObjectType));

      //Be sure the model support the UAM abstraction
      if (!objTv.canConvert<Ring::ObjectType>()) {
         qWarning() << "Cannot determine object type";
         continue;
      }

      const auto objT = qvariant_cast<Ring::ObjectType>(objTv);

      Call*          c  = nullptr;
      // Account*       a  = nullptr; TODO: uncomment when account is needed
      ContactMethod* cm = nullptr;
      Person*        p  = nullptr;

      // Deduce each kind of objects from the relations
      switch(objT) {
         case Ring::ObjectType::Person         :
            p  = qvariant_cast<Person*>(idx.data(static_cast<int>(Ring::Role::Object)));
            cm = p->phoneNumbers().size() == 1 ? p->phoneNumbers()[0] : nullptr;
            // a  = cm ? cm->account() : nullptr; TODO: uncomment when account is needed
            break;
         case Ring::ObjectType::ContactMethod  :
            cm = qvariant_cast<ContactMethod*>(idx.data(static_cast<int>(Ring::Role::Object)));
            // a  = cm->account(); TODO: uncomment when account is needed
            p  = cm->contact();
            //TODO maybe add "QList<Call*> currentCalls()" const to ContactMethod::?
            break;
         case Ring::ObjectType::Call           :
            c  = qvariant_cast<Call*>(idx.data(static_cast<int>(Ring::Role::Object)));
            cm = c->peerContactMethod();
            p  = cm ? cm->contact() : nullptr;
            // a  = c->account(); TODO: uncomment when account is needed
            break;
         case Ring::ObjectType::Media          : //TODO
         case Ring::ObjectType::Certificate    : //TODO
         case Ring::ObjectType::ContactRequest   : //TODO
         case Ring::ObjectType::COUNT__        :
            break;
      }

      // Perform the actions
      switch(action) {
         case UserActionModel::Action::ACCEPT          :
            if (UserActions::accept(c))
               d_ptr->updateActions();
            break;
         case UserActionModel::Action::HOLD            :
            switch(d_ptr->m_CurrentActionsState[UserActionModel::Action::HOLD]) {
               case Qt::Checked:
                  if (UserActions::unhold(c))
                     d_ptr->updateActions();
                  break;
               case Qt::Unchecked:
                  if (UserActions::hold(c))
                     d_ptr->updateActions();
                  break;
               case Qt::PartiallyChecked:
                  //Invalid
                  break;
            };
            break;
         case UserActionModel::Action::MUTE_AUDIO      :
            {
               bool mute = d_ptr->m_CurrentActionsState[UserActionModel::Action::MUTE_AUDIO] != Qt::Checked;
               UserActions::muteAudio(c, mute);
               d_ptr->updateActions();
            }
            break;
         case UserActionModel::Action::MUTE_VIDEO      :
            {
               bool mute = d_ptr->m_CurrentActionsState[UserActionModel::Action::MUTE_VIDEO] != Qt::Checked;
               UserActions::muteVideo(c, mute);
               d_ptr->updateActions();
            }
            break;
         case UserActionModel::Action::SERVER_TRANSFER :
            UserActions::transfer(c);
            break;
         case UserActionModel::Action::RECORD          :
            if (UserActions::recordAudio(c)) //TODO handle other recording types
               d_ptr->updateActions();
            break;
         case UserActionModel::Action::HANGUP          :
            if (UserActions::hangup(c))
               d_ptr->updateActions();
            break;
         case UserActionModel::Action::JOIN            :
            //TODO unimplemented
            break;
         case UserActionModel::Action::ADD_NEW         :
            if (UserActions::addNew())
               d_ptr->updateActions();
            break;
         case UserActionModel::Action::TOGGLE_VIDEO      :
            break;
         case UserActionModel::Action::ADD_CONTACT       :
            UserActions::addPerson(cm);
            break;
         case UserActionModel::Action::ADD_TO_CONTACT    :
            UserActions::addToPerson(cm);
            break;
         case UserActionModel::Action::DELETE_CONTACT    :
            UserActions::deleteContact(p);
            break;
         case UserActionModel::Action::EMAIL_CONTACT     :
            UserActions::sendEmail(p);
            break;
         case UserActionModel::Action::COPY_CONTACT      :
            GlobalInstances::actionExtender().copyInformation(
                RingMimes::payload(c, cm, p)
            );
            break;
         case UserActionModel::Action::BOOKMARK          :
            UserActions::bookmark(cm);
            break;
         case UserActionModel::Action::VIEW_CHAT_HISTORY :
            if (p)
               GlobalInstances::actionExtender().viewChatHistory(p);
            else
               GlobalInstances::actionExtender().viewChatHistory(cm);
            break;
         case UserActionModel::Action::ADD_CONTACT_METHOD:
            UserActions::addToPerson(p);
            break;
         case UserActionModel::Action::CALL_CONTACT      :
            UserActions::callAgain(cm);
            break;
         case UserActionModel::Action::EDIT_CONTACT      :
            UserActions::editPerson(p);
            break;
         case UserActionModel::Action::REMOVE_HISTORY    :
            UserActions::removeFromHistory(c);
            break;
         case UserActionModel::Action::COUNT__:
            break;
      };
   }

   return true; //TODO handle errors
}

UserActionModel* UserActionModel::operator<<(UserActionModel::Action& action)
{
   execute(action);
   return this;
}


UserActionModel* operator<<(UserActionModel* m,UserActionModel::Action action)
{
   return (!m)? nullptr : (*m) << action;
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
QAbstractItemModel* UserActionModel::activeActionModel() const
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

///Use a custom selection model
void UserActionModel::setSelectionModel(QItemSelectionModel* sm)
{
   d_ptr->m_pSelectionModel = sm;
   connect(sm, &QItemSelectionModel::currentRowChanged , d_ptr.data(), &UserActionModelPrivate::updateActions);
   connect(sm, &QItemSelectionModel::selectionChanged  , d_ptr.data(), &UserActionModelPrivate::updateActions);

   d_ptr->updateActions();
}

#include <useractionmodel.moc>
