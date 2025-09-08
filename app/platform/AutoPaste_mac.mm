#import <Cocoa/Cocoa.h>
#import <ApplicationServices/ApplicationServices.h>
#import <Carbon/Carbon.h> // kVK_Command

static bool ensure_accessibility_permission() {
    // Prompt the user once if not trusted
    NSDictionary* opts = @{ (__bridge NSString*)kAXTrustedCheckOptionPrompt: @YES };
    return AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)opts);
}

void platform_paste_mac() {
    if (!ensure_accessibility_permission()) return;

    CGEventSourceRef src = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
    if (!src) src = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    CGEventRef cmdDown = CGEventCreateKeyboardEvent(src, (CGKeyCode)kVK_Command, true);
    CGEventRef vDown   = CGEventCreateKeyboardEvent(src, (CGKeyCode)9, true);  // 'v'
    CGEventSetFlags(vDown, kCGEventFlagMaskCommand);
    CGEventRef vUp     = CGEventCreateKeyboardEvent(src, (CGKeyCode)9, false);
    CGEventRef cmdUp   = CGEventCreateKeyboardEvent(src, (CGKeyCode)kVK_Command, false);

    CGEventPost(kCGHIDEventTap, cmdDown);
    CGEventPost(kCGHIDEventTap, vDown);
    CGEventPost(kCGHIDEventTap, vUp);
    CGEventPost(kCGHIDEventTap, cmdUp);

    if (cmdDown) CFRelease(cmdDown);
    if (vDown) CFRelease(vDown);
    if (vUp) CFRelease(vUp);
    if (cmdUp) CFRelease(cmdUp);
    if (src) CFRelease(src);
}
