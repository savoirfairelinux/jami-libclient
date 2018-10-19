/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                                  *
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
#include <string>
#include <vector>

namespace lrc
{

namespace api
{

namespace video
{

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

} // namespace video
} // namespace api
} // namespace lrc
