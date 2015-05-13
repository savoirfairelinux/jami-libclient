/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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
#ifndef MEDIA_RECORDING_H
#define MEDIA_RECORDING_H

#include <QtCore/QObject>
#include <itembase.h>

#include <typedefs.h>

class RecordingPlaybackManager;

namespace Media {

class RecordingPrivate;

/**
 * @class Recording a representation of one or more media recording
 */
class LIB_EXPORT Recording : public ItemBase<QObject>
{
   Q_OBJECT

   friend class ::RecordingPlaybackManager;

public:
   //Types
   typedef double Position;

   //Properties
   Q_PROPERTY( Recording::Type     type                 READ type                                                   )
   Q_PROPERTY( QUrl                path                 READ path                                                   )
   Q_PROPERTY( Recording::Position position             READ position                                               )
   Q_PROPERTY( int                 duration             READ duration                                               )
   Q_PROPERTY( QString             formattedTimeElapsed READ formattedTimeElapsed NOTIFY formattedTimeElapsedChanged)
   Q_PROPERTY( QString             formattedDuration    READ formattedDuration    NOTIFY formattedDurationChanged   )
   Q_PROPERTY( QString             formattedTimeLeft    READ formattedTimeLeft    NOTIFY formattedTimeLeftChanged   )

   enum class Type {
      AUDIO_VIDEO, /*!< The recording is a single file, playable by the daemon */
      TEXT       , /*!< The recording is an encoded text stream and a position */
      /*FILE*/
   };

   //Constructor
   explicit Recording(const Recording::Type type);
   virtual ~Recording();

   //Getter
   Recording::Type     type                () const;
   QUrl                path                () const;
   Recording::Position position            () const;
   int                 duration            () const;
   QString             formattedTimeElapsed() const;
   QString             formattedDuration   () const;
   QString             formattedTimeLeft   () const;

   //Setter
   void setPath(const QUrl& path);

private:
   RecordingPrivate* d_ptr;
   Q_DECLARE_PRIVATE(Recording)

public Q_SLOTS:
   void play (                         );
   void stop (                         );
   void pause(                         );
   void seek ( Recording::Position pos );
   void reset(                         );

Q_SIGNALS:
   /**
    * The recording playback position changed
    * @args pos The position, in percent
    */
   void playbackPositionChanged(Recording::Position pos);
   ///The recording playback has stopped
   void stopped();
   ///The recording playback has started
   void started();
   ///Emitted when the formatted elapsed time string change
   void formattedTimeElapsedChanged(const QString& formattedValue);
   ///Emitted when the formatted duration string change
   void formattedDurationChanged   (const QString& formattedValue);
   ///Emitted when the formatted time left string change
   void formattedTimeLeftChanged   (const QString& formattedValue);

};

}

#endif