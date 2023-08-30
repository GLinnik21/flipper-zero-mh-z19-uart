#include "mh_z19_ui.h"

#include <gui/elements.h>

#include "mh_z19_uart_icons.h"
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

    canvas_set_bitmap_mode(canvas, 1);

    if(!app->power_data.is_5V_enabled) {
        canvas_draw_icon(canvas, 5, 4, &I_Alert_9x8);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 19, 12, "Oops! 5V is off");
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text(
            canvas,
            5,
            28,
            "Enable manually in\n"
            "GPIO app -> 5V on GPIO -> ON\n"
            "or put Flipper on charge.");
    } else {
        static char buffer[5] = {0};
        furi_mutex_acquire(app->thread_data.mutex, FuriWaitForever);
        snprintf(buffer, sizeof(buffer), "%lu", app->ppm);
        furi_mutex_release(app->thread_data.mutex);

        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 63, 31, AlignCenter, AlignCenter, buffer);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 93, 37, "ppm");
    }
}
