#include "keyboard.h"
#include "wayland_adapter.h"
#include <core/event.h>
#include <platform/linux_adapter.h>

static key key_from_scancode(u16 scan_code);

void wayland_keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard, u32 format, i32 fd, u32 size) {
    (void)wl_keyboard;
    (void)format;
    (void)fd;
    (void)size;
}

void wayland_keyboard_handle_enter(
    void *data, struct wl_keyboard *wl_keyboard, u32 serial, struct wl_surface *surface, struct wl_array *keys) {
    (void)wl_keyboard;
    linux_adapter *adapter = (linux_adapter *)data;

    adapter->adapter_state->keyboard_focus = INVALID_UUID;
    for (uuid i = 0; i < adapter->platform_state->windows.count; i++) {
        window *window = adapter->platform_state->windows.data[i];
        if (window->platform_state->surface == surface) {
            adapter->adapter_state->keyboard_focus = i;
        }
    }
}

void wayland_keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard, u32 serial, struct wl_surface *surface) {
    (void)wl_keyboard;
    (void)serial;
    (void)surface;
    linux_adapter *adapter = (linux_adapter *)data;

    adapter->adapter_state->keyboard_focus = INVALID_UUID;
}

void wayland_keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard, u32 serial, u32 time, u32 key, u32 state) {
    (void)wl_keyboard;
    (void)serial;
    (void)time;
    linux_adapter *adapter = (linux_adapter *)data;

    if (adapter->adapter_state->keyboard_focus != INVALID_UUID) {
        event_data event = { .u32 = key_from_scancode(key) };
        event_fire(state == WL_KEYBOARD_KEY_STATE_PRESSED ? EVENT_TYPE_KEY_PRESSED : EVENT_TYPE_KEY_RELEASED, event);
    }
}

void wayland_keyboard_handle_modifiers(
    void *data, struct wl_keyboard *wl_keyboard, u32 serial, u32 mods_depressed, u32 mods_latched, u32 mods_locked, u32 group) {
    (void)data;
    (void)wl_keyboard;
    (void)serial;
    (void)mods_depressed;
    (void)mods_latched;
    (void)mods_locked;
    (void)group;
}

void wayland_keyboard_handle_repeat_info(void *data, struct wl_keyboard *wl_keyboard, i32 rate, i32 delay) {
    (void)data;
    (void)wl_keyboard;
    (void)rate;
    (void)delay;
}

static key key_from_scancode(u16 scan_code) {
    switch (scan_code) {
    /** @brief Letters */
    case 0x1E: return KEY_A;
    case 0x30: return KEY_B;
    case 0x2E: return KEY_C;
    case 0x20: return KEY_D;
    case 0x12: return KEY_E;
    case 0x21: return KEY_F;
    case 0x22: return KEY_G;
    case 0x23: return KEY_H;
    case 0x17: return KEY_I;
    case 0x24: return KEY_J;
    case 0x25: return KEY_K;
    case 0x26: return KEY_L;
    case 0x32: return KEY_M;
    case 0x31: return KEY_N;
    case 0x18: return KEY_O;
    case 0x19: return KEY_P;
    case 0x10: return KEY_Q;
    case 0x13: return KEY_R;
    case 0x1F: return KEY_S;
    case 0x14: return KEY_T;
    case 0x16: return KEY_U;
    case 0x2F: return KEY_V;
    case 0x11: return KEY_W;
    case 0x2D: return KEY_X;
    case 0x15: return KEY_Y;
    case 0x2C: return KEY_Z;

    /** @brief Numbers */
    case 0x0B: return KEY_NUM0;
    case 0x02: return KEY_NUM1;
    case 0x03: return KEY_NUM2;
    case 0x04: return KEY_NUM3;
    case 0x05: return KEY_NUM4;
    case 0x06: return KEY_NUM5;
    case 0x07: return KEY_NUM6;
    case 0x08: return KEY_NUM7;
    case 0x09: return KEY_NUM8;
    case 0x0A: return KEY_NUM9;

    /** @brief Special keys */
    case 0x0045: return KEY_PAUSE;
    case 0x0001: return KEY_ESCAPE;
    case 0x001D: return KEY_LCONTROL;
    case 0xE01D: return KEY_RCONTROL;
    case 0x002A: return KEY_LSHIFT;
    case 0x0036: return KEY_RSHIFT;
    case 0x0038: return KEY_LALT;
    case 0xE038: return KEY_RALT;
    case 0xE05B: return KEY_LSYSTEM;
    case 0xE05C: return KEY_RSYSTEM;
    case 0x0027: return KEY_SEMICOLON;
    case 0x0033: return KEY_COMMA;
    case 0x0034: return KEY_PERIOD;
    case 0x002B: return KEY_PIPE;
    case 0x0035: return KEY_SLASH;
    case 0x0029: return KEY_TILDE;
    case 0x000D: return KEY_EQUAL;
    case 0x000C: return KEY_DASH;
    case 0x0039: return KEY_SPACE;
    case 0x001C: return KEY_RETURN;
    case 0x000E: return KEY_BACKSPACE;
    case 0x000F: return KEY_TAB;
    case 0xE049: return KEY_PAGEUP;
    case 0xE051: return KEY_PAGEDOWN;
    case 0xE04F: return KEY_END;
    case 0xE047: return KEY_HOME;
    case 0xE052: return KEY_INSERT;
    case 0xE053: return KEY_DELETE;
    case 0xE04B: return KEY_LEFT;
    case 0xE04D: return KEY_RIGHT;
    case 0xE048: return KEY_UP;
    case 0xE050: return KEY_DOWN;
    case 0x0028: return KEY_APOSTROPHE;
    case 0x0056: return KEY_NON_US_SLASH;
    case 0x003A: return KEY_CAPS_LOCK;
    case 0xE037: return KEY_PRINT_SCREEN;
    case 0x0046: return KEY_SCROLL_LOCK;
    case 0x001A: return KEY_LBRACE;
    case 0x001B: return KEY_RBRACE;

    /** @brief Numpad */
    case 0x004E: return KEY_KP_ADD;
    case 0x004A: return KEY_KP_SUBTRACT;
    case 0x0037: return KEY_KP_MULTIPLY;
    case 0xE035: return KEY_KP_DIVIDE;
    case 0x0052: return KEY_KP_0;
    case 0x004F: return KEY_KP_1;
    case 0x0050: return KEY_KP_2;
    case 0x0051: return KEY_KP_3;
    case 0x004B: return KEY_KP_4;
    case 0x004C: return KEY_KP_5;
    case 0x004D: return KEY_KP_6;
    case 0x0047: return KEY_KP_7;
    case 0x0048: return KEY_KP_8;
    case 0x0049: return KEY_KP_9;
    case 0x0053: return KEY_KP_DECIMAL;
    case 0xE01C: return KEY_KP_ENTER;
    case 0xE045: return KEY_KP_LOCK;

    /** @brief Function keys */
    case 0x003B: return KEY_F1;
    case 0x003C: return KEY_F2;
    case 0x003D: return KEY_F3;
    case 0x003E: return KEY_F4;
    case 0x003F: return KEY_F5;
    case 0x0040: return KEY_F6;
    case 0x0041: return KEY_F7;
    case 0x0042: return KEY_F8;
    case 0x0043: return KEY_F9;
    case 0x0044: return KEY_F10;
    case 0x0057: return KEY_F11;
    case 0x0058: return KEY_F12;
    case 0x0064: return KEY_F13;
    case 0x0065: return KEY_F14;
    case 0x0066: return KEY_F15;

    default: return KEY_MAX_KEYS;
    }
}
