#include "AppInfo.h"

#if defined(CP_PLATFORM_WINDOWS)
#include <windows.h>
#include <psapi.h>
#endif

namespace AppInfo {

bool foregroundApp(QString* appName, int* pid) {
#if defined(CP_PLATFORM_WINDOWS)
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return false;
    DWORD processId = 0; GetWindowThreadProcessId(hwnd, &processId);
    if (pid) *pid = (int)processId;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        wchar_t buf[MAX_PATH]; DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, buf, &size)) {
            QString path = QString::fromWCharArray(buf);
            if (appName) *appName = path.section('/', -1).section('\\', -1);
            CloseHandle(hProcess);
            return true;
        }
        CloseHandle(hProcess);
    }
    return false;
#else
    // macOS and Linux/X11/Wayland: stub; macOS provided by platform/AppInfo_mac.mm
    Q_UNUSED(appName); Q_UNUSED(pid);
    return false;
#endif
}

}
