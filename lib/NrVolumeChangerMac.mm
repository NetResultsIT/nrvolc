#include "NrVolumeChangerMac.h"
#include "cocoaHelper.h"

#import <AppKit/NSSound.h>
#import <AudioToolbox/AudioServices.h>

#include <QDebug>
#include <QtMacExtras>



NrVolumeChangerMacImpl::NrVolumeChangerMacImpl(QObject *p) :
    NrVolumeChanger(p)
{

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
    qDebug() << "getting default input device id ("  << defaultOutputDeviceID << ") " << (ret == 0 ? "OK" : "KO");
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
    qDebug() << "getting default output device id" << ret << defaultOutputDeviceID;
    devid = defaultOutputDeviceID;
    return devid;
}


int NrVolumeChangerMacImpl::setInputDeviceVolume(int devId, double percent)
{
    qDebug() << "Setting volume for input device" << devId << " at " << percent;

    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };


    return setDeviceProperty(devId, &propertyAddress, percent);
}


int NrVolumeChangerMacImpl::setOutputDeviceVolume(int devId, double percent)
{
    qDebug() << "Setting volume for output device" << devId << " at " << percent;

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

    qDebug() << "set device property result" << ret << outPropertyData;
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
    qDebug() << "getting volume of in device" << devId;

    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        kAudioDevicePropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };


    return getDeviceProperty(devId, &propertyAddress);
}

double NrVolumeChangerMacImpl::getOutputDeviceVolume(int devId) const
{
    qDebug() << "getting volume of out device" << devId;

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

    qDebug() << "volume for device" << devId << outPropertyData << "result: " << ret;// << outPropertyDataSize;
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


UInt32 getNumberOfDevices(AudioDeviceID *dev_array)
{
    UInt32 propertySize;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioHardwarePropertyDevices;
    pa.mScope = kAudioObjectPropertyScopeGlobal;
    pa.mElement = kAudioObjectPropertyElementMaster;

    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &pa, 0, NULL, &propertySize);

    AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, NULL, &propertySize, dev_array);

    return (propertySize / sizeof(AudioDeviceID));
}



typedef enum {
    kAudioTypeUnknown = 0,
    kAudioTypeInput = 1,
    kAudioTypeOutput = 2,
    kAudioTypeSystemOutput = 3
} ASDeviceType;

typedef enum {
  kOutputFormatDefault = 0,
  kOutputFormatName = 1
} ASOutputFormat;

constexpr int DEVICE_NAME_LEN = 26;


void getDeviceName(AudioDeviceID deviceID, char *deviceName) {
    UInt32 propertySize = 256;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyDeviceName;
    pa.mScope = kAudioObjectPropertyScopeGlobal;
    pa.mElement = kAudioObjectPropertyElementMaster;

    AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &propertySize, deviceName);
}

void getDeviceVolume_delete(AudioDeviceID deviceID, float *vol_left, float *vol_right) {
    OSStatus err;
    UInt32 size;
    UInt32 channel[2];

    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyPreferredChannelsForStereo;
    pa.mScope = kAudioDevicePropertyScopeOutput;
    pa.mElement = kAudioObjectPropertyElementMaster;

    size = sizeof(channel);
    err = AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &size, &channel);
    if (err != noErr) {
        return;
    }

    *vol_right = -1.0;
    *vol_left = -1.0;
    pa.mSelector = kAudioDevicePropertyVolumeScalar;
    size = sizeof(float);

    pa.mElement = channel[0];
    err = AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &size, vol_left);

    pa.mElement = channel[1];
    err |= AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &size, vol_right);

    if (err != noErr) {
        return;
    }
}

void getDeviceTransportType(AudioDeviceID deviceID, AudioDevicePropertyID *transportType) {
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyTransportType;
    pa.mScope = kAudioObjectPropertyScopeGlobal;
    pa.mElement = kAudioObjectPropertyElementMaster;

    UInt32 size = sizeof(transportType);

    AudioObjectGetPropertyData(deviceID, &pa, 0, 0, &size, transportType);
}

char *deviceTypeName(ASDeviceType device_type) {
    switch (device_type) {
        case kAudioTypeInput:
            return "input";
        case kAudioTypeOutput:
            return "output";
        case kAudioTypeSystemOutput:
            return "system";
        default:
            return "unknown";
    }
}

void printProperties_delete(AudioDeviceID deviceID, ASDeviceType typeRequested, ASOutputFormat outputFormat) {
    char deviceName[DEVICE_NAME_LEN];
    float vol_left, vol_right;
    ASDeviceType device_type;
    UInt32 transportType;

    switch (typeRequested) {
        case kAudioTypeInput:
            //if (!isAnInputDevice(deviceID)) return;
            device_type = kAudioTypeInput;
            break;

        case kAudioTypeOutput:
            //if (!isAnOutputDevice(deviceID)) return;
            device_type = kAudioTypeOutput;
            break;

        case kAudioTypeSystemOutput:
            //device_type = getDeviceType(deviceID);
            if (device_type != kAudioTypeOutput) return;
            break;
        default:
            break;
    }

    getDeviceName(deviceID, deviceName);
    //getDeviceVolume(deviceID, &vol_left, &vol_right);
    getDeviceTransportType(deviceID, &transportType);

    switch (outputFormat) {
      case kOutputFormatName:
        printf("%s\n", deviceName);
        break;
      default:
        printf("outputformat: default\n");
        printf("[%3u] - %6s %-26s", (unsigned int) deviceID, deviceTypeName(device_type), deviceName);
        if (vol_left < -0.1 || vol_right < -0.1) {
          printf("\n");
        }
        else {
          printf(" :: [%.3f:%.3f]\n", vol_left, vol_right);
        }
/*
        if (transportType == kAudioDeviceTransportTypeAggregate) {
          AudioObjectID sub_device[32];
          UInt32 outSize = sizeof(sub_device);
          getAggregateDeviceSubDeviceList(deviceID, sub_device, &outSize);
          for (int j = 0; j < outSize / sizeof(AudioObjectID); j++) {
            printf("\t");
            printProperties(sub_device[j], device_type, outputFormat);
          }
        }*/
    }

}


bool isAnOutputDevice(AudioDeviceID deviceID) {
    UInt32 propertySize = 256;

    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyStreams;
    pa.mScope = kAudioDevicePropertyScopeOutput;
    pa.mElement = kAudioObjectPropertyElementMaster;

    // if there are any output streams, then it is an output
    AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);
    if (propertySize > 0) return true;

    return false;
}

bool isAnInputDevice(AudioDeviceID deviceID) {
    UInt32 propertySize = 256;

    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyStreams;
    pa.mScope = kAudioDevicePropertyScopeInput;
    pa.mElement = kAudioObjectPropertyElementMaster;

    // if there are any input streams, then it is an input
    AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);
    if (propertySize > 0) return kAudioTypeInput;

    return false;
}

std::map<std::string, std::string> NrVolumeChangerMacImpl::getDeviceList(NRVOLC::DeviceType dt) const
{
    std::map<std::string, std::string> list;

    //void showAllDevices(ASDeviceType typeRequested, ASOutputFormat outputFormat) {
        AudioDeviceID dev_array[64];
        int numberOfDevices = 0;

        numberOfDevices = getNumberOfDevices(dev_array);
        ASDeviceType typeRequested;
        ASOutputFormat outputFormat = kOutputFormatDefault;

        if (dt == NRVOLC::INPUT_DEVICE) {
            typeRequested = kAudioTypeInput;
            //outputFormat = kOutputFormatDefault;
            printf("Getting input device\n");
        } else {
            typeRequested = kAudioTypeOutput;
            //outputFormat = kOutputFormatDefault;
            printf("Getting output device\n");
        }

        for (int i = 0; i < numberOfDevices; ++i) {
            printf("Getting properties of audio device %d\n", dev_array[i]);
            //printProperties(dev_array[i], typeRequested, outputFormat);
            if (typeRequested == kAudioTypeInput && isAnInputDevice(dev_array[i]))
                qDebug() << "input dev volume: " << getInputDeviceVolume(dev_array[i]);
            else if (typeRequested == kAudioTypeOutput&& isAnOutputDevice(dev_array[i]))
                qDebug() << "output dev volume: " << getOutputDeviceVolume(dev_array[i]);
        }
    //}

    return list;
}



double NrVolumeChangerMacImpl::getOutputDeviceVolume(std::string deviceUid) const
{
    return -1;
}


double NrVolumeChangerMacImpl::getInputDeviceVolume(std::string deviceUid) const
{
    return -1;
}


int NrVolumeChangerMacImpl::setInputDeviceVolume(std::string deviceUid, double percent)
{
    return -1;
}

int NrVolumeChangerMacImpl::setOutputDeviceVolume(std::string deviceUid, double percent)
{
    return -1;
}

