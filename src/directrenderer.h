/*
 *  Copyright (C) 2012-2022 Savoir-faire Linux Inc.
 *  Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "dbus/videomanager.h"
#include "videomanager_interface.h"
#include "video/renderer.h"
#include "typedefs.h"

namespace lrc {
namespace video {

class DirectRenderer : public Renderer
{
    Q_OBJECT
public:
    DirectRenderer(const QString& id, const QSize& res);
    DirectRenderer();
    virtual ~DirectRenderer();

    // Renderer interface.
    void update(const QSize& res, const QString& shmPath) override;
    lrc::api::video::Frame currentFrame() const override;
    virtual DRing::SinkTarget& getTarget() = 0;
    static std::unique_ptr<DirectRenderer> CreateInstance(const QString& id, const QSize& res);
public Q_SLOTS:
    void startRendering() override;
    void stopRendering() override;
};

} // namespace video
} // namespace lrc
