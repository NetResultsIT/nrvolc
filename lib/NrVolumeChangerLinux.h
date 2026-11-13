#ifndef NRVOLC_LINUX_H
#define NRVOLC_LINUX_H
#include "VolumeChanger.h"
class NrVolumeChangerLinuxImpl : public NrVolumeChanger {
public:
  NrVolcErrorType setDefaultInputVolume(double) override;
  NrVolcErrorType getDefaultInputVolume(double &) const override;
  NrVolcErrorType setDefaultOutputVolume(double) override;
  NrVolcErrorType getDefaultOutputVolume(double &) const override;
  NrVolcErrorType getDeviceList(std::map<std::string, std::string> &,
                                NRVOLC::DeviceType) const override;
  NrVolcErrorType setInputDeviceVolume(const std::string &, double) override;
  NrVolcErrorType getInputDeviceVolume(const std::string &,
                                       double &) const override;
  NrVolcErrorType setOutputDeviceVolume(const std::string &, double) override;
  NrVolcErrorType getOutputDeviceVolume(const std::string &,
                                        double &) const override;
};
#endif
