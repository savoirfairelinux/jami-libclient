/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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

// Std
#include <memory>

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"
#include "namedirectory.h"

namespace lrc
{

//~ class BaseRender;

class RenderersPimpl;

namespace api
{
class Lrc;

class NewShmRenderer;

#ifdef ENABLE_LIBWRAP
    #define RENDERER_TYPE lrc::api::NewShmRenderer
#else
    #define RENDERER_TYPE lrc::api::NewShmRenderer
#endif

namespace video
{
    enum class ColorSpace {
        BGRA,
        RGBA
    };

/** [jn] form legacy
 * This class is used by Renderer class to expose video data frame
 * that could be owned by instances of this class or shared.
 * If an instance carries data, "storage.size()" is greater than 0
 * and equals to "size", "ptr" is equals to "storage.data()".
 * If shared data is carried, only "ptr" and "size" are set.
 */
    struct Frame {
    uint8_t* ptr     {nullptr};
    std::size_t size {0};
    std::vector<uint8_t> storage {};
    };

    class Device {
        Device(){};
        ~Device(){};
    };
} // namespace video

class Renderers : public QObject {
    Q_OBJECT

public:
    Renderers();
    ~Renderers();

    std::shared_ptr<RENDERER_TYPE> getRenderer(const std::string& callId);
    std::shared_ptr<RENDERER_TYPE> getPreviewRenderer();
    void removeRenderer(const std::string& rendererId);
    
    void switchDevice(const std::string& rendererId);
    void startPreview();
    void stopPreview();
    bool isPreviewing();

    bool hasRenderer(const std::string& rendererId); // [jn] renderer id can be call id or local
    std::shared_ptr<RENDERER_TYPE> from(const std::string& rendererId); // [jn] renderer id can be call id or local
    

Q_SIGNALS:
    void renderLocalStarted();
    void renderLocalStopped();
    void renderRemoteStarted(const std::string& rendererId);
    void renderRemoteStopped(const std::string& rendererId);


private:
    std::unique_ptr<RenderersPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
