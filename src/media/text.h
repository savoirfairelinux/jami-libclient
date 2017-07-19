/****************************************************************************
 *   Copyright (C) 2015-2017 Savoir-faire Linux                               *
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
#include <typedefs.h>

class MediaTextPrivate;
class Call;
class CallPrivate;
class InstantMessagingModel;
class IMConversationManagerPrivate;

namespace Media {

class TextRecording;

class LIB_EXPORT Text : public Media::Media
{
   Q_OBJECT
   friend class ::CallPrivate;
   friend class ::IMConversationManagerPrivate;
public:

   virtual Media::Type type() override;

   //Getter
   TextRecording* recording   (                         ) const;
   bool           hasMimeType ( const QString& mimeType ) const;
   QStringList    mimeTypes   (                         ) const;

   //Mutator
   void send(const QMap<QString,QString>& message, const bool isMixed = false);

private:
   Text(Call* parent, const Media::Direction direction);
   virtual ~Text();

   MediaTextPrivate* d_ptr;

Q_SIGNALS:
   void messageSent    (const QMap<QString,QString>& m);
   void messageReceived(const QMap<QString,QString>& m);
   void mimeTypesChanged();
};

}

