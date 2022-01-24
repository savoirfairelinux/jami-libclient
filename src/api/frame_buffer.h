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

namespace lrc {

namespace api {

namespace video {

// TODO. A big hack, fix me.
// This is a duplicate of what declared in daemon

class FrameBufferBase
{
public:
    FrameBufferBase() = delete;
    FrameBufferBase(size_t size)
        : owner_(false)
        , bufferSize_(size)
    {
        assert(bufferSize_ != 0);
        videoBuffer_ = reinterpret_cast<uint8_t*>(std::malloc(size));
        assert(videoBuffer_ != nullptr);
    };

    FrameBufferBase(uint8_t* buf, size_t size)
        : owner_(false)
        , videoBuffer_(buf)
        , bufferSize_(size)
    {
        assert(videoBuffer_ != nullptr);
        assert(bufferSize_ != 0);
    };

    virtual ~FrameBufferBase()
    {
        if (owner_) {
            assert(videoBuffer_ != nullptr);
            assert(bufferSize_ != 0);
            std::free(videoBuffer_);
        }
    }

    uint8_t* ptr() { return videoBuffer_; };
    std::size_t size() const { return bufferSize_; };

    int format {0}; // as listed by AVPixelFormat (avutils/pixfmt.h)
    int width {0};  // frame width
    int height {0}; // frame height
    // True if the instance owns the internal buffer.
    bool owner_ {false};
    std::size_t bufferSize_ {0};
    uint8_t* videoBuffer_ {nullptr};
};

} // namespace video
} // namespace api
} // namespace lrc
