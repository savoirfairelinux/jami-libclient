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

#include <api/video.h>

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

//Getters
bool
Renderer::isRendering() const
{
    return isRendering_;
}

std::string
Renderer::getId() const
{
    return id_;
}

void
Renderer::stopRendering()
{
    VideoManager::instance().stopCamera();
    isRendering_ = false;
}

void
Renderer::startRendering()
{
    if (isRendering_)
        return;
    VideoManager::instance().startCamera();
    isRendering_ = true;
}

}}} // end of namespace lrc::api::video
