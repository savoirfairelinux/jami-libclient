/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>      *
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
#include <string>

// Qt
#include <qobject.h>

// Data
#include "api/conversation.h"

namespace lrc
{

namespace api
{

class NewCallModelI : public QObject {
    Q_OBJECT
public:
    enum class Media {
        NONE,
        AUDIO,
        VIDEO
    };

    virtual void hangUp(const std::string& callId) const = 0;
    virtual void togglePause(const std::string& callId) const = 0;
    virtual void toggleMedia(const std::string& callId, const Media media) const = 0;
    virtual void toggleRecoringdAudio(const std::string& callId) const = 0;
    virtual void setQuality(const std::string& callId, const double quality) const = 0;
    virtual void transfer(const std::string& callId, const std::string& to) const = 0;
    virtual void addParticipant(const std::string& callId, const std::string& participant) = 0;
    virtual void removeParticipant(const std::string& callId, const std::string& participant) = 0;
};

} // namespace api
} // namespace lrc
