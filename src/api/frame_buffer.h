/****************************************************************************
 *    Copyright (C) 2018-2022 Savoir-faire Linux Inc.                       *
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

// TODO. A big hack, fix me.
// This is a duplicate of what declared in daemon

#if ENABLE_LIBWRAP

#include "videomanager_interface.h"

using VideoFrameBufferIf = DRing::VideoFrameBufferIf;
using VideoFrameBufferIfPtr = DRing::SinkTarget::VideoFrameBufferIfPtr;
using VideoBufferType = DRing::VideoBufferType;
#else
extern "C" {
struct AVFrame;
}

namespace lrc {
namespace api {
namespace video {

class VideoFrameBufferIf
{
public:
    VideoFrameBufferIf() = delete;
    VideoFrameBufferIf(const VideoFrameBufferIf&) = delete;
    VideoFrameBufferIf(const VideoFrameBufferIf&&) = delete;

    VideoFrameBufferIf(std::size_t size);
    VideoFrameBufferIf(uint8_t* buf, size_t size);
    virtual ~VideoFrameBufferIf();

    virtual void allocateMemory(int format, int width, int height, int align) = 0;
    virtual std::size_t size() const = 0;
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual int planes() const = 0;
    virtual int stride(int plane) const = 0;
    virtual uint8_t* ptr(int plane = 0) = 0;
    // Carefull. Just temporary.
    virtual AVFrame* avframe() = 0;
    // Format as listed by AVPixelFormat (avutils/pixfmt.h)
    virtual int format() const = 0;
};

} // namespace video
} // namespace api
} // namespace lrc

using VideoFrameBufferIfPtr = std::unique_ptr<lrc::api::video::VideoFrameBufferIf>;
#endif