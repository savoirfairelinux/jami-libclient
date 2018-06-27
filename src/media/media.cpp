/****************************************************************************
 *   Copyright (C) 2015-2018 Savoir-faire Linux                               *
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

const Matrix2D<media::Media::State, media::Media::Action, bool> media::MediaPrivate::m_mValidTransitions ={{
   /*                MUTE   UNMUTE  TERMINATE */
   /* ACTIVE   */ {{ true  , true  , true     }},
   /* MUTED    */ {{ true  , true  , true     }},
   /* IDLE     */ {{ true  , false , true     }},
   /* OVER     */ {{ false , false , false    }},
}};

//Use the Media::MediaPrivate wrapper to avoid vtable issues
#define MEDF &MediaPrivate
const Matrix2D<media::Media::State, media::Media::Action, media::MediaTransitionFct> media::MediaPrivate::m_mCallbacks ={{
   /*                     MUTE           UNMUTE         TERMINATE     */
   /* ACTIVE   */ {{ MEDF::mute    , MEDF::nothing , MEDF::terminate }},
   /* MUTED    */ {{ MEDF::nothing , MEDF::unmute  , MEDF::terminate }},
   /* IDLE     */ {{ MEDF::mute    , MEDF::nothing , MEDF::terminate }},
   /* OVER     */ {{ MEDF::nothing , MEDF::nothing , MEDF::nothing   }},
}};
#undef MEDF

namespace media {

MediaPrivate::MediaPrivate(Media* parent) :
 m_State(media::Media::State::ACTIVE),m_pCall(nullptr),q_ptr(parent),m_Direction(Media::Direction::OUT)
{

}

media::Media::Media(Call* parent, const media::Media::Direction dir) :
    QObject(parent), d_ptr(new MediaPrivate(this))
{
   Q_ASSERT(parent);
   d_ptr->m_pCall = parent;
   d_ptr->m_Direction = dir;
}

media::Media::~Media()
{
   delete d_ptr;
}

}

/**
 * Method to be re-implemented by each concrete media classes
 *
 * This can (and should) be performed asynchronously
 *
 * @return if the operation has already failed
 */
bool media::Media::mute()
{
   return false;
}

bool media::Media::unmute()
{
   return false;
}

bool media::Media::terminate()
{
   return false;
}

bool media::MediaPrivate::mute()
{
   return q_ptr->mute();
}

bool media::MediaPrivate::unmute()
{
   return q_ptr->unmute();
}

bool media::MediaPrivate::terminate()
{
   return q_ptr->terminate();
}

bool media::MediaPrivate::nothing()
{
   return true;
}

Call* media::Media::call() const
{
   return d_ptr->m_pCall;
}

media::Media::Direction media::Media::direction() const
{
   return d_ptr->m_Direction;
}

media::Media::State media::Media::state() const
{
   return d_ptr->m_State;
}

bool media::Media::performAction(const media::Media::Action action)
{
   const media::Media::State s = d_ptr->m_State;

   //TODO implement a state machine
   const bool ret = (d_ptr->*(d_ptr->m_mCallbacks)[d_ptr->m_State][action])();

   if (d_ptr->m_State != s && ret) {
      emit stateChanged(d_ptr->m_State, s);
   }

   return ret;
}

media::Media* operator<<(media::Media* m, media::Media::Action a)
{
   m->performAction(a);
   return m;
}

void media::MediaPrivate::muteConfirmed()
{
   const auto ll = m_State;
   m_State = media::Media::State::MUTED;
   emit q_ptr->stateChanged(m_State, ll);
}

void media::MediaPrivate::unmuteConfirmed()
{
   const auto ll = m_State;
   switch(q_ptr->type()) {
      case media::Media::Type::AUDIO:
      case media::Media::Type::VIDEO:
      case media::Media::Type::FILE:
         m_State = media::Media::State::ACTIVE;
         emit q_ptr->stateChanged(m_State, ll);
         break;
      case media::Media::Type::TEXT:
         m_State = media::Media::State::IDLE;
         emit q_ptr->stateChanged(m_State, ll);
         break;
      case media::Media::Type::COUNT__:
         break;
   };
}
