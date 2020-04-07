#pragma once
// clang format off
#include <windows.h>
// clang format on
#include <powrprof.h>
#include <string>

class HibernateHandler {
    static void setPrivilege(HANDLE token,            // access token handle
                             std::wstring privilege,  // name of privilege to enable/disable
                             bool enablePrivilege     // to enable or disable privilege
    ) {
        TOKEN_PRIVILEGES tp;
        LUID luid;

        if (!LookupPrivilegeValue(nullptr,            // lookup privilege on local system
                                  privilege.c_str(),  // privilege to lookup
                                  &luid))             // receives LUID of privilege
        {
            throw std::exception("LookupPrivilegeValue error: " + GetLastError());
        }

        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        if (enablePrivilege)
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        else
            tp.Privileges[0].Attributes = 0;

        // Enable the privilege or disable all privileges.

        if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
            throw std::exception("AdjustTokenPrivileges error: " + GetLastError());
        }

        if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
            throw std::exception("The token does not have the specified privilege.");
        }
    }

public:
    static void hibernate() {
        HANDLE token;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
            setPrivilege(token, SE_SHUTDOWN_NAME, true);
            SetSuspendState(TRUE, TRUE, FALSE);
            CloseHandle(token);
        }
    }
};
