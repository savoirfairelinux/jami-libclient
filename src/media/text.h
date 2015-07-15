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
#ifndef MEDIA_TEXT_H
#define MEDIA_TEXT_H

#include <media/media.h>
#include <typedefs.h>

class MediaTextPrivate;
class Call;
class CallPrivate;
class InstantMessagingModel;

namespace Media {

class TextRecording;

class LIB_EXPORT Text : public Media::Media
{
   Q_OBJECT
   friend class ::CallPrivate;
public:

   virtual Media::Type type() override;

   //Getter
   TextRecording* recording() const;

   //Mutator
   void send(const QMap<QString,QString>& message);

private:
   Text(Call* parent, const Media::Direction direction);
   virtual ~Text();

   MediaTextPrivate* d_ptr;

Q_SIGNALS:
   void messageSent    (const QMap<QString,QString>& m);
   void messageReceived(const QMap<QString,QString>& m);
};

}

#endif
