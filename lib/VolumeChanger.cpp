#include "VolumeChanger.h"

#ifdef Q_OS_MACOS
#include "NrVolumeChangerMac.h"
#endif

#ifdef Q_OS_WIN
#include "NrVolumeChangerWin.h"
#endif

#ifdef Q_OS_LINUX
#include "NrVolumeChangerLinux.h"
#endif

NrVolumeChanger::NrVolumeChanger(QObject *p)
    : QObject(p)
{
    //empty ctor
}

/*!
 * \brief NrVolumeChanger::getInstance is a factory method that creates an instance of NrVolumeChanges for the appropriate platform
 * \return a pointer to the actual implementation on the correct platform. It might be nullptr if an error occurs.
 */
NrVolumeChanger* NrVolumeChanger::getInstance()
{
#ifdef Q_OS_MAC
    return new NrVolumeChangerMacImpl();
#elif defined( Q_OS_WIN )
    return new NrVolumeChangerWinImpl();
#elif defined( Q_OS_LINUX )
    return new NrVolumeChangerLinuxImpl();
#endif
    return nullptr;
}
