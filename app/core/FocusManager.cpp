#include "FocusManager.h"

#if defined(CP_PLATFORM_WINDOWS)
#  include <windows.h>
#elif defined(CP_PLATFORM_MAC)
#  import <Cocoa/Cocoa.h>
#elif defined(CP_PLATFORM_LINUX)
#  include <X11/Xlib.h>
#  include <X11/Xatom.h>
#  include <X11/Xutil.h>
#  include <cstdlib>
#  include <cstring>
#  include <strings.h>
#endif

FocusManager::FocusManager(QObject* parent) : QObject(parent) {}

void FocusManager::rememberForeground() {
#if defined(CP_PLATFORM_WINDOWS)
    m_hwnd = reinterpret_cast<void*>(GetForegroundWindow());
#elif defined(CP_PLATFORM_MAC)
    NSRunningApplication* app = [[NSWorkspace sharedWorkspace] frontmostApplication];
    m_pid = app ? (qint64)app.processIdentifier : 0;
#elif defined(CP_PLATFORM_LINUX)
    // Only for X11 sessions; Wayland not supported here
    const char* sess = getenv("XDG_SESSION_TYPE");
    if (sess && strcasecmp(sess, "wayland") == 0) { m_win = 0; return; }
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) { m_win = 0; return; }
    Atom prop = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    if (prop == None) { XCloseDisplay(dpy); m_win = 0; return; }
    Atom actual; int format; unsigned long nitems, bytes;
    unsigned char* data = nullptr;
    if (XGetWindowProperty(dpy, DefaultRootWindow(dpy), prop, 0, (~0L), False,
                           AnyPropertyType, &actual, &format, &nitems, &bytes, &data) == Success && data && nitems >= 1) {
        m_win = *(Window*)data;
        XFree(data);
    } else {
        m_win = 0;
    }
    XCloseDisplay(dpy);
#else
    (void)0;
#endif
}

bool FocusManager::restoreForeground() {
#if defined(CP_PLATFORM_WINDOWS)
    HWND hwnd = reinterpret_cast<HWND>(m_hwnd);
    if (!hwnd) return false;
    // Best-effort foreground activation
    DWORD curTid = GetCurrentThreadId();
    DWORD tgtTid = GetWindowThreadProcessId(hwnd, nullptr);
    bool attached = AttachThreadInput(curTid, tgtTid, TRUE);
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    SetFocus(hwnd);
    if (attached) AttachThreadInput(curTid, tgtTid, FALSE);
    return true;
#elif defined(CP_PLATFORM_MAC)
    if (m_pid <= 0) return false;
    NSRunningApplication* app = [NSRunningApplication runningApplicationWithProcessIdentifier:(pid_t)m_pid];
    if (!app) return false;
    // Bring the previous app to front
    [app activateWithOptions:(NSApplicationActivateIgnoringOtherApps | NSApplicationActivateAllWindows)];
    return true;
#elif defined(CP_PLATFORM_LINUX)
    if (m_win == 0) return false;
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return false;
    Atom netActive = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    XEvent e; std::memset(&e, 0, sizeof(e));
    e.xclient.type = ClientMessage;
    e.xclient.message_type = netActive;
    e.xclient.display = dpy;
    e.xclient.window = (Window)m_win;
    e.xclient.format = 32;
    e.xclient.data.l[0] = 2; // 2 = pager, best effort
    e.xclient.data.l[1] = CurrentTime;
    e.xclient.data.l[2] = 0;
    Window root = DefaultRootWindow(dpy);
    long mask = SubstructureRedirectMask | SubstructureNotifyMask;
    XSendEvent(dpy, root, False, mask, &e);
    XFlush(dpy);
    XCloseDisplay(dpy);
    return true;
#else
    return false;
#endif
}
