#ifndef NRVOLC_COCOA_WRAPPER_H
#define NRVOLC_COCOA_WRAPPER_H

#include <VolumeChanger.h>

class NRVOLC_LIB_EXPORT NrVolumeChangerMacImpl : public NrVolumeChanger
{
    int getDefaultInputDeviceId() const;
    int getDefaultOutputDeviceId() const;
public:
    NrVolumeChangerMacImpl(QObject *parent=nullptr);
    virtual double getVolume() const;
    virtual int setVolume(double percent);
};

#endif
