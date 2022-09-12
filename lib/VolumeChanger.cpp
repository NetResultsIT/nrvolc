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
}

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
