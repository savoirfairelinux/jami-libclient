/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
 *   Authors : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>*
 *             Alexandre Lision <alexandre.lision@savoirfairelinux.com>     *
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
#include "videorenderermanager.h"


#include <vector>

//Qt
#include <QtCore/QMutex>

//Ring
#include "../dbus/videomanager.h"
#include "video/device.h"
#include <call.h>
#include <callmodel.h>
#include "video/renderer.h"
#include "video/devicemodel.h"
#include "video/channel.h"
#include "video/rate.h"
#include "video/resolution.h"
#include "private/videorate_p.h"
#include "private/call_p.h"
#if defined(Q_OS_DARWIN)
#include "private/directrenderer.h"
#else
#include "private/shmrenderer.h"
#endif

constexpr static const char LOCAL_DEVICE[] = "local";

//Static member
VideoRendererManager* VideoRendererManager::m_spInstance = nullptr;

class VideoRendererManagerPrivate : public QObject
{
   Q_OBJECT

public:
   VideoRendererManagerPrivate(VideoRendererManager* parent);

   //Attributes
   bool           m_PreviewState;
   uint           m_BufferSize  ;
   QHash<QByteArray,Video::Renderer*> m_lRenderers;

private:
   VideoRendererManager* q_ptr;

public Q_SLOTS:
   void startedDecoding(const QString& id, const QString& shmPath, int width, int height);
   void stoppedDecoding(const QString& id, const QString& shmPath);
   void deviceEvent();

};

VideoRendererManagerPrivate::VideoRendererManagerPrivate(VideoRendererManager* parent) : QObject(parent), q_ptr(parent),
m_BufferSize(0),m_PreviewState(false)
{

}

///Constructor
VideoRendererManager::VideoRendererManager():QThread(), d_ptr(new VideoRendererManagerPrivate(this))
{
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   connect( &interface , SIGNAL(deviceEvent())                           , d_ptr.data(), SLOT(deviceEvent())                           );
   connect( &interface , &VideoManagerInterface::startedDecoding, d_ptr.data(), &VideoRendererManagerPrivate::startedDecoding);
   connect( &interface , &VideoManagerInterface::stoppedDecoding, d_ptr.data(), &VideoRendererManagerPrivate::stoppedDecoding);
}


VideoRendererManager::~VideoRendererManager()
{
//    delete d_ptr;
}

///Singleton
VideoRendererManager* VideoRendererManager::instance()
{
   if (!m_spInstance) {
      m_spInstance = new VideoRendererManager();
   }
   return m_spInstance;
}

///Return the call Renderer or nullptr
Video::Renderer* VideoRendererManager::getRenderer(const Call* call) const
{
   if (!call) return nullptr;
   return d_ptr->m_lRenderers[call->dringId().toLatin1()];
}

///Get the video preview Renderer
Video::Renderer* VideoRendererManager::previewRenderer()
{
   if (!d_ptr->m_lRenderers[LOCAL_DEVICE]) {
      if ((!Video::DeviceModel::instance()->activeDevice()) || (!Video::DeviceModel::instance()->activeDevice()->activeChannel())) {
         qWarning() << "No device found";
         return nullptr;
      }

      Video::Resolution* res = Video::DeviceModel::instance()->activeDevice()->activeChannel()->activeResolution();

      if (!res) {
         qWarning() << "Misconfigured video device";
         return nullptr;
      }

#if defined(Q_OS_DARWIN)
      d_ptr->m_lRenderers[LOCAL_DEVICE] = new Video::DirectRenderer(LOCAL_DEVICE, res->size());
#else
      d_ptr->m_lRenderers[LOCAL_DEVICE] = new Video::ShmRenderer(LOCAL_DEVICE,"",res->size());
#endif
   }
   return d_ptr->m_lRenderers[LOCAL_DEVICE];
}

///Stop video preview
void VideoRendererManager::stopPreview()
{
   //d_ptr->stoppedDecoding(LOCAL_DEVICE,"");
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   interface.stopCamera();
   d_ptr->m_PreviewState = false;
}

///Start video preview
void VideoRendererManager::startPreview()
{
   // d_ptr->startedDecoding(LOCAL_DEVICE,"",500,500);

   if (d_ptr->m_PreviewState) return;
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   interface.startCamera();
   d_ptr->m_PreviewState = true;
}

///Is the video model fetching preview from a camera
bool VideoRendererManager::isPreviewing()
{
   return d_ptr->m_PreviewState;
}

///@todo Set the video buffer size
void VideoRendererManager::setBufferSize(uint size)
{
   d_ptr->m_BufferSize = size;
}

///Event callback
void VideoRendererManagerPrivate::deviceEvent()
{
   //TODO is there anything useful to do?
}

///A video is not being rendered
void VideoRendererManagerPrivate::startedDecoding(const QString& id, const QString& shmPath, int width, int height)
{
   const QSize res = QSize(width,height);
   const QByteArray rid = id.toLatin1();
   qWarning() << "startedDecoding for sink id: " << id;

   if (m_lRenderers[rid] == nullptr ) {
#if defined(Q_OS_DARWIN)
      m_lRenderers[rid] = new Video::DirectRenderer(rid, res);
      qWarning() << "Calling registerFrameListener";
      DBus::VideoManager::instance().registerSinkTarget(id, [this, id, width, height] (const unsigned char* frame) {
          static_cast<Video::DirectRenderer*>(m_lRenderers[id.toLatin1()])->onNewFrame(
                                    QByteArray::fromRawData(reinterpret_cast<const char *>(frame), width*height));
      });
#else
      m_lRenderers[rid] = new Video::ShmRenderer(rid,shmPath,res);
#endif
      m_lRenderers[rid]->moveToThread(q_ptr);
      if (!q_ptr->isRunning())
         q_ptr->start();
   }
   else {
      m_lRenderers[rid]->setSize(res);
#ifdef ENABLE_LIBWRAP
      DBus::VideoManager::instance().registerSinkTarget(id, [this, id, width, height] (const unsigned char* frame) {
          static_cast<Video::DirectRenderer*>(m_lRenderers[id.toLatin1()])->onNewFrame(
                                    QByteArray::fromRawData(reinterpret_cast<const char *>(frame), width*height));
      });
#endif

#if !defined(Q_OS_DARWIN)
      static_cast<Video::ShmRenderer*>(m_lRenderers[rid])->setShmPath(shmPath);
#endif
   }

   m_lRenderers[rid]->startRendering();
   Video::Device* dev = Video::DeviceModel::instance()->getDevice(id);
   if (dev) {
      emit dev->renderingStarted(m_lRenderers[rid]);
   }
   if (id != LOCAL_DEVICE) {
      qDebug() << "Starting video for call" << id;
      Call* c = CallModel::instance()->getCall(id);
      if (c)
         c->d_ptr->registerRenderer(m_lRenderers[rid]);
   }
   else {
      m_PreviewState = true;
      emit q_ptr->previewStateChanged(true);
      emit q_ptr->previewStarted(m_lRenderers[rid]);
   }
}

///A video stopped being rendered
void VideoRendererManagerPrivate::stoppedDecoding(const QString& id, const QString& shmPath)
{
   Q_UNUSED(shmPath)
   Video::Renderer* r = m_lRenderers[id.toLatin1()];
   if ( r ) {
      r->stopRendering();
   }
   qDebug() << "Video stopped for call" << id <<  "Renderer found:" << (m_lRenderers[id.toLatin1()] != nullptr);
//    emit videoStopped();

   Video::Device* dev = Video::DeviceModel::instance()->getDevice(id);
   if (dev) {
      emit dev->renderingStopped(r);
   }
   if (id == LOCAL_DEVICE) {
      m_PreviewState = false;
      emit q_ptr->previewStateChanged(false);
      emit q_ptr->previewStopped(r);
   }
//    r->mutex()->lock();
   m_lRenderers[id.toLatin1()] = nullptr;
   delete r;
}

void VideoRendererManager::switchDevice(const Video::Device* device) const
{
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   interface.switchInput(device->id());
}

#include <videorenderermanager.moc>
