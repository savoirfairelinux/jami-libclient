/*
 *  Copyright (C) 2012-2022 Savoir-faire Linux Inc.
 *  Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>
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

#include "video/renderer.h"

using namespace lrc::video;
using namespace lrc::api::video;

class ShmRenderer final : public Renderer
{
    Q_OBJECT
public:
    // Constructor
    ShmRenderer(const QString& id, const QString& shmPath, const QSize& res);
    virtual ~ShmRenderer();

    void stopShm();
    bool startShm();

    int fps() const;
    void setShmPath(const QString& path);

    // Renderer interface.
    void update(const QString& res, const QString& shmPath) override;
    Frame currentFrame() const override;

public Q_SLOTS:
    void startRendering() override;
    void stopRendering() override;

private:
    struct Impl;
    std::unique_ptr<Impl> d_ptr;
};
