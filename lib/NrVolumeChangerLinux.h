#ifndef NRVOLC_ALSA_WRAPPER_H
#define NRVOLC_ALSA_WRAPPER_H

#include <VolumeChanger.h>

class AudioObjectPropertyAddress;

class NRVOLC_LIB_EXPORT NrVolumeChangerLinuxImpl : public NrVolumeChanger
{
    int getDefaultInputDeviceId() const;
    int getDefaultOutputDeviceId() const;

    NrVolcErrorType setInputDeviceVolume(int devId, double percent);
    NrVolcErrorType getInputDeviceVolume(int devId, double &volume) const;
    NrVolcErrorType setOutputDeviceVolume(int devId, double percent);
    NrVolcErrorType getOutputDeviceVolume(int devId, double &volume) const;
public:
    NrVolumeChangerLinuxImpl(QObject *parent=nullptr);
    virtual NrVolcErrorType setDefaultInputVolume(double percent) override;
    virtual NrVolcErrorType getDefaultInputVolume(double &volume) const override;
    virtual NrVolcErrorType setDefaultOutputVolume(double percent) override;
    virtual NrVolcErrorType getDefaultOutputVolume(double &volume) const override;
};

#endif
