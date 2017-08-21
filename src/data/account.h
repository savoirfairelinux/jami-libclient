/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
#include <memory>
#include "newcallmodel.h"
#include "contactmodel.h"
#include "conversationmodel.h"

namespace lrc
{

namespace account
{

enum class Type {
    INVALID,
    RING,
    SIP
};

struct Info
{
    std::string id;
    Type type = account::Type::INVALID;
    std::shared_ptr<NewCallModel> callModel;
    std::shared_ptr<ContactModel> contactModel;
    std::shared_ptr<ConversationModel> conversationModel;
};

}

using AccountsInfoMap = std::map<std::string, account::Info>;

} // namespace lrc
