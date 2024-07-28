#if PLATFORM_LINUX
#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief Checks if a file or a directory exists.
 *
 * @param[in] path The path of the file to check.
 * @param[in] type The type of the file to check.
 *
 * @retval TRUE The file exists
 * @retval FALSE The file does not exist
 */
b8 filesystem_node_exists(const char *path, filesystem_node_type type) {
    struct stat buffer;
    if (stat(path, &buffer) != 0) {
        return FALSE;
    }

    if (type == FILESYSTEM_NODE_TYPE_FILE) {
        return S_ISREG(buffer.st_mode);
    } else if (type == FILESYSTEM_NODE_TYPE_DIRECTORY) {
        return S_ISDIR(buffer.st_mode);
    }

    return FALSE;
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
b8 filesystem_node_read(const char *path, void *content, u64 *size_requirement) {
    struct stat buffer;
    if (stat(path, &buffer) != 0) {
        return FALSE;
    }

    u64 size = buffer.st_size;

    if (content == NULL || *size_requirement < size) {
        *size_requirement = size;
        return TRUE;
    }

    FILE *f = fopen(path, "rb");
    fread(content, 1, size, f);
    fclose(f);
    return TRUE;
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
b8 filesystem_node_write(const char *path, void *content, u64 size, b8 create) {
    struct stat buffer;
    if (stat(path, &buffer) == 0) {
        if (S_ISDIR(buffer.st_mode)) {
            return FALSE;
        }
    } else if (!create) {
        return FALSE;
    }

    FILE *f = fopen(path, "wb");
    fwrite(content, 1, size, f);
    fclose(f);
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
b8 filesystem_handle_open(const char *path, filesystem_open_mode mode, filesystem_handle *handle) {
    switch (mode) {
    case FILESYSTEM_OPEN_MODE_READ: {
        *handle = fopen(path, "rb");
        return *handle != NULL;
    }
    case FILESYSTEM_OPEN_MODE_WRITE: {
        *handle = fopen(path, "wb");
        return *handle != NULL;
    }
    case FILESYSTEM_OPEN_MODE_APPEND: {
        *handle = fopen(path, "ab");
        return *handle != NULL;
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
b8 filesystem_handle_close(filesystem_handle handle) { return fclose(handle) == 0; }

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
b8 filesystem_handle_read(filesystem_handle handle, u64 size, void *content, u64 *read_size) {
    *read_size = 0;
    return fread(content, 1, size, handle) == size;
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
b8 filesystem_handle_read_line(filesystem_handle handle, u64 max_size, void *content, u64 *line_size) {
    *line_size = fread(content, 1, max_size, handle);

    for (u64 i = 0; i < *line_size; i++) {
        if (((char *)content)[i] == '\n') {
            fseek(handle, -(*line_size - i + 1), SEEK_CUR);
            *line_size = i;
            ((char *)content)[i] = '\0';
            return TRUE;
        }
    }

    u64 file_size;
    u64 current_pos = ftell(handle);
    fseek(handle, 0, SEEK_END);
    file_size = ftell(handle);
    fseek(handle, current_pos, SEEK_SET);
    return file_size == current_pos;
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
b8 filesystem_handle_write(filesystem_handle handle, void *content, u64 size) { return fwrite(content, 1, size, handle) == size; }

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
    switch (mode) {
    case FILESYSTEM_SEEK_MODE_BEGIN: {
        return fseek(handle, offset, SEEK_SET) == 0;
    }
    case FILESYSTEM_SEEK_MODE_CURRENT: {
        return fseek(handle, offset, SEEK_CUR) == 0;
    }
    case FILESYSTEM_SEEK_MODE_END: {
        return fseek(handle, offset, SEEK_END) == 0;
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
b8 filesystem_handle_get_position(filesystem_handle handle, u64 *position) {
    *position = ftell(handle);
    return *position != -1;
}

/**
 * @brief Deletes a node.
 *
 * @param[in] path The path of the node to delete.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_node_delete(const char *path) { return unlink(path) == 0; }

#endif
