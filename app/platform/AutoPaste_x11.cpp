#include <cstdlib>
#include <cstring>
#include <cstdio>

#if defined(CP_HAVE_XTEST)
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#endif

static bool is_wayland() {
    const char* s = std::getenv("XDG_SESSION_TYPE");
    if (s && std::strcmp(s, "wayland") == 0) return true;
    // Also check WAYLAND_DISPLAY
    return std::getenv("WAYLAND_DISPLAY") != nullptr;
}

static void paste_wayland_external() {
    // Try wtype first, then ydotool
    int ret = std::system("wtype -M ctrl v -m ctrl 2>/dev/null");
    if (ret != 0) {
        std::system("ydotool key 29:1 55:1 55:0 29:0 2>/dev/null"); // 29=CTRL_L, 55=V (may vary by layout)
    }
}

static void paste_x11_xtest() {
#if defined(CP_HAVE_XTEST)
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    // Press Ctrl
    KeyCode ctrl = XKeysymToKeycode(dpy, XStringToKeysym("Control_L"));
    KeyCode v = XKeysymToKeycode(dpy, XStringToKeysym("v"));
    if (!ctrl || !v) { XCloseDisplay(dpy); return; }
    XTestFakeKeyEvent(dpy, ctrl, True, CurrentTime);
    XTestFakeKeyEvent(dpy, v, True, CurrentTime);
    XTestFakeKeyEvent(dpy, v, False, CurrentTime);
    XTestFakeKeyEvent(dpy, ctrl, False, CurrentTime);
    XFlush(dpy);
    XCloseDisplay(dpy);
#endif
}

void platform_paste_x11_or_wayland() {
    if (is_wayland()) paste_wayland_external();
    else paste_x11_xtest();
}
