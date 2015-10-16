/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
 *   Author: Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>  *
 *   Author: Alexandre Lision <alexandre.lision@savoirfairelinux.com>       *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
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
constexpr static const char DIRECT_RENDERER_ID[] = "direct";

class VideoRendererManagerPrivate final : public QObject
{
    Q_OBJECT

    friend class VideoRendererManager;

private:
    VideoRendererManagerPrivate(VideoRendererManager* parent);

    //Attributes
    bool                               m_PreviewState;
    uint                               m_BufferSize  ;
    QHash<QByteArray,Video::Renderer*> m_hRenderers  ;
    QHash<Video::Renderer*,QByteArray> m_hRendererIds;

    //Helper
    void removeRenderer(const QByteArray& rid);

    VideoRendererManager* q_ptr;

private Q_SLOTS:
   void startedDecoding(const QString& id, const QString& shmPath, int width, int height);
   void stoppedDecoding(const QString& id, const QString& shmPath);
};

VideoRendererManagerPrivate::VideoRendererManagerPrivate(VideoRendererManager* parent)
    : QObject(parent)
    , q_ptr(parent)
    , m_BufferSize(0)
    , m_PreviewState(false)
{}

///Constructor
VideoRendererManager::VideoRendererManager()
    : QObject(QCoreApplication::instance())
    , d_ptr(new VideoRendererManagerPrivate(this))
{
   VideoManagerInterface& interface = VideoManager::instance();
   connect( &interface , &VideoManagerInterface::startedDecoding, d_ptr.data(), &VideoRendererManagerPrivate::startedDecoding);
   connect( &interface , &VideoManagerInterface::stoppedDecoding, d_ptr.data(), &VideoRendererManagerPrivate::stoppedDecoding);
}


VideoRendererManager::~VideoRendererManager()
{}

///Meyer's singleton
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
    if (not call or not call->hasRemote())
        return nullptr;

    const auto rid = call->dringId().toLatin1();
    if (not d_ptr->m_hRenderers.contains(rid))
        return nullptr;

    return d_ptr->m_hRenderers[rid];
}

Video::Renderer* VideoRendererManager::makePreviewRenderer() const
{
    if ((!Video::DeviceModel::instance()->activeDevice()) or
        (!Video::DeviceModel::instance()->activeDevice()->activeChannel())) {
        qWarning() << "No device found";
        return nullptr;
    }

    Video::Resolution* res = Video::DeviceModel::instance()->activeDevice()->activeChannel()->activeResolution();

    if (!res) {
        qWarning() << "Misconfigured video device";
        return nullptr;
    }

    Video::Renderer* r;

#ifdef ENABLE_LIBWRAP
    r = new Video::DirectRenderer(PREVIEW_RENDERER_ID, res->size());
#else
    r = new Video::ShmRenderer(PREVIEW_RENDERER_ID, "", res->size());
#endif

    d_ptr->m_hRenderers[PREVIEW_RENDERER_ID] = r;
    d_ptr->m_hRendererIds[r] = PREVIEW_RENDERER_ID;

    return r;
}

///Get the video preview Renderer
Video::Renderer* VideoRendererManager::previewRenderer() const
{
    if (!d_ptr->m_hRenderers.contains(PREVIEW_RENDERER_ID))
        return makePreviewRenderer();
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
    Q_UNUSED(shmPath); //When directly linked, there is no SHM

    const QSize res = QSize(width,height);
    QByteArray rid = id.toLatin1();

#ifdef ENABLE_LIBWRAP
    if (id != PREVIEW_RENDERER_ID)
        rid = DIRECT_RENDERER_ID;
#endif

    Video::Renderer* r;

    qDebug() << "Search for renderer" << rid;
    if (not m_hRenderers.contains(rid)) {
#ifdef ENABLE_LIBWRAP
        r = new Video::DirectRenderer(rid, res);
#else
        r = new Video::ShmRenderer(rid, shmPath, res);
#endif
        m_hRenderers[rid] = r;
        m_hRendererIds[r] = rid;
    } else {
        r = m_hRenderers[rid];

        r->setSize(res);

#ifndef ENABLE_LIBWRAP
        dynamic_cast<Video::ShmRenderer*>(r)->setShmPath(shmPath);
#endif
    }

    qDebug() << "Starting renderer" << rid;
    r->startRendering();

    if (auto dev = Video::DeviceModel::instance()->getDevice(id))
        emit dev->renderingStarted(r);

    if (id != PREVIEW_RENDERER_ID) {
        if (auto c = CallModel::instance()->getCall(id))
            c->d_ptr->registerRenderer(r); // FIXME: bad private access
    } else {
        m_PreviewState = true;
        emit q_ptr->previewStateChanged(true);
        emit q_ptr->previewStarted(r);
    }
}

/**
 * @warning This method can be called multiple time for the same renderer
 */
void VideoRendererManagerPrivate::removeRenderer(const QByteArray& rid)
{
    if (not m_hRenderers.contains(rid))
        return;

    auto r = m_hRenderers[rid];
    Q_CHECK_PTR(r);

   if (auto c = CallModel::instance()->getCall(rid))
       c->d_ptr->removeRenderer(r); // FIXME: bad private access

   r->stopRendering();
   qDebug() << "Stopped renderer" << rid;

   if (auto dev = Video::DeviceModel::instance()->getDevice(rid))
      emit dev->renderingStopped(r);

   if (rid == PREVIEW_RENDERER_ID) {
      m_PreviewState = false;
      emit q_ptr->previewStateChanged(false);
      emit q_ptr->previewStopped(r);
   }

   m_hRendererIds.remove(r);
   m_hRenderers.remove(rid);
   delete r;
}

///A video stopped being rendered
void VideoRendererManagerPrivate::stoppedDecoding(const QString& id, const QString& shmPath)
{
    Q_UNUSED(shmPath);

    const QByteArray rid = id.toLatin1();

#ifdef ENABLE_LIBWRAP
    if (id != PREVIEW_RENDERER_ID)
        rid = DIRECT_RENDERER_ID;
#endif

    removeRenderer(rid);
}

void VideoRendererManager::switchDevice(const Video::Device* device) const
{
   VideoManager::instance().switchInput(device->id());
}

#include <videorenderermanager.moc>
