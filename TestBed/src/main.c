#include <entry.h>
#include <stdio.h>

b8 init(application* app) {
    printf("init\n");
    return TRUE;
}

void deinit(application* app) {
    printf("deinit\n");
}

b8 create_application(application* app) {
    app->init = init;
    app->deinit = deinit;
    return TRUE;
}