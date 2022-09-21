#ifndef NRVOLC_WIN_IMPL_H
#define NRVOLC_WIN_IMPL_H

#include <VolumeChanger.h>

struct IMMDevice;
struct IAudioEndpointVolume;

class NRVOLC_LIB_EXPORT NrVolumeChangerWinImpl : public NrVolumeChanger
{
    IMMDevice* getDefaultInputDeviceId() const;
    IMMDevice* getDefaultOutputDeviceId() const;
    IAudioEndpointVolume* getDeviceEndpointVolume(IMMDevice *defaultDevice) const;

    int setDeviceVolume(IMMDevice*, double percent);
    double getDeviceVolume(IMMDevice* devId) const;
public:
    NrVolumeChangerWinImpl(QObject *parent=nullptr);
    virtual int setDefaultInputVolume(double percent);
    virtual double getDefaultInputVolume() const;
    virtual int setDefaultOutputVolume(double percent);
    virtual double getDefaultOutputVolume() const;
};

#endif
