/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
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
#include "baserender.h"

// Models and database
#include "api/lrc.h"

namespace lrc
{

using namespace api;

BaseRender::BaseRender(int width, int height)
: QObject()
, width_(width)
, height_(height)
{
}

BaseRender::~BaseRender()
{
}

bool
BaseRender::isRendering() const
{
    return false;
}

lrc::api::video::Frame
BaseRender::currentFrame() const
{
    return lrc::api::video::Frame();
}

lrc::api::video::ColorSpace
BaseRender::colorSpace() const
{
    return lrc::api::video::ColorSpace();
}
   
void
BaseRender::stopRendering()
{

}

bool
BaseRender::startRendering()
{
    return false;
}

int
BaseRender::getWidth()
{
    return width_;
}

int
BaseRender::getHeight()
{
    return height_;
}

} // namespace lrc
