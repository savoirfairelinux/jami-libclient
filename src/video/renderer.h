/****************************************************************************
 *    Copyright (C) 2012-2022 Savoir-faire Linux Inc.                       *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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

// Base
#include <QtCore/QObject>
#include "api/newvideo.h"
#include <typedefs.h>

// Std
#include <memory>
#include <vector>
#include <cstdint>

// Qt
class QMutex;
struct AVFrame;

namespace Video {

class RendererPrivate;
class ShmRendererPrivate;
class ShmRenderer;
class DirectRendererPrivate;
class DirectRenderer;

/**
 * This class provide a rendering object to be used by clients
 * to get the video content. This object is not intended to be
 * extended outside of the LibRingClient.
 *
 * Each platform transparently provide its own implementation.
 */
class LIB_EXPORT Renderer : public QObject
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
    Q_OBJECT
#pragma GCC diagnostic pop

    friend class Video::ShmRendererPrivate;
    friend class Video::ShmRenderer;
    friend class Video::DirectRendererPrivate;
    friend class Video::DirectRenderer;

public:
    /**
     * Each platform may have its preferred color space. To be able to use a
     * client on multiple platforms, they need to check the colorspace.
     */
    enum class ColorSpace {
        BGRA, /*!< 32bit BLUE  GREEN RED ALPHA */
        RGBA, /*!< 32bit ALPHA GREEN RED BLUE  */
    };

    // Constructor
    Renderer(const QString& id, const QSize& res);
    virtual ~Renderer();

    // Getters
    virtual bool isRendering() const;
    virtual FrameBufferPtr currentFrame() const = 0;
    virtual QSize size() const;
    virtual QMutex* mutex() const;
    virtual ColorSpace colorSpace() const = 0;
#if defined(ENABLE_LIBWRAP)
    virtual std::unique_ptr<AVFrame, void (*)(AVFrame*)> currentAVFrame() const = 0;
#endif
    void setSize(const QSize& size) const;

Q_SIGNALS:
    void frameUpdated(); // Emitted when a new frame is ready
    void stopped();
    void started();

public Q_SLOTS:
    virtual void startRendering() = 0;
    virtual void stopRendering() = 0;

private:
    RendererPrivate* d_ptr;
    Q_DECLARE_PRIVATE(Renderer)
};

} // namespace Video
