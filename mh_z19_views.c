#include "mh_z19_views.h"
#include "mh_z19_app_i.h"
#include "mh_z19_uart_icons.h"

#include <gui/elements.h>

struct MhZ19MainView {
    View* view;
};

struct MhZ19AlertView {
    View* view;
};

typedef struct {
    uint32_t ppm;
    bool is_warming_up;
    uint32_t elapsed_time_ms;
    uint32_t warmup_duration_ms;
} MhZ19MainViewModel;

typedef struct {
    // Even though we don't need data for the alert view,
    // we need at least one member to ensure the struct has size > 0
    bool dummy;
} MhZ19AlertViewModel;

static void mh_z19_main_view_draw_callback(Canvas* canvas, void* _model) {
    MhZ19MainViewModel* model = _model;
    canvas_clear(canvas);

    canvas_set_bitmap_mode(canvas, 1);

    // If sensor is warming up, show a message
    if(model->is_warming_up) {
        uint32_t elapsed_time_sec = model->elapsed_time_ms / 1000;
        uint32_t remaining_time_sec = (model->warmup_duration_ms / 1000) - elapsed_time_sec;
        
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 63, 22, AlignCenter, AlignCenter, "Warming up...");
        
        static char time_buf[20] = {0};
        snprintf(time_buf, sizeof(time_buf), "%lus remaining", remaining_time_sec);
        canvas_draw_str_aligned(canvas, 63, 37, AlignCenter, AlignCenter, time_buf);
        
        // Draw progress bar
        float progress = (float)elapsed_time_sec / (model->warmup_duration_ms / 1000);
        uint8_t bar_width = 100;
        uint8_t filled_width = (uint8_t)(progress * bar_width);
        
        canvas_draw_frame(canvas, 64 - (bar_width/2), 47, bar_width, 8);
        if(filled_width > 0) {
            canvas_draw_box(canvas, 64 - (bar_width/2), 47, filled_width, 8);
        }
    } else {
        // Show CO2 ppm value
        static char buffer[5] = {0};
        snprintf(buffer, sizeof(buffer), "%lu", model->ppm);

        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 63, 31, AlignCenter, AlignCenter, buffer);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 93, 37, "ppm");
    }
}

static void mh_z19_alert_view_draw_callback(Canvas* canvas, void* _model) {
    UNUSED(_model);
    canvas_clear(canvas);

    canvas_set_bitmap_mode(canvas, 1);

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
}

static bool mh_z19_main_view_input_callback(InputEvent* event, void* context) {
    UNUSED(context);
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        return false; // Let the scene manager handle back button
    }
    return false;
}

static bool mh_z19_alert_view_input_callback(InputEvent* event, void* context) {
    UNUSED(context);
    if(event->type == InputTypeShort || event->type == InputTypeLong) {
        if(event->key == InputKeyBack) {
            return false; // Let the scene manager handle back button (both short and long presses)
        }
    }
    return false; // Don't consume any inputs to allow exiting the app
}

MhZ19MainView* mh_z19_main_view_alloc() {
    MhZ19MainView* main_view = malloc(sizeof(MhZ19MainView));
    main_view->view = view_alloc();

    view_allocate_model(main_view->view, ViewModelTypeLocking, sizeof(MhZ19MainViewModel));
    view_set_context(main_view->view, main_view);
    view_set_draw_callback(main_view->view, mh_z19_main_view_draw_callback);
    view_set_input_callback(main_view->view, mh_z19_main_view_input_callback);

    MhZ19MainViewModel* model = view_get_model(main_view->view);
    model->ppm = 0;
    model->is_warming_up = false;
    model->elapsed_time_ms = 0;
    model->warmup_duration_ms = 180000; // 3 minutes from datasheet
    view_commit_model(main_view->view, false);

    return main_view;
}

void mh_z19_main_view_free(MhZ19MainView* main_view) {
    furi_assert(main_view);
    view_free(main_view->view);
    free(main_view);
}

View* mh_z19_main_view_get_view(MhZ19MainView* main_view) {
    furi_assert(main_view);
    return main_view->view;
}

void mh_z19_main_view_set_data(MhZ19MainView* main_view, uint32_t ppm) {
    furi_assert(main_view);
    with_view_model(main_view->view, MhZ19MainViewModel * model, { model->ppm = ppm; }, true);
}

void mh_z19_main_view_set_warming_up(
    MhZ19MainView* main_view, bool is_warming_up, uint32_t elapsed_time_ms, uint32_t warmup_duration_ms) {
    furi_assert(main_view);
    with_view_model(
        main_view->view,
        MhZ19MainViewModel * model,
        {
            model->is_warming_up = is_warming_up;
            model->elapsed_time_ms = elapsed_time_ms;
            model->warmup_duration_ms = warmup_duration_ms;
        },
        true);
}

MhZ19AlertView* mh_z19_alert_view_alloc() {
    MhZ19AlertView* alert_view = malloc(sizeof(MhZ19AlertView));
    alert_view->view = view_alloc();

    // For static views with no data, we can skip model allocation
    view_set_context(alert_view->view, alert_view);
    view_set_draw_callback(alert_view->view, mh_z19_alert_view_draw_callback);
    view_set_input_callback(alert_view->view, mh_z19_alert_view_input_callback);

    return alert_view;
}

void mh_z19_alert_view_free(MhZ19AlertView* alert_view) {
    furi_assert(alert_view);
    view_free(alert_view->view);
    free(alert_view);
}

View* mh_z19_alert_view_get_view(MhZ19AlertView* alert_view) {
    furi_assert(alert_view);
    return alert_view->view;
}
