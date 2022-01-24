/****************************************************************************
 *    Copyright (C) 2012-2022 Savoir-faire Linux Inc.                       *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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

#ifdef ENABLE_LIBWRAP

// Base
#include <QtCore/QObject>
#include "typedefs.h"
#include "video/renderer.h"
#include "videomanager_interface.h"

// Qt
class QMutex;
class QTimer;
class QThread;

namespace Video {

/// Manage shared memory and convert it to QByteArray
class LIB_EXPORT DirectRenderer final : public Renderer
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
    Q_OBJECT
#pragma GCC diagnostic pop

public:
    // Constructor
    DirectRenderer(const QString& id, const QSize& res);
    virtual ~DirectRenderer();

    // Getter
    const DRing::SinkTarget& sinkTarget() const;
    virtual ColorSpace colorSpace() const override;
    virtual VideoBufferIfPtr currentFrame() const override;
    virtual std::unique_ptr<AVFrame, void (*)(AVFrame*)> currentAVFrame() const override;
    void configureTarget();

    // SinkTarget callbacks
    VideoBufferIfPtr pullFrameBuffer(std::size_t bytes);
    void pushFrameBuffer(VideoBufferIfPtr buf);

private:
    DRing::SinkTarget sinkTarget_;
    mutable VideoBufferIfPtr daemonFramePtr_;
    mutable size_t frameCounter_ {0};

public Q_SLOTS:
    virtual void startRendering() override;
    virtual void stopRendering() override;
};

} // namespace Video

#endif
