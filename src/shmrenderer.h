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

#ifndef ENABLE_LIBWRAP

// Base
#include "video/renderer.h"
#include "typedefs.h"
#include "api/frame_buffer.h"

// Qt
class QMutex;

// Private
struct SHMHeader;
struct AVFrame;

namespace Video {
class ShmRendererPrivate;
/* FrameBuffer is a generic video frame container */

/// Manage shared memory and convert it to QByteArray
class LIB_EXPORT ShmRenderer final : public Renderer
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
    Q_OBJECT
#pragma GCC diagnostic pop

public:
    // Constructor
    ShmRenderer(const QString& id, const QString& shmPath, const QSize& res);
    virtual ~ShmRenderer();

    // Mutators
    void stopShm();
    bool startShm();

    // Getters
    int fps() const;
    virtual FrameBufferBasePtr currentFrame() const override;
    virtual ColorSpace colorSpace() const override;

    // Setters
    void setShmPath(const QString& path);

private:
    std::unique_ptr<ShmRendererPrivate> d_ptr;

public Q_SLOTS:
    void startRendering() override;
    void stopRendering() override; // Unused
};

} // namespace Video

#endif
