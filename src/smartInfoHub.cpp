#include "smartInfoHub.h"
#include <dbus/videomanager.h>
#include "dbus/callmanager.h"
#include "./callmodel.h"

int SmartInfoHub::answer=0;
QString SmartInfoHub::callID;

SmartInfoHub::SmartInfoHub(){
}

void SmartInfoHub::smartInfo(int tRefreshMs){

  qDebug() << "Enter in smartInfoHub";
  CallManager::instance().getConferenceList();
  CallManager::instance().launchSmartInfo(tRefreshMs);
}

void SmartInfoHub::setAnswer(int pAnswer){
  SmartInfoHub::answer = pAnswer;
}

int SmartInfoHub::getInformation(){

  qDebug() << "CallID: " << SmartInfoHub::callID;
  return SmartInfoHub::answer;
}

void SmartInfoHub::setCallID(QString pCallID){
  callID = pCallID;
}
