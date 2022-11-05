#ifndef NRVOLC_LIB_H
#define NRVOLC_LIB_H

#include <QObject>

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


class NRVOLC_LIB_EXPORT NrVolumeChanger : public QObject
{
    Q_OBJECT
public:
    NrVolumeChanger(QObject *parent=nullptr);
    virtual int setDefaultInputVolume(double percent) = 0;
    virtual double getDefaultInputVolume() const = 0;
    virtual int setDefaultOutputVolume(double percent) = 0;
    virtual double getDefaultOutputVolume() const = 0;
    static NrVolumeChanger* getInstance();
#ifndef Q_OS_LINUX
    virtual std::map<std::string, std::string> getDeviceList(NRVOLC::DeviceType=NRVOLC::ANY_DEVICE) const = 0;
    virtual double getOutputDeviceVolume(std::string deviceUid) const = 0;
    virtual double getInputDeviceVolume(std::string deviceUid) const = 0;
    virtual int setInputDeviceVolume(std::string deviceUid, double percent) = 0;
    virtual int setOutputDeviceVolume(std::string deviceUid, double percent) = 0;
#endif
};

#endif // LIB_H
