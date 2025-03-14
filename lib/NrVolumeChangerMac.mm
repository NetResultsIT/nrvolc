#include "NrVolumeChangerMac.h"

#import <AppKit/NSSound.h>
#import <AudioToolbox/AudioServices.h>

//#include <QDebug>
//#include <QtMacExtras>


NrVolumeChangerMacImpl::NrVolumeChangerMacImpl() :
    NrVolumeChanger()
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


NrVolcErrorType NrVolumeChangerMacImpl::setInputDeviceVolume(int devId, double percent)
{
    //qDebug() << "Setting volume for input device" << devId << " at " << percent;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };

    return setDeviceProperty(devId, &propertyAddress, percent);
}


NrVolcErrorType NrVolumeChangerMacImpl::setOutputDeviceVolume(int devId, double percent)
{
    //qDebug() << "Setting volume for output device" << devId << " at " << percent;//
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    return setDeviceProperty(devId, &propertyAddress, percent);
}


NrVolcErrorType NrVolumeChangerMacImpl::setDeviceProperty(int devId, AudioObjectPropertyAddress *propertyAddress, double percent)
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
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerMacImpl::setDefaultInputVolume(double percent)
{
    int devId = getDefaultInputDeviceId();
    return setInputDeviceVolume(devId, percent);
}


NrVolcErrorType NrVolumeChangerMacImpl::setDefaultOutputVolume(double percent)
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

NrVolcErrorType NrVolumeChangerMacImpl::getDefaultOutputVolume(double &volume) const
{
    int devid = getDefaultOutputDeviceId();
    volume = getOutputDeviceVolume(devid);
    return NRVOLC_NO_ERROR;
}

NrVolcErrorType NrVolumeChangerMacImpl::getDefaultInputVolume(double &volume) const
{
    int devid = getDefaultInputDeviceId();
    volume = getInputDeviceVolume(devid);
    return NRVOLC_NO_ERROR;
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


std::string getDeviceUid(AudioDeviceID deviceID)
{
  std::string devUid;
    UInt32 propertySize = 256;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyDeviceUID;
    pa.mScope = kAudioObjectPropertyScopeGlobal;
    pa.mElement = kAudioObjectPropertyElementMaster;

    CFStringRef deviceUIDString;
    //UInt32 propSize = sizeof(deviceUIDString);
    AudioObjectGetPropertyData(
            deviceID,
            &pa,
            0,
            NULL,
            &propertySize,
            &deviceUIDString);


    CFIndex deviceUIDLength = CFStringGetLength(deviceUIDString) + 1;
    char *ASCIIDeviceUID = (char*) malloc(deviceUIDLength);
    if ( !ASCIIDeviceUID )
        return devUid;

    if (CFStringGetCString (
            deviceUIDString,
            ASCIIDeviceUID,
            deviceUIDLength,
            kCFStringEncodingASCII))
    {
        devUid = ASCIIDeviceUID;
    }

    //AudioObjectGetPropertyData(deviceID2, &pa, 0, NULL, &propertySize, deviceUid);
    return devUid;
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



NrVolcErrorType NrVolumeChangerMacImpl::getDeviceList(std::map<std::string, std::string> &devices,
                                                      NRVOLC::DeviceType dt) const
{
    std::map<std::string, std::string> list;

    AudioDeviceID dev_array[64];
    int numberOfDevices = 0;

    numberOfDevices = getNumberOfDevices(dev_array);

    for (int i = 0; i < numberOfDevices; ++i) {
        int did = dev_array[i];

        char devname[256];
        getDeviceName(did, devname);
        std::string devUid = getDeviceUid(did);

        if (dt == NRVOLC::INPUT_DEVICE && isAnInputDevice(did)) {
            //qDebug() << "input dev volume: " << getInputDeviceVolume(did);
            list.insert({(devname), devUid});
        }
        else if (dt == NRVOLC::OUTPUT_DEVICE && isAnOutputDevice(did)) {
            //qDebug() << "output dev volume: " << getOutputDeviceVolume(dev_array[i]);
            list.insert({(devname), devUid});
        } else if (dt == NRVOLC::ANY_DEVICE) {
            list.insert({(devname), devUid});
        }
    }

    devices = list;
    return NRVOLC_NO_ERROR;
}


std::map<std::string, std::string> getDeviceList2(NRVOLC::DeviceType dt)
{
    std::map<std::string, std::string> list;

    AudioDeviceID dev_array[64];
    int numberOfDevices = 0;

    numberOfDevices = getNumberOfDevices(dev_array);

    for (int i = 0; i < numberOfDevices; ++i) {
        int did = dev_array[i];

        std::string devUid = getDeviceUid(did);

        if (dt == NRVOLC::INPUT_DEVICE && isAnInputDevice(did)) {
            //qDebug() << "input dev volume: " << getInputDeviceVolume(did);
            list.insert({(devUid), std::to_string(did)});
        }
        else if (dt == NRVOLC::OUTPUT_DEVICE && isAnOutputDevice(did)) {
            //qDebug() << "output dev volume: " << getOutputDeviceVolume(dev_array[i]);
            list.insert({(devUid), std::to_string(did)});
        } else if (dt == NRVOLC::ANY_DEVICE) {
            list.insert({(devUid), std::to_string(did)});
        }
    }

    return list;
}


AudioDeviceID NrVolumeChangerMacImpl::getDeviceID(const std::string &devuid) const
{
    std::map<std::string, std::string> list = getDeviceList2(NRVOLC::ANY_DEVICE);
    std::string uid = list.at(devuid);
    int did = std::stoi(uid);
    return (AudioDeviceID)did;
}


NrVolcErrorType NrVolumeChangerMacImpl::getOutputDeviceVolume(const std::string &deviceUid, double &volume) const
{
    AudioDeviceID adid = getDeviceID(deviceUid);
    volume = getOutputDeviceVolume(adid);
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerMacImpl::getInputDeviceVolume(const std::string &deviceUid, double &volume) const
{
    AudioDeviceID adid = getDeviceID(deviceUid);
    volume = getInputDeviceVolume(adid);
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerMacImpl::setInputDeviceVolume(const std::string &deviceUid, double percent)
{
    AudioDeviceID devid = getDeviceID(deviceUid);
    return setInputDeviceVolume(devid, percent);
}

NrVolcErrorType NrVolumeChangerMacImpl::setOutputDeviceVolume(const std::string &deviceUid, double percent)
{
    AudioDeviceID devid = getDeviceID(deviceUid);
    return setOutputDeviceVolume(devid, percent);
}

