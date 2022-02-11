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

#include "renderer.h"

#include <QSize>
#include <QMutex>

namespace lrc {
namespace video {

using namespace lrc::api::video;

Renderer::Renderer(const QString& id, const QSize& res)
    : id_(id)
    , size_(res)
    , QObject(nullptr)
{}

Renderer::~Renderer() {}

QString
Renderer::id() const
{
    return id_;
}

QSize
Renderer::size() const
{
    return size_;
}

void
Renderer::update(const QSize& size, const QString&)
{
    size_ = size;
}

} // namespace video
} // namespace lrc
