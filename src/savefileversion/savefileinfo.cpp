#include <Windows.h>
#include <iostream>
#include <fstream>
#include "FileInfo.h"
using namespace std;

#define SAVE_INFO 1

int main()
{
    OPENFILENAME ofn;
    char fNames[5 * MAX_PATH] = { 0 };       // buffer for file name
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFilter = "Execute Files (*.exe)\0*.exe\0";
    ofn.lpstrFile = fNames;
    ofn.lpstrTitle = "Select execute file";
    ofn.nMaxFile = MAX_PATH;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.lpstrInitialDir = "";
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileName(&ofn)) {
        DWORD size = GetFileVersionInfoSize(ofn.lpstrFile, NULL);
        if (size > 0) {
            uint8_t *lpVersionInfo = new uint8_t[size];
            if (GetFileVersionInfo(ofn.lpstrFile, NULL, size, lpVersionInfo))
            {
                UINT uLen;
                VS_FIXEDFILEINFO *lpFfi;

                if (VerQueryValue(lpVersionInfo, "\\", (LPVOID *)&lpFfi, &uLen)) {
                    file_version_t ver = {
                        (uint16_t)HIWORD(lpFfi->dwFileVersionMS),
                        (uint16_t)LOWORD(lpFfi->dwFileVersionMS),
                        (uint16_t)HIWORD(lpFfi->dwFileVersionLS),
                        (uint16_t)LOWORD(lpFfi->dwFileVersionLS)
                    };
                    
                    printf("Version: %d.%d.%d.%d\n", ver.major, ver.minor,
                        ver.revision, ver.build);

#if SAVE_INFO
                    ifstream exeFile(ofn.lpstrFile, ios::in);
                    exeFile.seekg(0, exeFile.end);
                    size_t exeFileSize = exeFile.tellg();
                    exeFile.seekg(0, exeFile.beg);
                    exeFile.close();

                    printf("Size: %d\n", exeFileSize);

                    ofn.lpstrTitle = "Select info file";
                    ofn.lpstrFilter = "Version Files (*.info)\0*.info\0";
#else
                    ofn.lpstrTitle = "Select version file";
                    ofn.lpstrFilter = "Version Files (*.ver)\0*.ver\0";
#endif
                    if (GetSaveFileName(&ofn)) {
                        ofstream file(ofn.lpstrFile, ios::out | ios::binary | ios::trunc);
#if SAVE_INFO
                        file_info_t info = {
                            ver,
                            exeFileSize
                        };
                        file.write((const char *)&info, sizeof(file_info_t));
#else
                        file.write((const char *)&lpFfi->dwFileVersionMS, 4);
                        file.write((const char *)&lpFfi->dwFileVersionLS, 4);
#endif
                        file.close();
                    }
                }
            }
            delete[] lpVersionInfo;
        }
    }
    return 0;
}