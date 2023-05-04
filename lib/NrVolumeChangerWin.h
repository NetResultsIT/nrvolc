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
    int setDefaultInputVolume(double percent);
    double getDefaultInputVolume() const;
    int setDefaultOutputVolume(double percent);
    double getDefaultOutputVolume() const;

    std::map<std::string, std::string> getDeviceList(NRVOLC::DeviceType devicetype=NRVOLC::ANY_DEVICE) const;
    double getInputDeviceVolume(std::string) const;
    double getOutputDeviceVolume(std::string) const;
    int setInputDeviceVolume(std::string deviceUid, double percent);
    int setOutputDeviceVolume(std::string deviceUid, double percent);
};

#endif
