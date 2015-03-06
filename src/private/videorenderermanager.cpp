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
   QMutex*        m_SSMutex     ;
   QHash<QByteArray,Video::Renderer*> m_lRenderers;

private:
   VideoRendererManager* q_ptr;

private Q_SLOTS:
   void startedDecoding(const QString& id, const QString& shmPath, int width, int height);
   void stoppedDecoding(const QString& id, const QString& shmPath);
   void deviceEvent();

};

VideoRendererManagerPrivate::VideoRendererManagerPrivate(VideoRendererManager* parent) : QObject(parent), q_ptr(parent),
m_BufferSize(0),m_PreviewState(false),m_SSMutex(new QMutex())
{

}

///Constructor
VideoRendererManager::VideoRendererManager():QThread(), d_ptr(new VideoRendererManagerPrivate(this))
{
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   connect( &interface , SIGNAL(deviceEvent())                           , d_ptr.data(), SLOT(deviceEvent())                           );
   connect( &interface , SIGNAL(startedDecoding(QString,QString,int,int,bool)), d_ptr.data(), SLOT(startedDecoding(QString,QString,int,int)));
   connect( &interface , SIGNAL(stoppedDecoding(QString,QString,bool))        , d_ptr.data(), SLOT(stoppedDecoding(QString,QString))        );
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
   if (!d_ptr->m_lRenderers["local"]) {
      Video::Resolution* res = Video::DeviceModel::instance()->activeDevice()->activeChannel()->activeResolution();
      if (!res) {
         qWarning() << "Misconfigured video device";
         return nullptr;
      }
#if defined(Q_OS_DARWIN)
      d_ptr->m_lRenderers["local"] = new Video::DirectRenderer("local", res->size());

#else
      d_ptr->m_lRenderers["local"] = new Video::ShmRenderer("local","",res->size());
#endif
   }
   return d_ptr->m_lRenderers["local"];
}

///Stop video preview
void VideoRendererManager::stopPreview()
{
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   interface.stopCamera();
   d_ptr->m_PreviewState = false;
}

///Start video preview
void VideoRendererManager::startPreview()
{
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

   if (m_lRenderers[id.toLatin1()] == nullptr ) {
#if defined(Q_OS_DARWIN)
      m_lRenderers[id.toLatin1()] = new Video::DirectRenderer(id.toLatin1(), res);
#else
      m_lRenderers[id.toLatin1()] = new Video::ShmRenderer(id.toLatin1(),shmPath,res);
#endif
      m_lRenderers[id.toLatin1()]->moveToThread(q_ptr);
      if (!q_ptr->isRunning())
         q_ptr->start();
   }
   else {
      // Video::Renderer* Renderer = m_lRenderers[id.toLatin1()];
      //TODO: do direct renderer stuff here
      m_lRenderers[id.toLatin1()]->setSize(res);
#if !defined(Q_OS_DARWIN)
      static_cast<Video::ShmRenderer*>(m_lRenderers[id.toLatin1()])->setShmPath(shmPath);
#endif
   }

   m_lRenderers[id.toLatin1()]->startRendering();
   Video::Device* dev = Video::DeviceModel::instance()->getDevice(id);
   if (dev) {
      emit dev->renderingStarted(m_lRenderers[id.toLatin1()]);
   }
   if (id != "local") {
      qDebug() << "Starting video for call" << id;
      Call* c = CallModel::instance()->getCall(id);
      if (c)
         c->d_ptr->registerRenderer(m_lRenderers[id.toLatin1()]);
   }
   else {
      m_PreviewState = true;
      emit q_ptr->previewStateChanged(true);
      emit q_ptr->previewStarted(m_lRenderers[id.toLatin1()]);
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
   if (id == "local") {
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

QMutex* VideoRendererManager::startStopMutex() const
{
   return d_ptr->m_SSMutex;
}

#include <videorenderermanager.moc>
