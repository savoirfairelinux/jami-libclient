/****************************************************************************
 *   Copyright (C) 2009-2015 by Savoir-Faire Linux                          *
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

#ifndef CALL_MODEL2_H
#define CALL_MODEL2_H

#include <QAbstractItemModel>
#include <QMap>
#include "typedefs.h"

//Ring
#include "call.h"
class Account;
struct InternalStruct;
class ContactMethod;
class CallModelPrivate;

//Typedef
typedef QMap<uint, Call*>  CallMap;
typedef QList<Call*>       CallList;

///CallModel: Central model/frontend to deal with dring
class LIB_EXPORT CallModel : public QAbstractItemModel
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
   public:
      ///Accepted (mime) payload types
      enum DropPayloadType {
         NONE    = 0   , /*!< No drop payload is supported                                            */
         CALL    = 1<<0, /*!< The item support the dropping of a current or history call              */
         HISTORY = 1<<1, /*!< The item support the dropping of a history call                         */
         CONTACT = 1<<2, /*!< The item support the dropping of a contact id                           */
         NUMBER  = 1<<3, /*!< The item support the dropping of an URI                                 */
         TEXT    = 1<<4, /*!< The item support the dropping of random text (to be interpreted as URI) */
         ACCOUNT = 1<<5, /*!< The item support the dropping of an account id                          */
      };

      //Properties
      Q_PROPERTY(int  size          READ size         )
      Q_PROPERTY(bool hasConference READ hasConference)
      Q_PROPERTY(int  callCount     READ rowCount     )

      //Constructors, initializer and destructors
      virtual ~CallModel( );

      //Call related
      Q_INVOKABLE Call* dialingCall      ( const QString& peerName=QString(), Account* account=nullptr );
      Q_INVOKABLE void  attendedTransfer ( Call* toTransfer , Call* target              );
      Q_INVOKABLE void  transfer         ( Call* toTransfer , const ContactMethod* target );
      QModelIndex getIndex               ( Call* call                                   );

      //Conference related
      Q_INVOKABLE bool createConferenceFromCall ( Call* call1, Call* call2      );
      Q_INVOKABLE bool mergeConferences         ( Call* conf1, Call* conf2      );
      Q_INVOKABLE bool addParticipant           ( Call* call2, Call* conference );
      Q_INVOKABLE bool detachParticipant        ( Call* call                    );

      //Getters
      Q_INVOKABLE bool     isValid             ();
      Q_INVOKABLE int      size                ();
      Q_INVOKABLE CallList getActiveCalls      ();
      Q_INVOKABLE CallList getActiveConferences();
      Q_INVOKABLE int      acceptedPayloadTypes();
      Q_INVOKABLE bool     hasConference       () const;
      Q_INVOKABLE bool     isConnected         () const;

      Q_INVOKABLE Call* getCall ( const QString& callId  ) const;
      Q_INVOKABLE Call* getCall ( const QModelIndex& idx ) const;

      //Model implementation
      virtual bool          setData      ( const QModelIndex& index, const QVariant &value, int role   ) override;
      virtual QVariant      data         ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
      virtual int           rowCount     ( const QModelIndex& parent = QModelIndex()                   ) const override;
      virtual Qt::ItemFlags flags        ( const QModelIndex& index                                    ) const override;
      virtual int           columnCount  ( const QModelIndex& parent = QModelIndex()                   ) const override;
      virtual QModelIndex   parent       ( const QModelIndex& index                                    ) const override;
      virtual QModelIndex   index        ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
      virtual QVariant      headerData   ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
      virtual QStringList   mimeTypes    (                                                             ) const override;
      virtual QMimeData*    mimeData     ( const QModelIndexList &indexes                              ) const override;
      virtual bool          dropMimeData ( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent ) override;
      virtual QHash<int,QByteArray> roleNames() const override;

      //Singleton
      static CallModel* instance();

   private:
      explicit CallModel();
      QScopedPointer<CallModelPrivate> d_ptr;
      Q_DECLARE_PRIVATE(CallModel)

      //Singleton
      static CallModel* m_spInstance;

   Q_SIGNALS:
      ///Emitted when a call state change
      void callStateChanged        ( Call* call, Call::State previousState   );
      ///Emitted when a new call is incoming
      void incomingCall            ( Call* call                              );
      ///Emitted when a conference is created
      void conferenceCreated       ( Call* conf                              );
      ///Emitted when a conference change state or participant
      void conferenceChanged       ( Call* conf                              );
      ///Emitted when a conference is removed
      void conferenceRemoved       ( Call* conf                              );
      ///Emitted when a call is added
      void callAdded               ( Call* call               , Call* parent );
};
Q_DECLARE_METATYPE(CallModel*)

#endif
