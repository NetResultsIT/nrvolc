#ifndef NRVOLC_COCOA_WRAPPER_H
#define NRVOLC_COCOA_WRAPPER_H

#include "CoreAudio/AudioHardware.h"
#include "VolumeChanger.h"


class NRVOLC_LIB_EXPORT NrVolumeChangerMacImpl : public NrVolumeChanger
{
    int getDefaultInputDeviceId() const;
    int getDefaultOutputDeviceId() const;

    NrVolcErrorType setInputDeviceVolume(int devId, double percent);
    double getInputDeviceVolume(int devId) const;
    NrVolcErrorType setOutputDeviceVolume(int devId, double percent);
    double getOutputDeviceVolume(int devId) const;
    AudioDeviceID getDeviceID(const std::string &uid) const;
    //TODO make these two templated
    NrVolcErrorType setDeviceProperty(int devId, AudioObjectPropertyAddress *propAddr, double propValue);
    double getDeviceProperty(int devId, AudioObjectPropertyAddress *propAddr) const;

public:
    NrVolumeChangerMacImpl();
    virtual NrVolcErrorType setDefaultInputVolume(double percent) override;
    virtual NrVolcErrorType getDefaultInputVolume(double &volume) const override;
    virtual NrVolcErrorType setDefaultOutputVolume(double percent) override;
    virtual NrVolcErrorType getDefaultOutputVolume(double &volume) const override;
    virtual NrVolcErrorType getDeviceList(std::map<std::string, std::string> &devices,
                          NRVOLC::DeviceType dt = NRVOLC::ANY_DEVICE) const override;
    virtual NrVolcErrorType setInputDeviceVolume(const std::string &deviceUid, double percent) override;
    virtual NrVolcErrorType getInputDeviceVolume(const std::string &deviceUid, double &volume) const override;
    virtual NrVolcErrorType setOutputDeviceVolume(const std::string &deviceUid, double percent) override;
    virtual NrVolcErrorType getOutputDeviceVolume(const std::string &deviceUid, double &volume) const override;
};

#endif
