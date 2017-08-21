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
#include "newcallmodel.h"

NewCallModel::NewCallModel(QObject* parent)
:QObject(parent)
{

}


NewCallModel::~NewCallModel()
{

}

const NewCall::Info&
NewCallModel::createCall()
{
    return NewCall::Info(0, std::time_t(), NewCall::Status::INVALID_STATUS);
}

void
NewCallModel::sendMessage(const std::string& callId, const std::string& body) const
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
NewCallModel::toggleMuteaUdio(const std::string& callId) const
{

}

void
NewCallModel::toggleMuteVideo(const std::string& callId) const
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
