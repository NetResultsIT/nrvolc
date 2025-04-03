#ifndef NRVOLC_COCOA_WRAPPER_H
#define NRVOLC_COCOA_WRAPPER_H

#include "CoreAudio/AudioHardware.h"
#include "VolumeChanger.h"


class NRVOLC_LIB_EXPORT NrVolumeChangerMacImpl : public NrVolumeChanger
{
    NrVolcErrorType getDefaultInputDeviceId(int &devId) const;
    NrVolcErrorType getDefaultOutputDeviceId(int &devId) const;

    NrVolcErrorType setInputDeviceVolume(int devId, double percent);
    NrVolcErrorType getInputDeviceVolume(int devId, double &volume) const;
    NrVolcErrorType setOutputDeviceVolume(int devId, double percent);
    NrVolcErrorType getOutputDeviceVolume(int devId, double &volume) const;
    NrVolcErrorType getDeviceID(const std::string &uid, AudioDeviceID &devId) const;
    //TODO make these two templated
    NrVolcErrorType setDeviceVolume(int devId, AudioObjectPropertyAddress *propAddr, double propValue);
    NrVolcErrorType getDeviceVolume(int devId, AudioObjectPropertyAddress *propAddr, double &volume) const;

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
