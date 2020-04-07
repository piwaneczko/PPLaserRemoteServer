#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "FileInfo.h"
using namespace std;
namespace fs = filesystem;

#define SAVE_INFO 1

int main(const int argc, char *argv[]) {
    OPENFILENAME ofn;
    char fNames[MAX_PATH] = {0};  // buffer for file name
    const auto givenPath = string(argc == 2 && fs::exists(argv[1]) ? argv[1] : "");
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFilter = "Execute Files (*.exe)\0*.exe\0";
    ofn.lpstrFile = fNames;
    ofn.lpstrTitle = "Select execute file";
    ofn.nMaxFile = MAX_PATH;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.lpstrInitialDir = "";
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (!givenPath.empty() || GetOpenFileName(&ofn)) {
        if (!givenPath.empty()) {
            strcpy_s(ofn.lpstrFile, MAX_PATH, givenPath.c_str());
        }
        const auto size = GetFileVersionInfoSize(ofn.lpstrFile, nullptr);
        if (size > 0) {
            uint8_t *lpVersionInfo = new uint8_t[size];
            if (GetFileVersionInfo(ofn.lpstrFile, NULL, size, lpVersionInfo)) {
                UINT uLen;
                VS_FIXEDFILEINFO *lpFfi;

                if (VerQueryValue(lpVersionInfo, "\\", reinterpret_cast<LPVOID *>(&lpFfi), &uLen)) {
                    file_version_t ver = {uint16_t(HIWORD(lpFfi->dwFileVersionMS)),
                                          uint16_t(LOWORD(lpFfi->dwFileVersionMS)),
                                          uint16_t(HIWORD(lpFfi->dwFileVersionLS)),
                                          uint16_t(LOWORD(lpFfi->dwFileVersionLS))};

                    printf("Version: %d.%d.%d.%d\n", ver.major, ver.minor, ver.revision, ver.build);

#if SAVE_INFO
                    ifstream exeFile(ofn.lpstrFile, ios::in);
                    exeFile.seekg(0, exeFile.end);
                    const auto exeFileSize = size_t(exeFile.tellg());
                    exeFile.seekg(0, exeFile.beg);
                    exeFile.close();

                    printf("Size: %d\n", int(exeFileSize));

                    ofn.lpstrTitle = "Select info file";
                    ofn.lpstrFilter = "Version Files (*.info)\0*.info\0";
#else
                    ofn.lpstrTitle = "Select version file";
                    ofn.lpstrFilter = "Version Files (*.ver)\0*.ver\0";
#endif
                    if (!givenPath.empty() || GetSaveFileName(&ofn)) {
                        auto filename = string(ofn.lpstrFile);
                        filename = filename.substr(0, filename.find(".exe"));
#if SAVE_INFO
                        filename += ".info";
#else
                        filename += ".ver";
#endif
                        ofstream file(filename, ios::out | ios::binary | ios::trunc);
#if SAVE_INFO
                        const file_info_t info = {ver, exeFileSize};
                        file.write(reinterpret_cast<const char *>(&info), sizeof(file_info_t));
#else
                        file.write(reinterpret_cast<const char *>(&lpFfi->dwFileVersionMS), 4);
                        file.write(reinterpret_cast<const char *>(&lpFfi->dwFileVersionLS), 4);
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
