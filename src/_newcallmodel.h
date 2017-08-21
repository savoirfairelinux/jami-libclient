/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
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

// Qt
#include <qobject.h>

// Data
#include "_callinfo.h"

// Std
#include <memory>


typedef std::map<std::string, std::shared_ptr<CallInfo>> CallsInfo;

class NewCallModel : public QObject {
    Q_OBJECT
    public:
    explicit NewCallModel(QObject* parent = nullptr);
    ~NewCallModel();

    const CallInfo& createCall();
    void sendMessage(const std::string& callId, const std::string& body) const;
    void hangUp(const std::string& callId) const;
    void togglePause(const std::string& callId) const;
    void toggleMuteaUdio(const std::string& callId) const;
    void toggleMuteVideo(const std::string& callId) const;
    void toggleRecoringdAudio(const std::string& callId) const;
    void setQuality(const std::string& callId /*TODO: add parameter for quality*/) const;
    void transfer(const std::string& callId, const std::string& to) const;
    void addParticipant(const std::string& callId, const std::string& participant);
    void removeParticipant(const std::string& callId, const std::string& participant);

    private:
    CallsInfo calls_;

};
