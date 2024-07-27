#pragma once

#include <common.h>
#include "xdg-decoration-client-protocol.h"
#include <libdecor-0/libdecor.h>
#include <platform/platform.h>

void wayland_decoration_handle_configure_ssd(void *data, struct zxdg_toplevel_decoration_v1 *decoration, u32 mode);
void wayland_decoration_handle_error_csd(struct libdecor *decorator, enum libdecor_error error, const char *message);
void wayland_decoration_handle_configure_csd(struct libdecor_frame *frame, struct libdecor_configuration *configuration, void *data);
void wayland_decoration_handle_close_csd(struct libdecor_frame *frame, void *data);
void wayland_decoration_handle_commit_csd(struct libdecor_frame *frame, void *data);
void wayland_decoration_handle_dismiss_popup_csd(struct libdecor_frame *frame, const char *seat_name, void *data);

b8 wayland_setup_csd(window *window);
