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
class DirectRendererPrivate;

/// Manage shared memory and convert it to QByteArray
class LIB_EXPORT DirectRenderer final : public Renderer
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
    Q_OBJECT
#pragma GCC diagnostic pop

public:
    // Constructor
    DirectRenderer(const QString& id, const QSize& res, bool useAVFrame);
    virtual ~DirectRenderer();

    // Getter
    const DRing::SinkTarget& sinkTarget() const;
    const DRing::AVSinkTarget& avSinkTarget() const;
    virtual ColorSpace colorSpace() const override;
    virtual FrameBufferPtr currentFrame() const override;
    virtual std::unique_ptr<AVFrame, void (*)(AVFrame*)> currentAVFrame() const override;
    void configureTarget(bool useAVFrame);

public Q_SLOTS:
    virtual void startRendering() override;
    virtual void stopRendering() override;

private:
    std::unique_ptr<DirectRendererPrivate> d_ptr;
};

} // namespace Video

#endif
