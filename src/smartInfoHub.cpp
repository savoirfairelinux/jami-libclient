#include "smartInfoHub.h"
#include <dbus/videomanager.h>
#include "dbus/callmanager.h"

SmartInfoHub::SmartInfoHub(){
}

void SmartInfoHub::smartInfo(){
  CallManager::instance().launchSmartInfo(500);
}
