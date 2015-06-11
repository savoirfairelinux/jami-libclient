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
#ifndef USERACTIONS_H
#define USERACTIONS_H

#include "media/media.h"
#include "media/audio.h"
#include "media/video.h"

/**
 * This code used to be in the KDE client. It doesn't really fit well in well
 * in what libringclient is supposed to do, but as it has to be replicated for
 * each clients, then UserActionModel will provide an abstract way to call those
 * functions.
 */
namespace UserActions {

bool addNew();
bool accept(const QList<Call*> calls);
bool hangup(const QList<Call*> calls);
bool refuse(const QList<Call*> calls);
bool hold(const QList<Call*> calls);
bool unhold(const QList<Call*> calls);
bool transfer(const QList<Call*> calls);
bool recordAudio(const QList<Call*> calls);
bool recordVideo(const QList<Call*> calls);
bool recordText(const QList<Call*> calls);
bool muteAudio(const QList<Call*> calls, bool state);
bool muteVideo(const QList<Call*> calls, bool state);

bool addNew()
{
   Call* call = CallModel::instance()->dialingCall();
      CallModel::instance()->selectionModel()->setCurrentIndex(CallModel::instance()->getIndex(call), QItemSelectionModel::ClearAndSelect);
      return true;
}

bool accept(const QList<Call*> calls)
{
   bool ret = true;

   //Add a new call if none is there
   if (!calls.size()) {
      return addNew();
   }

   for (Call* call : calls) {
      const Call::State state = call->state();
      //TODO port to lifeCycle code
      if (state == Call::State::RINGING || state == Call::State::CURRENT || state == Call::State::HOLD
         || state == Call::State::BUSY || state == Call::State::FAILURE || state == Call::State::ERROR) {
         qDebug() << "Calling when item currently ringing, current, hold or busy. Opening an item.";
         Call* c2 = CallModel::instance()->dialingCall();
         CallModel::instance()->selectionModel()->setCurrentIndex(CallModel::instance()->getIndex(c2), QItemSelectionModel::ClearAndSelect);
      }
      else {
         try {
            call->performAction(Call::Action::ACCEPT);
         }
         catch(const char * msg) {
//                KMessageBox::error(Ring::app(),i18n(msg));
            ret = false;
         }
      }
   }
   return ret;
} //accept

///Call
bool hangup(const QList<Call*> calls)
{
   bool ret = true;
   for (Call* call : calls) {
      if (call) {
         try {
            call->performAction(Call::Action::REFUSE);
         }
         catch(const char * msg) {
//             KMessageBox::error(Ring::app(),i18n(msg));
         }
      }
   }
   return ret;
} //hangup

///Refuse call
bool refuse(const QList<Call*> calls)
{
   bool ret = true;
   for (Call* call : calls) {
      if(!call) {
         qDebug() << "Error : Hanging up when no item selected. Should not happen.";
      }
      else {
         try {
            call->performAction(Call::Action::REFUSE);
         }
         catch(const char * msg) {
//             KMessageBox::error(Ring::app(),i18n(msg));
         }
      }
   }
   return ret;
}

///Put call on hold
bool hold(const QList<Call*> calls)
{
   bool ret = true;
   for (Call* call : calls) {
      if(!call) {
         qDebug() << "Error : Holding when no item selected. Should not happen.";
      }
      else {
         try {
            call->performAction(Call::Action::HOLD);
         }
         catch(const char * msg) {
//             KMessageBox::error(Ring::app(),i18n(msg));
         }
      }
   }
   return ret;
}

///Remove call from hold
bool unhold(const QList<Call*> calls)
{
   bool ret = true;
   for (Call* call : calls) {
      if(!call) {
         qDebug() << "Error : Un-Holding when no item selected. Should not happen.";
      }
      else {
         try {
            call->performAction(Call::Action::HOLD);
         }
         catch(const char * msg) {
//             KMessageBox::error(Ring::app(),i18n(msg));
         }
      }
   }
   return ret;
}

///Transfer a call
bool transfer(const QList<Call*> calls)
{
   bool ret = true;
   for (Call* call : calls) {
      if(!call) {
         qDebug() << "Error : Transferring when no item selected. Should not happen.";
      }
      else {
         try {
            call->performAction(Call::Action::TRANSFER);
         }
         catch(const char * msg) {
//             KMessageBox::error(Ring::app(),i18n(msg));
         }
      }
   }
   return ret;
}

///Record a call
bool recordAudio(const QList<Call*> calls)
{
   bool ret = true;
   for (Call* call : calls) {
      if(!call) {
         qDebug() << "Error : Recording when no item selected. Should not happen.";
      }
      else {
         try {
            call->performAction(Call::Action::RECORD_AUDIO);
         }
         catch(const char * msg) {
//             KMessageBox::error(Ring::app(),i18n(msg));
         }
      }
   }
   return ret;
}

///Record a call
bool recordVideo(const QList<Call*> calls)
{
   bool ret = true;
   for (Call* call : calls) {
      if(!call) {
         qDebug() << "Error : Recording when no item selected. Should not happen.";
      }
      else {
         try {
            call->performAction(Call::Action::RECORD_VIDEO);
         }
         catch(const char * msg) {
//             KMessageBox::error(Ring::app(),i18n(msg));
         }
      }
   }
   return ret;
}

///Record a call
bool recordText(const QList<Call*> calls)
{
   bool ret = true;
   for (Call* call : calls) {
      if(!call) {
         qDebug() << "Error : Recording when no item selected. Should not happen.";
      }
      else {
         try {
            call->performAction(Call::Action::RECORD_TEXT);
         }
         catch(const char * msg) {
//             KMessageBox::error(Ring::app(),i18n(msg));
         }
      }
   }
   return ret;
}

///Mute call audio
bool muteAudio(const QList<Call*> calls, bool state)
{
   bool ret = true;
   for (Call* call : calls) {
      if(!call) {
         qDebug() << "Error : Muting audio when no item selected. Should not happen.";
      }
      else {
         if (!call->firstMedia<Media::Video>(Media::Media::Direction::OUT)) {
            qDebug() << "Error : No audio media for this call";
         } else if (state) {
            if (!call->firstMedia<Media::Audio>(Media::Media::Direction::OUT)->mute())
               qDebug() << "Error : Could not mute audio of selected call";
         } else {
            if (!call->firstMedia<Media::Audio>(Media::Media::Direction::OUT)->unmute())
               qDebug() << "Error : Could not un-mute audio of selected call";
         }
      }
   }
   return ret;
}

///Mute call video
bool muteVideo(const QList<Call*> calls, bool state)
{
   bool ret = true;
   for (Call* call : calls) {
      if(!call) {
         qDebug() << "Error : Muting video when no item selected. Should not happen.";
      }
      else {
         if (!call->firstMedia<Media::Video>(Media::Media::Direction::OUT)) {
              qDebug() << "Error : No video media for this call";
         } else if (state) {
            if (!call->firstMedia<Media::Video>(Media::Media::Direction::OUT)->mute())
               qDebug() << "Error : Could not mute video of selected call";
         } else {
            if (!call->firstMedia<Media::Video>(Media::Media::Direction::OUT)->unmute())
               qDebug() << "Error : Could not un-mute video of selected call";
         }
      }
   }
   return ret;
}

} //namespace UserActions

#endif
