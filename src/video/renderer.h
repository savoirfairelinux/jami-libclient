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
#ifndef VIDEO_ABSTRACT_RENDERER_H
#define VIDEO_ABSTRACT_RENDERER_H

//Base
#include <QtCore/QObject>
#include <typedefs.h>

//Qt
class QMutex;

//Ring
#include "device.h"


namespace Video {

class RendererPrivate;
class ShmRendererPrivate;
class DirectRendererPrivate;
class DirectRenderer;

/**
 * This class provide a rendering object to be used by clients
 * to get the video content. This object is not intended to be
 * extended outside of the LibRingClient.
 *
 * Each platform transparently provide its own implementation.
 */
class LIB_EXPORT Renderer : public QObject {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
Q_OBJECT
#pragma GCC diagnostic pop

friend class Video::ShmRendererPrivate;
friend class Video::DirectRendererPrivate;
friend class Video::DirectRenderer;

public:
   //Constructor
   Renderer (const QByteArray& id,  const QSize& res);
   virtual ~Renderer();

   //Getters
   virtual bool              isRendering     () const;
   virtual const QByteArray& currentFrame    () const;
   virtual QSize             size            () const;
   virtual QMutex*           mutex           () const;

   //Setters
   void setRendering(bool rendering)            const;
   void setSize(const QSize& size)              const;

Q_SIGNALS:
   ///Emitted when a new frame is ready
   void frameUpdated();
   void stopped     ();
   void started     ();

public Q_SLOTS:
   virtual void startRendering() = 0;
   virtual void stopRendering () = 0;


private:
   QScopedPointer<RendererPrivate> d_ptr;
   Q_DECLARE_PRIVATE(Renderer)

};

}

#endif
