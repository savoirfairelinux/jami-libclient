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
#include <cstddef>
#include <assert.h>

#ifdef ENABLE_LIBWRAP

#include "videomanager_interface.h"

using VideoBufferIf = DRing::VideoBufferIf;
using VideoBufferIfPtr = DRing::SinkTarget::VideoBufferIfPtr;
using VideoBufferType = DRing::VideoBufferType;
#else
extern "C" {
struct AVFrame;
}

namespace lrc {
namespace api {
namespace video {

class LIB_EXPORT VideoBufferIf
{
public:
    VideoBufferIf() = default;
    VideoBufferIf(const VideoBufferIf&) = delete;
    VideoBufferIf(const VideoBufferIf&&) = delete;

    virtual ~VideoBufferIf() {};

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

class LIB_EXPORT GenericVideoBuffer : public VideoBufferIf
{
public:
    GenericVideoBuffer() {};
    GenericVideoBuffer(const GenericVideoBuffer&) = delete;
    GenericVideoBuffer(const GenericVideoBuffer&&) = delete;
    GenericVideoBuffer(std::size_t size)
        : bufferSize_(size) {};

    GenericVideoBuffer(uint8_t* buf, std::size_t size)
        : videoBuffer_(buf)
        , bufferSize_(size) {};

    ~GenericVideoBuffer() {};

    void allocateMemory(int format, int width, int height, int align) override {};
    std::size_t size() const override { return bufferSize_; };
    int width() const override { return width_; };
    int height() const override { return height_; };
    int planes() const override { return 1; };
    int stride(int plane = 0) const override
    {
        assert(plane == 0);
        assert(height_ > 0);
        return bufferSize_ / height_;
    };
    uint8_t* ptr(int plane = 0) override
    {
        assert(plane == 0);
        return videoBuffer_;
    };
    int format() const override { return format_; };
    AVFrame* avframe() override
    {
        assert(false);
        return {};
    };

private:
    int format_ {0};
    int width_ {0};
    int height_ {0};

    // True if the instance own the inner buffer.
    bool owner_ {false};
    uint8_t* videoBuffer_ {nullptr};
    std::size_t bufferSize_ {0};
};

} // namespace video
} // namespace api
} // namespace lrc

using VideoBufferIfPtr = std::unique_ptr<lrc::api::video::VideoBufferIf>;

#endif