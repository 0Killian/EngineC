#include "filesystem.h"
#include "core/log.h"

#if PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/**
 * @brief Checks if a file or a directory exists.
 * 
 * @param[in] path The path of the file to check.
 * @param[in] type The type of the file to check.
 * 
 * @retval TRUE The file exists
 * @retval FALSE The file does not exist
 */
b8 filesystem_node_exists(const char* path, filesystem_node_type type) {
    if (type == FILESYSTEM_NODE_TYPE_FILE) {
        return (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES && !(GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY));
    } else if (type == FILESYSTEM_NODE_TYPE_DIRECTORY) {
        return (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES && GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY);
    }
}

/**
 * @brief Reads the content of a file.
 * 
 * @note This function can also be used to tell the size of a file, by passing NULL as the content pointer.
 * @note Trying to read from a directory will fail.
 * 
 * @param[in] path The path of the file to read.
 * @param[out] content A pointer to a memory region to store the resulting content. To obtain the needed size, pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_node_read(const char* path, void** content, u64* size_requirement) {
    HANDLE handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    u64 size = GetFileSize(handle, NULL);

    if (content == NULL || *size_requirement < size) {
        *size_requirement = size;
        CloseHandle(handle);
        return TRUE;
    }

    *content = malloc(*size_requirement);
    ReadFile(handle, *content, size, NULL, NULL);
}

/**
 * @brief Writes the content of a file.
 * 
 * @note Trying to write to a directory will fail.
 * 
 * @param[in] path The path of the file to write.
 * @param[in] content The content to write.
 * @param[in] size The size of the content to write.
 * @param[in] create If TRUE, the file will be created if it does not exist.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_node_write(const char* path, void* content, u64 size, b8 create) {
    HANDLE handle = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, create ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    WriteFile(handle, content, size, NULL, NULL);
    CloseHandle(handle);
    return TRUE;
}

/**
 * @brief Opens a file.
 * 
 * @note Trying to open a directory will fail.
 * 
 * @param[in] path The path of the file to open.
 * @param[in] mode The mode of the file to open.
 * @param[out] handle A pointer to the resulting handle.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_handle_open(const char* path, filesystem_open_mode mode, filesystem_handle* handle) {
    switch(mode) {
        case FILESYSTEM_OPEN_MODE_READ: {
            *handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            return *handle != INVALID_HANDLE_VALUE;
        }
        case FILESYSTEM_OPEN_MODE_WRITE: {
            *handle = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            return *handle != INVALID_HANDLE_VALUE;
        }
        case FILESYSTEM_OPEN_MODE_APPEND: {
            *handle = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            return *handle != INVALID_HANDLE_VALUE;
        }
        default: {
            return FALSE;
        }
    }
}

/**
 * @brief Closes a file.
 * 
 * @param[in] handle The handle of the file to close.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_handle_close(filesystem_handle handle) {
    return CloseHandle(handle);
}

/**
 * @brief Reads the content of a file through a handle.
 * 
 * @param[in] handle The handle of the file to read.
 * @param[in] size The size of the content to read.
 * @param[out] content A pointer to a memory region to store the resulting content.
 * @param[out] read_size A pointer to the size of the content read.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_handle_read(filesystem_handle handle, u64 size, void* content, u64* read_size) {
    *read_size = 0;
    return ReadFile(handle, content, size, (LPDWORD)read_size, NULL);
}


/**
 * @brief Reads the next line of an opened file handle.
 * 
 * @param[in] handle The handle of the file to read.
 * @param[in] max_size The maximum size of the content to read.
 * @param[out] content A pointer to a memory region to store the resulting content.
 * @param[out] line_size A pointer to the size of the resulting content.
 * 
 * @return TRUE Success (or EOF reached)
 * @return FALSE Failure (file handling issues or buffer too small)
 */
b8 filesystem_handle_read_line(filesystem_handle handle, u64 max_size, void* content, u64* line_size) {
    *line_size = 0;
    if (!ReadFile(handle, content, max_size, (LPDWORD)line_size, NULL)) {
        return FALSE;
    }

    for (u64 i = 0; i < *line_size; i++) {
        if (((char*)content)[i] == '\n') {
            SetFilePointer(handle, -(*line_size - i + 1), NULL, FILE_CURRENT);
            *line_size = i;
            ((char*)content)[i] = '\0';
            return TRUE;
        }
    }

    return GetFileSize(handle, NULL) == SetFilePointer(handle, 0, NULL, FILE_CURRENT);
}

/**
 * @brief Writes the content of a file through a handle.
 * 
 * @param[in] handle The handle of the file to write.
 * @param[in] content The content to write.
 * @param[in] size The size of the content to write.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_handle_write(filesystem_handle handle, void* content, u64 size) {
    return WriteFile(handle, content, size, NULL, NULL);
}

/**
 * @brief Seek to a different position in a file through a handle.
 * 
 * @param[in] handle The handle of the file to seek.
 * @param[in] offset The offset to seek to.
 * @param[in] mode The mode of the seek.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_handle_seek(filesystem_handle handle, i64 offset, filesystem_seek_mode mode) {
    switch(mode) {
        case FILESYSTEM_SEEK_MODE_BEGIN: {
            return SetFilePointer(handle, offset, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER;
        }
        case FILESYSTEM_SEEK_MODE_CURRENT: {
            return SetFilePointer(handle, offset, NULL, FILE_CURRENT) != INVALID_SET_FILE_POINTER;
        }
        case FILESYSTEM_SEEK_MODE_END: {
            return SetFilePointer(handle, offset, NULL, FILE_END) != INVALID_SET_FILE_POINTER;
        }
        default: {
            return FALSE;
        }
    }
}

/**
 * @brief Get the current position in a file through a handle.
 * 
 * @param[in] handle The handle of the file to get the position.
 * @param[out] position A pointer to the resulting position.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_handle_get_position(filesystem_handle handle, u64* position) {
    *position = SetFilePointer(handle, 0, NULL, FILE_CURRENT);
    return *position != INVALID_SET_FILE_POINTER;
}

/**
 * @brief Deletes a node.
 * 
 * @param[in] path The path of the node to delete.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_node_delete(const char* path) {
    return DeleteFileA(path);
}

#endif