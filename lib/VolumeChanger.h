#ifndef NRVOLC_LIB_H
#define NRVOLC_LIB_H

//#include <QObject>
#include <string>
#include <map>
#include "NrVolcErrors.h"

namespace NRVOLC {
    enum DeviceType {
        INPUT_DEVICE    = 0,
        OUTPUT_DEVICE   = 1,
        ANY_DEVICE      = 2,
    };
}


#if !defined(WIN32) || defined(NRVOLC_STATIC)
// define NRVOLC_LIB_EXPORT to be "nothing"
#define NRVOLC_LIB_EXPORT
#else
 #ifdef NRVOLC_DLL
   #define NRVOLC_LIB_EXPORT __declspec(dllexport)
 #elif !defined(NRVOLC_STATIC)
   #define NRVOLC_LIB_EXPORT __declspec(dllimport)
 #endif
#endif



/*!
 * \brief The NrVolumeChanger class is an abstact class that defines the interface that should be implemented to change volume
 * on various platforms
 */
class NRVOLC_LIB_EXPORT NrVolumeChanger
{
    //Q_OBJECT
public:
    NrVolumeChanger();
    virtual NrVolcErrorType setDefaultInputVolume(double percent) = 0;
    virtual NrVolcErrorType getDefaultInputVolume(double &volume) const = 0;
    virtual NrVolcErrorType setDefaultOutputVolume(double percent) = 0;
    virtual NrVolcErrorType getDefaultOutputVolume(double &volume) const = 0;
    static NrVolumeChanger* getInstance();
#ifndef Q_OS_LINUX
    virtual NrVolcErrorType getDeviceList(std::map<std::string, std::string> &devices,
                                          NRVOLC::DeviceType=NRVOLC::ANY_DEVICE) const = 0;
    virtual NrVolcErrorType setInputDeviceVolume(const std::string &deviceUid, double percent) = 0;
    virtual NrVolcErrorType getInputDeviceVolume(const std::string &deviceUid, double &volume) const = 0;
    virtual NrVolcErrorType setOutputDeviceVolume(const std::string &deviceUid, double percent) = 0;
    virtual NrVolcErrorType getOutputDeviceVolume(const std::string &deviceUid, double &volume) const = 0;
#endif
};

#endif // LIB_H
