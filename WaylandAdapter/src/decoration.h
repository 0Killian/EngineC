#pragma once

#include "xdg-decoration-client-protocol.h"
#include <common.h>
#include <libdecor-0/libdecor.h>
#include <platform/platform.h>

/**
 * @brief Called when the compositor sends a decoration configure event
 *
 * @param[in] data The user data registered with @ref zxdg_toplevel_decoration_v1_add_listener
 * @param[in] decoration The decoration object
 * @param[in] mode The decoration mode
 */
void wayland_decoration_handle_configure_ssd(void *data, struct zxdg_toplevel_decoration_v1 *decoration, u32 mode);

/**
 * @brief Called when libdecor encountes an error
 *
 * @param[in] decorator The libdecor object
 * @param[in] error The error
 * @param[in] message The error message
 */
void wayland_decoration_handle_error_csd(struct libdecor *decorator, enum libdecor_error error, const char *message);

/**
 * @brief Called when libdecor resizes the window
 *
 * @param[in] frame The libdecor frame
 * @param[in] configuration The libdecor configuration
 * @param[in] data The user data
 */
void wayland_decoration_handle_configure_csd(struct libdecor_frame *frame,
                                             struct libdecor_configuration *configuration,
                                             void *data);

/**
 * @brief Called when libdecor closes the window
 *
 * @param[in] frame The libdecor frame
 * @param[in] data The user data
 */
void wayland_decoration_handle_close_csd(struct libdecor_frame *frame, void *data);

/**
 * @brief Called when libdecor commits the window
 *
 * @param[in] frame The libdecor frame
 * @param[in] data The user data
 */
void wayland_decoration_handle_commit_csd(struct libdecor_frame *frame, void *data);

/**
 * @brief Called when libdecor dismisses a popup
 *
 * @param[in] frame The libdecor frame
 * @param[in] seat_name The name of the seat
 * @param[in] data The user data
 */
void wayland_decoration_handle_dismiss_popup_csd(struct libdecor_frame *frame, const char *seat_name, void *data);

b8 wayland_setup_csd(window *window);
