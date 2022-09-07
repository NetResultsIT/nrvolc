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
    qDebug() << "getting default input device id" << ret << defaultOutputDeviceID;
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

    qDebug() << "volume for device:" << devId << ret << outPropertyData << outPropertyDataSize;
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
