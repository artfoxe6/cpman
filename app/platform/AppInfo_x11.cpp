#include "../core/AppInfo.h"

#include <cstdlib>
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

namespace AppInfo {

static bool getActiveWindow(Display* dpy, Window* outWin) {
    Atom prop = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    if (prop == None) return false;
    Atom actual; int format; unsigned long nitems, bytes;
    unsigned char* data = nullptr;
    int status = XGetWindowProperty(dpy, DefaultRootWindow(dpy), prop, 0, (~0L), False,
                                    AnyPropertyType, &actual, &format, &nitems, &bytes, &data);
    if (status != Success || !data) return false;
    if (nitems < 1) { XFree(data); return false; }
    Window w = *(Window*)data;
    XFree(data);
    *outWin = w;
    return w != 0;
}

static bool getPid(Display* dpy, Window w, int* pid) {
    Atom prop = XInternAtom(dpy, "_NET_WM_PID", True);
    if (prop == None) return false;
    Atom actual; int format; unsigned long nitems, bytes;
    unsigned char* data = nullptr;
    int status = XGetWindowProperty(dpy, w, prop, 0, 1, False, XA_CARDINAL, &actual, &format, &nitems, &bytes, &data);
    if (status != Success || !data) return false;
    if (nitems >= 1) {
        long p = *(long*)data;
        if (pid) *pid = int(p);
        XFree(data);
        return true;
    }
    XFree(data);
    return false;
}

static bool getClassOrName(Display* dpy, Window w, QString* name) {
    if (!name) return false;
    // Try WM_CLASS
    XClassHint ch; memset(&ch, 0, sizeof(ch));
    if (XGetClassHint(dpy, w, &ch)) {
        if (ch.res_class) {
            *name = QString::fromUtf8(ch.res_class);
            if (ch.res_name) XFree(ch.res_name);
            if (ch.res_class) XFree(ch.res_class);
            return true;
        }
        if (ch.res_name) XFree(ch.res_name);
        if (ch.res_class) XFree(ch.res_class);
    }
    // Try _NET_WM_NAME (UTF8)
    Atom utf8 = XInternAtom(dpy, "UTF8_STRING", False);
    Atom prop = XInternAtom(dpy, "_NET_WM_NAME", True);
    if (prop != None) {
        Atom actual; int format; unsigned long nitems, bytes;
        unsigned char* data = nullptr;
        int status = XGetWindowProperty(dpy, w, prop, 0, (~0L), False, utf8, &actual, &format, &nitems, &bytes, &data);
        if (status == Success && data) {
            *name = QString::fromUtf8(reinterpret_cast<char*>(data));
            XFree(data);
            return true;
        }
        if (data) XFree(data);
    }
    // Fallback WM_NAME (may be locale encoded)
    XTextProperty tp; if (XGetWMName(dpy, w, &tp) && tp.value) {
        *name = QString::fromLocal8Bit(reinterpret_cast<char*>(tp.value));
        if (tp.value) XFree(tp.value);
        return true;
    }
    return false;
}

bool foregroundApp(QString* appName, int* pid) {
    // Skip on Wayland sessions; use desktop portal if needed (not implemented here)
    const char* sess = std::getenv("XDG_SESSION_TYPE");
    if (sess && std::strcmp(sess, "wayland") == 0) return false;

    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return false;
    Window w = 0;
    bool ok = getActiveWindow(dpy, &w);
    if (ok && w != 0) {
        QString name;
        int p = 0;
        getClassOrName(dpy, w, &name);
        getPid(dpy, w, &p);
        if (appName) *appName = name;
        if (pid) *pid = p;
        XCloseDisplay(dpy);
        return true;
    }
    XCloseDisplay(dpy);
    return false;
}

}
