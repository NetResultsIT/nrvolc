#ifndef NRVOLC_ALSA_WRAPPER_H
#define NRVOLC_ALSA_WRAPPER_H

#include <VolumeChanger.h>

class AudioObjectPropertyAddress;

class NRVOLC_LIB_EXPORT NrVolumeChangerLinuxImpl : public NrVolumeChanger
{
    int getDefaultInputDeviceId() const;
    int getDefaultOutputDeviceId() const;

    int setInputDeviceVolume(int devId, double percent);
    double getInputDeviceVolume(int devId) const;
    int setOutputDeviceVolume(int devId, double percent);
    double getOutputDeviceVolume(int devId) const;
public:
    NrVolumeChangerLinuxImpl(QObject *parent=nullptr);
    virtual int setDefaultInputVolume(double percent);
    virtual double getDefaultInputVolume() const;
    virtual int setDefaultOutputVolume(double percent);
    virtual double getDefaultOutputVolume() const;
};

#endif
