#include "NrVolumeChangerLinux.h"

#include <QDebug>


NrVolumeChangerLinuxImpl::NrVolumeChangerLinuxImpl(QObject *parent)
    : NrVolumeChanger(parent)
{

}

int NrVolumeChangerLinuxImpl::setDefaultInputVolume(double percent)
{

    return 0;
}

int NrVolumeChangerLinuxImpl::setDefaultOutputVolume(double percent)
{


    return 0;
}



double NrVolumeChangerLinuxImpl::getDefaultInputVolume() const
{

    return 0;
}

double NrVolumeChangerLinuxImpl::getDefaultOutputVolume() const
{

    return 0;
}




