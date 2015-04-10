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
#ifndef VIDEO_SHM_RENDERER_H
#define VIDEO_SHM_RENDERER_H

//Base
#include "video/renderer.h"
#include "typedefs.h"

//Qt
class QMutex;

//Ring
#include "video/device.h"

//Private
struct SHMHeader;


namespace Video {
class ShmRendererPrivate;

///Manage shared memory and convert it to QByteArray
class LIB_EXPORT ShmRenderer : public Renderer {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

   friend class VideoRendererManagerPrivate ;

   public:
      //Constructor
      ShmRenderer (const QByteArray& id, const QString& shmPath, const QSize& res);
      virtual ~ShmRenderer();

      //Mutators
      void stopShm  ();
      bool startShm ();

      //Getters
      virtual int   fps() const;
      virtual const QByteArray& currentFrame() const override;

      //Setters
      void setShmPath(const QString& path);

   private:
      QScopedPointer<ShmRendererPrivate> d_ptr;
      Q_DECLARE_PRIVATE(ShmRenderer)

   public Q_SLOTS:
      void startRendering();
      void stopRendering ();
};

}

#endif
