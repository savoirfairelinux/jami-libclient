/****************************************************************************
 *   Copyright (C) 2018 Savoir-faire Linux                                  *
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

// LRC
#include "api/newvideo.h"
#include "dbus/videomanager.h"
#ifdef ENABLE_LIBWRAP
 #include "directrenderer.h"
#else
 #include "shmrenderer.h"
#endif

// std
#include <mutex>

namespace lrc
{

using namespace api::video;

class RendererPimpl: public QObject
{
public:
    RendererPimpl(const Renderer& linked, const std::string& id,
        Settings videoSettings, const std::string& shmPath);
    ~RendererPimpl();

    const Renderer& linked;

    std::string id_;
    Settings videoSettings_;
    QThread thread_;

    /**
     * Convert a string (wxh) to a QSize
     * @param res the string to convert
     * @return the QSize object
     */
    static QSize stringToQSize(const std::string& res);

    std::mutex rendering_mtx_;

#ifdef ENABLE_LIBWRAP
     std::unique_ptr<Video::DirectRenderer> renderer;
#else
     std::unique_ptr<Video::ShmRenderer> renderer;
#endif
};

namespace api
{

namespace video
{

Renderer::Renderer(const std::string& id, Settings videoSettings,
    const std::string& shmPath)
: pimpl_(std::make_unique<RendererPimpl>(*this, id, videoSettings, shmPath))
{}

Renderer::~Renderer()
{
    stopRendering();
}

void
Renderer::initThread()
{
    if (!pimpl_->renderer)
        return;
#ifdef ENABLE_LIBWRAP
    VideoManager::instance().registerSinkTarget(
        QString::fromStdString(pimpl_->id_), pimpl_->renderer->target());
#endif
    if (!pimpl_->thread_.isRunning())
       pimpl_->thread_.start();
}

void
Renderer::update(const std::string& res, const std::string& shmPath)
{
    if (!pimpl_->thread_.isRunning())
       pimpl_->thread_.start();

    // res = "WIDTHxHEIGHT"
    QSize size = RendererPimpl::stringToQSize(res);
    pimpl_->renderer->setSize(size);

#ifdef ENABLE_LIBWRAP
    VideoManager::instance().registerSinkTarget(pimpl_->id_.c_str(),
        pimpl_->renderer->target());
#else //ENABLE_LIBWRAP
    pimpl_->renderer->setShmPath(shmPath.c_str());
#endif
}

bool
Renderer::isRendering() const
{
    std::lock_guard<std::mutex> lk(pimpl_->rendering_mtx_);
    if (pimpl_->renderer)
        return pimpl_->renderer->isRendering();
    return false;
}

std::string
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

QSize
Renderer::size() const
{
    if (pimpl_->renderer)
        return pimpl_->renderer->size();
    return QSize();
}

void
Renderer::quit()
{
    pimpl_->thread_.quit();
    pimpl_->thread_.wait();
}

void
Renderer::startRendering()
{
    if (pimpl_->renderer) {
        std::lock_guard<std::mutex> lk(pimpl_->rendering_mtx_);
        if (pimpl_->renderer->isRendering())
            return;
        pimpl_->renderer->startRendering();
    }
}

void
Renderer::stopRendering()
{
    if (pimpl_->renderer) {
        std::lock_guard<std::mutex> lk(pimpl_->rendering_mtx_);
        pimpl_->renderer->stopRendering();
    }
}

}} // end of api::video

RendererPimpl::RendererPimpl(const Renderer& linked, const std::string& id,
    Settings videoSettings, const std::string& shmPath)
: linked(linked)
, id_(id), videoSettings_(videoSettings)
{
    QSize size = stringToQSize(videoSettings.size);
#ifdef ENABLE_LIBWRAP
    renderer = std::make_unique<Video::DirectRenderer>(id.c_str(), size);
#else  // ENABLE_LIBWRAP
    renderer = std::make_unique<Video::ShmRenderer>(id.c_str(), shmPath.c_str(), size);
#endif
    renderer->moveToThread(&thread_);
}

RendererPimpl::~RendererPimpl()
{

}

QSize
RendererPimpl::stringToQSize(const std::string& res)
{
    QString sizeStr = res.c_str();
    auto sizeSplited = sizeStr.split('x');
    if (sizeSplited.size() != 2) return {};
    auto width = sizeSplited.at(0).toInt();
    auto height = sizeSplited.at(1).toInt();
    return QSize(width, height);
}

} // end of namespace lrc

#include "api/moc_newvideo.cpp"
