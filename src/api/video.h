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

#include "typedefs.h"

#include <QObject>
#include <QPair>

#include <map>
#include <memory>
#include <string>
#include <vector>

extern "C" {
#include <libavutil/frame.h>
}

namespace lrc {
namespace api {
namespace video {
Q_NAMESPACE
Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

constexpr static const char PREVIEW_RENDERER_ID[] = "local";

using Channel = QString;
using Resolution = QString;
using Framerate = float;
using FrameratesList = QVector<Framerate>;
using ResRateList = QVector<QPair<Resolution, FrameratesList>>;
using Capabilities = QMap<Channel, ResRateList>;

/**
 * This class is used by Renderer class to expose video data frame
 * that could be owned by instances of this class or shared.
 * Currently, DirectRenderer implementation (libwrap) uses avframe
 * containers and ShmRenderer implementations uses raw buffers.
 */

struct Frame
{
    // Used by SHM renderer.
    uint8_t* ptr {nullptr};
    size_t size {0};

    // Used by direct renderer(avframerenderer implementation).
    // clang-format off
    std::unique_ptr<AVFrame, void (*)(AVFrame*)> avframe {nullptr, [](AVFrame*) {}};
    // clang-format on
};

enum class DeviceType { CAMERA, DISPLAY, FILE, INVALID };
Q_ENUM_NS(DeviceType)

// This class describes the current video input device.
struct RenderedDevice
{
    QString name;
    DeviceType type = DeviceType::INVALID;
};

// This class describes current video settings
struct Settings
{
    Channel channel = "";
    QString name = "";
    QString id = "";
    Framerate rate = 0;
    Resolution size = "";
};
} // namespace video
} // namespace api
} // namespace lrc
