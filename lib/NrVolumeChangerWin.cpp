#include "NrVolumeChangerWin.h"

#include <QDebug>

NrVolumeChangerWinImpl::NrVolumeChangerWinImpl(QObject *parent)
    : NrVolumeChanger(parent)
{

}

int NrVolumeChangerWinImpl::setDefaultInputVolume(double percent)
{
    return 0;
}

int NrVolumeChangerWinImpl::setDefaultOutputVolume(double percent)
{

    return 0;
}


double NrVolumeChangerWinImpl::getDefaultInputVolume() const
{
    return 0;
}
double NrVolumeChangerWinImpl::getDefaultOutputVolume() const
{

    return 0;
}
