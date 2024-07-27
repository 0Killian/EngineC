/**
 * @file filesystem.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the interface of the filesystem layer. Each platform must implement this interface in a separate .c
 * file.
 * @version 0.1
 * @date 2024-06-22
 */

#pragma once

#include "common.h"

typedef enum filesystem_node_type {
    FILESYSTEM_NODE_TYPE_FILE = 0,
    FILESYSTEM_NODE_TYPE_DIRECTORY
} filesystem_node_type;

typedef enum filesystem_open_mode {
    FILESYSTEM_OPEN_MODE_READ = 0,
    FILESYSTEM_OPEN_MODE_WRITE,
    FILESYSTEM_OPEN_MODE_APPEND
} filesystem_open_mode;

typedef enum filesystem_seek_mode {
    FILESYSTEM_SEEK_MODE_BEGIN = 0,
    FILESYSTEM_SEEK_MODE_CURRENT,
    FILESYSTEM_SEEK_MODE_END
} filesystem_seek_mode;

typedef void* filesystem_handle;

/**
 * @brief Checks if a file or a directory exists.
 * 
 * @param[in] path The path of the file to check.
 * @param[in] type The type of the file to check.
 * 
 * @retval TRUE The file exists
 * @retval FALSE The file does not exist
 */
b8 filesystem_node_exists(const char* path, filesystem_node_type type);

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
b8 filesystem_node_read(const char* path, void* content, u64* size_requirement);

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
b8 filesystem_node_write(const char* path, void* content, u64 size, b8 create);

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
b8 filesystem_handle_open(const char* path, filesystem_open_mode mode, filesystem_handle* handle);

/**
 * @brief Closes a file.
 * 
 * @param[in] handle The handle of the file to close.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_handle_close(filesystem_handle handle);

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
b8 filesystem_handle_read(filesystem_handle handle, u64 size, void* content, u64* read_size);


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
b8 filesystem_handle_read_line(filesystem_handle handle, u64 max_size, void* content, u64* line_size);

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
b8 filesystem_handle_write(filesystem_handle handle, void* content, u64 size);

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
b8 filesystem_handle_seek(filesystem_handle handle, i64 offset, filesystem_seek_mode mode);

/**
 * @brief Get the current position in a file through a handle.
 * 
 * @param[in] handle The handle of the file to get the position.
 * @param[out] position A pointer to the resulting position.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_handle_get_position(filesystem_handle handle, u64* position);

/**
 * @brief Deletes a node.
 * 
 * @param[in] path The path of the node to delete.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 filesystem_node_delete(const char* path);
