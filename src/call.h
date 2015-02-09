/****************************************************************************
 *   Copyright (C) 2009-2015 by Savoir-Faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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
#ifndef CALL_H
#define CALL_H

#include "categorizedcompositenode.h"
#include <itembase.h>
#include <time.h>

//Qt
#include <QtCore/QDebug>
class QString;
class QTimer;

//Ring
#include "typedefs.h"
#include "historytimecategorymodel.h"
class Account;
class InstantMessagingModel;
class UserActionModel;
class PhoneNumber;
class TemporaryPhoneNumber;
class CollectionInterface;
namespace Video {
   class Renderer;
}

class Call;

//Private
class CallPrivate;


/**
 * This class represent a call object from a client perspective. It is
 * fully stateful and has all properties required for a client. This object
 * is created by the CallModel class and its state can be modified by sending
 * Call::Action to the call using the '<<' operator.
 *
 * History calls will have the Call::State::OVER set by default. The LifeCycleState
 * system is designed to ensure that the call never go backward in its expected
 * lifecycle and should be used instead of "if"/"switch" on individual states
 * when possible. This will avoid accidentally forgetting a state.
**/
class  LIB_EXPORT Call : public ItemBase<QObject>
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:
   friend class CallModel;
   friend class CallModelPrivate;

   //Enum

   ///Model roles
   enum Role {
      Name               = 100, /*!< The peer name from SIP or Contacts */
      Number             = 101, /*!< The peer URI / phone number (as text) */
      Direction2         = 102, /*!<  */
      Date               = 103, /*!< The date when the call started */
      Length             = 104, /*!< The current length of the call */
      FormattedDate      = 105, /*!< An human readable starting date */
      HasRecording       = 106, /*!< If the call has a recording attached */
      Historystate       = 107, /*!<  */
      Filter             = 108, /*!<  */
      FuzzyDate          = 109, /*!<  */
      IsBookmark         = 110, /*!<  */
      Security           = 111, /*!<  */
      Department         = 112, /*!<  */
      Email              = 113, /*!<  */
      Organisation       = 114, /*!<  */
      Object             = 117, /*!<  */
      PhotoPtr           = 118, /*!<  */
      CallState          = 119, /*!<  */
      Id                 = 120, /*!<  */
      StartTime          = 121, /*!<  */
      StopTime           = 122, /*!<  */
      IsRecording        = 123, /*!<  */
      PhoneNu            = 124, /*!<  */
      IsPresent          = 125, /*!<  */
      SupportPresence    = 126, /*!<  */
      IsTracked          = 127, /*!<  */
      CategoryIcon       = 128, /*!<  */
      CallCount          = 129, /*!< The number of calls made with the same phone number */
      TotalSpentTime     = 130, /*!< The total time spent speaking to with this phone number*/
      Missed             = 131, /*!< This call has been missed */
      CallLifeCycleState = 132, /*!<  */
      DropState          = 300, /*!< GUI related state to keep track of metadata during drag and drop */
      DTMFAnimState      = 400, /*!< GUI related state to hold animation key(s) */
      LastDTMFidx        = 401, /*!< The last DTMF (button) sent on this call */
      DropPosition       = 402, /*!< GUI related state to keep track of metadata during drag and drop */
   };

   enum DropAction {
      Conference = 100,
      Transfer   = 101,
   };

   ///Possible call states
   enum class State : unsigned int{
      INCOMING        = 0, /*!<  Ringing incoming call */
      RINGING         = 1, /*!< Ringing outgoing call */
      CURRENT         = 2, /*!< Call to which the user can speak and hear */
      DIALING         = 3, /*!< Call which numbers are being added by the user */
      HOLD            = 4, /*!< Call is on hold */
      FAILURE         = 5, /*!< Call has failed */
      BUSY            = 6, /*!< Call is busy */
      TRANSFERRED     = 7, /*!< Call is being transferred.  During this state, the user can enter the new number. */
      TRANSF_HOLD     = 8, /*!< Call is on hold for transfer */
      OVER            = 9, /*!< Call is over and should not be used */
      ERROR           = 10,/*!< This state should never be reached */
      CONFERENCE      = 11,/*!< This call is the current conference*/
      CONFERENCE_HOLD = 12,/*!< This call is a conference on hold*/
      INITIALIZATION  = 13,/*!< The call have been placed, but the peer hasn't confirmed yet */
      COUNT__,
   };
   Q_ENUMS(State)

   /**
   * @enum Call::LegacyHistoryState
   * History items create before December 2013 will have a "state" field
   * mixing direction and missed. Newer items will have separated fields for that.
   *
   * Ring-KDE will keep support for at least a year
   */
   enum class LegacyHistoryState : int //DEPRECATED remove
   {
      INCOMING, /*!< The call is coming from the peer */
      OUTGOING, /*!< The call is going to the peer */
      MISSED  , /*!< The call has been missed */
      NONE      /*!< The legacy state is not set */
   };

   ///@enum Direction If the user have been called or have called
   enum class Direction : int {
      INCOMING, /*!< Someone has called      */
      OUTGOING, /*!< The user called someone */
   };
   Q_ENUMS(Direction)

   ///Is the call between one or more participants
   enum class Type {
      CALL      , /*!< A simple call                  */
      CONFERENCE, /*!< A composition of other calls   */
      HISTORY   , /*!< A call from a previous session */
   };

   /** @enum Call::DaemonState
   * This enum have all the states a call can take for the daemon.
   */
   enum class DaemonState : unsigned int
   {
      RINGING = 0, /*!< Ringing outgoing or incoming call */
      CURRENT = 1, /*!< Call to which the user can speak and hear */
      BUSY    = 2, /*!< Call is busy */
      HOLD    = 3, /*!< Call is on hold */
      HUNG_UP = 4, /*!< Call is over  */
      FAILURE = 5, /*!< Call has failed */
      COUNT__,
   };

   /** @enum Call::Action
   * This enum have all the actions you can make on a call.
   */
   enum class Action : unsigned int
   {
      ACCEPT   = 0, /*!< Accept, create or place call or place transfer */
      REFUSE   = 1, /*!< Red button, refuse or hang up */
      TRANSFER = 2, /*!< Put into or out of transfer mode*/
      HOLD     = 3, /*!< Hold or unhold the call */
      RECORD   = 4, /*!< Enable or disable recording */
      COUNT__,
   };

   /** @enum Call::LifeCycleState
    * This enum help track the call meta state
    * @todo Eventually add a meta state between progress and finished for
    *  calls that are still relevant enough to be in the main UI, such
    *  as BUSY OR FAILURE while also finished
    */
   enum class LifeCycleState {
      INITIALIZATION = 0, /*!< Anything before the media transfer start   */
      PROGRESS       = 1, /*!< The peers are in communication (or hold)   */
      FINISHED       = 2, /*!< Everything is over, there is no going back */
      COUNT__
   };

   ///"getHistory()" fields
   class HistoryMapFields {
   public:
      constexpr static const char* ACCOUNT_ID        = "accountid"      ;
      constexpr static const char* CALLID            = "callid"         ;
      constexpr static const char* DISPLAY_NAME      = "display_name"   ;
      constexpr static const char* PEER_NUMBER       = "peer_number"    ;
      constexpr static const char* RECORDING_PATH    = "recordfile"     ;
      constexpr static const char* STATE             = "state"          ;
      constexpr static const char* TIMESTAMP_START   = "timestamp_start";
      constexpr static const char* TIMESTAMP_STOP    = "timestamp_stop" ;
      constexpr static const char* MISSED            = "missed"         ;
      constexpr static const char* DIRECTION         = "direction"      ;
      constexpr static const char* CONTACT_USED      = "contact_used"   ;
      constexpr static const char* CONTACT_UID       = "contact_uid"    ;
      constexpr static const char* NUMBER_TYPE       = "number_type"    ;
   };

   ///@class HistoryStateName history map fields state names
   class HistoryStateName {
   public:
      constexpr static const char* MISSED         = "missed"  ;
      constexpr static const char* INCOMING       = "incoming";
      constexpr static const char* OUTGOING       = "outgoing";
   };

   //Read only properties
   Q_PROPERTY( Call::State        state            READ state             NOTIFY stateChanged     )
   Q_PROPERTY( QString            id               READ id                                        )
   Q_PROPERTY( Account*           account          READ account                                   )
   Q_PROPERTY( bool               isHistory        READ isHistory                                 )
   Q_PROPERTY( uint               stopTimeStamp    READ stopTimeStamp                             )
   Q_PROPERTY( uint               startTimeStamp   READ startTimeStamp                            )
   Q_PROPERTY( bool               isSecure         READ isSecure                                  )
   Q_PROPERTY( Video::Renderer*   videoRenderer    READ videoRenderer                             )
   Q_PROPERTY( QString            formattedName    READ formattedName                             )
   Q_PROPERTY( QString            length           READ length                                    )
   Q_PROPERTY( bool               hasRecording     READ hasRecording                              )
   Q_PROPERTY( bool               recording        READ isRecording                               )
   Q_PROPERTY( UserActionModel*   userActionModel  READ userActionModel   CONSTANT                )
   Q_PROPERTY( QString            toHumanStateName READ toHumanStateName                          )
   Q_PROPERTY( bool               missed           READ isMissed                                  )
   Q_PROPERTY( Direction          direction        READ direction                                 )
   Q_PROPERTY( bool               hasVideo         READ hasVideo                                  )
   Q_PROPERTY( Call::LegacyHistoryState historyState     READ historyState                        )

   //Read/write properties
   Q_PROPERTY( PhoneNumber*       peerPhoneNumber  READ peerPhoneNumber                           )
   Q_PROPERTY( QString            peerName         READ peerName          WRITE setPeerName       )
   Q_PROPERTY( QString            transferNumber   READ transferNumber    WRITE setTransferNumber )
   Q_PROPERTY( QString            recordingPath    READ recordingPath     WRITE setRecordingPath  )
   Q_PROPERTY( QString            dialNumber       READ dialNumber        WRITE setDialNumber      NOTIFY dialNumberChanged(QString))

   //Constructors & Destructors
   static Call* buildHistoryCall  (const QMap<QString,QString>& hc);

   //Static getters
   static const QString      toHumanStateName ( const Call::State );

   //Getters
   Call::State              state            () const;
   const QString            id               () const;
   PhoneNumber*             peerPhoneNumber  () const;
   const QString            peerName         () const;
   Call::LegacyHistoryState historyState     () const;
   bool                     isRecording      () const;
   Account*                 account          () const;
   bool                     isHistory        () const;
   time_t                   stopTimeStamp    () const;
   time_t                   startTimeStamp   () const;
   bool                     isSecure         () const;
   const QString            transferNumber   () const;
   const QString            dialNumber       () const;
   const QString            recordingPath    () const;
   Video::Renderer*         videoRenderer    () const;
   const QString            formattedName    () const;
   bool                     hasRecording     () const;
   QString                  length           () const;
   QVariant                 roleData         (int role) const;
   UserActionModel*         userActionModel  () const;
   QString                  toHumanStateName () const;
   bool                     isHistory        ()      ;
   bool                     isMissed         () const;
   Call::Direction          direction        () const;
   bool                     hasVideo         () const;
   Call::LifeCycleState     lifeCycleState   () const;
   Call::Type               type             () const;

   //Automated function
   Call::State performAction(Call::Action action);

   //Setters
   void setTransferNumber ( const QString&     number     );
   void setDialNumber     ( const QString&     number     );
   void setDialNumber     ( const PhoneNumber* number     );
   void setRecordingPath  ( const QString&     path       );
   void setPeerName       ( const QString&     name       );
   void setAccount        ( Account*           account    );

   //Mutators
   void appendText(const QString& str);
   void backspaceItemText();
   void reset();
   void sendTextMessage(const QString& message);

   //syntactic sugar
   Call* operator<<( Call::Action& c);

private:
   explicit Call(const QString& confId, const QString& account);
   ~Call();
   Call(Call::State startState, const QString& callId, const QString& peerName = QString(), PhoneNumber* number = nullptr, Account* account = nullptr); //TODO MOVE TO PRIVATE

   CallPrivate* d_ptr;
   Q_DECLARE_PRIVATE(Call)

public Q_SLOTS:
   void playRecording();
   void stopRecording();
   void seekRecording(double position);
   void playDTMF(const QString& str);

Q_SIGNALS:
   ///Emitted when a call change (state or details)
   void changed();
   ///@TODO deprecated, use QObject::sender()
   void changed(Call* self);
   ///Emitted when the call is over //TODO remove the argument
   void isOver(Call*);
   ///The recording playback position changed
   void playbackPositionChanged(int,int);
   ///The recording playback has stopped
   void playbackStopped();
   ///The recording playback has started
   void playbackStarted();
   ///Notify that a DTMF have been played
   void dtmfPlayed(const QString& str);
   ///Notify of state change
   void stateChanged();
   ///The call start timestamp changed, this usually indicate the call has started
   void startTimeStampChanged(time_t newTimeStamp);
   ///The dial number has changed
   void dialNumberChanged(const QString& number);
};

Q_DECLARE_METATYPE(Call*)

QDebug LIB_EXPORT operator<<(QDebug dbg, const Call::State& c       );
QDebug LIB_EXPORT operator<<(QDebug dbg, const Call::DaemonState& c );
QDebug LIB_EXPORT operator<<(QDebug dbg, const Call::Action& c      );

#endif
