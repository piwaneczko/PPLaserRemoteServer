#include "audiovolume.hpp"
#include "gui.hpp"

#define SAFE_RELEASE(punk) \
    if ((punk) != NULL) {  \
        (punk)->Release(); \
        (punk) = NULL;     \
    }

// Maximum volume level on trackbar
#define MAX_VOL 100

AudioVolumeCallback::AudioVolumeCallback(Gui &gui) : cRef_(1), gui_(gui), guid() {}

AudioVolumeCallback::~AudioVolumeCallback() {}

// IUnknown methods -- AddRef, Release, and QueryInterface

ULONG STDMETHODCALLTYPE AudioVolumeCallback::AddRef() {
    return InterlockedIncrement(&cRef_);
}

ULONG STDMETHODCALLTYPE AudioVolumeCallback::Release() {
    const ULONG ulRef = InterlockedDecrement(&cRef_);
    if (0 == ulRef) {
        delete this;
    }
    return ulRef;
}

HRESULT STDMETHODCALLTYPE AudioVolumeCallback::QueryInterface(REFIID riid, VOID **ppvInterface) {
    if (IID_IUnknown == riid) {
        AddRef();
        *ppvInterface = static_cast<IUnknown *>(this);
    } else if (__uuidof(IAudioEndpointVolumeCallback) == riid) {
        AddRef();
        *ppvInterface = static_cast<IAudioEndpointVolumeCallback *>(this);
    } else {
        *ppvInterface = nullptr;
        return E_NOINTERFACE;
    }
    return S_OK;
}

// Callback method for endpoint-volume-change notifications.

HRESULT STDMETHODCALLTYPE AudioVolumeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) {
    if (pNotify == nullptr) {
        return E_INVALIDARG;
    }
    if (pNotify->guidEventContext != guid) {
        gui_.volumeChanged(pNotify->bMuted ? 0.0f : MAX_VOL * pNotify->fMasterVolume);
    }
    return S_OK;
}

AudioVolume::AudioVolume(Gui *gui) : gui_(gui), callback_(*gui) {
    CoInitialize(nullptr);
    try {
        auto hr = CoCreateGuid(&callback_.guid);
        if (FAILED(hr)) throw std::exception("Guid creation failed!");

        // Get enumerator for audio endpoint devices.
        hr = CoCreateInstance(
            __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void **>(&enumerator));
        if (FAILED(hr)) throw std::exception("MMDeviceEnumerator instance creation failed!");

        // Get default audio-rendering device.
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
        if (FAILED(hr)) throw std::exception("GetDefaultAudioEndpoint failed!");

        hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, reinterpret_cast<void **>(&endptVol));
        if (FAILED(hr)) throw std::exception("Activate of IAudioEndpointVolume failed!");

        hr = endptVol->RegisterControlChangeNotify(static_cast<IAudioEndpointVolumeCallback *>(&callback_));
        if (FAILED(hr)) throw std::exception("RegisterControlChangeNotify failed!");

    } catch (const std::exception &e) {
        MessageBox(nullptr,
                   (L"While initializing audio device, following error occured: " + s2ws(e.what())).c_str(),
                   L"Volume access warning",
                   MB_OK | MB_ICONASTERISK);
    }
}

AudioVolume::~AudioVolume() {
    SAFE_RELEASE(enumerator)
    SAFE_RELEASE(device)
    SAFE_RELEASE(endptVol)
    CoUninitialize();
}

float AudioVolume::volume() const {
    float volume;
    BOOL mute;
    endptVol->GetMute(&mute);
    endptVol->GetMasterVolumeLevelScalar(&volume);
    return mute ? 0.0f : MAX_VOL * volume;
}
