/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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
#ifndef VIDEO_DIRECT_RENDERER_H
#define VIDEO_DIRECT_RENDERER_H

//Base
#include <QtCore/QObject>
#include "typedefs.h"
#include "video/renderer.h"

//Qt
class QMutex;
class QTimer;
class QThread;

//Ring
#include "video/device.h"


namespace Video {
class DirectRendererPrivate;

///Manage shared memory and convert it to QByteArray
class LIB_EXPORT DirectRenderer final : public Renderer {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

public:

   //Constructor
   DirectRenderer (const QByteArray& id, const QSize& res);
   virtual ~DirectRenderer();

   //Getter
   virtual ColorSpace colorSpace() const override;

   void onNewFrame(const QByteArray& frame);

public Q_SLOTS:
   virtual void startRendering() override;
   virtual void stopRendering () override;

private:
   QScopedPointer<DirectRendererPrivate> d_ptr;
   Q_DECLARE_PRIVATE(DirectRenderer)

};

}

#endif
