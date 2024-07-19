#include "platform.h"
#include "core/log.h"
#include "core/dynamic_array.h"
#include "core/memory.h"
#include "core/event.h"
#include <stdio.h>

#if PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>

/** @brief State of the platform layer. */
typedef struct platform_state {
    DYNARRAY(window *) windows;
    platform_window_closed_callback window_closed_callback;
    platform_window_resized_callback window_resized_callback;
    // TODO: Other callbacks
} platform_state;

typedef struct window_platform_state {
    HWND handle;
} window_platform_state;

static platform_state *state;

LRESULT CALLBACK wnd_proc_bootstrap(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK wnd_proc_stub(HWND, UINT, WPARAM, LPARAM);
LRESULT wnd_proc(window*, UINT, WPARAM, LPARAM);

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
 * @brief Registers the callback to be called when the window is resized.
 * 
 * @note There is only one callback registered at a time.
 * 
 * @param [in] callback The callback to register.
 */
void platform_register_window_resized_callback(platform_window_resized_callback callback) {
    state->window_resized_callback = callback;
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

// TODO: Mouse Button
// TODO: Mouse Movement
// TODO: Mouse Wheel
// TODO: Key

LRESULT CALLBACK wnd_proc_bootstrap(HWND handle, UINT msg, WPARAM wp, LPARAM lp) {
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

LRESULT CALLBACK wnd_proc_stub(HWND handle, UINT msg, WPARAM wp, LPARAM lp) {
    u32 index = (u32)GetWindowLongPtrA(handle, GWLP_USERDATA);
    return wnd_proc(state->windows.data[index], msg, wp, lp);
}

LRESULT wnd_proc(window *window, UINT msg, WPARAM wp, LPARAM lp) {
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

            if (state->window_resized_callback != NULL) {
                state->window_resized_callback(window);
            }

            event_data data = { .vec2f = { (f32)width, (f32)height } };

            if (!event_fire(EVENT_TYPE_WINDOW_RESIZED, data)) {
                LOG_WARN("Failed to fire EVENT_TYPE_WINDOW_RESIZED");
            }
        }
    } break;

    // TODO: Mouse and Key events
    default:
        return DefWindowProcA(window->platform_state->handle, msg, wp, lp);
    }
}

#endif