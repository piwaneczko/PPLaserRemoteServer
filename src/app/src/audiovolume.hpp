#pragma once
#include <windows.h>
// WIndows.h must be included as first
#include <commctrl.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

class GUI;

/**
 * Client implementation of IAudioEndpointVolumeCallback
 * interface. When a method in the IAudioEndpointVolume
 * interface changes the volume level or muting state of the
 * endpoint device, the change initiates a call to the
 * client's IAudioEndpointVolumeCallback::OnNotify method.
 */
class AudioVolumeCallback : public IAudioEndpointVolumeCallback {
    LONG cRef_;
    GUI &gui_;

    friend class AudioVolume;
    GUID guid;

public:
    AudioVolumeCallback(GUI &gui);
    virtual ~AudioVolumeCallback();
    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface);
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
};
class AudioVolume {
    GUI *gui_;
    AudioVolumeCallback callback_;
    IMMDeviceEnumerator *enumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioEndpointVolume *endptVol = nullptr;

public:
    explicit AudioVolume(GUI *gui);
    ~AudioVolume();

    float volume() const;
};
