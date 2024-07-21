#include "platform.h"
#include "core/log.h"
#include "core/dynamic_array.h"
#include "core/memory.h"
#include "core/event.h"
#include <stdio.h>

#if PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>

/** @brief State of the platform layer. */
typedef struct platform_state {
    DYNARRAY(window *) windows;
    platform_window_closed_callback window_closed_callback;

    f64 clock_frequency;
    LARGE_INTEGER clock_start_time;
} platform_state;

typedef struct window_platform_state {
    HWND handle;
} window_platform_state;

static platform_state *state;

static LRESULT CALLBACK wnd_proc_bootstrap(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK wnd_proc_stub(HWND, UINT, WPARAM, LPARAM);
static LRESULT wnd_proc(window*, UINT, WPARAM, LPARAM);
static void clock_setup();

static u8 convert_platform_color(platform_console_color foreground, platform_console_color background) {
    return foreground & 0xF | (background & 0xF) << 4;
}

static void show_error_message_box(const char *title, const char *message) {
    DWORD error_code = GetLastError();
    LPSTR message_buf = NULL;

    u64 size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buf, 0, NULL);

    if (size) {
        // TODO: Function to format string
        const char *prefix = ": '";
        const char *suffix = "'.";
        char *err_message = mem_alloc(MEMORY_TAG_STRING, size + strlen(message) + strlen(prefix) + strlen(suffix) + 1);
        sprintf_s(err_message, size + strlen(message) + strlen(prefix) + strlen(suffix) + 1, "%s%s%s%s", message, prefix, message_buf, suffix);
        LocalFree(message_buf);

        MessageBoxA(NULL, err_message, "Error", MB_OK | MB_ICONERROR);
        LOG_FATAL(err_message);
        mem_free(err_message);
    } else {
        MessageBoxA(NULL, message, "Error", MB_OK | MB_ICONERROR);
        LOG_FATAL("Window registration failed");
    }
}

/**
 * @brief Initializes the platform layer.
 * 
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the platform (with state != NULL).
 * 
 * @param [in] state_storage A pointer to a memory region to store the state of the platform layer. To obtain the needed size, pass NULL.
 * @param [out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_init(void *state_storage, u64 *size_requirement) {
    if (state_storage == NULL) {
        // Fill out the size requirement and boot out
        *size_requirement = sizeof(platform_state);
        return TRUE;
    }

    state = state_storage;
    mem_zero(state, sizeof(platform_state));

    // Detect corruptions and terminate the application at any time
    #ifdef DEBUG
        if(!HeapSetInformation(GetProcessHeap(), HeapEnableTerminationOnCorruption, NULL, 0)) {
            LOG_WARN("HeapSetInformation failed with code %d. Application will run without heap corruption detection.", GetLastError());
        }
    #endif

    // DPI Awareness
    if (!SetProcessDPIAware()) {
        LOG_WARN("SetProcessDPIAware failed with code %d. Application will run without DPI awareness.", GetLastError());
    }

    // Window class
    HICON icon = LoadIconA(GetModuleHandleA(NULL), IDI_APPLICATION);
    WNDCLASSEXA window_class = {
        .cbSize = sizeof(WNDCLASSEXA),
        .style = CS_DBLCLKS, // Double clicks
        .lpfnWndProc = wnd_proc_bootstrap,
        .cbClsExtra = 0,
        .cbWndExtra = sizeof(u64),
        .hInstance = GetModuleHandleA(NULL),
        .hIcon = icon,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
        .lpszMenuName = NULL,
        .lpszClassName = "EngineWindow",
        .hIconSm = icon
    };

    if (!RegisterClassExA(&window_class)) {
        show_error_message_box("Error", "Failed to register window class");

        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Deinitializes the platform layer.
 * 
 * @param [in] state A pointer to the state of the platform layer.
 */
void platform_deinit(void *) {
    if (state) {
        for (u32 i = 0; i < state->windows.count; i++) {
            if (state->windows.data[i]) {
                platform_window_destroy(state->windows.data[i]);
            }
        }
        DYNARRAY_CLEAR(state->windows);
    }
    memset(state, 0, sizeof(platform_state));
}

static void console_write(u8 color, const char *message, HANDLE console_handle) {
    CONSOLE_SCREEN_BUFFER_INFO buffer_info;
    GetConsoleScreenBufferInfo(console_handle, &buffer_info);

    SetConsoleTextAttribute(console_handle, color);
    OutputDebugStringA(message);
    DWORD written = 0;
    WriteConsoleA(console_handle, message, (DWORD)strlen(message), &written, NULL);

    SetConsoleTextAttribute(console_handle, buffer_info.wAttributes);
}

/**
 * @brief Writes a message to the console.
 * 
 * Message considered as errors should use @ref platform_console_write_error instead.
 * 
 * @param[in] foreground The color of the text.
 * @param[in] background The color of the background.
 * @param[in] message The message to write.
 */
void platform_console_write(platform_console_color foreground, platform_console_color background, const char *message) {
    console_write(convert_platform_color(foreground, background), message, GetStdHandle(STD_OUTPUT_HANDLE));
}

/**
 * @brief Writes an error message to the console.
 * 
 * @param[in] foreground The color of the text.
 * @param[in] background The color of the background.
 * @param[in] message The message to write.
 */
void platform_console_write_error(platform_console_color foreground, platform_console_color background, const char *message) {
    console_write(convert_platform_color(foreground, background), message, GetStdHandle(STD_ERROR_HANDLE));
}


/**
 * @brief Allocates a region of memory.
 * 
 * @param[in] size The size of the region to allocate.
 * @retval A pointer to the allocated region.
 */
void* platform_allocate(u64 size) {
    return HeapAlloc(GetProcessHeap(), 0, size);
}

/**
 * @brief Frees a region of memory.
 * 
 * @param[in] pointer A pointer to the region to free.
 */
void platform_free(void *pointer) {
    HeapFree(GetProcessHeap(), 0, pointer);
}

#ifdef DEBUG
/**
 * @brief Get the address of the caller of the current function.
 * 
 * @note The "current" function is the caller of this function, so the address retrieved is the 2nd address of the call stack.
 * 
 * @return The address of the caller of the current function.
 */
void* platform_get_caller() {
    #ifndef _MSC_VER
        return __builtin_return_address(1);
    #else
        #ifndef HIDE_MSVC_WARNING
            #warning "platform_get_caller will always return NULL on MSVC. To get rid of this warning, define HIDE_MSVC_WARNING."
        #endif
        return NULL;
    #endif
}
#endif

/**
 * @brief Opens a Dynamic Library file.
 * 
 * @param [in] name The name of the library to open.
 * @param [out] result A pointer to a memory region to store the result.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_dynamic_library_open(const char *name, dynamic_library *result) {
    *result = LoadLibraryA(name);
    return *result != NULL;
}

/**
 * @brief Closes a Dynamic Library file.
 * 
 * @param [in] library The library to close.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_dynamic_library_close(dynamic_library library) {
    return FreeLibrary(library) == TRUE;
}

/**
 * @brief Gets a symbol from a Dynamic Library file.
 * 
 * @param [in] library The library to get the symbol from.
 * @param [in] name The name of the symbol to get.
 * @param [out] result A pointer to a memory region to store the result.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_dynamic_library_get_symbol(dynamic_library library, const char *name, void **result) {
    *result = GetProcAddress(library, name);
    return *result != NULL;
}

/**
 * @brief Creates a new window from the specified config.
 * 
 * @note The window is shown immediately.
 * 
 * @param [in] config The config of the window to create.
 * @param [out] result A pointer to a memory region to store the result pointer.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_window_create(const window_config *config, window **result) {
    if (result == NULL) {
        return FALSE;
    }

    window *window = mem_alloc(MEMORY_TAG_PLATFORM, sizeof(struct window));
    u32 index = 0xFFFFFFFF;
    for (u32 i = 0; i < state->windows.count; i++) {
        if (state->windows.data[i] == NULL) {
            state->windows.data[i] = window;
            index = i;
            break;
        }
    }

    if (index == 0xFFFFFFFF) {
        DYNARRAY_PUSH(state->windows, window);
        index = state->windows.count - 1;
    }

    i32 client_x = config->position_x;
    i32 client_y = config->position_y;
    u32 client_width = config->width;
    u32 client_height = config->height;

    i32 window_x = client_x;
    i32 window_y = client_y;
    i32 window_width = client_width;
    i32 window_height = client_height;

    u32 window_style = WS_OVERLAPPEDWINDOW;
    u32 window_ex_style = WS_EX_APPWINDOW;

    RECT window_rect = {};
    AdjustWindowRectEx(&window_rect, window_style, 0, window_ex_style);

    window_x += window_rect.left;
    window_y += window_rect.top;
    window_width += window_rect.right - window_rect.left;
    window_height += window_rect.bottom - window_rect.top;

    if (config->title) {
        // TODO: String functions
        window->title = (char *)mem_alloc(MEMORY_TAG_STRING, strlen(config->title) + 1);
        strcpy(window->title, config->title);
    } else {
        // TODO: String functions
        const char *title = "Engine Window";
        window->title = (char *)mem_alloc(MEMORY_TAG_STRING, strlen(title) + 1);
        strcpy(window->title, title);
    }

    window->width = client_width;
    window->height = client_height;
    window->device_pixel_ratio = 1.0f;

    window->platform_state = mem_alloc(MEMORY_TAG_PLATFORM, sizeof(window_platform_state));
    mem_zero(window->platform_state, sizeof(window_platform_state));

    window->platform_state->handle = CreateWindowExA(window_ex_style, "EngineWindow", window->title, 
        window_style, window_x, window_y, window_width, window_height, NULL, NULL, GetModuleHandleA(NULL), (LPVOID)(u64)index);
    
    if (window->platform_state->handle == NULL) {
        show_error_message_box("Error", "Window creation failed");
        return FALSE;
    }

    ShowWindow(window->platform_state->handle, SW_SHOW);

    *result = window;
    return TRUE;
}

/**
 * @brief Destroys the given window.
 * 
 * @param [in] window The window to destroy.
 */
void platform_window_destroy(window *window) {
    DestroyWindow(window->platform_state->handle);
    mem_free(window->title);
    mem_free(window->platform_state);
    mem_zero(window, sizeof(window));
    mem_free(window);

    b8 found = FALSE;
    for (u32 i = 0; i < state->windows.count; i++) {
        if (state->windows.data[i] == window) {
            state->windows.data[i] = NULL;
            found = TRUE;
            break;
        }
    }

    if (!found) {
        LOG_WARN("Tried to destroy an unregistered window");
    }
}

/**
 * @brief Sets the window title.
 * 
 * @note The title is copied, so it is safe to use a temporary string.
 * 
 * @param [in] window The window to set the title of.
 * @param [in] title The title to set.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_window_set_title(window *window, const char *title) {
    if (title == NULL) {
        return FALSE;
    }

    mem_free(window->title);

    window->title = (char*)mem_alloc(MEMORY_TAG_STRING, strlen(title) + 1);
    strcpy(window->title, title);
    SetWindowTextA(window->platform_state->handle, window->title);
    return TRUE;
}

/**
 * @brief Retrieves platform-specific messages and process them.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_process_messages() {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return TRUE;
}

/**
 * @brief Registers the callback to be called when the window is closed.
 * 
 * @note There is only one callback registered at a time.
 * 
 * @param [in] callback The callback to register.
 */
void platform_register_window_closed_callback(platform_window_closed_callback callback) {
    state->window_closed_callback = callback;
}

/**
 * @brief Gets the time in seconds.
 * 
 * @return The time in seconds.
 */
f32 platform_get_time() {
    if (!state->clock_frequency) {
        clock_setup();
    }

    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * state->clock_frequency;
}

/**
 * @brief Sleeps for the specified number of milliseconds.
 * 
 * @param [in] milliseconds The number of milliseconds to sleep.
 */
API void platform_sleep(u32 milliseconds) {
    Sleep(milliseconds);
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

static LRESULT CALLBACK wnd_proc_bootstrap(HWND handle, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg != WM_NCCREATE) {
        return DefWindowProcA(handle, msg, wp, lp);
    }

    u32 index = (u32)(u64)((LPCREATESTRUCTA)lp)->lpCreateParams;

    SetWindowLongPtrA(handle, GWLP_USERDATA, (LONG_PTR)(u64)index);
    SetWindowLongPtrA(handle, GWLP_WNDPROC, (LONG_PTR)wnd_proc_stub);

    window *window = state->windows.data[index];
    window->platform_state->handle = handle;

    wnd_proc(window, msg, wp, lp);
}

static LRESULT CALLBACK wnd_proc_stub(HWND handle, UINT msg, WPARAM wp, LPARAM lp) {
    u32 index = (u32)GetWindowLongPtrA(handle, GWLP_USERDATA);
    return wnd_proc(state->windows.data[index], msg, wp, lp);
}

static LRESULT wnd_proc(window *window, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_DPICHANGED: {
        i32 x_dpi = LOWORD(wp);
        window->device_pixel_ratio = (f32)x_dpi / USER_DEFAULT_SCREEN_DPI;

        LOG_INFO("Device Pixel Ratio: %f", window->device_pixel_ratio);
        return 0;
    }

    case WM_CLOSE:
        if (state->window_closed_callback != NULL) {
            state->window_closed_callback(window);
        }

        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE: {
        RECT rect;
        GetClientRect(window->platform_state->handle, &rect);

        u32 width = rect.right - rect.left;
        u32 height = rect.bottom - rect.top;

        if (width != window->width || height != window->height) {
            window->resizing = TRUE;
            window->frames_since_resize = 0;

            window->width = width;
            window->height = height;

            event_data data = { .vec2f = { (f32)width, (f32)height } };

            if (!event_fire(EVENT_TYPE_WINDOW_RESIZED, data)) {
                LOG_WARN("Failed to fire EVENT_TYPE_WINDOW_RESIZED");
            }
        }
    } break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        u16 scan_code = (lp >> 16) & 0xFF;
        b8 is_extended = (lp & (1 << 24)) != 0;
        if (is_extended) {
            scan_code |= 0xE000;
        }

        key key = key_from_scancode(scan_code);

        if (key == KEY_MAX_KEYS) {
            LOG_TRACE("Unknown scancode: %d", scan_code);
        } else {
            if (!event_fire(EVENT_TYPE_KEY_PRESSED, (event_data){ .key = key })) {
                LOG_WARN("Failed to fire EVENT_TYPE_KEY_PRESSED");
            }
        }
    } break;

    case WM_KEYUP:
    case WM_SYSKEYUP: {
        u16 scan_code = (lp >> 16) & 0xFF;
        b8 is_extended = (lp & (1 << 24)) != 0;
        if (is_extended) {
            scan_code |= 0xE000;
        }

        key key = key_from_scancode(scan_code);

        if (key == KEY_MAX_KEYS) {
            LOG_TRACE("Unknown scancode: %d", scan_code);
        } else {
            if (!event_fire(EVENT_TYPE_KEY_RELEASED, (event_data){ .key = key })) {
                LOG_WARN("Failed to fire EVENT_TYPE_KEY_RELEASED");
            }
        }
    } break;

    case WM_LBUTTONDOWN: {
        if (!event_fire(EVENT_TYPE_MOUSE_BUTTON_PRESSED, (event_data){ .u32 = 0 })) {
            LOG_WARN("Failed to fire EVENT_TYPE_MOUSE_BUTTON_PRESSED");
        }
    } break;

    case WM_LBUTTONUP: {
        if (!event_fire(EVENT_TYPE_MOUSE_BUTTON_RELEASED, (event_data){ .u32 = 0 })) {
            LOG_WARN("Failed to fire EVENT_TYPE_MOUSE_BUTTON_RELEASED");
        }
    } break;

    case WM_MBUTTONDOWN: {
        if (!event_fire(EVENT_TYPE_MOUSE_BUTTON_PRESSED, (event_data){ .u32 = 1 })) {
            LOG_WARN("Failed to fire EVENT_TYPE_MOUSE_BUTTON_PRESSED");
        }
    }

    case WM_MBUTTONUP: {
        if (!event_fire(EVENT_TYPE_MOUSE_BUTTON_RELEASED, (event_data){ .u32 = 1 })) {
            LOG_WARN("Failed to fire EVENT_TYPE_MOUSE_BUTTON_RELEASED");
        }
    } break;

    case WM_RBUTTONDOWN: {
        if (!event_fire(EVENT_TYPE_MOUSE_BUTTON_PRESSED, (event_data){ .u32 = 2 })) {
            LOG_WARN("Failed to fire EVENT_TYPE_MOUSE_BUTTON_PRESSED");
        }
    } break;

    case WM_RBUTTONUP: {
        if (!event_fire(EVENT_TYPE_MOUSE_BUTTON_RELEASED, (event_data){ .u32 = 2 })) {
            LOG_WARN("Failed to fire EVENT_TYPE_MOUSE_BUTTON_RELEASED");
        }
    } break;

    case WM_XBUTTONDOWN: {
        if (!event_fire(EVENT_TYPE_MOUSE_BUTTON_PRESSED, (event_data){ .u32 = HIWORD(wp) + 2 })) {
            LOG_WARN("Failed to fire EVENT_TYPE_MOUSE_BUTTON_PRESSED");
        }
    } break;

    case WM_XBUTTONUP: {
        if (!event_fire(EVENT_TYPE_MOUSE_BUTTON_RELEASED, (event_data){ .u32 = HIWORD(wp) + 2 })) {
            LOG_WARN("Failed to fire EVENT_TYPE_MOUSE_BUTTON_RELEASED");
        }
    } break;

    case WM_MOUSEMOVE: {
        event_data data = { .vec2f = { (f32)GET_X_LPARAM(lp), (f32)GET_Y_LPARAM(lp) } };
        event_fire(EVENT_TYPE_MOUSE_MOVED, data);
    } break;

    case WM_MOUSEWHEEL: {
        i16 delta = GET_WHEEL_DELTA_WPARAM(wp);
        event_data data = { .f32 = (f32)delta / WHEEL_DELTA };
        event_fire(EVENT_TYPE_MOUSE_WHEEL, data);
    } break;
    
    default:
        return DefWindowProcA(window->platform_state->handle, msg, wp, lp);
    }
}

static void clock_setup() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    state->clock_frequency = 1.0f / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&state->clock_start_time);
}

#endif