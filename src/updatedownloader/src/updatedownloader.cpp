/**
 * \brief       Source of the update downloader class
 * \file        XmlConfig.cpp
 * \author      Pawe³ Iwaneczko
 * \copyright   Copyright 2016 HTeam. All rights reserved.
 */

#include "UpdateDownloader.hpp"
#include <fstream>
#include <filesystem>

namespace fs = experimental::filesystem::v1;

HRESULT UpdateDownloader::QueryInterface(REFIID riid, void ** ppvObject)
{
    return bindStatusCallbackReturnCode;
}

ULONG UpdateDownloader::AddRef(void)
{
    return 0;
}

ULONG UpdateDownloader::Release(void)
{
    return 0;
}

HRESULT UpdateDownloader::OnStartBinding(DWORD dwReserved, IBinding * pib)
{
    return bindStatusCallbackReturnCode;
}

HRESULT UpdateDownloader::GetPriority(LONG * pnPriority)
{
    return bindStatusCallbackReturnCode;
}

HRESULT UpdateDownloader::OnLowResource(DWORD reserved)
{
    return bindStatusCallbackReturnCode;
}

HRESULT UpdateDownloader::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
{
    if (downloadFunc != nullptr && ulStatusCode == BINDSTATUS_DOWNLOADINGDATA) {
        downloadFunc(ulProgress / float(info.fileSize > 0 ? info.fileSize : 1), downloadFuncUserPtr);
    }
    return bindStatusCallbackReturnCode;
}

HRESULT UpdateDownloader::OnStopBinding(HRESULT hresult, LPCWSTR szError)
{
    return bindStatusCallbackReturnCode;
}

HRESULT UpdateDownloader::GetBindInfo(DWORD * grfBINDF, BINDINFO * pbindinfo)
{
    return bindStatusCallbackReturnCode;
}

HRESULT UpdateDownloader::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC * pformatetc, STGMEDIUM * pstgmed)
{
    return bindStatusCallbackReturnCode;
}

HRESULT UpdateDownloader::OnObjectAvailable(REFIID riid, IUnknown * punk)
{
    return bindStatusCallbackReturnCode;
}

UpdateDownloader::UpdateDownloader(wstring versionFileUrl, wstring fileUrl) :
    versionFileUrl(versionFileUrl), fileUrl(fileUrl), downloadFuncUserPtr(nullptr),
    downloadFunc(),
    bindStatusCallbackReturnCode(S_OK)
{
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define VERSION_FILE_NAME L"version.ver"
wstring UpdateDownloader::CheckVersion(uint8_t flags)
{
    wstring result;
    if (URLDownloadToFile(NULL, versionFileUrl.c_str(), VERSION_FILE_NAME, 0, NULL) == S_OK) {
        if (fs::exists(VERSION_FILE_NAME)) {
            ifstream file(VERSION_FILE_NAME, ios::in | ios::binary);
            file.seekg(0, file.end);
            if ((size_t)file.tellg() == sizeof(file_info_t)) {
                file.seekg(0, file.beg);
                file.read((char *)&info, sizeof(file_info_t));
                if (file.good()) {
                    file_version_t actual, server = info.version;
                    if (GetFileVersion(actual)) {
                        bool new_ver = false;
                        new_ver |= ((flags & UD_CHECK_MAJOR   ) != 0) && actual.major    < server.major;
                        new_ver |= ((flags & UD_CHECK_MINOR   ) != 0) && actual.minor    < server.minor;
                        new_ver |= ((flags & UD_CHECK_REVISION) != 0) && actual.revision < server.revision;
                        new_ver |= ((flags & UD_CHECK_BUILD   ) != 0) && actual.build    < server.build;
                        if (new_ver) {
                            result =
                                to_wstring(server.major) + L"." +
                                to_wstring(server.minor) + L"." +
                                to_wstring(server.revision) + L"." +
                                to_wstring(server.build);
                        }
                    }
                }
                file.close();
            }
            fs::remove(VERSION_FILE_NAME);
        }
    }
    return result;
}
wstring GetTempFilePath()
{
    wstring filePath;
    wchar_t temp[MAX_PATH] = { 0 };
    uint32_t len = GetTempPath(MAX_PATH, temp);
    if (len > 0) {
        wchar_t modulePath[MAX_PATH];
        GetModuleFileName((HINSTANCE)&__ImageBase, modulePath, MAX_PATH);
        filePath = modulePath;
        size_t slashPos = filePath.find_last_of(L'\\');
        if (slashPos != filePath.npos) {
            filePath = filePath.substr(slashPos + 1);
        }
        filePath = temp + filePath;
    }
    return filePath;
}
wstring GetOtherFilePath() {
    OPENFILENAME ofn;
    wchar_t fNames[5 * MAX_PATH] = { 0 };       // buffer for file name
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFilter = L"Execute Files (*.exe)\0*.exe\0";
    ofn.lpstrFile = fNames;
    ofn.lpstrTitle = L"Select setup file path";
    ofn.nMaxFile = MAX_PATH;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.lpstrInitialDir = L"";
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    wstring result = L"";
    if (GetSaveFileName(&ofn)) {
        result = ofn.lpstrFile;
        if (result.find(L".exe") == result.npos) {
            result += L".exe";
        }
    }
    return result;
}
int UpdateDownloader::DownloadAndUpdate(function<UpdateProgressFP> func, void * userPtr, bool temp)
{
    int result = E_ABORT;
    wstring filePath = temp ? GetTempFilePath() : GetOtherFilePath();
    if (!filePath.empty()) {
        if (info.fileSize == 0) CheckVersion(); 
        downloadFunc = func;
        downloadFuncUserPtr = userPtr;
        result = URLDownloadToFile(NULL, fileUrl.c_str(), filePath.c_str(), 0, this);
        if (result == S_OK && int(ShellExecute(NULL, L"open", filePath.c_str(), NULL, NULL, SW_SHOWNORMAL)) <= 32)
            result = E_ABORT;
        downloadFunc = nullptr;
        downloadFuncUserPtr = nullptr;
        info.fileSize = 0;
    }
    return result;
}
void UpdateDownloader::AbortDownload() {
    bindStatusCallbackReturnCode = E_ABORT;
}
bool UpdateDownloader::GetFileVersion(file_version_t & ver, wstring filePath)
{
    if (filePath.empty()) {
        wchar_t modulePath[MAX_PATH];
        GetModuleFileName((HINSTANCE)&__ImageBase, modulePath, MAX_PATH);
        filePath = modulePath;
    }
    bool result = false;
    DWORD size = GetFileVersionInfoSize(filePath.c_str(), NULL);
    if (size > 0) {
        uint8_t *lpVersionInfo = new uint8_t[size];
        if (GetFileVersionInfo(filePath.c_str(), NULL, size, lpVersionInfo))
        {
            UINT uLen;
            VS_FIXEDFILEINFO *lpFfi;

            if (VerQueryValue(lpVersionInfo, L"\\", (LPVOID *)&lpFfi, &uLen)) {
                ver.major = (uint32_t)HIWORD(lpFfi->dwFileVersionMS);
                ver.minor = (uint32_t)LOWORD(lpFfi->dwFileVersionMS);
                ver.revision = (uint32_t)HIWORD(lpFfi->dwFileVersionLS);
                ver.build = (uint32_t)LOWORD(lpFfi->dwFileVersionLS);
                result = true;
            }
        }
        delete[] lpVersionInfo;
    }
    return result;
}