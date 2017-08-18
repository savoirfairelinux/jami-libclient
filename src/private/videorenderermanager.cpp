/****************************************************************************
 *   Copyright (C) 2012-2017 Savoir-faire Linux                          *
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

class VideoRendererManagerPrivate final : public QObject
{
   Q_OBJECT

public:
   VideoRendererManagerPrivate(VideoRendererManager* parent);

   //Attributes
   bool                               m_PreviewState;
   uint                               m_BufferSize  ;
   QHash<QByteArray,Video::Renderer*> m_hRenderers  ;
   QHash<Video::Renderer*,QByteArray> m_hRendererIds;
   QHash<Video::Renderer*, QThread*>  m_hThreads    ;

   //Helper
   void removeRenderer(Video::Renderer* r);

private:
   VideoRendererManager* q_ptr;

public Q_SLOTS:
   void startedDecoding(const QString& id, const QString& shmPath, int width, int height);
   void stoppedDecoding(const QString& id, const QString& shmPath);
   void callIsOver();

};

VideoRendererManagerPrivate::VideoRendererManagerPrivate(VideoRendererManager* parent) : QObject(parent), q_ptr(parent),
m_BufferSize(0),m_PreviewState(false)
{

}

///Constructor
VideoRendererManager::VideoRendererManager():QObject(QCoreApplication::instance()), d_ptr(new VideoRendererManagerPrivate(this))
{
   VideoManagerInterface& interface = VideoManager::instance();
   connect( &interface , &VideoManagerInterface::startedDecoding, d_ptr.data(), &VideoRendererManagerPrivate::startedDecoding);
   connect( &interface , &VideoManagerInterface::stoppedDecoding, d_ptr.data(), &VideoRendererManagerPrivate::stoppedDecoding);
}


VideoRendererManager::~VideoRendererManager()
{
//    delete d_ptr;
}

///Singleton
VideoRendererManager& VideoRendererManager::instance()
{
    static auto instance = new VideoRendererManager;
    return *instance;
}

int VideoRendererManager::size() const
{
   return d_ptr->m_hRenderers.size();
}

///Return the call Renderer or nullptr
Video::Renderer* VideoRendererManager::getRenderer(const Call* call) const
{
   if ((!call) || (!call->hasRemote()) || !d_ptr->m_hRenderers.contains(call->dringId().toLatin1()))
      return nullptr;

   return d_ptr->m_hRenderers[call->dringId().toLatin1()];
}


// helper for the new model
Video::Renderer* VideoRendererManager::getRenderer(const std::string& callId) const
{
   return (d_ptr->m_hRenderers.contains(callId.c_str()))
          ? d_ptr->m_hRenderers[callId.c_str()]
          : nullptr;
}



///Get the video preview Renderer
Video::Renderer* VideoRendererManager::previewRenderer()
{
   if (!d_ptr->m_hRenderers.contains(PREVIEW_RENDERER_ID)) {

      if ((!Video::DeviceModel::instance().activeDevice()) || (!Video::DeviceModel::instance().activeDevice()->activeChannel())) {
         qWarning() << "No device found";
         return nullptr;
      }

      Video::Resolution* res = Video::DeviceModel::instance().activeDevice()->activeChannel()->activeResolution();

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
      d_ptr->m_hRendererIds[r] = PREVIEW_RENDERER_ID;

   }
   return d_ptr->m_hRenderers[PREVIEW_RENDERER_ID];
}

///Stop video preview
void VideoRendererManager::stopPreview()
{

   VideoManager::instance().stopCamera();

   d_ptr->m_PreviewState = false;

}

///Start video preview
void VideoRendererManager::startPreview()
{

   if (d_ptr->m_PreviewState)
      return;

   VideoManager::instance().startCamera();

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
   Q_UNUSED(shmPath) //When directly linked, there is no SHM
   const QSize      res = QSize(width,height);
   const QByteArray rid = id.toLatin1();

   qWarning() << "startedDecoding for sink id: " << id;

   Video::Renderer* r = nullptr;

   if (!m_hRenderers.contains(rid)) {

#ifdef ENABLE_LIBWRAP

      r = new Video::DirectRenderer(rid, res);

      qWarning() << "Calling registerFrameListener";
      m_hRenderers[rid] = r;
      m_hRendererIds[r]=rid;

      VideoManager::instance().registerSinkTarget(id, static_cast<Video::DirectRenderer*>(r)->target());

#else //ENABLE_LIBWRAP

      r = new Video::ShmRenderer(rid,shmPath,res);
      m_hRenderers[rid] = r;
      m_hRendererIds[r]=rid;

#endif


      QThread* t = new QThread(this);
      m_hThreads[r] = t;

      r->moveToThread(t);

      if (!t->isRunning())
         t->start();

   }
   else {
      r = m_hRenderers.value(rid);

      QThread* t = m_hThreads.value(r);

      if (t && !t->isRunning())
         t->start();

      r->setSize(res);

#ifdef ENABLE_LIBWRAP
    VideoManager::instance().registerSinkTarget(id, static_cast<Video::DirectRenderer*>(r)->target());
#else //ENABLE_LIBWRAP

      static_cast<Video::ShmRenderer*>(r)->setShmPath(shmPath);

#endif

   }

   r->startRendering();

   Video::Device* dev = Video::DeviceModel::instance().getDevice(id);

   if (dev)
      emit dev->renderingStarted(r);

   if (id != PREVIEW_RENDERER_ID) {
      qDebug() << "Starting video for call" << id;

      Call* c = CallModel::instance().getCall(id);

      if (c)
          c->d_ptr->registerRenderer(r);
      else {
          //We don't have the conference yet
          QObject::connect(&CallModel::instance(), &CallModel::conferenceCreated, [=](Call* conf) {
              Q_UNUSED(conf)
              Call* c = CallModel::instance().getCall(id);

              if (c) {
                  c->d_ptr->registerRenderer(r);
              }
          });
      }

   }
   else {
      m_PreviewState = true;
      emit q_ptr->previewStateChanged(true);
      emit q_ptr->previewStarted(r);
   }
}

/// Deletes the renderer and related resources
#ifndef Q_OS_WIN
void VideoRendererManagerPrivate::removeRenderer(Video::Renderer* r)
{
    const auto id = m_hRendererIds.value(r);
    auto t = m_hThreads.value(r);

    m_hRendererIds.remove(r);
    m_hRenderers.remove(id);
    m_hThreads.remove(r);

    if (t) {
        t->deleteLater();
    }

    r->deleteLater();
}

/**
 * A video stopped being rendered
 *
 * @warning This method can be called multiple time for the same renderer
 */
void VideoRendererManagerPrivate::stoppedDecoding(const QString& id, const QString& shmPath)
{
    Q_UNUSED(shmPath)

    if (!m_hRenderers.contains(id.toLatin1()) || !m_hRenderers.contains(id.toLatin1())) {
        qWarning() << "Cannot stop decoding, renderer" << id << "not found";
        return; // nothing to do
    }

    auto r = m_hRenderers.value(id.toLatin1());

    Call* c = CallModel::instance().getCall(id);

    // TODO: the current implementeation of CallPrivate::removeRenderer() does nothing
    if (c && c->lifeCycleState() == Call::LifeCycleState::FINISHED) {
        c->d_ptr->removeRenderer(r);
    }

    r->stopRendering();

    qDebug() << "Video stopped for call" << id <<  "Renderer found:" << m_hRenderers.contains(id.toLatin1());

    Video::Device* dev = Video::DeviceModel::instance().getDevice(id);

    if (dev)
        emit dev->renderingStopped(r);

    if (id == PREVIEW_RENDERER_ID) {
        m_PreviewState = false;
        emit q_ptr->previewStateChanged(false);
        emit q_ptr->previewStopped(r);
    }

    QThread* t = m_hThreads.value(r);

    if (t) {
        t->quit();
        t->wait();
    }

    // decoding stopped; remove the renderer, if/when call is over
    if (c && c->lifeCycleState() == Call::LifeCycleState::FINISHED) {
        removeRenderer(r);
    } else if (c) {
        connect(c, &Call::isOver, this, &VideoRendererManagerPrivate::callIsOver);
    }
}

void VideoRendererManagerPrivate::callIsOver()
{
    if (auto call = qobject_cast<Call *>(sender())) {
        if (auto r = m_hRenderers.value(call->dringId().toLatin1()))
            removeRenderer(r);
        else
            qDebug() << "Could not delete renderer, it might have already been removed:" << call->dringId();

        // remove the connection from this call to this
        disconnect(call, &Call::isOver, this, 0);
    }
}

#else

/* We use the previous implementation of VideoRendererManager for Windows
systems because the new causes segfault on camera stop.
This is a highly sensible code modify with caution. */

void VideoRendererManagerPrivate::removeRenderer(Video::Renderer* r)
{
   if (!r || !m_hRenderers.contains(m_hRendererIds[r]))
      return;

   const QByteArray id = m_hRendererIds[r];

   //Quit if for some reasons the renderer is not found
   if ( !r ) {
      qWarning() << "Cannot stop rendering, renderer" << id << "not found";
      return;
   }

   Call* c = CallModel::instance().getCall(id);

   if (c && c->lifeCycleState() == Call::LifeCycleState::FINISHED) {
      c->d_ptr->removeRenderer(r);
   }

   r->stopRendering();

   qDebug() << "Video stopped for call" << id <<  "Renderer found:" << m_hRenderers.contains(id);

   Video::Device* dev = Video::DeviceModel::instance().getDevice(id);

   if (dev)
      emit dev->renderingStopped(r);

   if (id == PREVIEW_RENDERER_ID) {
      m_PreviewState = false;
      emit q_ptr->previewStateChanged(false);
      emit q_ptr->previewStopped(r);
   }

   QThread* t = m_hThreads[r];

   if (t) {
       t->quit();
       t->wait();
   }

   if (c && c->lifeCycleState() == Call::LifeCycleState::FINISHED) {

       m_hRendererIds.remove(r);
       m_hRenderers.remove(id);

       m_hThreads[r] = nullptr;
       if (t) {
           t->deleteLater();
       }

       r->deleteLater();
   }
}

void VideoRendererManagerPrivate::stoppedDecoding(const QString& id, const QString& shmPath)
{
   Q_UNUSED(shmPath)

   if (m_hRenderers.contains(id.toLatin1())) {
      removeRenderer(m_hRenderers[id.toLatin1()]);
   }
}

//The moc is generated before preprocessing so we need an implementation too
void VideoRendererManagerPrivate::callIsOver() {}

#endif

void VideoRendererManager::switchDevice(const Video::Device* device) const
{
   VideoManagerInterface& interface = VideoManager::instance();
   interface.switchInput(device->id());
}

#include <videorenderermanager.moc>
