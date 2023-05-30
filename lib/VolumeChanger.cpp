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

/*!
 * \brief NrVolumeChanger::getInstance is a factory method that creates an instance of NrVolumeChanges for the appropriate platform
 * \return a pointer to the actual implementation on the correct platform. It might be nullptr if an error occurs.
 */
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
