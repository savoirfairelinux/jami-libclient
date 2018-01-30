/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                                  *
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
#include <vector>
#include <map>

// Data
#include "interaction.h"

namespace lrc
{

namespace api
{

namespace conversation
{

struct Info
{
    std::string uid = "";
    std::string accountId;
    std::vector<std::string> participants;
    std::string callId;
    std::string confId;
    std::map<uint64_t, interaction::Info> interactions;
    uint64_t lastMessageUid = 0;
    unsigned int unreadMessages = 0;
};

} // namespace conversation
} // namespace api
} // namespace lrc
