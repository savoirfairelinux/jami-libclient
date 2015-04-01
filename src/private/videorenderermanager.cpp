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

//libstdc++
#include <vector>

//Qt
#include <QtCore/QMutex>

//Ring
#include <dbus/videomanager.h>
#include <video/device.h>
#include <call.h>
#include <callmodel.h>
#include <video/renderer.h>
#include <video/devicemodel.h>
#include <video/channel.h>
#include <video/rate.h>
#include <video/resolution.h>
#include "private/videorate_p.h"
#include "private/call_p.h"

#ifdef ENABLE_LIBWRAP
 #include "private/directrenderer.h"
#else
 #include "private/shmrenderer.h"
#endif

constexpr static const char PREVIEW_RENDERER_ID[] = "local";

//Static member
VideoRendererManager* VideoRendererManager::m_spInstance = nullptr;

class VideoRendererManagerPrivate : public QObject
{
   Q_OBJECT

public:
   VideoRendererManagerPrivate(VideoRendererManager* parent);

   //Attributes
   bool                               m_PreviewState;
   uint                               m_BufferSize  ;
   QHash<QByteArray,Video::Renderer*> m_hRenderers  ;
   QHash<Video::Renderer*, QThread*>  m_hThreads    ;


private:
   VideoRendererManager* q_ptr;

public Q_SLOTS:
   void startedDecoding(const QString& id, const QString& shmPath, int width, int height);
   void stoppedDecoding(const QString& id, const QString& shmPath);

};

VideoRendererManagerPrivate::VideoRendererManagerPrivate(VideoRendererManager* parent) : QObject(parent), q_ptr(parent),
m_BufferSize(0),m_PreviewState(false)
{

}

///Constructor
VideoRendererManager::VideoRendererManager():QObject(QCoreApplication::instance()), d_ptr(new VideoRendererManagerPrivate(this))
{
   VideoManagerInterface& interface = DBus::VideoManager::instance();
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


int VideoRendererManager::size() const
{
   return d_ptr->m_hRenderers.size();
}

///Return the call Renderer or nullptr
Video::Renderer* VideoRendererManager::getRenderer(const Call* call) const
{
   if ((!call) || (!call->hasRemote())) return nullptr;

   return d_ptr->m_hRenderers[call->dringId().toLatin1()];
}

///Get the video preview Renderer
Video::Renderer* VideoRendererManager::previewRenderer()
{
   if (!d_ptr->m_hRenderers[PREVIEW_RENDERER_ID]) {

      if ((!Video::DeviceModel::instance()->activeDevice()) || (!Video::DeviceModel::instance()->activeDevice()->activeChannel())) {
         qWarning() << "No device found";
         return nullptr;
      }

      Video::Resolution* res = Video::DeviceModel::instance()->activeDevice()->activeChannel()->activeResolution();

      if (!res) {
         qWarning() << "Misconfigured video device";
         return nullptr;
      }

      Video::Renderer* r = nullptr;

#ifdef ENABLE_LIBWRAP
      r = new Video::DirectRenderer(PREVIEW_RENDERER_ID, res->size());
#else //ENABLE_LIBWRAP
      r = new Video::ShmRenderer(PREVIEW_RENDERER_ID,"",res->size());
#endif

      QThread* t = new QThread(this);
      d_ptr->m_hThreads[r] = t;

      r->moveToThread(t);

      d_ptr->m_hRenderers[PREVIEW_RENDERER_ID] = r;

   }
   return d_ptr->m_hRenderers[PREVIEW_RENDERER_ID];
}

///Stop video preview
void VideoRendererManager::stopPreview()
{

   DBus::VideoManager::instance().stopCamera();

   d_ptr->m_PreviewState = false;

}

///Start video preview
void VideoRendererManager::startPreview()
{

   if (d_ptr->m_PreviewState)
      return;

   DBus::VideoManager::instance().startCamera();

   d_ptr->m_PreviewState = true;

}

///Is the video model fetching preview from a camera
bool VideoRendererManager::isPreviewing() const
{
   return d_ptr->m_PreviewState;
}

///@todo Set the video buffer size
void VideoRendererManager::setBufferSize(uint size)
{
   d_ptr->m_BufferSize = size;
}

///A video is not being rendered
void VideoRendererManagerPrivate::startedDecoding(const QString& id, const QString& shmPath, int width, int height)
{
   const QSize      res = QSize(width,height);
   const QByteArray rid = id.toLatin1();

   qWarning() << "startedDecoding for sink id: " << id;

   Video::Renderer* r = m_hRenderers[rid];

   if (r == nullptr ) {

#ifdef ENABLE_LIBWRAP

      r = new Video::DirectRenderer(rid, res);

      qWarning() << "Calling registerFrameListener";

      DBus::VideoManager::instance().registerSinkTarget(id, [this, id, width, height] (const unsigned char* frame) {
         static_cast<Video::DirectRenderer*>(m_hRenderers[id.toLatin1()])->onNewFrame(
            QByteArray::fromRawData(reinterpret_cast<const char *>(frame), width*height)
         );
      });

#else //ENABLE_LIBWRAP

      r = new Video::ShmRenderer(rid,shmPath,res);

#endif

      m_hRenderers[rid] = r;

      QThread* t = new QThread(this);
      m_hThreads[r] = t;

      r->moveToThread(t);

      if (!t->isRunning())
         t->start();

   }
   else {

      r->setSize(res);

#ifdef ENABLE_LIBWRAP

      DBus::VideoManager::instance().registerSinkTarget(id, [this, id, width, height] (const unsigned char* frame) {
         static_cast<Video::DirectRenderer*>(m_hRenderers[id.toLatin1()])->onNewFrame(
            QByteArray::fromRawData(reinterpret_cast<const char *>(frame), width*height)
         );
      });

#else //ENABLE_LIBWRAP

      static_cast<Video::ShmRenderer*>(r)->setShmPath(shmPath);

#endif

   }

   r->startRendering();

   Video::Device* dev = Video::DeviceModel::instance()->getDevice(id);

   if (dev)
      emit dev->renderingStarted(r);

   if (id != PREVIEW_RENDERER_ID) {
      qDebug() << "Starting video for call" << id;

      Call* c = CallModel::instance()->getCall(id);

      if (c)
         c->d_ptr->registerRenderer(r);

   }
   else {
      m_PreviewState = true;
      emit q_ptr->previewStateChanged(true);
      emit q_ptr->previewStarted(r);
   }
}

///A video stopped being rendered
void VideoRendererManagerPrivate::stoppedDecoding(const QString& id, const QString& shmPath)
{
   Q_UNUSED(shmPath)
   Video::Renderer* r = m_hRenderers[id.toLatin1()];

   //Quit if for some reasons the renderer is not found
   if ( !r ) {
      qWarning() << "Cannot stop renrering, renderer" << id << "not found";
      return;
   }

   r->stopRendering();

   qDebug() << "Video stopped for call" << id <<  "Renderer found:" << (m_hRenderers[id.toLatin1()] != nullptr);

   Video::Device* dev = Video::DeviceModel::instance()->getDevice(id);

   if (dev)
      emit dev->renderingStopped(r);

   if (id == PREVIEW_RENDERER_ID) {
      m_PreviewState = false;
      emit q_ptr->previewStateChanged(false);
      emit q_ptr->previewStopped(r);
   }

   m_hRenderers[id.toLatin1()] = nullptr;

   QThread* t = m_hThreads[r];
   m_hThreads[r] = nullptr;

   if (t) {
      t->quit();
      t->wait();
   }

   delete r;

   t->deleteLater();
}

void VideoRendererManager::switchDevice(const Video::Device* device) const
{
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   interface.switchInput(device->id());
}

#include <videorenderermanager.moc>
