#import <Cocoa/Cocoa.h>
#include <QString>

namespace AppInfo {

bool foregroundApp(QString* appName, int* pid) {
    NSRunningApplication* app = [[NSWorkspace sharedWorkspace] frontmostApplication];
    if (!app) return false;
    if (appName) *appName = QString::fromNSString(app.localizedName);
    if (pid) *pid = (int)app.processIdentifier;
    return true;
}

}

