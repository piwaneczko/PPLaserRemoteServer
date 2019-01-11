/**
 * \brief       Header of the update downloader class.
 * \file        UpdateDownloader.hpp
 * \author      Pawe³ Iwaneczko
 * \copyright   Copyright 2016 HTeam. All rights reserved.
 */
#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <functional>
#include "FileInfo.h"

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "version.lib")

using namespace std;

#define UD_CHECK_MAJOR    0b0001u
#define UD_CHECK_MINOR    0b0010u
#define UD_CHECK_REVISION 0b0100u
#define UD_CHECK_BUILD    0b1000u
#define UD_CHECK_BASE     0b0011u
#define UD_CHECK_ALL      0b1111u

typedef void (UpdateProgressFP)(float progress, void *userPtr);

class UpdateDownloader : public IBindStatusCallback
{
private:
    wstring versionFileUrl, fileUrl;
    function<UpdateProgressFP> downloadFunc;
    file_info_t info;
    void *downloadFuncUserPtr;
    HRESULT bindStatusCallbackReturnCode;
private:
    /** IUnknown **/
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);
private:
    /** IBindCallback **/
    HRESULT STDMETHODCALLTYPE OnStartBinding(
        /* [in] */ DWORD dwReserved,
        /* [in] */ __RPC__in_opt IBinding *pib);

    HRESULT STDMETHODCALLTYPE GetPriority(
        /* [out] */ __RPC__out LONG *pnPriority);

    HRESULT STDMETHODCALLTYPE OnLowResource(
        /* [in] */ DWORD reserved);

    HRESULT STDMETHODCALLTYPE OnProgress(
        /* [in] */ ULONG ulProgress,
        /* [in] */ ULONG ulProgressMax,
        /* [in] */ ULONG ulStatusCode,
        /* [unique][in] */ __RPC__in_opt LPCWSTR szStatusText);

    HRESULT STDMETHODCALLTYPE OnStopBinding(
        /* [in] */ HRESULT hresult,
        /* [unique][in] */ __RPC__in_opt LPCWSTR szError);

    /* [local] */ HRESULT STDMETHODCALLTYPE GetBindInfo(
        /* [out] */ DWORD *grfBINDF,
        /* [unique][out][in] */ BINDINFO *pbindinfo);

    /* [local] */ HRESULT STDMETHODCALLTYPE OnDataAvailable(
        /* [in] */ DWORD grfBSCF,
        /* [in] */ DWORD dwSize,
        /* [in] */ FORMATETC *pformatetc,
        /* [in] */ STGMEDIUM *pstgmed);

    HRESULT STDMETHODCALLTYPE OnObjectAvailable(
        /* [in] */ __RPC__in REFIID riid,
        /* [iid_is][in] */ __RPC__in_opt IUnknown *punk);
public:
    /**
     * Constructor of update downloader
     * \param [in] versionFileUrl Url to download 16B file version
     * \param [in] serverFileUrl Url to installer file
     */
    UpdateDownloader(wstring versionFileUrl, wstring serverFileUrl);
    /**
     * Check the current version with server version (blocking)
     * \param[in] flags Flaga sprawdzania poziomów wersji
     * \return If success ? New version name : Empty string
     */
    wstring CheckVersion(uint8_t flags = UD_CHECK_ALL);
    /**
     * Download file from URL (blocking)
     * \param [in] func Progress update function callback
     * \param [in] userPtr User pointer
     * \param [in] temp    Download to temporary folder flag
     * return S_OK      - No error
     *        E_ABORT   - Aborted
     *        else      - Error code
     */
    int DownloadAndUpdate(function<UpdateProgressFP> func = nullptr, void *userPtr = nullptr, bool temp = true);
    /**
     * Abort files download
     */
    void AbortDownload();
public:
    /**
     * Get file version numer
     * \param [out] ver Version structure object
     * \param [in] filePath Path to the file. Default it is caller module
     */
    static bool GetFileVersion(file_version_t &ver, wstring filePath = L"");
};

