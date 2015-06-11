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
#include "media.h"

#include "../private/media_p.h"
#include <call.h>

const Matrix2D<Media::Media::State, Media::Media::Action, bool> Media::MediaPrivate::m_mValidTransitions ={{
   /*                MUTE   UNMUTE  TERMINATE */
   /* ACTIVE   */ {{ true  , true  , true     }},
   /* MUTED    */ {{ true  , true  , true     }},
   /* IDLE     */ {{ true  , false , true     }},
   /* OVER     */ {{ false , false , false    }},
}};

//Use the Media::MediaPrivate wrapper to avoid vtable issues
#define MEDF &MediaPrivate
const Matrix2D<Media::Media::State, Media::Media::Action, Media::MediaTransitionFct> Media::MediaPrivate::m_mCallbacks ={{
   /*                     MUTE           UNMUTE         TERMINATE     */
   /* ACTIVE   */ {{ MEDF::mute    , MEDF::nothing , MEDF::terminate }},
   /* MUTED    */ {{ MEDF::nothing , MEDF::unmute  , MEDF::terminate }},
   /* IDLE     */ {{ MEDF::mute    , MEDF::nothing , MEDF::terminate }},
   /* OVER     */ {{ MEDF::nothing , MEDF::nothing , MEDF::nothing   }},
}};
#undef MEDF

namespace Media {

MediaPrivate::MediaPrivate(Media* parent) :
 m_State(Media::Media::State::ACTIVE),m_pCall(nullptr),q_ptr(parent),m_Direction(Media::Direction::OUT)
{

}

Media::Media(Call* parent, const Direction dir) : QObject(parent), d_ptr(new MediaPrivate(this))
{
   Q_ASSERT(parent);
   d_ptr->m_pCall = parent;
   d_ptr->m_Direction = dir;
}

Media::~Media()
{

}

}

/**
 * Method to be re-implemented by each concrete media classes
 *
 * This can (and should) be performed asynchronously
 *
 * @return if the operation has already failed
 */
bool Media::Media::mute()
{
   return false;
}

bool Media::Media::unmute()
{
   return false;
}

bool Media::Media::terminate()
{
   return false;
}

bool Media::MediaPrivate::mute()
{
   return q_ptr->mute();
}

bool Media::MediaPrivate::unmute()
{
   return q_ptr->unmute();
}

bool Media::MediaPrivate::terminate()
{
   return q_ptr->terminate();
}

bool Media::MediaPrivate::nothing()
{
   return true;
}

Call* Media::Media::call() const
{
   return d_ptr->m_pCall;
}

Media::Media::Direction Media::Media::direction() const
{
   return d_ptr->m_Direction;
}

Media::Media::State Media::Media::state() const
{
   return d_ptr->m_State;
}

bool Media::Media::performAction(const Media::Media::Action action)
{
   const Media::Media::State s = d_ptr->m_State;

   //TODO implement a state machine
   const bool ret = (d_ptr->*(d_ptr->m_mCallbacks)[d_ptr->m_State][action])();

   if (d_ptr->m_State != s && ret) {
      emit stateChanged(d_ptr->m_State, s);
   }

   return ret;
}

Media::Media* operator<<(Media::Media* m, Media::Media::Action a)
{
   m->performAction(a);
   return m;
}

void Media::MediaPrivate::muteConfirmed()
{
   auto ll = m_State;
   m_State = Media::Media::State::MUTED;
   emit q_ptr->stateChanged(m_State, ll);
}

void Media::MediaPrivate::unmuteConfirmed()
{
   auto ll = m_State;
   switch(q_ptr->type()) {
      case Media::Media::Type::AUDIO:
      case Media::Media::Type::VIDEO:
      case Media::Media::Type::FILE:
         m_State = Media::Media::State::ACTIVE;
         emit q_ptr->stateChanged(m_State, ll);
         break;
      case Media::Media::Type::TEXT:
         m_State = Media::Media::State::IDLE;
         emit q_ptr->stateChanged(m_State, ll);
         break;
      case Media::Media::Type::COUNT__:
         break;
   };
}
