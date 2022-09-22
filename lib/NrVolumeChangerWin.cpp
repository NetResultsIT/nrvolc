#include "NrVolumeChangerWin.h"

#include <QDebug>


#include <stdio.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>

NrVolumeChangerWinImpl::NrVolumeChangerWinImpl(QObject *parent)
    : NrVolumeChanger(parent)
{

}

IMMDevice* NrVolumeChangerWinImpl::getDefaultInputDevice() const
{
    HRESULT hr;
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (hr != 0)
        return nullptr;

    IMMDevice *defaultDevice = NULL;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &defaultDevice);
    deviceEnumerator->Release();
    deviceEnumerator = NULL;
    if (hr != 0)
        return nullptr;

    return defaultDevice;
}

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }


IMMDevice* NrVolumeChangerWinImpl::getDefaultOutputDevice() const
{
    HRESULT hr;
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (hr != 0)
        return nullptr;

    IMMDevice *defaultDevice = NULL;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);

    deviceEnumerator->Release();
    deviceEnumerator = NULL;
    if (hr != 0)
        return nullptr;

    return defaultDevice;
}


IMMDeviceCollection* NrVolumeChangerWinImpl::listDevices(int i_flowtype) const
{
    HRESULT hr;
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (hr != 0)
        return nullptr;

    EDataFlow edf = (EDataFlow)i_flowtype;

    IMMDeviceCollection *pCollection;
    hr = deviceEnumerator->EnumAudioEndpoints(edf, DEVICE_STATE_ACTIVE, &pCollection);
    if (hr != S_OK) {
        qDebug() << "Error enumerating devices";
    }
//    uint count;
//    IMMDevice *pEndpoint;
//    IPropertyStore *pProps = NULL;
//    LPWSTR pwszID = NULL;
//    hr = pCollection->GetCount(&count);
//        //EXIT_ON_ERROR(hr)

//    if (count == 0)
//    {
//        qDebug("No endpoints found.\n");
//    }

//    // Each loop prints the name of an endpoint device.
//    for (ULONG i = 0; i < count; i++)
//    {
//        // Get pointer to endpoint number i.
//        hr = pCollection->Item(i, &pEndpoint);
//        //EXIT_ON_ERROR(hr)

//        // Get the endpoint ID string.
//        hr = pEndpoint->GetId(&pwszID);
//        //EXIT_ON_ERROR(hr)

//        hr = pEndpoint->OpenPropertyStore(
//                          STGM_READ, &pProps);
//        //EXIT_ON_ERROR(hr)

//        PROPVARIANT varName;
//        // Initialize container for property value.
//        PropVariantInit(&varName);

//        // Get the endpoint's friendly-name property.
//        hr = pProps->GetValue(
//                       PKEY_Device_FriendlyName, &varName);
//        //EXIT_ON_ERROR(hr)

//        // Print endpoint friendly name and endpoint ID.
//        qDebug() << "Endpoint " <<
//               i << QString::fromWCharArray(varName.pwszVal) << QString::fromWCharArray(pwszID);

//        CoTaskMemFree(pwszID);
//        pwszID = NULL;
//        PropVariantClear(&varName);
//        SAFE_RELEASE(pProps)
//        SAFE_RELEASE(pEndpoint)
//    }

    deviceEnumerator->Release();
    deviceEnumerator = NULL;
    return pCollection;
}


std::map<std::string, std::string> NrVolumeChangerWinImpl::getDeviceList(NRVOLC::DeviceType devicetype) const
{
    std::map<std::string, std::string> map;
    HRESULT hr;
    EDataFlow edf;
    switch(devicetype) {
    case NRVOLC::ANY_DEVICE:
    edf = eAll;
    break;
    case NRVOLC::INPUT_DEVICE:
    edf = eCapture;
    break;
    case NRVOLC::OUTPUT_DEVICE:
    edf = eRender;
    break;
    }

    IMMDeviceCollection *pCollection = listDevices((int)edf);

    uint count;
    IMMDevice *pEndpoint;
    IPropertyStore *pProps = NULL;
    LPWSTR pwszID = NULL;
    hr = pCollection->GetCount(&count);
        //EXIT_ON_ERROR(hr)

    if (count == 0)
    {
        qDebug("No endpoints found.\n");
    }

    // Each loop prints the name of an endpoint device.
    for (ULONG i = 0; i < count; i++)
    {
        // Get pointer to endpoint number i.
        hr = pCollection->Item(i, &pEndpoint);
        //EXIT_ON_ERROR(hr)

        // Get the endpoint ID string.
        hr = pEndpoint->GetId(&pwszID);
        //EXIT_ON_ERROR(hr)

        hr = pEndpoint->OpenPropertyStore(
                          STGM_READ, &pProps);
        //EXIT_ON_ERROR(hr)

        PROPVARIANT varName;
        // Initialize container for property value.
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(
                       PKEY_Device_FriendlyName, &varName);
        //EXIT_ON_ERROR(hr)

        // Print endpoint friendly name and endpoint ID.
        QString devName = QString::fromWCharArray(varName.pwszVal);
        QString devUid = QString::fromWCharArray(pwszID);
        qDebug() << "Endpoint " << i << devName << devUid;
        map.insert({devName.toStdString(), devUid.toStdString()});

        CoTaskMemFree(pwszID);
        pwszID = NULL;
        PropVariantClear(&varName);
        SAFE_RELEASE(pProps)
        SAFE_RELEASE(pEndpoint)
    }
    SAFE_RELEASE(pCollection);

    return map;
}


IMMDevice* NrVolumeChangerWinImpl::getDeviceById(const QString &uid) const
{
    HRESULT hr;
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (hr != 0)
        return nullptr;

    IMMDeviceCollection *pCollection;
    hr = deviceEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &pCollection);
    if (hr != S_OK) {
        qDebug() << "Error enumerating devices";
    }
    uint count;
    IMMDevice *pEndpoint;
    IMMDevice *pDevice = nullptr;
    LPWSTR pwszID = NULL;
    hr = pCollection->GetCount(&count);
        //EXIT_ON_ERROR(hr)

    if (count == 0)
    {
        qDebug("No endpoints found.\n");
    }

    // Each loop prints the name of an endpoint device.
    for (ULONG i = 0; i < count; i++)
    {
        // Get pointer to endpoint number i.
        hr = pCollection->Item(i, &pEndpoint);
        //EXIT_ON_ERROR(hr)

        // Get the endpoint ID string.
        hr = pEndpoint->GetId(&pwszID);
        //EXIT_ON_ERROR(hr)

        QString devUid = QString::fromWCharArray(pwszID);
        CoTaskMemFree(pwszID);
        pwszID = NULL;

        // Print endpoint friendly name and endpoint ID.
        //qDebug() << "Endpoint " << devUid;

        if (devUid == uid) {
            //qDebug() << "We found the device we were looking for";
            pDevice = pEndpoint;
        } else {
            SAFE_RELEASE(pEndpoint)
        }
    }

    deviceEnumerator->Release();
    deviceEnumerator = NULL;
    return pDevice;
}

IAudioEndpointVolume* NrVolumeChangerWinImpl::getDeviceEndpointVolume(IMMDevice *defaultDevice) const
{
    HRESULT hr;
    IAudioEndpointVolume *endpointVolume = NULL;
    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
    defaultDevice->Release();
    defaultDevice = NULL;
    return endpointVolume;
}


int NrVolumeChangerWinImpl::setDefaultInputVolume(double percent)
{
    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDefaultInputDevice();
    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    // -------------------------
    hr = endpointVolume->SetMasterVolumeLevelScalar((float)percent/100, NULL);

    return 0;
}

int NrVolumeChangerWinImpl::setDefaultOutputVolume(double percent)
{

    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDefaultOutputDevice();
    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    // -------------------------
    hr = endpointVolume->SetMasterVolumeLevelScalar((float)percent/100, NULL);

    return 0;
}


int NrVolumeChangerWinImpl::setInputDeviceVolume(std::string deviceUid, double percent)
{
    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDeviceById(QString::fromStdString(deviceUid));
    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    // -------------------------
    hr = endpointVolume->SetMasterVolumeLevelScalar((float)percent/100, NULL);

    return 0;
}

int NrVolumeChangerWinImpl::setOutputDeviceVolume(std::string deviceUid, double percent)
{
    return setInputDeviceVolume(deviceUid, percent);
}

double NrVolumeChangerWinImpl::getDefaultInputVolume() const
{
    // -------------------------
    HRESULT hr;

    IMMDevice *defaultDevice = getDefaultInputDevice();
    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    // -------------------------
    float currentVolume = 0;
    hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    //printf("Current volume as a scalar is: %f\n", currentVolume);

    return currentVolume * 100;
}


double NrVolumeChangerWinImpl::getDefaultOutputVolume() const
{
    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDefaultOutputDevice();
    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    // -------------------------
    float currentVolume = 0;
    hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    //printf("Current volume as a scalar is: %f\n", currentVolume);

    return currentVolume * 100;
}


double NrVolumeChangerWinImpl::getInputDeviceVolume(std::string devUid) const
{
    return getOutputDeviceVolume(devUid);
}

double NrVolumeChangerWinImpl::getOutputDeviceVolume(std::string devUid) const
{
    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDeviceById(QString::fromStdString(devUid));
    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    // -------------------------
    float currentVolume = 0;
    hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    //printf("Current volume as a scalar is: %f\n", currentVolume);

    return currentVolume * 100;
}


/*
#include <stdio.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
void Usage()
{
    printf("Usage: \n");
    printf(" SetVolume [Reports the current volume]\n");
    printf(" SetVolume -d <new volume in decibels> [Sets the current default render device volume to the new volume]\n");
    printf(" SetVolume -f <new volume as an amplitude scalar> [Sets the current default render device volume to the new volume]\n");
}

int _tmain(int argc, _TCHAR* argv[]){
    HRESULT hr;
    bool decibels = false;
    bool scalar = false;
    double newVolume;
    if (argc != 3 && argc != 1)
    {    Usage();    return -1;  }
    if (argc == 3)
    {
        if (argv[1][0] == '-')
        {
            if (argv[1][1] == 'f')
            {         scalar = true;      }
            else if (argv[1][1] == 'd')
            {        decibels = true;      }
        }    else    {      Usage();      return -1;    }
        newVolume = _tstof(argv[2]);
    }
    // -------------------------
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    IMMDevice *defaultDevice = NULL;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    deviceEnumerator->Release();
    deviceEnumerator = NULL;
    IAudioEndpointVolume *endpointVolume = NULL;
    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
    defaultDevice->Release();
    defaultDevice = NULL;
    // -------------------------
    float currentVolume = 0;
    endpointVolume->GetMasterVolumeLevel(&currentVolume);
    printf("Current volume in dB is: %f\n", currentVolume);
    hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    printf("Current volume as a scalar is: %f\n", currentVolume);
    if (decibels)
    {    hr = endpointVolume->SetMasterVolumeLevel((float)newVolume, NULL);  }
    else if (scalar)  {    hr = endpointVolume->SetMasterVolumeLevelScalar((float)newVolume, NULL);  }
    endpointVolume->Release();
    CoUninitialize();
    return 0;
}
*/



