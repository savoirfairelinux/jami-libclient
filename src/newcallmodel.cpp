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

// LRC
#include "api/newaccountmodel.h"
#include "api/call.h"

namespace lrc
{

using namespace api;

class NewCallModelPimpl
{
public:
    NewCallModelPimpl(NewCallModel& linked);
    ~NewCallModelPimpl();

    void sendMessage(const std::string& callId, const std::string& body) const;

    const NewCallModel& linked;
    NewCallModel::CallInfoMap calls;
};

NewCallModel::NewCallModel(const account::Info& owner)
: QObject()
, owner(owner)
, pimpl_(std::make_unique<NewCallModelPimpl>(*this))
{

}

NewCallModel::~NewCallModel()
{

}

const call::Info&
NewCallModel::createCall(const std::string& contactUri)
{

}

const call::Info&
NewCallModel::getCallFromURI(const std::string& uri) const
{
    return {};
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

NewCallModelPimpl::NewCallModelPimpl(NewCallModel& linked)
: linked(linked)
{

}

NewCallModelPimpl::~NewCallModelPimpl()
{

}

void
NewCallModelPimpl::sendMessage(const std::string& callId, const std::string& body) const
{

}

} // namespace lrc

#include "api/moc_newcallmodel.cpp"
