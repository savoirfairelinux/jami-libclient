/*
 *  Copyright (C) 2015-2016 Savoir-faire Linux Inc.
 *  Author: Olivier Grégoire <olivier.gregoire@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */
#include "smartInfoHub.h"

#include <dbus/videomanager.h>
#include <dbus/callmanager.h>
#include <dbus/callmanager.h>

SmartInfoHub::SmartInfoHub(){
}

void SmartInfoHub::smartInfo(int tRefreshMs){

  CallManager::instance().getConferenceList();
  CallManager::instance().launchSmartInfo("bonjour", tRefreshMs);
}

//Retrieve information from the map and implement all the variables
void SmartInfoHub::setMapInfo(QMap<QString, QString> info){
    information_ = info;
    QList<QString> keyInfo = info.keys();

    emit SmartInfoHub::newInfo(keyInfo);
}


SmartInfoHub& SmartInfoHub::instance(){
    //Singleton
    static SmartInfoHub instance_;
    return instance_;
}

void SmartInfoHub::start(){
    qDebug() << "Start SmartInfo";
    connect(&CallManager::instance(), SIGNAL(SmartInfo(MapStringString)), this , SLOT(sSmartInfo(MapStringString)));
    SmartInfoHub::smartInfo(refreshTimeInformationMS_);
}

void SmartInfoHub::stop(){
    qDebug() << "Stop SmartInfo";
    SmartInfoHub::smartInfo(0);
}

void SmartInfoHub::setRefreshTime(int timeMS){
    SmartInfoHub::refreshTimeInformationMS_ = timeMS;
}

void SmartInfoHub::sSmartInfo(MapStringString map){
    SmartInfoHub::setMapInfo(map);
}
/*****************************************************************************
 *                                                                           *
 *                               Getter                                      *
 *                                                                           *
 ****************************************************************************/

float SmartInfoHub::localFps(){
    const QString LOCAL_FPS = "local FPS";
    if(information_[LOCAL_FPS] != NULL)
        return information_[LOCAL_FPS].toFloat();
    else
        return 0;
}

float SmartInfoHub::remoteFps(){
    const QString REMOTE_FPS = "remote FPS";
    if(information_[REMOTE_FPS] != NULL)
        return information_[REMOTE_FPS].toFloat();
    else
        return 0;
}

int SmartInfoHub::remoteWidth(){
    const QString REMOTE_WIDTH = "remote width";
    if(information_[REMOTE_WIDTH] != NULL)
        return information_[REMOTE_WIDTH].toInt();
    else
        return 0;
}

int SmartInfoHub::remoteHeight(){
    const QString REMOTE_HEIGHT = "remote height";
    if(information_[REMOTE_HEIGHT] != NULL)
        return information_[REMOTE_HEIGHT].toInt();
    else
        return 0;
}

QString SmartInfoHub::localVideoCodec(){
    const QString LOCAL_VIDEO_CODEC = "local video codec";
    if(information_[LOCAL_VIDEO_CODEC] != NULL)
        return information_[LOCAL_VIDEO_CODEC];
    else
        return SmartInfoHub::DEFAULT_RETURN_VALUE_QSTRING;
}

QString SmartInfoHub::remoteVideoCodec(){
    const QString REMOTE_VIDEO_CODEC = "remote video codec";
    if(information_[REMOTE_VIDEO_CODEC] != NULL)
        return information_[REMOTE_VIDEO_CODEC];
    else
        return SmartInfoHub::DEFAULT_RETURN_VALUE_QSTRING;
}

QString SmartInfoHub::remoteAudioCodec(){
    const QString REMOTE_AUDIO_CODEC = "remote audio codec";
    if(information_[REMOTE_AUDIO_CODEC] != NULL)
        return information_[REMOTE_AUDIO_CODEC];
    else
        return SmartInfoHub::DEFAULT_RETURN_VALUE_QSTRING;
}
