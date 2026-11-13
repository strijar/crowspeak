#include "runtime/FirmwareReleaseCheck.h"

#include "runtime/FirmwareReleaseCheck.h"
namespace handheld {
ReleaseCheckResult checkFirmwareRelease(const char*,const char*,char* version,size_t capacity) {
 if(version && capacity) version[0]=0;
 return ReleaseCheckResult::Failed;
}
}
