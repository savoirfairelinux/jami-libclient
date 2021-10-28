/****************************************************************************
 *    Copyright (C) 2018-2021 Savoir-faire Linux Inc.                                  *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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

#include "api/newvideo.h"

#include "dbus/videomanager.h"
#ifdef ENABLE_LIBWRAP
#include "directrenderer.h"
#else
#include "shmrenderer.h"
#endif

#include <QSize>

#include <mutex>

namespace lrc {

using namespace api::video;

class RendererPimpl : public QObject
{
public:
    RendererPimpl(Renderer& linked,
                  const QString& id,
                  Settings videoSettings,
                  const QString& shmPath,
                  const bool useAVFrame);
    ~RendererPimpl();

    Renderer& linked_;

    QString id_;
    Settings videoSettings_;
    QThread thread_;
    bool usingAVFrame_;

    /**
     * Convert a string (WxH) to a QSize
     * @param res the string to convert
     * @return the QSize object
     */
    static QSize stringToQSize(const QString& res);

#ifdef ENABLE_LIBWRAP
    std::unique_ptr<Video::DirectRenderer> renderer;
#else
    std::unique_ptr<Video::ShmRenderer> renderer;
#endif

public Q_SLOTS:
    /**
     * Detect when the current frame is updated
     */
    void slotFrameUpdated();
};

namespace api {

namespace video {

Renderer::Renderer(const QString& id,
                   Settings videoSettings,
                   const QString& shmPath,
                   const bool useAVFrame)
    : pimpl_(std::make_unique<RendererPimpl>(*this, id, videoSettings, shmPath, useAVFrame))
{}

Renderer::~Renderer()
{
#ifdef ENABLE_LIBWRAP
    if (pimpl_->usingAVFrame_) {
        VideoManager::instance().registerAVSinkTarget(pimpl_->id_, {});
    } else {
        VideoManager::instance().registerSinkTarget(pimpl_->id_, {});
    }
#endif // ENABLE_LIBWRAP

    pimpl_.reset();
}

void
Renderer::update(const QString& res, const QString& shmPath)
{
    if (!pimpl_->thread_.isRunning())
        pimpl_->thread_.start();

    // res = "WIDTHxHEIGHT"
    QSize size = RendererPimpl::stringToQSize(res);
    pimpl_->renderer->setSize(size);

#ifdef ENABLE_LIBWRAP
    if (pimpl_->usingAVFrame_) {
        VideoManager::instance().registerAVSinkTarget(pimpl_->id_, pimpl_->renderer->avTarget());
    } else {
        VideoManager::instance().registerSinkTarget(pimpl_->id_, pimpl_->renderer->target());
    }
#else // ENABLE_LIBWRAP
    pimpl_->renderer->setShmPath(shmPath);
#endif
}

bool
Renderer::isRendering() const
{
    if (pimpl_->renderer)
        return pimpl_->renderer->isRendering();
    return false;
}

void
Renderer::useAVFrame(bool useAVFrame)
{
    pimpl_->usingAVFrame_ = useAVFrame;
#ifdef ENABLE_LIBWRAP
    pimpl_->renderer->configureTarget(useAVFrame);
#endif
}

bool
Renderer::useDirectRenderer() const
{
#ifdef ENABLE_LIBWRAP
    return true;
#else
    return false;
#endif
}

QString
Renderer::getId() const
{
    return pimpl_->id_;
}

Frame
Renderer::currentFrame() const
{
    // TODO(sblin) remove Video::Frame when deleting old models.
    auto frame = pimpl_->renderer->currentFrame();
    Frame result;
    result.ptr = frame.ptr;
    result.size = frame.size;
    result.storage = frame.storage;
    result.height = frame.height;
    result.width = frame.width;
    return result;
}

#if defined(ENABLE_LIBWRAP)
std::unique_ptr<AVFrame, void (*)(AVFrame*)>
Renderer::currentAVFrame() const
{
    return pimpl_->renderer->currentAVFrame();
}
#endif

QSize
Renderer::size() const
{
    return pimpl_->renderer->size();
}

} // namespace video
} // namespace api

RendererPimpl::RendererPimpl(Renderer& linked,
                             const QString& id,
                             Settings videoSettings,
                             const QString& shmPath,
                             bool useAVFrame)
    : linked_(linked)
    , id_(id)
    , videoSettings_(videoSettings)
    , usingAVFrame_(useAVFrame)
{
    QSize size = stringToQSize(videoSettings.size);
#ifdef ENABLE_LIBWRAP
    renderer = std::make_unique<Video::DirectRenderer>(id, size, usingAVFrame_);
#else // ENABLE_LIBWRAP
    renderer = std::make_unique<Video::ShmRenderer>(id, shmPath, size);
#endif
    renderer->moveToThread(&thread_);

    connect(&thread_, &QThread::finished, [this] { renderer.reset(); });

    connect(&linked,
            &Renderer::startRendering,
            renderer.get(),
            &Video::Renderer::startRendering,
            Qt::QueuedConnection);

    connect(renderer.get(), &Video::Renderer::frameUpdated, [this] {
        emit linked_.frameUpdated(id_);
    });

    connect(
        renderer.get(),
        &Video::Renderer::started,
        this,
        [this] { emit linked_.started(id_); },
        Qt::DirectConnection);
    connect(
        renderer.get(),
        &Video::Renderer::stopped,
        this,
        [this] { emit linked_.stopped(id_); },
        Qt::DirectConnection);

#ifdef ENABLE_LIBWRAP
    if (usingAVFrame_) {
        VideoManager::instance().registerAVSinkTarget(id_, renderer->avTarget());
    } else {
        VideoManager::instance().registerSinkTarget(id_, renderer->target());
    }
#endif

    thread_.start();
}

RendererPimpl::~RendererPimpl()
{
    thread_.quit();
    thread_.wait();
}

QSize
RendererPimpl::stringToQSize(const QString& res)
{
    QString sizeStr = res;
    auto sizeSplited = sizeStr.split('x');
    if (sizeSplited.size() != 2)
        return {};
    auto width = sizeSplited.at(0).toInt();
    auto height = sizeSplited.at(1).toInt();
    return QSize(width, height);
}

} // end of namespace lrc

#include "api/moc_newvideo.cpp"
