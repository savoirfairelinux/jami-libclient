/******************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                            *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>   *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Lesser General Public               *
 *   License as published by the Free Software Foundation; either             *
 *   version 2.1 of the License, or (at your option) any later version.       *
 *                                                                            *
 *   This library is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *   Lesser General Public License for more details.                          *
 *                                                                            *
 *   You should have received a copy of the Lesser GNU General Public License *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/
#ifndef VIDEORENDERERMANAGER_H
#define VIDEORENDERERMANAGER_H
//Base
#include <typedefs.h>
#include <QtCore/QThread>

//Qt
#include <QtCore/QHash>

//Ring
#include "video/device.h"
namespace Video {
   class Renderer;
}
class Call;
class QMutex;
struct SHMHeader;


class VideoRendererManagerPrivate;

///VideoModel: Video event dispatcher
class VideoRendererManager : public QObject {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:
   //Singleton
   static VideoRendererManager* instance();

   //Getters
   bool             isPreviewing   () const;
   Video::Renderer* previewRenderer()      ;
   int              size           () const;

   //Helpers
   Video::Renderer* getRenderer(const Call* call) const;
   void setBufferSize(uint size);
   void switchDevice(const Video::Device* device) const;

private:
   //Constructor
   explicit VideoRendererManager();
   virtual ~VideoRendererManager();

   QScopedPointer<VideoRendererManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(VideoRendererManager)

   //Static attributes
   static VideoRendererManager* m_spInstance;

public Q_SLOTS:
   void stopPreview ();
   void startPreview();

Q_SIGNALS:
   ///The preview started/stopped
   void previewStateChanged(bool startStop);
   void previewStarted(Video::Renderer* Renderer);
   void previewStopped(Video::Renderer* Renderer);

};


#endif
