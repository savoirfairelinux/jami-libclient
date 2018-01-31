/****************************************************************************
 *   Copyright (C) 2009-2018 Savoir-faire Linux                          *
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
#pragma once

#include <itembase.h>
#include <time.h>

//Qt
#include <QtCore/QDebug>
class QString;
class QTimer;

//Ring
#include "typedefs.h"
#include "historytimecategorymodel.h"
#include "media/media.h"
#include "itemdataroles.h"
class Account               ;
class InstantMessagingModel ;
class UserActionModel       ;
class ContactMethod         ;
class TemporaryContactMethod;
class CollectionInterface   ;
class Certificate           ;
class Person                ;

namespace Video {
   class Manager;
   class Renderer;
   class ManagerPrivate;
}

namespace Media {
   class Media;
   class Audio;
   class Video;
   class Text;
   class Recording;
}

class Call;

//Private
class CallPrivate;

namespace RingMimes
{
   QMimeData* payload(const Call*, const ContactMethod*, const Person*);
}


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
class  LIB_EXPORT Call : public ItemBase
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:
   friend class CallModel            ;
   friend class CategorizedHistoryModel;
   friend class CallModelPrivate     ;
   friend class IMConversationManager;
   friend class VideoRendererManager;
   friend class VideoRendererManagerPrivate;
   friend class Media::Media;
   friend class Media::Audio;
   friend class Media::Video;
   friend class Media::Text;
   friend class MediaTypeInference;
   friend class IMConversationManagerPrivate;
   friend QMimeData* RingMimes::payload(const Call*, const ContactMethod*, const Person*);

   //Enum

   ///Model roles
   enum class Role {
      Name               = static_cast<int>(Ring::Role::UserRole) + 100, /*!< The peer name from SIP or Persons */
      Number             , /*!< The peer URI / phone number (as text)                               */
      Direction          , /*!<                                                                     */
      Date               , /*!< The date when the call started                                      */
      Length             , /*!< The current length of the call                                      */
      FormattedDate      , /*!< An human readable starting date                                     */
      Historystate       , /*!<                                                                     */
      Filter             , /*!<                                                                     */
      FuzzyDate          , /*!<                                                                     */
      IsBookmark         , /*!<                                                                     */
      Security           , /*!<                                                                     */
      Department         , /*!<                                                                     */
      Email              , /*!<                                                                     */
      Organisation       , /*!<                                                                     */
      HasAVRecording     , /*!<                                                                     */
      Object             , /*!<                                                                     */
      Photo              , /*!<                                                                     */
      State              , /*!<                                                                     */
      StartTime          , /*!<                                                                     */
      StopTime           , /*!<                                                                     */
      IsAVRecording      , /*!<                                                                     */
      ContactMethod      , /*!<                                                                     */
      IsPresent          , /*!<                                                                     */
      SupportPresence    , /*!<                                                                     */
      IsTracked          , /*!<                                                                     */
      CategoryIcon       , /*!<                                                                     */
      CallCount          , /*!< The number of calls made with the same phone number                 */
      TotalSpentTime     , /*!< The total time spent speaking to with this phone number             */
      Missed             , /*!< This call has been missed                                           */
      LifeCycleState     , /*!<                                                                     */
      Certificate        , /*!< The certificate (for encrypted calls)                               */
      HasAudioRecording  , /*!<                                                                     */
      HasVideoRecording  , /*!<                                                                     */
      HumanStateName     , /*!<                                                                     */
      DTMFAnimState      , /*!< GUI related state to hold animation key(s)                          */
      LastDTMFidx        , /*!< The last DTMF (button) sent on this call                            */
      DropPosition       , /*!< GUI related state to keep track of metadata during drag and drop    */
      DateOnly           ,
      DateTime           ,
      SecurityLevel      , //TODO REMOVE use the extensions
      SecurityLevelIcon  , //TODO REMOVE use the extensions
      DropState          = static_cast<int>(Ring::Role::DropState), /*!< GUI related state to keep track of metadata during drag and drop */
   };

   enum DropAction {
      Conference = 100,
      Transfer   = 101,
   };

   ///Possible call states
   enum class State : unsigned int{
   #pragma push_macro("ERROR")
   #undef ERROR
      NEW             = 0, /*!< The call has been created, but no dialing number been set                         */
      INCOMING        = 1, /*!< Ringing incoming call                                                             */
      RINGING         = 2, /*!< Ringing outgoing call                                                             */
      CURRENT         = 3, /*!< Call to which the user can speak and hear                                         */
      DIALING         = 4, /*!< Call which numbers are being added by the user                                    */
      HOLD            = 5, /*!< Call is on hold by this side of the communication, @see Call::HoldFlags           */
      FAILURE         = 6, /*!< Call has failed                                                                   */
      BUSY            = 7, /*!< Call is busy                                                                      */
      TRANSFERRED     = 8, /*!< Call is being transferred.  During this state, the user can enter the new number. */
      TRANSF_HOLD     = 9, /*!< Call is on hold for transfer                                                      */
      OVER            = 10,/*!< Call is over and should not be used                                               */
      ERROR           = 11,/*!< This state should never be reached                                                */
      CONFERENCE      = 12,/*!< This call is the current conference                                               */
      CONFERENCE_HOLD = 13,/*!< This call is a conference on hold                                                 */
      INITIALIZATION  = 14,/*!< The call have been placed, but the peer hasn't confirmed yet                      */
      ABORTED         = 15,/*!< The call was dropped before being sent to the daemon                              */
      CONNECTED       = 16,/*!< The peer has been found, attempting negotiation                                   */
      COUNT__,
   #pragma pop_macro("ERROR")
   };
   Q_ENUMS(State)

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

   /** @enum Call::Action
   * This enum have all the actions you can make on a call.
   */
   enum class Action : unsigned int
   {
      ACCEPT       = 0, /*!< Accept, create or place call or place transfer */
      REFUSE       = 1, /*!< Red button, refuse or hang up                  */
      TRANSFER     = 2, /*!< Put into or out of transfer mode               */
      HOLD         = 3, /*!< Hold or unhold the call                        */
      RECORD_AUDIO = 4, /*!< Enable or disable audio recording              */
      RECORD_VIDEO = 5, /*!< Enable or disable video recording              */
      RECORD_TEXT  = 6, /*!< Enable or disable text  recording              */
      COUNT__,
   };
   Q_ENUMS(Action)

   /** @enum Call::LifeCycleState
    * This enum help track the call meta state
    * @todo Eventually add a meta state between progress and finished for
    *  calls that are still relevant enough to be in the main UI, such
    *  as BUSY OR FAILURE while also finished
    */
   enum class LifeCycleState {
      CREATION       = 0, /*!< Anything before creating the daemon call   */
      INITIALIZATION = 1, /*!< Anything before the media transfer start   */
      PROGRESS       = 2, /*!< The peers are in communication (or hold)   */
      FINISHED       = 3, /*!< Everything is over, there is no going back */
      COUNT__
   };
   Q_ENUMS(LifeCycleState)

   /** @enum Call::HoldFlags
    * Those flags help track the holding state of a call. Call::State::HOLD is
    * only used for outgoing holding.
    */
   enum class HoldFlags {
   #pragma push_macro("OUT")
   #pragma push_macro("IN")
   #undef OUT
   #undef IN
      NONE = 0x0 << 0, /*!< The call is not on hold        */
      OUT  = 0x1 << 0, /*!< This side put the peer on hold */
      IN   = 0x1 << 1, /*!< The peer put this side on hold */
      COUNT__
   #pragma pop_macro("OUT")
   #pragma pop_macro("IN")
   };
   Q_FLAGS(HoldFlags)

   ///TODO should be deprecated when a better factory system is implemented
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
      constexpr static const char* CERT_PATH         = "cert_path"      ;
   };

   //TODO should be deprecated when a better factory system is implemented
   ///@class HistoryStateName history map fields state names
   class HistoryStateName {
   public:
      constexpr static const char* MISSED         = "missed"  ;
      constexpr static const char* INCOMING       = "incoming";
      constexpr static const char* OUTGOING       = "outgoing";
   };

   //Read only properties
   Q_PROPERTY( Call::State        state              READ state             NOTIFY stateChanged     )
   Q_PROPERTY( QString            historyId          READ historyId                                 )
   Q_PROPERTY( Account*           account            READ account                                   )
   Q_PROPERTY( bool               isHistory          READ isHistory                                 )
   Q_PROPERTY( uint               stopTimeStamp      READ stopTimeStamp                             )
   Q_PROPERTY( uint               startTimeStamp     READ startTimeStamp                            )
   Q_PROPERTY( bool               isSecure           READ isSecure                                  )
   Q_PROPERTY( Video::Renderer*   videoRenderer      READ videoRenderer     NOTIFY videoStarted     )
   Q_PROPERTY( QString            formattedName      READ formattedName                             )
   Q_PROPERTY( QString            length             READ length                                    )
   Q_PROPERTY( bool               recordingAV        READ isAVRecording                             )
   Q_PROPERTY( UserActionModel*   userActionModel    READ userActionModel   CONSTANT                )
   Q_PROPERTY( QString            toHumanStateName   READ toHumanStateName                          )
   Q_PROPERTY( bool               missed             READ isMissed                                  )
   Q_PROPERTY( Direction          direction          READ direction                                 )
   Q_PROPERTY( bool               hasVideo           READ hasVideo                                  )
   Q_PROPERTY( Certificate*       certificate        READ certificate                               )
   Q_PROPERTY( bool               hasParentCall      READ hasParentCall                             )

   //Read/write properties
   Q_PROPERTY( ContactMethod*     peerContactMethod  READ peerContactMethod WRITE setPeerContactMethod)
   Q_PROPERTY( QString            peerName           READ peerName          WRITE setPeerName       )
   Q_PROPERTY( QString            transferNumber     READ transferNumber    WRITE setTransferNumber )
   Q_PROPERTY( QString            dialNumber         READ dialNumber        WRITE setDialNumber      NOTIFY dialNumberChanged(QString))

   //Constructors & Destructors
   static Call* buildHistoryCall  (const QMap<QString,QString>& hc);

   //Static getters
   static const QString      toHumanStateName ( const Call::State );

   //Getters
   Call::State              state            () const;
   const QString            historyId        () const;
   ContactMethod*           peerContactMethod() const;
   const QString            peerName         () const;
   bool                     isAVRecording    () const;
   Account*                 account          () const;
   bool                     isHistory        () const;
   time_t                   stopTimeStamp    () const;
   time_t                   startTimeStamp   () const;
   bool                     isSecure         () const;
   const QString            transferNumber   () const;
   const QString            dialNumber       () const;
   Video::Renderer*         videoRenderer    () const;
   const QString            formattedName    () const;
   QString                  length           () const;
   UserActionModel*         userActionModel  () const;
   QString                  toHumanStateName () const;
   bool                     isMissed         () const;
   Call::Direction          direction        () const;
   bool                     hasVideo         () const;
   Call::LifeCycleState     lifeCycleState   () const;
   Call::Type               type             () const;
   bool                     hasRemote        () const;
   Certificate*             certificate      () const;
   FlagPack<HoldFlags>      holdFlags        () const;
   bool                     hasParentCall    () const;
   QDateTime                dateTime         () const;
   QDate                    date             () const;

   Q_INVOKABLE QVariant   roleData         (int  role) const;
   Q_INVOKABLE QVariant   roleData         (Role role) const;
   Q_INVOKABLE QMimeData* mimePayload      (         ) const;

   template<typename T>
   T* firstMedia(Media::Media::Direction direction) const;
   QList<Media::Recording*> recordings  (Media::Media::Type type, Media::Media::Direction direction) const;
   QList<Media::Media*>     media       (Media::Media::Type type, Media::Media::Direction direction) const;
   bool                     hasMedia    (Media::Media::Type type, Media::Media::Direction direction) const;
   bool                     hasRecording(Media::Media::Type type, Media::Media::Direction direction) const;
   bool                     isRecording (Media::Media::Type type, Media::Media::Direction direction) const;
   QList<Media::Media*>     allMedia    (                                                          ) const;

   //Automated function
   Q_INVOKABLE Call::State performAction(Call::Action action);

   //Setters
   void setTransferNumber ( const QString&     number     );
   void setDialNumber     ( const QString&     number     );
   void setDialNumber     ( const ContactMethod* number   );
   void setPeerContactMethod( ContactMethod* cm           );
   void setPeerName       ( const QString&     name       );
   void setAccount        ( Account*           account    );
   void setParentCall     ( Call*               call      );

   //Mutators
   template<typename T>
   T* addOutgoingMedia(bool useExisting = true);
   Q_INVOKABLE void appendText(const QString& str);
   void backspaceItemText();
   void reset();
   bool joinToParent();

   //syntactic sugar
   Call* operator<<( Call::Action& c);

private:
   Call(const QString& confId, const QString& account);
   ~Call();
   explicit Call(Call::State startState, const QString& peerName = QString(), ContactMethod* number = nullptr, Account* account = nullptr); //TODO MOVE TO PRIVATE

   //Friend API
   const QString dringId() const;

   CallPrivate* d_ptr;
   Q_DECLARE_PRIVATE(Call)

public Q_SLOTS:
   void playDTMF(const QString& str);

Q_SIGNALS:
   ///Emitted when a call change (state or details)
   void changed();
   ///Emitted when the call is over
   void isOver();
   ///Notify that a DTMF have been played
   void dtmfPlayed(const QString& str);
   ///Notify of state change
   void stateChanged(Call::State newState, Call::State previousState);
   ///Notify that the lifeCycleStateChanged
   void lifeCycleStateChanged(Call::LifeCycleState newState, Call::LifeCycleState previousState);
   ///The call start timestamp changed, this usually indicate the call has started
   void startTimeStampChanged(time_t newTimeStamp);
   ///The dial number has changed
   void dialNumberChanged(const QString& number);
   ///Announce a new video renderer
   void videoStarted(Video::Renderer* renderer); //TODO remove, use the media signals
   ///Remove a new video renderer
   void videoStopped(Video::Renderer* renderer); //TODO remove, use the media signals
   ///Notify when a media is added
   void mediaAdded(Media::Media* media);
   ///Notify when a media state change
   void mediaStateChanged(Media::Media* media, const Media::Media::State s, const Media::Media::State m);
   ///The holding combination has changed
   void holdFlagsChanged(const FlagPack<HoldFlags>& current, const FlagPack<HoldFlags>& previous);
};

Q_DECLARE_METATYPE(Call*)
Q_DECLARE_METATYPE(Call::State)
Q_DECLARE_METATYPE(Call::Type)
Q_DECLARE_METATYPE(Call::Action)
Q_DECLARE_METATYPE(Call::Direction)
Q_DECLARE_METATYPE(Call::LifeCycleState)

DECLARE_ENUM_FLAGS(Call::HoldFlags)

Call* operator<<(Call* c, Call::Action a);
QDebug LIB_EXPORT operator<<(QDebug dbg, const Call::State& c       );
QDebug LIB_EXPORT operator<<(QDebug dbg, const Call::Action& c      );

#include <call.hpp>
