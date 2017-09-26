/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
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
#include <memory>
#include <string>
#include <map>

// Qt
#include <qobject.h>

namespace lrc
{

class NewCallModelPimpl;

namespace api
{

namespace account { struct Info; }
namespace call { struct Info; }
class NewAccountModel;

class NewCallModel : public QObject {
    Q_OBJECT
public:
    using CallInfoMap = std::map<std::string, std::shared_ptr<call::Info>>;

    const account::Info& owner;

    enum class Media {
        NONE,
        AUDIO,
        VIDEO
    };

    NewCallModel(NewAccountModel& parent, const account::Info& info);
    ~NewCallModel();

    const call::Info& createCall(const std::string& contactUri);

    void hangUp(const std::string& callId) const;
    void togglePause(const std::string& callId) const;
    void toggleMedia(const std::string& callId, const Media media) const;
    void toggleRecoringdAudio(const std::string& callId) const;
    void setQuality(const std::string& callId, const double quality) const;
    void transfer(const std::string& callId, const std::string& to) const;
    void addParticipant(const std::string& callId, const std::string& participant);
    void removeParticipant(const std::string& callId, const std::string& participant);

private:
    std::unique_ptr<NewCallModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
