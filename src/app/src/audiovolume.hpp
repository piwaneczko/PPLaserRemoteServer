#pragma once
#include <windows.h>
// WIndows.h must be included as first
#include <commctrl.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

class Gui;

/**
 * Client implementation of IAudioEndpointVolumeCallback
 * interface. When a method in the IAudioEndpointVolume
 * interface changes the volume level or muting state of the
 * endpoint device, the change initiates a call to the
 * client's IAudioEndpointVolumeCallback::OnNotify method.
 */
class AudioVolumeCallback : public IAudioEndpointVolumeCallback {
    LONG cRef_;
    Gui &gui_;

    friend class AudioVolume;
    GUID guid;

public:
    explicit AudioVolumeCallback(Gui &gui);
    virtual ~AudioVolumeCallback();
    ULONG STDMETHODCALLTYPE AddRef() override;

    ULONG STDMETHODCALLTYPE Release() override;

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) override;
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;
};
class AudioVolume {
    Gui *gui_;
    AudioVolumeCallback callback_;
    IMMDeviceEnumerator *enumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioEndpointVolume *endptVol = nullptr;

public:
    explicit AudioVolume(Gui *gui);
    ~AudioVolume();

    float volume() const;
};
