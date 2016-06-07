#include "smartInfoHub.h"
#include <dbus/videomanager.h>
#include "dbus/callmanager.h"
#include "callmodel.h"
#include <iostream>
#include <map>
#include <vector>
#include <QtCore/QUrl>
#include <QList>

SmartInfoHub::SmartInfoHub(){
}

void SmartInfoHub::smartInfo(int tRefreshMs){

  qDebug() << "Enter in smartInfoHub";
  CallManager::instance().getConferenceList();
  CallManager::instance().launchSmartInfo("bonjour", tRefreshMs);
}

//Retrieve information from the map and implement all the variables
void SmartInfoHub::setMapInfo(QMap<QString, QString> info){

    QList<QString> keyInfo = info.keys();

    emit SmartInfoHub::newInfo(keyInfo);

    for(int i = 0; i < keyInfo.size(); i++){
        if(keyInfo.at(i) == "remote FPS")
            SmartInfoHub::remoteFps_      = info["remote FPS"].toFloat();
        if(keyInfo.at(i) == "local FPS")
            SmartInfoHub::localFps_       = info["local FPS"].toFloat();
        if(keyInfo.at(i) == "remote width")
            SmartInfoHub::remoteWidth_    = info["remote width"].toInt();
        if(keyInfo.at(i) == "remote height")
            SmartInfoHub::remoteHeight_    = info["remote height"].toInt();
        if(keyInfo.at(i) == "local video codec")
            SmartInfoHub::localVideoCodec_ = info["local video codec"];
        if(keyInfo.at(i) == "remote video codec")
            SmartInfoHub::remoteVideoCodec_= info["remote video codec"];

        //qDebug()<<keyInfo.at(i);
    }
    if(info.find("remote FPS")!= info.end()){
        SmartInfoHub::remoteFps_=info["remote FPS"].toFloat();
    }
}


SmartInfoHub& SmartInfoHub::instance(){
    //Singleton
    static SmartInfoHub instance_;
    return instance_;
}

/*****************************************************************************
 *                                                                           *
 *                               Getter                                      *
 *                                                                           *
 ****************************************************************************/

float SmartInfoHub::getLocalFps(){
     return localFps_;
}

float SmartInfoHub::getRemoteFps(){
    return remoteFps_;
}

int SmartInfoHub::getRemoteWidth(){
    return remoteWidth_;
}

int SmartInfoHub::getRemoteHeight(){
    return remoteHeight_;
}

QString SmartInfoHub::getLocalVideoCodec(){
    return localVideoCodec_;
}

QString SmartInfoHub::getRemoteVideoCodec(){
    return remoteVideoCodec_;
}

QString SmartInfoHub::getRemoteAudioCodec(){
    return remoteAudioCodec_;
}
