#include <entry.h>

#undef LOG_SCOPE
#define LOG_SCOPE "APP_MAIN"

b8 init(application* app) {
    LOG_TRACE("init");
    LOG_DEBUG("init");
    LOG_INFO("init");
    LOG_WARN("init");
    LOG_ERROR("init");
    LOG_FATAL("init");
    return TRUE;
}

void deinit(application* app) {
    LOG_TRACE("deinit");
    LOG_DEBUG("deinit");
    LOG_INFO("deinit");
    LOG_WARN("deinit");
    LOG_ERROR("deinit");
    LOG_FATAL("deinit");
}

b8 create_application(application* app) {
    app->init = init;
    app->deinit = deinit;

    app->window_config.name = "main";
    app->window_config.title = "Main";
    app->window_config.position_x = 300;
    app->window_config.position_y = 300;
    app->window_config.width = 800;
    app->window_config.height = 600;

    return TRUE;
}