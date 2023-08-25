#include "mh_z19_ui.h"

#include "mh_z19_app_i.h"

void mh_z19_app_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    MhZ19App* app = context;

    furi_message_queue_put(app->event_queue, event, FuriWaitForever);
}

void mh_z19_app_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);

    MhZ19App* app = context;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    static char buffer[50];

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    snprintf(buffer, sizeof(buffer), "ppm: %lu", app->ppm);
    furi_mutex_release(app->mutex);

    canvas_draw_str(canvas, 0, 10, buffer);
}
