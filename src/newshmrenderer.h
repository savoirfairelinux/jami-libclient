/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
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
#pragma once

// Std
#include <memory>

// Qt
#include <qobject.h>

// Lrc
//~ #include "renderers.h"
#include "baserender.h"

class SHMHeader;

namespace lrc
{

class NewShmRendererPimpl;

namespace api
{
class Lrc;


class NewShmRenderer : public BaseRender {
    Q_OBJECT

public:
    NewShmRenderer(const std::string& shmPath, int width, int height, bool startedDecoding);
    ~NewShmRenderer();

   bool isRendering() const;
   lrc::api::video::Frame currentFrame();

   void stopRendering();
   bool startRendering()const ;

private:
    std::unique_ptr<NewShmRendererPimpl> pimpl_;

};

} // namespace api
} // namespace lrc
