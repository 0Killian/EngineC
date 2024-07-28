#include <core/event.h>
#include <core/memory.h>
#include <core/toml.h>
#include <entry.h>
#include <platform/platform.h>

#undef LOG_SCOPE
#define LOG_SCOPE "APP_MAIN"

static void print_entry(toml_table_entry *entry, u32 ident_count) {
    switch (entry->type) {
    case TOML_TABLE_ENTRY_TYPE_BOOL:
        LOG_INFO("%*s%s = %s", ident_count * 4, "", entry->name, entry->value.b8 ? "true" : "false");
        break;
    case TOML_TABLE_ENTRY_TYPE_FLOAT: LOG_INFO("%*s%s = %f", ident_count * 4, "", entry->name, entry->value.f32); break;
    case TOML_TABLE_ENTRY_TYPE_INT64: LOG_INFO("%*s%s = %lld", ident_count * 4, "", entry->name, entry->value.int64); break;
    case TOML_TABLE_ENTRY_TYPE_STRING: LOG_INFO("%*s%s = %s", ident_count * 4, "", entry->name, entry->value.string); break;
    case TOML_TABLE_ENTRY_TYPE_TABLE: {
        LOG_INFO("%*s%s = {", ident_count * 4, "", entry->name);
        for (u32 i = 0; i < entry->value.table.count; i++) {
            print_entry(&entry->value.table.data[i], ident_count + 1);
        }
        LOG_INFO("%*s}", ident_count * 4, "");
        break;
    }
    default: break;
    }
}

b8 init(application *app) {
    const char *test_data = "";
    toml_table toml = {};

    if (!toml_parse(test_data, &toml)) {
        return FALSE;
    }

    toml_free(&toml);

    test_data = " \
\n#Useless spaces eliminated. \
\ntitle=\"TOML Example\" \
\n[owner] \
\nname=\"Lance Uppercut\" \
\n[database] \
\nserver=\"192.168.1.1\" \
\nconnection_max=5000 \
\nenabled=true \
\n[servers] \
\n[servers.alpha] \
\nip=\"10.0.0.1\" \
\ndc=\"eqdc10\" \
\n[servers.beta] \
\nip=\"10.0.0.2\" \
\ndc=\"eqdc10\" \
\n[clients]";

    if (!toml_parse(test_data, &toml)) {
        return FALSE;
    }

    for (u32 i = 0; i < toml.categories.count; i++) {
        toml_table_category *category = &toml.categories.data[i];
        LOG_INFO("[%s]", category->name);
        u32 ident_count = 4;
        for (u32 j = 0; j < category->entries.count; j++) {
            print_entry(&category->entries.data[j], ident_count);
        }
    }

    toml_free(&toml);

    return TRUE;
}

b8 update(application *app, f32 delta_time) {
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

void deinit(application *app) {}

b8 create_application(application *app) {
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
