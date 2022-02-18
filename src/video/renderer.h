/*
 *  Copyright (C) 2018-2022 Savoir-faire Linux Inc.
 *  Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
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

#include "api/newvideo.h"
#include "typedefs.h"

#include <QObject>
#include <QSize>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace lrc {
namespace video {

class Renderer : public QObject
{
    Q_OBJECT
public:
    Renderer(const QString& id, const QSize& res);
    virtual ~Renderer();

    /**
     * @return renderer's id
     */
    QString id() const;

    /**
     * @return current renderer dimensions
     */
    QSize size() const;

    /**
     * Update size and shmPath of a renderer
     * @param size new renderer dimensions
     * @param shmPath new shmPath
     */
    virtual void update(const QSize& size, const QString& shmPath = {});

    /**
     * @return current rendered frame
     */
    virtual lrc::api::video::Frame currentFrame() const = 0;

public Q_SLOTS:
    virtual void startRendering() = 0;
    virtual void stopRendering() = 0;

Q_SIGNALS:
    void frameUpdated();
    void stopped();
    void started();
    void frameBufferRequested(AVFrame* avFrame);

private:
    QString id_;
    QSize size_;
};

} // namespace video
} // namespace lrc
