/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
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

#include <api/newvideo.h>

#include <dbus/videomanager.h>

namespace lrc
{

namespace api
{

namespace video
{

Renderer::Renderer(const std::string& id, Settings videoSettings)
: id_(id), videoSettings_(videoSettings)
{
    QString sizeStr = videoSettings.size.c_str();
    auto sizeSplited = sizeStr.split('x');
    if (sizeSplited.size() != 2) {
        return;
    }
    auto width = sizeSplited.at(0).toInt();
    auto height = sizeSplited.at(1).toInt();
#ifdef ENABLE_LIBWRAP
      renderer = std::make_unique<Video::DirectRenderer>(id.c_str(), QSize(width, height));
#else //ENABLE_LIBWRAP
      renderer = std::make_unique<Video::ShmRenderer>(id.c_str(), "", QSize(width, height));
#endif
    renderer->moveToThread(&thread_);
}

Renderer::~Renderer()
{
    stopRendering();
}

void
Renderer::initThread()
{
#ifdef ENABLE_LIBWRAP
    VideoManager::instance().registerSinkTarget(id_, renderer->target());
#endif
    if (!thread_.isRunning())
       thread_.start();
}

void
Renderer::update(const std::string& res, const std::string shmPath)
{
    if (!thread_.isRunning())
       thread_.start();

    QString sizeStr = res.c_str();
    auto sizeSplited = sizeStr.split('x');
    if (sizeSplited.size() != 2) {
        return;
    }
    auto width = sizeSplited.at(0).toInt();
    auto height = sizeSplited.at(1).toInt();
    renderer->setSize(QSize(width, height));

#ifdef ENABLE_LIBWRAP
    VideoManager::instance().registerSinkTarget(id.c_str(), renderer->target());
#else //ENABLE_LIBWRAP
    renderer->setShmPath(shmPath.c_str());
#endif
}

//Getters
bool
Renderer::isPreviewing() const
{
    return isPreviewing_;
}

std::string
Renderer::getId() const
{
    return id_;
}

void
Renderer::stopPreviewing()
{
    VideoManager::instance().stopCamera();
    isPreviewing_ = false;
}

void
Renderer::startPreviewing()
{
    if (isPreviewing_)
        return;
    VideoManager::instance().startCamera();
    isPreviewing_ = true;
}

void
Renderer::stopRendering()
{
    renderer->stopRendering();
}

void
Renderer::startRendering()
{
    renderer->startRendering();
}

}}} // end of namespace lrc::api::video

#include "api/moc_newvideo.cpp"
