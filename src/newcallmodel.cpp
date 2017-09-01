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
#include "api/newcallmodel.h"

#include "api/newaccountmodel.h"

namespace lrc
{

namespace api
{

class NewCallModelPimpl
{
public:
    NewCallModelPimpl(NewAccountModel& parent);
    ~NewCallModelPimpl();

    void sendMessage(const std::string& callId, const std::string& body) const;

    CallInfoMap calls;
    NewAccountModel& parent;
};

NewCallModel::NewCallModel(NewAccountModel& parent, const account::Info& info)
: owner(info)
, pimpl_(std::make_unique<NewCallModelPimpl>(parent))
{

}

NewCallModel::~NewCallModel()
{

}

const call::Info&
NewCallModel::createCall(const std::string& contactUri)
{

}

void
NewCallModel::hangUp(const std::string& callId) const
{

}

void
NewCallModel::togglePause(const std::string& callId) const
{

}

void
NewCallModel::toggleMedia(const std::string& callId, const Media media) const
{

}

void
NewCallModel::toggleRecoringdAudio(const std::string& callId) const
{

}

void
NewCallModel::setQuality(const std::string& callId, const double quality) const
{

}

void
NewCallModel::transfer(const std::string& callId, const std::string& to) const
{

}

void
NewCallModel::addParticipant(const std::string& callId, const std::string& participant)
{

}

void
NewCallModel::removeParticipant(const std::string& callId, const std::string& participant)
{

}

NewCallModelPimpl::NewCallModelPimpl(NewAccountModel& p)
: parent(p)
{

}

NewCallModelPimpl::~NewCallModelPimpl()
{

}

void
NewCallModelPimpl::sendMessage(const std::string& callId, const std::string& body) const
{

}

} // namespace api
} // namespace lrc
