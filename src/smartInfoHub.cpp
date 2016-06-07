#include "smartInfoHub.h"
#include <dbus/videomanager.h>

SmartInfoHub::SmartInfoHub(){
}

SmartInfoHub::smartInfo(){
  VideoManager::instance().smartInfo(500);
}
