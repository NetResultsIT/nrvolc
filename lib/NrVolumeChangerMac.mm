#include "NrVolumeChangerMac.h"

#import <AppKit/NSSound.h>
#import <AudioToolbox/AudioServices.h>
#import <cstdlib>

#define DEVICE_NAME_MAX_SIZE 256

NrVolumeChangerMacImpl::NrVolumeChangerMacImpl() :
    NrVolumeChanger()
{
 // empty ctor
}


NrVolcErrorType NrVolumeChangerMacImpl::getDefaultInputDeviceId(int &devId) const
{
    devId = -1;
    AudioObjectPropertyAddress propertyDefaultInput = {
        kAudioHardwarePropertyDefaultInputDevice,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    //get DefaultAudioOutput Id
    UInt32 defaultOutputDeviceIDSize = sizeof(UInt32), defaultOutputDeviceID;
    OSStatus status = AudioObjectGetPropertyData(
                AudioObjectID(kAudioObjectSystemObject),
                &propertyDefaultInput,
                0,
                nil,
                &defaultOutputDeviceIDSize,
                &defaultOutputDeviceID);

    if (status != kAudioHardwareNoError) {
      return NRVOLC_ERROR;
    }
    //qDebug() << "getting default input device id ("  << defaultOutputDeviceID << ") " << (ret == 0 ? "OK" : "KO");
    devId = defaultOutputDeviceID;
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerMacImpl::getDefaultOutputDeviceId(int &devId) const
{
    devId = -1;
    AudioObjectPropertyAddress propertyDefaultOutput = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    //get DefaultAudioOutput Id
    UInt32 defaultOutputDeviceIDSize = sizeof(UInt32), defaultOutputDeviceID;
    OSStatus status = AudioObjectGetPropertyData(
                AudioObjectID(kAudioObjectSystemObject),
                &propertyDefaultOutput,
                0,
                nil,
                &defaultOutputDeviceIDSize,
                &defaultOutputDeviceID);

    if (status != kAudioHardwareNoError) {
      return NRVOLC_ERROR;
    }
    //qDebug() << "getting default output device id" << ret << defaultOutputDeviceID;
    devId = defaultOutputDeviceID;
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerMacImpl::setInputDeviceVolume(int devId, double percent)
{
    //qDebug() << "Setting volume for input device" << devId << " at " << percent;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };

    return setDeviceVolume(devId, &propertyAddress, percent);
}


NrVolcErrorType NrVolumeChangerMacImpl::setOutputDeviceVolume(int devId, double percent)
{
    //qDebug() << "Setting volume for output device" << devId << " at " << percent;//
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    return setDeviceVolume(devId, &propertyAddress, percent);
}


NrVolcErrorType NrVolumeChangerMacImpl::setDeviceVolume(int devId, AudioObjectPropertyAddress *propertyAddress, double percent)
{
    UInt32 defaultOutputDeviceID = devId;
    //Set DefaultAudioOutput volume
    Float32 outPropertyData = percent/100.0;
    OSStatus status = AudioHardwareServiceSetPropertyData(defaultOutputDeviceID,
                                        propertyAddress,
                                        0,
                                        NULL,
                                        sizeof(Float32),
                                        &outPropertyData);

    if (status != kAudioHardwareNoError) {
      return NRVOLC_ERROR;
    }
    //qDebug() << "set device property result" << ret << outPropertyData;
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerMacImpl::setDefaultInputVolume(double percent)
{
    int devId;

    if (getDefaultInputDeviceId(devId) != NRVOLC_NO_ERROR) {
      return NRVOLC_ERROR;
    }

    return setInputDeviceVolume(devId, percent);
}


NrVolcErrorType NrVolumeChangerMacImpl::setDefaultOutputVolume(double percent)
{
    int devId;

    if (getDefaultOutputDeviceId(devId) != NRVOLC_NO_ERROR) {
      return NRVOLC_ERROR;
    }

    return setOutputDeviceVolume(devId, percent);
}


NrVolcErrorType NrVolumeChangerMacImpl::getInputDeviceVolume(int devId, double &volume) const
{
    //qDebug() << "getting volume of in device" << devId;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };

    return getDeviceVolume(devId, &propertyAddress, volume);
}


NrVolcErrorType NrVolumeChangerMacImpl::getOutputDeviceVolume(int devId, double &volume) const
{
    //qDebug() << "getting volume of out device" << devId;
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    return getDeviceVolume(devId, &propertyAddress, volume);
}


NrVolcErrorType NrVolumeChangerMacImpl::getDeviceVolume(int devId, AudioObjectPropertyAddress *propertyAddress, double &volume) const
{
    //Get DefaultAudioOutput volume
    Float32 outPropertyData = 0.1;
    UInt32 outPropertyDataSize = sizeof(Float32);
    OSStatus status = AudioHardwareServiceGetPropertyData(devId,
                                        propertyAddress,
                                        0,
                                        NULL,
                                        &outPropertyDataSize,
                                        &outPropertyData);

    if (status != kAudioHardwareNoError) {
      return NRVOLC_ERROR;
    }

    //qDebug() << "volume for device" << devId << outPropertyData << "result: " << ret;// << outPropertyDataSize;
    volume = outPropertyData * 100;
    return NRVOLC_NO_ERROR;
}

NrVolcErrorType NrVolumeChangerMacImpl::getDefaultOutputVolume(double &volume) const
{
    int devid;

    NrVolcErrorType ret = getDefaultOutputDeviceId(devid);

    if (ret != NRVOLC_NO_ERROR) {
      return ret;
    }

    return getOutputDeviceVolume(devid, volume);
}

NrVolcErrorType NrVolumeChangerMacImpl::getDefaultInputVolume(double &volume) const
{
    int devid;
    NrVolcErrorType ret = getDefaultInputDeviceId(devid);

    if (ret != NRVOLC_NO_ERROR) {
      return ret;
    }

    return getInputDeviceVolume(devid, volume);
}


/*!
 * \internal
 * \brief getDevices returns the number of audio devices and populates the passed array with the list of audio devices
 * \param pDeviceArray (output variable) the pointer to a deviceArray that will be populated with the device list
 * \param deviceCount (output variable) number of devices found
 * \return the number of devices present on the system
 */
NrVolcErrorType getDevices(AudioDeviceID **pDeviceArray, UInt32 &deviceCount)
{
    UInt32 propertySize;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioHardwarePropertyDevices;
    pa.mScope = kAudioObjectPropertyScopeGlobal;
    pa.mElement = kAudioObjectPropertyElementMaster;

    *pDeviceArray = nullptr;
    deviceCount = 0;

    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &pa, 0, NULL, &propertySize);

    if (status != kAudioHardwareNoError) {
      return NRVOLC_ERROR;
    }

    *pDeviceArray = static_cast<AudioDeviceID*>(malloc(propertySize));

    status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, NULL, &propertySize, *pDeviceArray);

    if (status != kAudioHardwareNoError) {
      free(*pDeviceArray);
      *pDeviceArray = nullptr;
      return NRVOLC_ERROR;
    }

    deviceCount = (propertySize / sizeof(AudioDeviceID));

    return NRVOLC_NO_ERROR;
}


NrVolcErrorType getDeviceName(AudioDeviceID deviceID, UInt32 devNameMaxSize, char *deviceName)
{
    UInt32 propertySize = devNameMaxSize;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyDeviceName;
    pa.mScope = kAudioObjectPropertyScopeGlobal;
    pa.mElement = kAudioObjectPropertyElementMaster;

    OSStatus status = AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &propertySize, deviceName);

    if (status != kAudioHardwareNoError) {
      return NRVOLC_ERROR;
    }

    return NRVOLC_NO_ERROR;
}


NrVolcErrorType getDeviceUid(AudioDeviceID deviceID, std::string &devUid)
{
    UInt32 propertySize = 256;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyDeviceUID;
    pa.mScope = kAudioObjectPropertyScopeGlobal;
    pa.mElement = kAudioObjectPropertyElementMaster;

    CFStringRef deviceUIDString;
    //UInt32 propSize = sizeof(deviceUIDString);
    OSStatus status = AudioObjectGetPropertyData(
            deviceID,
            &pa,
            0,
            NULL,
            &propertySize,
            &deviceUIDString);

    if (status != kAudioHardwareNoError) {
      return NRVOLC_ERROR;
    }

    NrVolcErrorType ret = NRVOLC_NO_ERROR;
    CFIndex deviceUIDLength = CFStringGetLength(deviceUIDString) + 1;
    char *ASCIIDeviceUID = (char*) malloc(deviceUIDLength);
    if ( !ASCIIDeviceUID )
        return NRVOLC_ERROR;

    if (CFStringGetCString (
            deviceUIDString,
            ASCIIDeviceUID,
            deviceUIDLength,
            kCFStringEncodingASCII))
    {
        devUid = ASCIIDeviceUID;
    } else {
      ret = NRVOLC_ERROR;
    }

    free (ASCIIDeviceUID);
    return ret;
}


bool isAnOutputDevice(AudioDeviceID deviceID)
{
    UInt32 propertySize = 256;

    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyStreams;
    pa.mScope = kAudioDevicePropertyScopeOutput;
    pa.mElement = kAudioObjectPropertyElementMaster;

    // if there are any output streams, then it's an output
    OSStatus status = AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);

    if (status != kAudioHardwareNoError) {
      return false;
    }
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
    OSStatus status = AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);

    if (status != kAudioHardwareNoError) {
      return false;
    }

    if (propertySize > 0) return true;

    return false;
}



NrVolcErrorType NrVolumeChangerMacImpl::getDeviceList(std::map<std::string, std::string> &devices,
                                                      NRVOLC::DeviceType dt) const
{
    AudioDeviceID *dev_array = nullptr;
    UInt32 numberOfDevices = 0;
    devices.clear();

    NrVolcErrorType ret = getDevices(&dev_array, numberOfDevices);

    if (ret != NRVOLC_NO_ERROR) {
      return ret;
    }

    for (UInt32 i = 0; i < numberOfDevices; ++i) {
        AudioDeviceID did = dev_array[i];

        char devname[DEVICE_NAME_MAX_SIZE];
        if (getDeviceName(did, DEVICE_NAME_MAX_SIZE, devname) != NRVOLC_NO_ERROR) {
          //device not found, check next one
          continue;
        }

        std::string devUid;

        if (getDeviceUid(did, devUid) != NRVOLC_NO_ERROR) {
          continue;
        }

        if (dt == NRVOLC::INPUT_DEVICE && isAnInputDevice(did)) {
            //qDebug() << "input dev volume: " << getInputDeviceVolume(did);
            devices.insert({(devname), devUid});
        }
        else if (dt == NRVOLC::OUTPUT_DEVICE && isAnOutputDevice(did)) {
            //qDebug() << "output dev volume: " << getOutputDeviceVolume(dev_array[i]);
            devices.insert({(devname), devUid});
        } else if (dt == NRVOLC::ANY_DEVICE) {
            devices.insert({(devname), devUid});
        }
    }

    free (dev_array);
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType getDeviceList2(std::map<std::string, std::string> &devices, NRVOLC::DeviceType dt)
{
    AudioDeviceID *dev_array = nullptr;
    UInt32 numberOfDevices = 0;
    devices.clear();

    NrVolcErrorType ret = getDevices(&dev_array, numberOfDevices);

    if (ret != NRVOLC_NO_ERROR) {
      return ret;
    }

    for (int i = 0; i < numberOfDevices; ++i) {
        AudioDeviceID did = dev_array[i];
        std::string devUid;

        if (getDeviceUid(did, devUid) != NRVOLC_NO_ERROR) {
          continue;
        }

        if (dt == NRVOLC::INPUT_DEVICE && isAnInputDevice(did)) {
            //qDebug() << "input dev volume: " << getInputDeviceVolume(did);
            devices.insert({(devUid), std::to_string(did)});
        }
        else if (dt == NRVOLC::OUTPUT_DEVICE && isAnOutputDevice(did)) {
            //qDebug() << "output dev volume: " << getOutputDeviceVolume(dev_array[i]);
            devices.insert({(devUid), std::to_string(did)});
        } else if (dt == NRVOLC::ANY_DEVICE) {
            devices.insert({(devUid), std::to_string(did)});
        }
    }

    free (dev_array);
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerMacImpl::getDeviceID(const std::string &devuid, AudioDeviceID &devId) const
{
    std::map<std::string, std::string> list;
    NrVolcErrorType ret = getDeviceList2(list, NRVOLC::ANY_DEVICE);

    if (ret != NRVOLC_NO_ERROR) {
      return ret;
    }

    auto it = list.find(devuid);
    if (it == list.end()) {
      //element not found
      return NRVOLC_ERROR;
    }

    std::string uid = it->second;
    devId = std::stoi(uid);
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerMacImpl::getOutputDeviceVolume(const std::string &deviceUid, double &volume) const
{
    AudioDeviceID adid;
    NrVolcErrorType ret = getDeviceID(deviceUid, adid);

    if (ret != NRVOLC_NO_ERROR) {
      return ret;
    }

    return getOutputDeviceVolume(adid, volume);
}


NrVolcErrorType NrVolumeChangerMacImpl::getInputDeviceVolume(const std::string &deviceUid, double &volume) const
{
    AudioDeviceID adid;
    NrVolcErrorType ret = getDeviceID(deviceUid, adid);

    if (ret != NRVOLC_NO_ERROR) {
      return ret;
    }

    return getInputDeviceVolume(adid, volume);
}


NrVolcErrorType NrVolumeChangerMacImpl::setInputDeviceVolume(const std::string &deviceUid, double percent)
{
    AudioDeviceID adid;
    NrVolcErrorType ret = getDeviceID(deviceUid, adid);

    if (ret != NRVOLC_NO_ERROR) {
      return ret;
    }

    return setInputDeviceVolume(adid, percent);
}

NrVolcErrorType NrVolumeChangerMacImpl::setOutputDeviceVolume(const std::string &deviceUid, double percent)
{
    AudioDeviceID adid;
    NrVolcErrorType ret = getDeviceID(deviceUid, adid);

    if (ret != NRVOLC_NO_ERROR) {
      return ret;
    }

    return setOutputDeviceVolume(adid, percent);
}

