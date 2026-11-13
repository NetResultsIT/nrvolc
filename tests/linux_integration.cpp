#include "NrVolumeChangerLinux.h"
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <string>

int main(int argc, char **argv) {
  NrVolumeChangerLinuxImpl v;
  double d = 1234;
  if (argc > 1 && std::string(argv[1]) == "offline") {
    auto start = std::chrono::steady_clock::now();
    assert(v.getDefaultOutputVolume(d) == NRVOLC_ERROR && d == 1234);
    assert(std::chrono::steady_clock::now() - start < std::chrono::seconds(4));
    std::map<std::string, std::string> unchanged{{"sentinel", "unchanged"}};
    assert(v.getDeviceList(unchanged, NRVOLC::ANY_DEVICE) == NRVOLC_ERROR);
    assert(unchanged.size() == 1 && unchanged["sentinel"] == "unchanged");
    assert(v.setDefaultOutputVolume(50) == NRVOLC_ERROR);
    assert(v.getOutputDeviceVolume("default", d) == NRVOLC_ERROR && d == 1234);
    assert(v.getInputDeviceVolume("pulse", d) == NRVOLC_ERROR && d == 1234);
    return 0;
  }
  if (argc > 1 && std::string(argv[1]) == "aliasbad") {
    assert(v.getOutputDeviceVolume("default", d) == NRVOLC_ERROR && d == 1234);
    assert(v.getOutputDeviceVolume("pulse", d) == NRVOLC_ERROR && d == 1234);
    return 0;
  }
  if (argc > 1 && std::string(argv[1]) == "reconnected") {
    assert(v.getDefaultOutputVolume(d) == NRVOLC_NO_ERROR);
    return 0;
  }
  if (argc > 1 && std::string(argv[1]) == "switched") {
    assert(v.getDefaultOutputVolume(d) == NRVOLC_NO_ERROR &&
           std::fabs(d - 125) < 0.1);
    assert(v.getDefaultInputVolume(d) == NRVOLC_NO_ERROR &&
           std::fabs(d - 65) < 0.1);
    d = 1234;
    assert(v.getOutputDeviceVolume("pulse:sink:nrvolc_a", d) == NRVOLC_ERROR &&
           d == 1234);
    return 0;
  }
  const std::string a = "pulse:sink:nrvolc_a", b = "pulse:sink:nrvolc_b",
                    src = "pulse:source:nrvolc_source";
  std::map<std::string, std::string> list;
  assert(v.getDeviceList(list, NRVOLC::ANY_DEVICE) == NRVOLC_NO_ERROR);
  bool foundA = false, foundB = false, foundSrc = false;
  for (auto &item : list) {
    foundA |= item.second == a;
    foundB |= item.second == b;
    foundSrc |= item.second == src;
    assert(item.second.find(".monitor") == std::string::npos);
  }
  assert(foundA && foundB && foundSrc);
  assert(v.getOutputDeviceVolume(a, d) == NRVOLC_NO_ERROR &&
         std::fabs(d - 80) < 0.1);
  assert(v.setOutputDeviceVolume(a, 40) == NRVOLC_NO_ERROR);
  assert(v.getOutputDeviceVolume(a, d) == NRVOLC_NO_ERROR &&
         std::fabs(d - 40) < 0.1);
  assert(v.getDefaultOutputVolume(d) == NRVOLC_NO_ERROR &&
         std::fabs(d - 40) < 0.1);
  assert(v.setInputDeviceVolume(src, 33) == NRVOLC_NO_ERROR);
  assert(v.getInputDeviceVolume(src, d) == NRVOLC_NO_ERROR &&
         std::fabs(d - 33) < 0.1);
  assert(v.setOutputDeviceVolume(a, -1) == NRVOLC_ERROR);
  assert(v.setOutputDeviceVolume(a, 101) == NRVOLC_ERROR);
  assert(v.setOutputDeviceVolume(a, std::numeric_limits<double>::quiet_NaN()) ==
         NRVOLC_ERROR);
  assert(v.setOutputDeviceVolume(a, std::numeric_limits<double>::infinity()) ==
         NRVOLC_ERROR);
  d = 1234;
  assert(v.getOutputDeviceVolume(src, d) == NRVOLC_ERROR && d == 1234);
  assert(v.getInputDeviceVolume(a, d) == NRVOLC_ERROR && d == 1234);
  assert(v.getOutputDeviceVolume("pulse:sink:missing", d) == NRVOLC_ERROR &&
         d == 1234);
  assert(v.getOutputDeviceVolume("default", d) == NRVOLC_NO_ERROR &&
         std::fabs(d - 40) < 0.1);
  assert(v.getOutputDeviceVolume("pulse", d) == NRVOLC_NO_ERROR &&
         std::fabs(d - 40) < 0.1);
  setenv("PULSE_SINK", "nrvolc_b", 1);
  d = 1234;
  assert(v.getOutputDeviceVolume("pulse", d) == NRVOLC_ERROR && d == 1234);
  unsetenv("PULSE_SINK");
  assert(v.getOutputDeviceVolume("spoof: device (hw:999,0)", d) ==
             NRVOLC_ERROR &&
         d == 1234);
  assert(v.getOutputDeviceVolume("spoof: device (hw:999999999999999999999,0)",
                                 d) == NRVOLC_ERROR &&
         d == 1234);
  assert(v.getOutputDeviceVolume("spoof: device (hw:+0,0)", d) ==
             NRVOLC_ERROR &&
         d == 1234);
  std::cout << "integration checks passed\n";
}
