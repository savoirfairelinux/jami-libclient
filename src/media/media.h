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
#ifndef MEDIA_H
#define MEDIA_H

#include <QtCore/QObject>

#include "typedefs.h"

class Call;
class CallModelPrivate;

namespace Media {
   class MediaPrivate;
}

namespace Media {

class LIB_EXPORT Media : public QObject
{
   Q_OBJECT
   friend class ::CallModelPrivate;
public:
   enum class Type {
      AUDIO = 0, /*!< */
      VIDEO = 1, /*!< */
      TEXT  = 2, /*!< */
      FILE  = 3, /*!< */
      COUNT__
   };

   enum class State {
      ACTIVE = 0, /*!< The media is currently in progress       */
      MUTED  = 1, /*!< The media has been paused                */
      IDLE   = 2, /*!< The media is passive, but in progress    */
      OVER   = 3, /*!< The media is terminated                  */
      COUNT__
   };

   enum class Direction {
      IN , /*!< The media is coming from the peer */
      OUT, /*!< The media is going to the peer    */
      COUNT__
   };

   enum class Action {
      MUTE     , /*!< Mute this media   */
      UNMUTE   , /*!< Unmute this media */
      TERMINATE, /*!< End this media    */
      COUNT__
   };

   //Getter
   virtual Media::Type type() = 0;
   Call* call() const;
   Direction direction() const;

   //Getters
   Media::Media::State state() const;
   bool performAction(const Media::Action);

   //TODO add an abstract history getter with specialisation per media

   virtual ~Media();

protected:

   //Protected mutators
   virtual bool mute();
   virtual bool unmute();
   virtual bool terminate();

   Media(Call* parent, const Direction direction);

Q_SIGNALS:
   void stateChanged(const Media::State state, const Media::State previous);

private:
   MediaPrivate* d_ptr;
   Q_DECLARE_PRIVATE(Media)
};

}
Q_DECLARE_METATYPE(Media::Media::Direction)

Media::Media* operator<<(Media::Media* m, Media::Media::Action a);

#endif
