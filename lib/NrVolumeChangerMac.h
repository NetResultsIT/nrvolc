#ifndef NRVOLC_COCOA_WRAPPER_H
#define NRVOLC_COCOA_WRAPPER_H

#include <VolumeChanger.h>

class AudioObjectPropertyAddress;

class NRVOLC_LIB_EXPORT NrVolumeChangerMacImpl : public NrVolumeChanger
{
    int getDefaultInputDeviceId() const;
    int getDefaultOutputDeviceId() const;

    int setInputDeviceVolume(int devId, double percent);
    double getInputDeviceVolume(int devId) const;
    int setOutputDeviceVolume(int devId, double percent);
    double getOutputDeviceVolume(int devId) const;
    //TODO make these two templated
    int setDeviceProperty(int devId, AudioObjectPropertyAddress *propAddr, double propValue);
    double getDeviceProperty(int devId, AudioObjectPropertyAddress *propAddr) const;
public:
    NrVolumeChangerMacImpl(QObject *parent=nullptr);
    virtual int setDefaultInputVolume(double percent);
    virtual double getDefaultInputVolume() const;
    virtual int setDefaultOutputVolume(double percent);
    virtual double getDefaultOutputVolume() const;
};

#endif
