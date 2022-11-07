#include "NrVolumeChangerMac.h"

#import <AppKit/NSSound.h>
#import <AudioToolbox/AudioServices.h>

//#include <QDebug>
//#include <QtMacExtras>


NrVolumeChangerMacImpl::NrVolumeChangerMacImpl(QObject *p) :
    NrVolumeChanger(p)
{
 // empty ctor
}


int NrVolumeChangerMacImpl::getDefaultInputDeviceId() const
{
    int devid = -1;
    AudioObjectPropertyAddress propertyDefaultInput = {
        kAudioHardwarePropertyDefaultInputDevice,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    //get DefaultAudioOutput Id
    UInt32 defaultOutputDeviceIDSize = sizeof(UInt32), defaultOutputDeviceID;
    OSStatus ret = AudioObjectGetPropertyData(
                AudioObjectID(kAudioObjectSystemObject),
                &propertyDefaultInput,
                0,
                nil,
                &defaultOutputDeviceIDSize,
                &defaultOutputDeviceID);
    //qDebug() << "getting default input device id ("  << defaultOutputDeviceID << ") " << (ret == 0 ? "OK" : "KO");
    devid = defaultOutputDeviceID;
    return devid;
}


int NrVolumeChangerMacImpl::getDefaultOutputDeviceId() const
{
    int devid = -1;
    AudioObjectPropertyAddress propertyDefaultOutput = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    //get DefaultAudioOutput Id
    UInt32 defaultOutputDeviceIDSize = sizeof(UInt32), defaultOutputDeviceID;
    OSStatus ret = AudioObjectGetPropertyData(
                AudioObjectID(kAudioObjectSystemObject),
                &propertyDefaultOutput,
                0,
                nil,
                &defaultOutputDeviceIDSize,
                &defaultOutputDeviceID);
    //qDebug() << "getting default output device id" << ret << defaultOutputDeviceID;
    devid = defaultOutputDeviceID;
    return devid;
}


int NrVolumeChangerMacImpl::setInputDeviceVolume(int devId, double percent)
{
    //qDebug() << "Setting volume for input device" << devId << " at " << percent;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };

    return setDeviceProperty(devId, &propertyAddress, percent);
}


int NrVolumeChangerMacImpl::setOutputDeviceVolume(int devId, double percent)
{
    //qDebug() << "Setting volume for output device" << devId << " at " << percent;//
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    return setDeviceProperty(devId, &propertyAddress, percent);
}


int NrVolumeChangerMacImpl::setDeviceProperty(int devId, AudioObjectPropertyAddress *propertyAddress, double percent)
{
    UInt32 defaultOutputDeviceID = devId;
    //Set DefaultAudioOutput volume
    Float32 outPropertyData = percent/100.0;
    OSStatus ret = AudioHardwareServiceSetPropertyData(defaultOutputDeviceID,
                                        propertyAddress,
                                        0,
                                        NULL,
                                        sizeof(Float32),
                                        &outPropertyData);

    //qDebug() << "set device property result" << ret << outPropertyData;
    return 0;
}


int NrVolumeChangerMacImpl::setDefaultInputVolume(double percent)
{
    int devId = getDefaultInputDeviceId();
    return setInputDeviceVolume(devId, percent);
}


int NrVolumeChangerMacImpl::setDefaultOutputVolume(double percent)
{
    int devId = getDefaultOutputDeviceId();
    return setOutputDeviceVolume(devId, percent);
}


double NrVolumeChangerMacImpl::getInputDeviceVolume(int devId) const
{
    //qDebug() << "getting volume of in device" << devId;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };

    return getDeviceProperty(devId, &propertyAddress);
}


double NrVolumeChangerMacImpl::getOutputDeviceVolume(int devId) const
{
    //qDebug() << "getting volume of out device" << devId;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    return getDeviceProperty(devId, &propertyAddress);
}


double NrVolumeChangerMacImpl::getDeviceProperty(int devId, AudioObjectPropertyAddress *propertyAddress) const
{
    //Get DefaultAudioOutput volume
    Float32 outPropertyData = 0.1;
    UInt32 outPropertyDataSize = sizeof(Float32);
    OSStatus ret = AudioHardwareServiceGetPropertyData(devId,
                                        propertyAddress,
                                        0,
                                        NULL,
                                        &outPropertyDataSize,
                                        &outPropertyData);

    //qDebug() << "volume for device" << devId << outPropertyData << "result: " << ret;// << outPropertyDataSize;
    return outPropertyData * 100;
}

double NrVolumeChangerMacImpl::getDefaultOutputVolume() const
{
    int devid = getDefaultOutputDeviceId();
    return getOutputDeviceVolume(devid);
}

double NrVolumeChangerMacImpl::getDefaultInputVolume() const
{
    int devid = getDefaultInputDeviceId();
    return getInputDeviceVolume(devid);
}


/*!
 * \internal
 * \brief getNumberOfDevices returns the number of audio devices and populates the passed array with the list of audio devices
 * \param o_pDeviceArray (output variable) the pointer to a deviceArray that will be populated with the device list
 * \return the number of devices present on the system
 */
UInt32 getNumberOfDevices(AudioDeviceID *o_pDeviceArray)
{
    UInt32 propertySize;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioHardwarePropertyDevices;
    pa.mScope = kAudioObjectPropertyScopeGlobal;
    pa.mElement = kAudioObjectPropertyElementMaster;

    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &pa, 0, NULL, &propertySize);

    AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, NULL, &propertySize, o_pDeviceArray);

    return (propertySize / sizeof(AudioDeviceID));
}


void getDeviceName(AudioDeviceID deviceID, char *deviceName)
{
    UInt32 propertySize = 256;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyDeviceName;
    pa.mScope = kAudioObjectPropertyScopeGlobal;
    pa.mElement = kAudioObjectPropertyElementMaster;

    AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &propertySize, deviceName);
}


bool isAnOutputDevice(AudioDeviceID deviceID)
{
    UInt32 propertySize = 256;

    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyStreams;
    pa.mScope = kAudioDevicePropertyScopeOutput;
    pa.mElement = kAudioObjectPropertyElementMaster;

    // if there are any output streams, then it's an output
    AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);
    if (propertySize > 0) return true;

    return false;
}

bool isAnInputDevice(AudioDeviceID deviceID)
{
    UInt32 propertySize = 256;

    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyStreams;
    pa.mScope = kAudioDevicePropertyScopeInput;
    pa.mElement = kAudioObjectPropertyElementMaster;

    // if there are any input streams, then it's an input
    AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);
    if (propertySize > 0) return true;

    return false;
}


std::map<std::string, std::string> NrVolumeChangerMacImpl::getDeviceList(NRVOLC::DeviceType dt) const
{
    std::map<std::string, std::string> list;

    AudioDeviceID dev_array[64];
    int numberOfDevices = 0;

    numberOfDevices = getNumberOfDevices(dev_array);

    for (int i = 0; i < numberOfDevices; ++i) {
        int did = dev_array[i];

        char devname[256];
        getDeviceName(did, devname);

        if (dt == NRVOLC::INPUT_DEVICE && isAnInputDevice(did)) {
            //qDebug() << "input dev volume: " << getInputDeviceVolume(did);
            list.insert({(devname), std::to_string(did)});
        }
        else if (dt == NRVOLC::OUTPUT_DEVICE && isAnOutputDevice(did)) {
            //qDebug() << "output dev volume: " << getOutputDeviceVolume(dev_array[i]);
            list.insert({(devname), std::to_string(did)});
        }
    }

    return list;
}



double NrVolumeChangerMacImpl::getOutputDeviceVolume(std::string deviceUid) const
{
    return getOutputDeviceVolume(stoi(deviceUid));
}


double NrVolumeChangerMacImpl::getInputDeviceVolume(std::string deviceUid) const
{
    return getInputDeviceVolume(stoi(deviceUid));
}


int NrVolumeChangerMacImpl::setInputDeviceVolume(std::string deviceUid, double percent)
{
    return setInputDeviceVolume(stoi(deviceUid), percent);
}

int NrVolumeChangerMacImpl::setOutputDeviceVolume(std::string deviceUid, double percent)
{
    return setOutputDeviceVolume(stoi(deviceUid), percent);
}

