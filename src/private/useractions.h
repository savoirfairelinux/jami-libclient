/****************************************************************************
 *   Copyright (C) 2012-2016 by Savoir-faire Linux                          *
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
#pragma once

// Qt
#include <QtCore/QProcess>
#include <QtCore/QMimeData>
#include <QtCore/QStringBuilder>

// LRC
#include <categorizedbookmarkmodel.h>
#include <categorizedhistorymodel.h>
#include <personmodel.h>
#include <interfaces/actionextenderi.h>
#include <interfaces/itemmodelstateserializeri.h>
#include <mime.h>

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

bool addNew           (                        );
bool accept           ( Call* call             );
bool hangup           ( Call* call             );
bool refuse           ( Call* call             );
bool hold             ( Call* call             );
bool unhold           ( Call* call             );
bool transfer         ( Call* call             );
bool recordAudio      ( Call* call             );
bool recordVideo      ( Call* call             );
bool recordText       ( Call* call             );
bool muteAudio        ( Call* call, bool state );
bool muteVideo        ( Call* call, bool state );
bool sendEmail        ( const Person* p        );
bool callAgain        ( ContactMethod* cm      );
bool bookmark         ( ContactMethod* cm      );
bool deleteContact    ( Person* p              );
bool removeFromHistory( Call* c                );
bool editPerson       ( Person* p              );
bool addToPerson      ( ContactMethod* cm      );
bool addToPerson      ( Person* p              );

bool addPerson(ContactMethod* cm, CollectionInterface* col = nullptr);

bool addNew()
{
   Call* call = CallModel::instance().dialingCall();
   CallModel::instance().selectionModel()->setCurrentIndex(CallModel::instance().getIndex(call), QItemSelectionModel::ClearAndSelect);
   return true;
}

bool accept(Call* call)
{
   bool ret = true;

   //Add a new call if none is there
   if (!call) {
      return addNew();
   }

   const Call::State state = call->state();
   //TODO port to lifeCycle code
   if (state == Call::State::RINGING || state == Call::State::CURRENT || state == Call::State::HOLD
      || state == Call::State::BUSY || state == Call::State::FAILURE || state == Call::State::ERROR) {
      qDebug() << "Calling when item currently ringing, current, hold or busy. Opening an item.";
      Call* c2 = CallModel::instance().dialingCall();
      CallModel::instance().selectionModel()->setCurrentIndex(CallModel::instance().getIndex(c2), QItemSelectionModel::ClearAndSelect);
   }
   else {
      try {
         call->performAction(Call::Action::ACCEPT);
      }
      catch(const char * msg) {
         ret = false;
      }
   }
   return ret;
} //accept

///Call
bool hangup(Call* call)
{
   bool ret = true;

   if (call) {
      try {
         call->performAction(Call::Action::REFUSE);
      }
      catch(const char * msg) {
         return false;
      }
   }

   return ret;
} //hangup

///Refuse call
bool refuse(Call* call)
{
   bool ret = true;

   if(!call) {
      qDebug() << "Error : Hanging up when no item selected. Should not happen.";
   }
   else {
      try {
         call->performAction(Call::Action::REFUSE);
      }
      catch(const char * msg) {
         return false;
      }
   }

   return ret;
}

///Put call on hold
bool hold(Call* call)
{
   bool ret = true;

   if(!call) {
      qDebug() << "Error : Holding when no item selected. Should not happen.";
   }
   else {
      try {
         call->performAction(Call::Action::HOLD);
      }
      catch(const char * msg) {
         return false;
      }
   }

   return ret;
}

///Remove call from hold
bool unhold(Call* call)
{
   bool ret = true;

   if(!call) {
      qDebug() << "Error : Un-Holding when no item selected. Should not happen.";
   }
   else {
      try {
         call->performAction(Call::Action::HOLD);
      }
      catch(const char * msg) {
         return false;
      }
   }

   return ret;
}

///Transfer a call
bool transfer(Call* call)
{
   bool ret = true;

   if(!call) {
      qDebug() << "Error : Transferring when no item selected. Should not happen.";
   }
   else {
      try {
         call->performAction(Call::Action::TRANSFER);
      }
      catch(const char * msg) {
         return false;
      }
   }

   return ret;
}

///Record a call
bool recordAudio(Call* call)
{
   bool ret = true;

   if(!call) {
      qDebug() << "Error : Recording when no item selected. Should not happen.";
   }
   else {
      try {
         call->performAction(Call::Action::RECORD_AUDIO);
      }
      catch(const char * msg) {
         return false;
      }
   }

   return ret;
}

///Record a call
bool recordVideo(Call* call)
{
   bool ret = true;

   if(!call) {
      qDebug() << "Error : Recording when no item selected. Should not happen.";
   }
   else {
      try {
         call->performAction(Call::Action::RECORD_VIDEO);
      }
      catch(const char * msg) {
         return false;
      }
   }

   return ret;
}

///Record a call
bool recordText(Call* call)
{
   bool ret = true;

   if(!call) {
      qDebug() << "Error : Recording when no item selected. Should not happen.";
   }
   else {
      try {
         call->performAction(Call::Action::RECORD_TEXT);
      }
      catch(const char * msg) {
         return false;
      }
   }

   return ret;
}

///Mute call audio
bool muteAudio(Call* call, bool state)
{
   bool ret = true;

   if(!call) {
      qDebug() << "Error : Muting audio when no item selected. Should not happen.";
      return false;
   }
   auto audioOut = call->firstMedia<Media::Audio>(Media::Media::Direction::OUT);
   if (!audioOut) {
      qDebug() << "Error : No audio media for this call";
      return false;
   }
   if (state) {
      if (!audioOut->mute())
         qDebug() << "Error : Could not mute audio of selected call";
   } else {
      if (!audioOut->unmute())
         qDebug() << "Error : Could not un-mute audio of selected call";
   }

   return ret;
}

///Mute call video
bool muteVideo(Call* call, bool state)
{
   bool ret = true;

   if(!call) {
      qDebug() << "Error : Muting video when no item selected. Should not happen.";
      return false;
   }
   auto videoOut = call->firstMedia<Media::Video>(Media::Media::Direction::OUT);
   if (!videoOut) {
      qDebug() << "Error : No video media for this call";
      return false;
   }
   if (state) {
      if (!videoOut->mute())
         qDebug() << "Error : Could not mute video of selected call";
   } else {
      if (!videoOut->unmute())
         qDebug() << "Error : Could not un-mute video of selected call";
   }

   return ret;
}

bool sendEmail(const Person* p)
{
   if (!p || p->preferredEmail().isEmpty())
      return false;

   qDebug() << "Sending email";
   QProcess *myProcess = new QProcess(QCoreApplication::instance());
   QStringList arguments;
#ifdef Q_OS_WIN32
   myProcess->start("start", { "mailto:"+p->preferredEmail() });
#elif defined Q_OS_DARWIN
   myProcess->start("open", { "mailto:"+p->preferredEmail() });
#else
   myProcess->start("xdg-email", { "mailto:"+p->preferredEmail() });
#endif
   return true;
}

bool callAgain(ContactMethod* cm)
{
   if (!cm)
      return false;

   Call* call = CallModel::instance().dialingCall(cm);

   if (call) {
      call->performAction(Call::Action::ACCEPT);
      return true;
   }

   return false;
}

bool bookmark(ContactMethod* cm)
{
   if (!cm)
      return false;

   CategorizedBookmarkModel::instance().addBookmark(cm);

   return true;
}

bool deleteContact(Person* p)
{
   if (!p)
      return false;

   if (GlobalInstances::actionExtender().warnDeletePerson(p))
      return p->remove();

   return false;
}

bool removeFromHistory(Call* c)
{
   if (c && c->collection()->supportedFeatures() & CollectionInterface::SupportedFeatures::REMOVE) {

      if (GlobalInstances::actionExtender().warnDeleteCall(c)) {
         CategorizedHistoryModel::instance().deleteItem(c); //TODO add add and remove to the manager
         return true;
      }
   }

   return false;
}

/**
 * If the client want GUI edition, this can be implemented as part of the
 * collection edition features themselves.
 */
bool editPerson( Person* p )
{
   if (!p)
      return false;

   return p->edit();
}

bool addPerson(ContactMethod* cm, CollectionInterface* col)
{
   // If the placeholder has not yet been replaced, it might point to a
   // deleted contact, as the user initiated this action, lets comply
   if (cm->contact() && !cm->contact()->isPlaceHolder())
      return false;

   // Try to get the best collection for this
   if (!col)
      col = GlobalInstances::itemModelStateSerializer().preferredCollection(
         &PersonModel::instance(),
         CollectionInterface::SupportedFeatures::ADD
      );

   // Take a random collection that match
   if (!col) {
      const QVector<CollectionInterface*> cols = PersonModel::instance()
         .collections(CollectionInterface::SupportedFeatures::ADD);

      if (cols.isEmpty())
         return false;

      //TODO support collection selection
      col = cols.first();
   }

   Person* p = new Person();

   p->setFormattedName(cm->primaryName());
   p->setContactMethods({cm});

   cm->setPerson(p);

   col->add(p);

   Q_ASSERT_X(p->collection() == col,
      "addPerson",
      "The collection doesn't match. This is an internal Ring bug, please "
      "report it"
   );

   if (!p->save())
      return false;

   return editPerson(p);
}

bool addToPerson(ContactMethod* cm)
{
   if (!cm)
      return false;

   Person* p = GlobalInstances::actionExtender().selectPerson();

   if (!p || !(p->collection()->supportedFeatures() & CollectionInterface::SupportedFeatures::EDIT))
      return false;

   auto numbers = p->phoneNumbers();
   numbers << cm;
   p->setContactMethods(numbers);

   return p->save();
}

bool addToPerson(Person* p)
{
   if (!p || !(p->collection()->supportedFeatures() & CollectionInterface::SupportedFeatures::EDIT))
      return false;

   ContactMethod* cm = GlobalInstances::actionExtender().selectContactMethod(
      Interfaces::ActionExtenderI::SelectContactMethodHint::RECENT |
      Interfaces::ActionExtenderI::SelectContactMethodHint::WITHOUT_PERSON
   );

   if (!cm)
      return false;

   auto numbers = p->phoneNumbers();
   numbers << cm;
   p->setContactMethods(numbers);

   return p->save();
}

} //namespace UserActions
