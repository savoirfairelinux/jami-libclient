#include "smartInfoHub.h"
#include <dbus/videomanager.h>
#include "dbus/callmanager.h"
#include "./callmodel.h"

int SmartInfoHub::answer=0;

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

  return SmartInfoHub::answer;
}
