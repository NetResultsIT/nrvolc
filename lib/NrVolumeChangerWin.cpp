#include "NrVolumeChangerWin.h"

#include <stdint.h>
#include <iostream>

#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>

#include <locale>
#include <codecvt>
#include <string>

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> strconverter;

#define SAFE_RELEASE(punk)  \
do{ \
        if ((punk) != nullptr)  \
    { (punk)->Release(); (punk) = nullptr; } \
}while(false)

std::string wstring_to_string(std::wstring wstr)
{
    return strconverter.to_bytes(wstr);
}

std::wstring string_to_wstring(std::string str)
{
    return strconverter.from_bytes(str);
}


NrVolumeChangerWinImpl::NrVolumeChangerWinImpl()
    : NrVolumeChanger()
{

}

IMMDevice* NrVolumeChangerWinImpl::getDefaultInputDevice() const
{
    HRESULT hr;
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (hr != 0)
        return nullptr;

    IMMDevice *defaultDevice = nullptr;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &defaultDevice);
    SAFE_RELEASE(deviceEnumerator);
    if (hr != 0)
        return nullptr;

    return defaultDevice;
}


IMMDevice* NrVolumeChangerWinImpl::getDefaultOutputDevice() const
{
    HRESULT hr;
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (hr != 0)
        return nullptr;

    IMMDevice *defaultDevice = nullptr;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);

    SAFE_RELEASE(deviceEnumerator);
    if (hr != 0)
        return nullptr;

    return defaultDevice;
}


IMMDeviceCollection* NrVolumeChangerWinImpl::listDevices(int i_flowtype) const
{
    HRESULT hr;
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (hr != 0)
        return nullptr;

    EDataFlow edf = (EDataFlow)i_flowtype;

    IMMDeviceCollection *pCollection = nullptr;
    hr = deviceEnumerator->EnumAudioEndpoints(edf, DEVICE_STATE_ACTIVE, &pCollection);
    if (hr != S_OK) {
        std::cerr << "Error enumerating devices" << std::endl;
    }

    SAFE_RELEASE(deviceEnumerator);
    return pCollection;
}


NrVolcErrorType NrVolumeChangerWinImpl::getDeviceList(std::map<std::string, std::string> &devices,
                                                      NRVOLC::DeviceType devicetype) const
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

    if (pCollection == nullptr) {
        //No device found
        return NRVOLC_ERROR;
    }

    UINT count;
    IMMDevice *pEndpoint;
    IPropertyStore *pProps = nullptr;
    LPWSTR pwszID = nullptr;
    hr = pCollection->GetCount(&count);
        //EXIT_ON_ERROR(hr)

    if (count == 0)
    {
        //qDebug("No endpoints found.\n");
    }

    // Each loop prints the name of an endpoint device.
    for (ULONG i = 0; i < count; i++)
    {
        // Get pointer to endpoint number i.
        hr = pCollection->Item(i, &pEndpoint);
        if (hr != S_OK) {
            SAFE_RELEASE(pEndpoint);
            continue;
        }

        // Get the endpoint ID string.
        hr = pEndpoint->GetId(&pwszID);
        if (hr != S_OK) {
            SAFE_RELEASE(pEndpoint);
            continue;
        }

        hr = pEndpoint->OpenPropertyStore(
                          STGM_READ, &pProps);
        if (hr != S_OK) {
            SAFE_RELEASE(pEndpoint);
            SAFE_RELEASE(pProps);
            continue;
        }

        PROPVARIANT varName;
        // Initialize container for property value.
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(
                       PKEY_Device_FriendlyName, &varName);
        if (hr != S_OK) {
            SAFE_RELEASE(pEndpoint);
            SAFE_RELEASE(pProps);
            continue;
        }

        // Print endpoint friendly name and endpoint ID.
        std::wstring devName = varName.pwszVal;
        std::wstring devUid = pwszID;
        std::cerr << "Endpoint " << i << devName.c_str() << devUid.c_str() << std::endl;
        std::string narrowDevName = wstring_to_string(devName);
        std::string narrowDevUid = wstring_to_string(devUid);
        map.insert({narrowDevName, narrowDevUid});

        CoTaskMemFree(pwszID);
        pwszID = NULL;
        PropVariantClear(&varName);
        SAFE_RELEASE(pProps);
        SAFE_RELEASE(pEndpoint);
    }
    SAFE_RELEASE(pCollection);

    devices = map;
    return NRVOLC_NO_ERROR;
}


IMMDevice* NrVolumeChangerWinImpl::getDeviceById(const std::string &uid) const
{
    HRESULT hr;
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (hr != 0)
        return nullptr;

    IMMDeviceCollection *pCollection = nullptr;
    hr = deviceEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &pCollection);
    if (hr != S_OK) {
        std::cerr << "Error enumerating devices" << std::endl;
        return nullptr;
    }

    UINT count;
    IMMDevice *pEndpoint;
    IMMDevice *pDevice = nullptr;
    LPWSTR pwszID = nullptr;
    hr = pCollection->GetCount(&count);
        //EXIT_ON_ERROR(hr)

    if (count == 0)
    {
        std::cerr << "No endpoints found.\n";
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

        std::wstring devUid(pwszID);
        CoTaskMemFree(pwszID);
        pwszID = nullptr;

        // Print endpoint friendly name and endpoint ID.
        //qDebug() << "Endpoint " << devUid;

        //convert the string to wstring for comparison
        std::wstring wideStrUid = string_to_wstring(uid);
        if (devUid == wideStrUid) {
            //qDebug() << "We found the device we were looking for";
            pDevice = pEndpoint;
        } else {
            SAFE_RELEASE(pEndpoint);
        }
    }

    SAFE_RELEASE(deviceEnumerator);
    return pDevice;
}

IAudioEndpointVolume* NrVolumeChangerWinImpl::getDeviceEndpointVolume(IMMDevice *defaultDevice) const
{
    if (defaultDevice == nullptr) {
        return nullptr;
    }

    HRESULT hr;
    IAudioEndpointVolume *endpointVolume = nullptr;
    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
    return endpointVolume;
}


NrVolcErrorType NrVolumeChangerWinImpl::setDefaultInputVolume(double percent)
{
    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDefaultInputDevice();

    if (defaultDevice == nullptr) {
        //TODO: (trasformare errore in enumerato con codici di errore)
        return NRVOLC_ERROR;
    }

    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    SAFE_RELEASE(defaultDevice);
    // -------------------------

    if (endpointVolume == nullptr) {
        //TODO: (trasformare errore in enumerato con codici di errore)
        return NRVOLC_ERROR;
    }

    hr = endpointVolume->SetMasterVolumeLevelScalar((float)percent/100, NULL);
    SAFE_RELEASE(endpointVolume);

    if (hr != S_OK) {
        return NRVOLC_ERROR;
    }

    return NRVOLC_NO_ERROR;
}

NrVolcErrorType NrVolumeChangerWinImpl::setDefaultOutputVolume(double percent)
{

    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDefaultOutputDevice();

    if (defaultDevice == nullptr) {
        return NRVOLC_ERROR;
    }

    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    SAFE_RELEASE(defaultDevice);
    // -------------------------

    if (endpointVolume == nullptr) {
        return NRVOLC_ERROR;
    }

    hr = endpointVolume->SetMasterVolumeLevelScalar((float)percent/100, NULL);

    //Releasing object no more useful
    SAFE_RELEASE(endpointVolume);

    if (hr != S_OK) {
        return NRVOLC_ERROR;
    }

    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerWinImpl::setInputDeviceVolume(const std::string &deviceUid, double percent)
{
    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDeviceById(deviceUid);

    if (defaultDevice == nullptr) {
        //device not found
        return NRVOLC_ERROR;
    }

    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    SAFE_RELEASE(defaultDevice);

    if (endpointVolume == nullptr)
    {
        return NRVOLC_ERROR;
    }
    // -------------------------

    hr = endpointVolume->SetMasterVolumeLevelScalar((float)percent/100, NULL);

    //Releasing object no more useful
    SAFE_RELEASE(endpointVolume);
    return NRVOLC_NO_ERROR;
}

NrVolcErrorType NrVolumeChangerWinImpl::setOutputDeviceVolume(const std::string &deviceUid, double percent)
{
    return setInputDeviceVolume(deviceUid, percent);
}

NrVolcErrorType NrVolumeChangerWinImpl::getDefaultInputVolume(double &volume) const
{
    // -------------------------
    HRESULT hr;

    IMMDevice *defaultDevice = getDefaultInputDevice();

    if (defaultDevice == nullptr) {
        //device not found
        return NRVOLC_ERROR;
    }

    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    SAFE_RELEASE(defaultDevice);
    // -------------------------

    if (endpointVolume == nullptr) {
        return NRVOLC_ERROR;
    }
    float currentVolume = 0;
    hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    //printf("Current volume as a scalar is: %f\n", currentVolume);

    //Releasing object no more useful
    SAFE_RELEASE(endpointVolume);
    if (hr != S_OK) {
        return NRVOLC_ERROR;
    }
    volume = currentVolume * 100;
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerWinImpl::getDefaultOutputVolume(double &volume) const
{
    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDefaultOutputDevice();

    if (defaultDevice == nullptr) {
        //device not found
        return NRVOLC_ERROR;
    }

    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    SAFE_RELEASE(defaultDevice);
    // -------------------------

    if (endpointVolume == nullptr)
    {
        return NRVOLC_ERROR;
    }

    float currentVolume = 0;
    hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    //printf("Current volume as a scalar is: %f\n", currentVolume);

    //Releasing object no more useful
    SAFE_RELEASE(endpointVolume);

    if (hr != S_OK) {
        return NRVOLC_ERROR;
    }

    volume = currentVolume * 100;
    return NRVOLC_NO_ERROR;
}


NrVolcErrorType NrVolumeChangerWinImpl::getInputDeviceVolume(const std::string &devUid, double &volume) const
{
    return getOutputDeviceVolume(devUid, volume);
}

NrVolcErrorType NrVolumeChangerWinImpl::getOutputDeviceVolume(const std::string &devUid, double &volume) const
{
    // -------------------------
    HRESULT hr;
    IMMDevice *defaultDevice = getDeviceById(devUid);

    if (defaultDevice == nullptr) {
        //device not found
        return NRVOLC_ERROR;
    }

    IAudioEndpointVolume *endpointVolume = getDeviceEndpointVolume(defaultDevice);
    SAFE_RELEASE(defaultDevice);
    // -------------------------

    if (endpointVolume == nullptr)
    {
        return NRVOLC_ERROR;
    }

    float currentVolume = 0;
    hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    //printf("Current volume as a scalar is: %f\n", currentVolume);

    //Releasing object no more useful
    SAFE_RELEASE(endpointVolume);
    volume = currentVolume * 100;
    return NRVOLC_NO_ERROR;
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



