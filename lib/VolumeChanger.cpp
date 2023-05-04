#include "VolumeChanger.h"

#ifdef __APPLE__
#include "NrVolumeChangerMac.h"
#endif

#ifdef WIN32
#include "NrVolumeChangerWin.h"
#endif

#if defined(__linux__) || defined(__linux)
#include "NrVolumeChangerLinux.h"
#endif

NrVolumeChanger::NrVolumeChanger()
    //: QObject(p)
{
    //empty ctor
}

NrVolumeChanger* NrVolumeChanger::getInstance()
{
#ifdef __APPLE__
    return new NrVolumeChangerMacImpl();
#elif defined( WIN32 )
    return new NrVolumeChangerWinImpl();
#elif defined(__linux__) || defined(__linux)
    return new NrVolumeChangerLinuxImpl();
#endif
    return nullptr;
}
