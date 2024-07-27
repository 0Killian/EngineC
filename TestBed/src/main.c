#include <entry.h>
#include <core/event.h>
#include <core/memory.h>
#include <platform/platform.h>

#undef LOG_SCOPE
#define LOG_SCOPE "APP_MAIN"

b8 init(application* app) {
    return TRUE;
}

b8 update(application* app, f32 delta_time) {
    // TODO: Rotating cube
    return TRUE;
}

b8 prepare_frame(application *app, frame_packet *packet) {
    // TODO: Rotating cube
    return TRUE;
}

b8 render_frame(application *app, frame_packet *packet) {
    // TODO: Rotating cube
    return TRUE;
}

void deinit(application* app) {
}

b8 create_application(application* app) {
    app->init = init;
    app->update = update;
    app->prepare_frame = prepare_frame;
    app->render_frame = render_frame;
    app->deinit = deinit;

    app->window_config.name = "main";
    app->window_config.title = "Main";
    app->window_config.position_x = 300;
    app->window_config.position_y = 300;
    app->window_config.width = 800;
    app->window_config.height = 600;

    return TRUE;
}
