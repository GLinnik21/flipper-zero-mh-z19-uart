#include "mh_z19_ui.h"

void input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HelloWorldContext* hello_context = context;

    furi_message_queue_put(hello_context->event_queue, event, FuriWaitForever);
}

void draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);

    HelloWorldContext* app = context;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    static char buffer[50];

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    snprintf(buffer, sizeof(buffer), "ppm: %lu", app->ppm);
    furi_mutex_release(app->mutex);

    canvas_draw_str(canvas, 0, 10, buffer);
}
