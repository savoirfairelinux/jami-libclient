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
#pragma once

#include <media/media.h>
#include "private/matrixutils.h"


namespace media {
class MediaPrivate;

typedef bool (MediaPrivate::*MediaTransitionFct)();

class MediaPrivate
{
   friend class media::Media;
public:
   MediaPrivate(Media* parent);

   static const Matrix2D<media::Media::State, media::Media::Action, bool> m_mValidTransitions;
   static const Matrix2D<media::Media::State, media::Media::Action, MediaTransitionFct> m_mCallbacks;

   //Actions
   bool mute     ();
   bool unmute   ();
   bool terminate();
   bool nothing  ();

   //Server changes callbacks
   void muteConfirmed();
   void unmuteConfirmed();

private:
   //Attributes
   media::Media::State m_State;
   Call* m_pCall;
   media::Media::Direction m_Direction;

   Media* q_ptr;
};

}
