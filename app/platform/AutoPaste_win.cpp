#include <windows.h>

void platform_paste_windows() {
    // Inject Ctrl+V
    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD; inputs[0].ki.wVk = VK_CONTROL; inputs[0].ki.dwFlags = 0;
    inputs[1].type = INPUT_KEYBOARD; inputs[1].ki.wVk = 'V'; inputs[1].ki.dwFlags = 0;
    inputs[2].type = INPUT_KEYBOARD; inputs[2].ki.wVk = 'V'; inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD; inputs[3].ki.wVk = VK_CONTROL; inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));
}

