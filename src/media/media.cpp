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

const Matrix2D<LRCMedia::Media::State, LRCMedia::Media::Action, bool> LRCMedia::MediaPrivate::m_mValidTransitions ={{
   /*                MUTE   UNMUTE  TERMINATE */
   /* ACTIVE   */ {{ true  , true  , true     }},
   /* MUTED    */ {{ true  , true  , true     }},
   /* IDLE     */ {{ true  , false , true     }},
   /* OVER     */ {{ false , false , false    }},
}};

//Use the Media::MediaPrivate wrapper to avoid vtable issues
#define MEDF &MediaPrivate
const Matrix2D<LRCMedia::Media::State, LRCMedia::Media::Action, LRCMedia::MediaTransitionFct> LRCMedia::MediaPrivate::m_mCallbacks ={{
   /*                     MUTE           UNMUTE         TERMINATE     */
   /* ACTIVE   */ {{ MEDF::mute    , MEDF::nothing , MEDF::terminate }},
   /* MUTED    */ {{ MEDF::nothing , MEDF::unmute  , MEDF::terminate }},
   /* IDLE     */ {{ MEDF::mute    , MEDF::nothing , MEDF::terminate }},
   /* OVER     */ {{ MEDF::nothing , MEDF::nothing , MEDF::nothing   }},
}};
#undef MEDF

namespace LRCMedia {

MediaPrivate::MediaPrivate(Media* parent) :
 m_State(LRCMedia::Media::State::ACTIVE),m_pCall(nullptr),q_ptr(parent),m_Direction(Media::Direction::OUT)
{

}

LRCMedia::Media::Media(Call* parent, const LRCMedia::Media::Direction dir) :
    QObject(parent), d_ptr(new MediaPrivate(this))
{
   Q_ASSERT(parent);
   d_ptr->m_pCall = parent;
   d_ptr->m_Direction = dir;
}

LRCMedia::Media::~Media()
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
bool LRCMedia::Media::mute()
{
   return false;
}

bool LRCMedia::Media::unmute()
{
   return false;
}

bool LRCMedia::Media::terminate()
{
   return false;
}

bool LRCMedia::MediaPrivate::mute()
{
   return q_ptr->mute();
}

bool LRCMedia::MediaPrivate::unmute()
{
   return q_ptr->unmute();
}

bool LRCMedia::MediaPrivate::terminate()
{
   return q_ptr->terminate();
}

bool LRCMedia::MediaPrivate::nothing()
{
   return true;
}

Call* LRCMedia::Media::call() const
{
   return d_ptr->m_pCall;
}

LRCMedia::Media::Direction LRCMedia::Media::direction() const
{
   return d_ptr->m_Direction;
}

LRCMedia::Media::State LRCMedia::Media::state() const
{
   return d_ptr->m_State;
}

bool LRCMedia::Media::performAction(const LRCMedia::Media::Action action)
{
   const LRCMedia::Media::State s = d_ptr->m_State;

   //TODO implement a state machine
   const bool ret = (d_ptr->*(d_ptr->m_mCallbacks)[d_ptr->m_State][action])();

   if (d_ptr->m_State != s && ret) {
      emit stateChanged(d_ptr->m_State, s);
   }

   return ret;
}

LRCMedia::Media* operator<<(LRCMedia::Media* m, LRCMedia::Media::Action a)
{
   m->performAction(a);
   return m;
}

void LRCMedia::MediaPrivate::muteConfirmed()
{
   const auto ll = m_State;
   m_State = LRCMedia::Media::State::MUTED;
   emit q_ptr->stateChanged(m_State, ll);
}

void LRCMedia::MediaPrivate::unmuteConfirmed()
{
   const auto ll = m_State;
   switch(q_ptr->type()) {
      case LRCMedia::Media::Type::AUDIO:
      case LRCMedia::Media::Type::VIDEO:
      case LRCMedia::Media::Type::FILE:
         m_State = LRCMedia::Media::State::ACTIVE;
         emit q_ptr->stateChanged(m_State, ll);
         break;
      case LRCMedia::Media::Type::TEXT:
         m_State = LRCMedia::Media::State::IDLE;
         emit q_ptr->stateChanged(m_State, ll);
         break;
      case LRCMedia::Media::Type::COUNT__:
         break;
   };
}
