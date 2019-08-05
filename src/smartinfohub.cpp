/****************************************************************************
 *    Copyright (C) 2016-2019 Savoir-faire Linux Inc.                               *
 *   Author: Olivier Grégoire <olivier.gregoire@savoirfairelinux.com>       *
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

#include <stdexcept>
#include <string>
#include <map>

#include "smartinfohub.h"
#include "private/smartInfoHub_p.h"
#include "typedefs.h"
#include "daemonproxy.h"

SmartInfoHub::SmartInfoHub()
{
    d_ptr = new SmartInfoHubPrivate;
    DaemonProxy::instance().signalSmartInfo().connect(sigc::mem_fun(*d_ptr,&SmartInfoHubPrivate::slotSmartInfo));
}

SmartInfoHub::~SmartInfoHub()
{}

void SmartInfoHub::start()
{
    DaemonProxy::instance().startSmartInfo(d_ptr->m_refreshTimeInformationMS);
}

void SmartInfoHub::stop()
{
    DaemonProxy::instance().stopSmartInfo();
}

SmartInfoHub& SmartInfoHub::instance()
{
    //Singleton
    static SmartInfoHub instance_;
    return instance_;
}

void SmartInfoHub::setRefreshTime(uint32_t timeMS)
{
    d_ptr->m_refreshTimeInformationMS = timeMS;
}

//Retrieve information from the map and implement all the variables
void SmartInfoHubPrivate::slotSmartInfo(const std::map<std::string, std::string>& map)
{
    m_information = map;
    Q_EMIT SmartInfoHub::instance().changed();
}
//Getter

bool SmartInfoHub::isConference() const
{
    try {
        return (d_ptr->m_information.at("type") == "conference");
    } catch (std::out_of_range& e) {
        return false;
    }
}

float SmartInfoHub::localFps() const
{
    try {
        return std::stof(d_ptr->m_information.at(LOCAL_FPS));
    } catch (std::out_of_range& e) {
        return 0.0;
    }
}

float SmartInfoHub::remoteFps() const
{
    try {
        return std::stof(d_ptr->m_information.at(REMOTE_FPS));
    } catch (std::out_of_range& e) {
        return 0.0;
    }
}

int SmartInfoHub::remoteWidth() const
{
    try {
        return std::stoi(d_ptr->m_information.at(REMOTE_WIDTH));
    } catch (std::out_of_range& e) {
        return 0;
    }
}

int SmartInfoHub::remoteHeight() const
{
    try {
        return std::stoi(d_ptr->m_information.at(REMOTE_HEIGHT));
    } catch (std::out_of_range& e) {
        return 0;
    }
}

int SmartInfoHub::localWidth() const
{
    try {
        return std::stoi(d_ptr->m_information.at(LOCAL_WIDTH));
    } catch (std::out_of_range& e) {
        return 0;
    }
}

int SmartInfoHub::localHeight() const
{
    try {
        return std::stoi(d_ptr->m_information.at(LOCAL_HEIGHT));
    } catch (std::out_of_range& e) {
        return 0;
    }
}

QString SmartInfoHub::callID() const
{
    try {
        return QString::fromStdString(d_ptr->m_information.at(CALL_ID));
    } catch (std::out_of_range& e) {
        return SmartInfoHubPrivate::DEFAULT_RETURN_VALUE_QSTRING;
    }
}

QString SmartInfoHub::localVideoCodec() const
{
    try {
        return QString::fromStdString(d_ptr->m_information.at(LOCAL_VIDEO_CODEC));
    } catch (std::out_of_range& e) {
        return SmartInfoHubPrivate::DEFAULT_RETURN_VALUE_QSTRING;
    }
}

QString SmartInfoHub::localAudioCodec() const
{
    try {
        return QString::fromStdString(d_ptr->m_information.at(LOCAL_AUDIO_CODEC));
    } catch (std::out_of_range& e) {
        return SmartInfoHubPrivate::DEFAULT_RETURN_VALUE_QSTRING;
    }
}

QString SmartInfoHub::remoteVideoCodec() const
{
    try {
        return QString::fromStdString(d_ptr->m_information.at(REMOTE_VIDEO_CODEC));
    } catch (std::out_of_range& e) {
        return SmartInfoHubPrivate::DEFAULT_RETURN_VALUE_QSTRING;
    }
}

QString SmartInfoHub::remoteAudioCodec() const
{
    try {
        return QString::fromStdString(d_ptr->m_information.at(REMOTE_AUDIO_CODEC));
    } catch (std::out_of_range& e) {
        return SmartInfoHubPrivate::DEFAULT_RETURN_VALUE_QSTRING;
    }
}
