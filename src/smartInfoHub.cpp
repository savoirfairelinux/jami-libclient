#include "smartInfoHub.h"
#include <dbus/videomanager.h>
#include "dbus/callmanager.h"

SmartInfoHub::SmartInfoHub(){
}

void SmartInfoHub::smartInfo(){
  qDebug() << "Enter in smartInfoHub";
  CallManager::instance().launchSmartInfo(500);
  CallManager::instance().getConferenceList();
}
