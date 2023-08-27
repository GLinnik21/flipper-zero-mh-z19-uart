
#include <gui/gui.h>
#include <furi_hal_uart.h>
#include <furi_hal_console.h>

#include "mh_z19_app_i.h"
#include "mh_z19_ui.h"
#include "mh_z19_uart.h"

#include "mh_z19_uart_tools.h"

#define MH_Z19_APP_POLL_INTERVAL_MS (5000U) // 5 seconds

static void mh_z19_app_run(MhZ19App* const app) {
    for(bool isRunning = true; isRunning;) {
        InputEvent event;
        const FuriStatus status =
            furi_message_queue_get(app->event_queue, &event, MH_Z19_APP_POLL_INTERVAL_MS);

        static uint8_t data[9] = {0};
        mh_z19_uart_read_co2(data);
        furi_hal_uart_tx(app->uart.channel, data, sizeof(data));

        if((status == FuriStatusOk) && (event.type = InputTypeShort)) {
            switch(event.key) {
            case InputKeyBack:
                isRunning = false;
                break;
            default:
                break;
            }
        }
        view_port_update(app->gui_data.view_port);
    }
}

MhZ19App* mh_z19_app_init() {
    MhZ19App* app = (MhZ19App*)malloc(sizeof(MhZ19App));

    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    app->gui_data.view_port = view_port_alloc();
    view_port_draw_callback_set(app->gui_data.view_port, mh_z19_app_draw_callback, app);
    view_port_input_callback_set(app->gui_data.view_port, mh_z19_app_input_callback, app);

    app->gui_data.gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui_data.gui, app->gui_data.view_port, GuiLayerFullscreen);

    mh_z19_app_uart_init(app);

    app->uart.rx_stream = furi_stream_buffer_alloc(126, 1);

    app->thread_data.mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    app->thread_data.worker_thread =
        furi_thread_alloc_ex("UARTListenerWorker", 1024, mh_z19_app_uart_listener_worker, app);
    furi_thread_start(app->thread_data.worker_thread);

    app->ppm = 0;

    return app;
}

void mh_z19_app_free(MhZ19App* app) {
    furi_thread_flags_set(furi_thread_get_id(app->thread_data.worker_thread), WorkerEventStop);
    furi_thread_join(app->thread_data.worker_thread);
    furi_thread_free(app->thread_data.worker_thread);

    furi_mutex_free(app->thread_data.mutex);

    furi_stream_buffer_free(app->uart.rx_stream);

    mh_z19_app_uart_deinit(app);

    furi_record_close(RECORD_GUI);

    gui_remove_view_port(app->gui_data.gui, app->gui_data.view_port);
    view_port_free(app->gui_data.view_port);

    furi_message_queue_free(app->event_queue);

    free(app);
}

int32_t mh_z19_app(void* p) {
    UNUSED(p);

    MhZ19App* app = mh_z19_app_init();
    mh_z19_app_run(app);
    mh_z19_app_free(app);

    return 0;
}
