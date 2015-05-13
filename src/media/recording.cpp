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
#include "recording.h"

//Qt
#include <QtCore/QUrl>

//DRing
#include "dbus/callmanager.h"

//Ring
#include <callmodel.h>

/**
 * Keep track of currently active recordings
 */
class RecordingPlaybackManager : public QObject
{
   Q_OBJECT
public:
   RecordingPlaybackManager();

   //Attributes
   QList<Media::Recording*>         m_lActiveRecordings;
   QHash<QString,Media::Recording*> m_hActiveRecordings;

   //Mutator
   void activateRecording   (Media::Recording* r);
   void desactivateRecording(Media::Recording* r);

   //Singleton
   static RecordingPlaybackManager* instance();

private:
   static RecordingPlaybackManager* m_spInstance;

public Q_SLOTS:
   void slotRecordPlaybackFilepath(const QString& callID, const QString& filepath );
   void slotRecordPlaybackStopped (const QString& filepath                        );
   void slotUpdatePlaybackScale   (const QString& filepath, int position, int size);
};

RecordingPlaybackManager* RecordingPlaybackManager::m_spInstance = nullptr;


RecordingPlaybackManager::RecordingPlaybackManager() : QObject(CallModel::instance())
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   connect(&callManager,&CallManagerInterface::recordPlaybackStopped , this, &RecordingPlaybackManager::slotRecordPlaybackStopped );
   connect(&callManager,&CallManagerInterface::updatePlaybackScale   , this, &RecordingPlaybackManager::slotUpdatePlaybackScale   );
   connect(&callManager,&CallManagerInterface::recordPlaybackFilepath, this, &RecordingPlaybackManager::slotRecordPlaybackFilepath);
}

namespace Media {

class RecordingPrivate {
public:
   RecordingPrivate(Recording* r);

   //Attributes
   QUrl m_Path;
   Recording::Type     m_Type    ;
   Recording::Position m_Position;
   int                 m_Duration;
   bool                m_IsPaused;
   int                 m_Elapsed ;
   int                 m_Left    ;

   //Constants
   constexpr static const char minutes[] = "%1:%2";
   constexpr static const char hours  [] = "%1:%2:%3";

   //Helper
   void notifySeek(int position, int size);

private:
   Recording* q_ptr;
};

constexpr const char RecordingPrivate::minutes[];
constexpr const char RecordingPrivate::hours  [];

RecordingPrivate::RecordingPrivate(Recording* r) : q_ptr(r),m_Position(0.0),m_Duration(0),m_IsPaused(false),
m_Elapsed(0),m_Left(0)
{

}

Recording::Recording(const Recording::Type type) : ItemBase<QObject>(nullptr), d_ptr(new RecordingPrivate(this))
{
   d_ptr->m_Type = type;
}

Recording::~Recording()
{
   delete d_ptr;
}

} //Media::

///Singleton
RecordingPlaybackManager* RecordingPlaybackManager::instance()
{
   if (!m_spInstance)
      m_spInstance = new RecordingPlaybackManager();

   return m_spInstance;
}

///Return this Recording type
Media::Recording::Type Media::Recording::type() const
{
   return d_ptr->m_Type;
}

///Return this recording path, if any
QUrl Media::Recording::path() const
{
   return d_ptr->m_Path;
}

///Get the current playback position (0.0 if not playing)
Media::Recording::Position Media::Recording::position() const
{
   return d_ptr->m_Position;
}

/**
 * Recording duration (in seconds). This is only available for audio/video
 * recordings and only after playback started.
 */
int Media::Recording::duration() const
{
   return d_ptr->m_Duration;
}

#define FORMATTED_TIME_MACRO(var) if (d_ptr->m_Duration > 3599) {\
      return QString(RecordingPrivate::hours)\
         .arg(var/3600                     )\
         .arg((var%3600)/60,2,10,QChar('0'))\
         .arg(var%60,2,10,QChar('0')       );\
   }\
   else {\
      return QString(RecordingPrivate::minutes)\
         .arg(var/60,2,10,QChar('0'))\
         .arg(var%60,2,10,QChar('0'));\
   }

///Get the a string in format hhh:mm:ss for the time elapsed since the playback started
QString Media::Recording::formattedTimeElapsed() const
{
   FORMATTED_TIME_MACRO(d_ptr->m_Elapsed);
}

///Get the a string in format hhh:mm:ss for the total time of this recording
QString Media::Recording::formattedDuration() const
{
   FORMATTED_TIME_MACRO(d_ptr->m_Duration);
}

///Get the a string in format hhh:mm:ss for the time elapsed before the playback end
QString Media::Recording::formattedTimeLeft() const
{
   FORMATTED_TIME_MACRO(d_ptr->m_Left);
}
#undef FORMATTED_TIME_MACRO

void Media::Recording::setPath(const QUrl& path)
{
   d_ptr->m_Path = path;
}

///Play (or resume) the playback
void Media::Recording::play()
{
   RecordingPlaybackManager::instance()->activateRecording(this);

   CallManagerInterface& callManager = DBus::CallManager::instance();
   const bool retval = callManager.startRecordedFilePlayback(path().path());
   if (retval)
      emit started();

   if (d_ptr->m_IsPaused) {
      seek(d_ptr->m_Position);
      d_ptr->m_IsPaused = false;
   }
}

///Stop the playback, cancel any pause point
void Media::Recording::stop()
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   Q_NOREPLY callManager.stopRecordedFilePlayback(path().path());
   emit stopped();

   RecordingPlaybackManager::instance()->desactivateRecording(this);
   d_ptr->m_IsPaused = false;
}

///Pause (or resume)
void Media::Recording::pause()
{
   if (d_ptr->m_IsPaused) {
      play();
   }
   else {
      stop();
      d_ptr->m_IsPaused = true;
   }
}

/**
 * Change position in this recording
 * @note only available during playback
 * @args pos The position, in percent
 */
void Media::Recording::seek(Recording::Position pos)
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   Q_NOREPLY callManager.recordPlaybackSeek(pos);
}

///Move the playback position to the origin
void Media::Recording::reset()
{
   seek(0.0);
}

///Update all internal playback metadatas
void Media::RecordingPrivate::notifySeek(int position, int size)
{
   const int oldElapsed  = m_Elapsed ;
   const int oldDuration = m_Duration;

   //Update the metadata
   m_Duration = size/1000;
   m_Position = ((double)position)/((double)size);
   m_Elapsed  = m_Duration * m_Position;
   m_Left     = m_Duration - m_Elapsed ;

   if (oldDuration != m_Duration)
      emit q_ptr->formattedDurationChanged(q_ptr->formattedDuration());

   if (oldElapsed != m_Elapsed) {
      emit q_ptr->formattedTimeElapsedChanged(q_ptr->formattedTimeElapsed());
      emit q_ptr->formattedTimeLeftChanged   (q_ptr->formattedTimeLeft   ());
   }

   emit q_ptr->playbackPositionChanged(m_Position);
}

///Callback when a recording start
void RecordingPlaybackManager::slotRecordPlaybackFilepath(const QString& callID, const QString& filepath)
{
   //TODO is this method dead code?
   qDebug() << "Playback started" << callID << filepath;
}

///Callback when a recording stop
void RecordingPlaybackManager::slotRecordPlaybackStopped(const QString& filepath)
{
   Media::Recording* r = m_hActiveRecordings[filepath];
   if (r) {
      desactivateRecording(r);
   }
}

///Callback when a recording position changed
void RecordingPlaybackManager::slotUpdatePlaybackScale(const QString& filepath, int position, int size)
{
   Media::Recording* r = m_hActiveRecordings[filepath];

   if (r) {
      r->d_ptr->notifySeek(position, size);
   }
   else
      qDebug() << "Unregistered recording position changed" << filepath;
}

///To avoid having to keep a list of all recording, manage a small active recording list
void RecordingPlaybackManager::activateRecording(Media::Recording* r)
{
   m_lActiveRecordings << r;
   m_hActiveRecordings[r->path().path()] = r;
}

///To avoid having to keep a list of all recording, manage a small active recording list
void RecordingPlaybackManager::desactivateRecording(Media::Recording* r)
{
   m_lActiveRecordings.removeAll(r);
   m_hActiveRecordings.remove(m_hActiveRecordings.key(r));
}

#include <recording.moc>
