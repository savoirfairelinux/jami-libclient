/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
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
#include <map>
#include <memory>
#include <string>
#include <vector>

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"

// Qt
#include <QObject>
#include <QThread>

// private LRC
#ifdef ENABLE_LIBWRAP
 #include "directrenderer.h"
#else
 #include "shmrenderer.h"
#endif

namespace lrc
{

namespace api
{

namespace video
{


constexpr static const char PREVIEW_RENDERER_ID[] = "local";

using Channel = std::string;
using Resolution = std::string;
using Framerate = uint64_t;
using FrameratesList = std::vector<Framerate>;
using Capabilities = std::map<Channel, std::map<Resolution, FrameratesList>>;

/*
struct Device
{
    std::string name;
    Capabilities capabilites;
};
*/

struct Settings
{
    Channel channel = "";
    std::string name = "";
    Framerate rate = 0;
    Resolution size = "";
};

class LIB_EXPORT Renderer : public QObject {
    Q_OBJECT
public:
    Renderer(const std::string& id, Settings videoSettings);
    ~Renderer();

    void initThread();
    void update(const std::string& res, const std::string shmPath);
    void quit();

    //Getters
    bool isRendering() const;
    std::string getId() const;

public Q_SLOTS:
    void startRendering();
    void stopRendering();

Q_SIGNALS:
    void started(const std::string& id);
    void stopped(const std::string& id);

private:
    // Move into pimpl
    std::string id_;
    Settings videoSettings_;
    QThread thread_;
    bool isRendering_;

#ifdef ENABLE_LIBWRAP
     std::unique_ptr<Video::DirectRenderer> renderer;
#else
     std::unique_ptr<Video::ShmRenderer> renderer;
#endif
};

} // namespace video
} // namespace api
} // namespace lrc
