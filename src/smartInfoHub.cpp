/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
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

#include "smartInfoHub.h"
#include "private/smartInfoHub_p.h"
#include "callmodel.h"
#include "typedefs.h"

#include <dbus/videomanager.h>
#include <dbus/callmanager.h>
#include <dbus/callmanager.h>



SmartInfoHub::SmartInfoHub(){
    d_ptr = new SmartInfoHubPrivate;
    connect(&CallManager::instance(), SIGNAL(SmartInfo(MapStringString)), d_ptr , SLOT(slotSmartInfo(MapStringString)));
}

void SmartInfoHub::start(){
    qDebug() << "Start SmartInfo";
    d_ptr->launchSmartInfo(d_ptr->m_refreshTimeInformationMS);
}

void SmartInfoHub::stop(){
    qDebug() << "Stop SmartInfo";
    d_ptr->launchSmartInfo(0);
}

void SmartInfoHubPrivate::launchSmartInfo(int tRefreshMs){
    
    CallManager::instance().launchSmartInfo(tRefreshMs);
}

//Retrieve information from the map and implement all the variables
void SmartInfoHubPrivate::setMapInfo(QMap<QString, QString> info){
    SmartInfoHubPrivate::m_information = info;
    SmartInfoHubPrivate::m_keyInfo = info.keys();
    emit SmartInfoHub::instance().changed();
}

SmartInfoHub& SmartInfoHub::instance(){
    //Singleton
    static SmartInfoHub instance_;
    return instance_;
}

void SmartInfoHub::setRefreshTime(int timeMS){
    d_ptr->m_refreshTimeInformationMS = timeMS;
}

void SmartInfoHubPrivate::slotSmartInfo(MapStringString map){
    SmartInfoHubPrivate::setMapInfo(map);
}

QList<QString> SmartInfoHub::keyChanged(){
    return d_ptr->m_keyInfo;
}
/*****************************************************************************
 *                                                                           *
 *                               Getter                                      *
 *                                                                           *
 ****************************************************************************/

float SmartInfoHub::localFps() const{
    if(d_ptr->m_information[LOCAL_FPS] != NULL)
        return d_ptr->m_information[LOCAL_FPS].toFloat();
    else
        return 0;
}

float SmartInfoHub::remoteFps() const{
    if(d_ptr->m_information[REMOTE_FPS] != NULL)
        return d_ptr->m_information[REMOTE_FPS].toFloat();
    else
        return 0;
}

int SmartInfoHub::remoteWidth() const{
    if(d_ptr->m_information[REMOTE_WIDTH] != NULL)
        return d_ptr->m_information[REMOTE_WIDTH].toInt();
    else
        return 0;
}

int SmartInfoHub::remoteHeight() const{
    if(d_ptr->m_information[REMOTE_HEIGHT] != NULL)
        return d_ptr->m_information[REMOTE_HEIGHT].toInt();
    else
        return 0;
}

QString SmartInfoHub::callID() const{
    if(d_ptr->m_information[CALL_ID] != NULL)
        return d_ptr->m_information[CALL_ID];
    else
        return d_ptr->m_DEFAULT_RETURN_VALUE_QSTRING;
}

QString SmartInfoHub::localVideoCodec() const{
    if(d_ptr->m_information[LOCAL_VIDEO_CODEC] != NULL)
        return d_ptr->m_information[LOCAL_VIDEO_CODEC];
    else
        return d_ptr->m_DEFAULT_RETURN_VALUE_QSTRING;
}

QString SmartInfoHub::localAudioCodec() const{
    if(d_ptr->m_information[LOCAL_AUDIO_CODEC] != NULL)
        return d_ptr->m_information[LOCAL_AUDIO_CODEC];
    else
        return d_ptr->m_DEFAULT_RETURN_VALUE_QSTRING;
}

QString SmartInfoHub::remoteVideoCodec() const{
    if(d_ptr->m_information[REMOTE_VIDEO_CODEC] != NULL)
        return d_ptr->m_information[REMOTE_VIDEO_CODEC];
    else
        return d_ptr->m_DEFAULT_RETURN_VALUE_QSTRING;
}

QString SmartInfoHub::remoteAudioCodec() const{
    if(d_ptr->m_information[REMOTE_AUDIO_CODEC] != NULL)
        return d_ptr->m_information[REMOTE_AUDIO_CODEC];
    else
        return d_ptr->m_DEFAULT_RETURN_VALUE_QSTRING;
}
