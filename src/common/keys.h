// The keyboard's keys: the controller inputs the keyboard presses, each with two keys. keyboard.cpp reads them, the
// launcher's Controls page sets them and the settings menu lists them. A key is a Windows virtual key code, which the
// Linux build numbers the same (the SDK's runtime/linux/win32.cpp), 0 none. The settings file keeps one changed from
// its defaults as key_<name> = [first, second]; one not there has its defaults.
//
// The VK_ names come from windows.h (the Linux build's own): include it first.
#pragma once

#include <cstdint>
#include <string_view>

namespace cv {

enum KeyInput : int {
    kKeyMoveUp, kKeyMoveDown, kKeyMoveLeft, kKeyMoveRight,          // the left stick
    kKeyCameraUp, kKeyCameraDown, kKeyCameraLeft, kKeyCameraRight,  // the right stick
    kKeyA, kKeyX, kKeyY, kKeyB, kKeyRT, kKeyLT, kKeyWhite, kKeyBlack,
    kKeyStart, kKeyBack,
    kKeyDpadUp, kKeyDpadDown, kKeyDpadLeft, kKeyDpadRight,
    kKeySprint,   // L3's sprint (keyboard.cpp)
    kKeySpeedUp,  // R3's speed-up
    kKeyInputCount
};

struct KeyInputInfo {
    const char* name;   // the settings file's key_<name>
    // in English (the launcher's words.h has it in its languages): a button by what the game's Controller Config
    // (Options) has it do at first, which that menu can change; the key stays with the button
    const char* label;
    uint16_t keys[2];  // the defaults
};

inline constexpr KeyInputInfo kKeyInputs[kKeyInputCount] = {
    {"move_up", "Move up", {'W', 0}},
    {"move_down", "Move down", {'S', 0}},
    {"move_left", "Move left", {'A', 0}},
    {"move_right", "Move right", {'D', 0}},
    {"camera_up", "Camera up", {VK_UP, 0}},
    {"camera_down", "Camera down", {VK_DOWN, 0}},
    {"camera_left", "Camera left", {VK_LEFT, 0}},
    {"camera_right", "Camera right", {VK_RIGHT, 0}},
    {"a", "Jump (A)", {'K', VK_SPACE}},
    {"x", "Attack (X)", {'J', 0}},
    {"y", "Ability (Y)", {'I', 0}},
    {"b", "Final attack / Confirm (B)", {'L', 0}},
    {"rt", "Guard (RT)", {'E', 0}},
    {"lt", "Camera reset (LT)", {'Q', 0}},
    {"white", "Change lock-on (LB)", {'Z', 0}},
    {"black", "Lock-on on / off (RB)", {'C', 0}},
    {"start", "Start", {VK_RETURN, 0}},
    {"back", "Back", {VK_BACK, 0}},
    {"dpad_up", "D-pad up", {VK_NUMPAD8, 0}},
    {"dpad_down", "D-pad down", {VK_NUMPAD2, 0}},
    {"dpad_left", "D-pad left", {VK_NUMPAD4, 0}},
    {"dpad_right", "D-pad right", {VK_NUMPAD6, 0}},
    {"sprint", "Sprint (L3)", {VK_LSHIFT, 0}},
    {"speed_up", "Speed-up (R3)", {VK_TAB, 0}},
};

// the order the launcher and the settings menu list them in: the sticks and the d-pad, then the buttons
inline constexpr KeyInput kKeyColumns[2][kKeyInputCount / 2] = {
    {kKeyMoveUp, kKeyMoveDown, kKeyMoveLeft, kKeyMoveRight, kKeyCameraUp, kKeyCameraDown, kKeyCameraLeft,
     kKeyCameraRight, kKeyDpadUp, kKeyDpadDown, kKeyDpadLeft, kKeyDpadRight},
    {kKeyA, kKeyX, kKeyY, kKeyB, kKeyRT, kKeyLT, kKeyWhite, kKeyBlack, kKeyStart, kKeyBack, kKeySprint, kKeySpeedUp},
};

// the input of a settings key, "key_a" kKeyA; -1 for one that is none
inline int KeyInputOf(std::string_view setting) {
    if (setting.substr(0, 4) != "key_") return -1;
    for (int i = 0; i < kKeyInputCount; i++)
        if (setting.substr(4) == kKeyInputs[i].name) return i;
    return -1;
}

// a settings value's two keys: the numbers in it, "[ 75, 32 ]" as the file has them, "75/32" as a test's set=
// writes them; a key past 255 or a third is left out, a missing second is none
inline void ParseKeys(std::string_view v, uint16_t out[2]) {
    out[0] = out[1] = 0;
    int n = 0;
    for (size_t i = 0; i < v.size() && n < 2;) {
        if (v[i] < '0' || v[i] > '9') {
            i++;
            continue;
        }
        uint32_t k = 0;
        for (; i < v.size() && v[i] >= '0' && v[i] <= '9'; i++) k = k * 10 + uint32_t(v[i] - '0');
        out[n++] = k <= 255 ? uint16_t(k) : 0;
    }
}

// keys a key name is asked for with the extended bit (GetKeyNameTextW): the arrows and the block above them, the
// right Ctrl and Alt, the numpad's / (without it they are the numpad's keys, the left ones, the main /)
inline bool KeyExtended(uint32_t vk) {
    return (vk >= VK_PRIOR && vk <= VK_DOWN) || vk == VK_INSERT || vk == VK_DELETE || vk == VK_RCONTROL ||
           vk == VK_RMENU || vk == VK_DIVIDE;
}

// a key the keyboard can take for an input (the launcher's): not Escape (it cancels), the Windows keys, Print Screen
// or Num Lock (Windows' own), nor a mouse button
inline bool KeyBindable(uint32_t vk) {
    if (vk < 0x08 || vk > 0xFE) return false;
    return vk != VK_ESCAPE && vk != VK_LWIN && vk != VK_RWIN && vk != VK_SNAPSHOT && vk != VK_NUMLOCK &&
           vk != VK_SHIFT && vk != VK_CONTROL && vk != VK_MENU;  // the generic ones: a side is always known
}

}  // namespace cv
