#ifndef NRVOLC_WIN_IMPL_H
#define NRVOLC_WIN_IMPL_H

#include <VolumeChanger.h>

struct IMMDevice;
struct IAudioEndpointVolume;
struct IMMDeviceCollection;

class NRVOLC_LIB_EXPORT NrVolumeChangerWinImpl : public NrVolumeChanger
{
    IMMDevice* getDefaultInputDevice() const;
    IMMDevice* getDefaultOutputDevice() const;
    IMMDevice* getDeviceById(const std::string& uid) const;
    IMMDevice* getDeviceByName(const std::string& uid) const;
    IAudioEndpointVolume* getDeviceEndpointVolume(IMMDevice *defaultDevice) const;
    IMMDeviceCollection* listDevices(int) const;

    int setDeviceVolume(IMMDevice*, double percent);
    double getDeviceVolume(IMMDevice* devId) const;

public:
    NrVolumeChangerWinImpl();

    NrVolcErrorType setDefaultInputVolume(double percent) override;
    NrVolcErrorType getDefaultInputVolume(double &volume) const override;
    NrVolcErrorType setDefaultOutputVolume(double percent) override;
    NrVolcErrorType getDefaultOutputVolume(double &volume) const override;

    NrVolcErrorType getDeviceList(std::map<std::string, std::string> &devices,
                                  NRVOLC::DeviceType devicetype=NRVOLC::ANY_DEVICE) const override;
    NrVolcErrorType setInputDeviceVolume(const std::string &deviceUid, double percent) override;
    NrVolcErrorType getInputDeviceVolume(const std::string &devUid, double &volume) const override;
    NrVolcErrorType setOutputDeviceVolume(const std::string &deviceUid, double percent) override;
    NrVolcErrorType getOutputDeviceVolume(const std::string &devUid, double &volume) const override;
};

#endif
