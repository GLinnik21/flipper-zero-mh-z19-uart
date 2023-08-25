
#include <gui/gui.h>
#include <furi_hal_uart.h>
#include <furi_hal_console.h>

#include "mh_z19_app_i.h"
#include "mh_z19_ui.h"
#include "mh_z19_uart.h"

#include "mh_z19_uart_tools.h"

static void mh_z19_app_run(MhZ19App* const context) {
    for(bool isRunning = true; isRunning;) {
        InputEvent event;
        const FuriStatus status =
            furi_message_queue_get(context->event_queue, &event, FuriWaitForever);

        if((status != FuriStatusOk) || (event.type != InputTypeShort)) {
            continue;
        }

        static uint8_t data[9] = {0};
        mh_z19_uart_read_co2(data);

        switch(event.key) {
        case InputKeyBack:
            isRunning = false;
            break;
        case InputKeyOk:
            furi_hal_uart_tx(context->uart_channel, data, sizeof(data));
            break;
        default:
            break;
        }
        view_port_update(context->view_port);
    }
}

MhZ19App* mh_z19_app_context_init() {
    MhZ19App* context = (MhZ19App*)malloc(sizeof(MhZ19App));

    context->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    context->view_port = view_port_alloc();
    view_port_draw_callback_set(context->view_port, draw_callback, context);
    view_port_input_callback_set(context->view_port, input_callback, context);

    context->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(context->gui, context->view_port, GuiLayerFullscreen);

    uart_init(context);

    context->rx_stream = furi_stream_buffer_alloc(126, 1);

    context->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    context->worker_thread =
        furi_thread_alloc_ex("UARTListenerWorker", 1024, uart_listener_worker, context);
    furi_thread_start(context->worker_thread);

    context->ppm = 0;

    return context;
}

void mh_z19_app_context_free(MhZ19App* context) {
    furi_thread_flags_set(furi_thread_get_id(context->worker_thread), WorkerEventStop);
    furi_thread_join(context->worker_thread);
    furi_thread_free(context->worker_thread);

    furi_mutex_free(context->mutex);

    furi_stream_buffer_free(context->rx_stream);

    uart_deinit(context);

    furi_record_close(RECORD_GUI);

    gui_remove_view_port(context->gui, context->view_port);
    view_port_free(context->view_port);

    furi_message_queue_free(context->event_queue);

    free(context);
}

int32_t mh_z19_app(void* p) {
    UNUSED(p);

    MhZ19App* context = mh_z19_app_context_init();
    mh_z19_app_run(context);
    mh_z19_app_context_free(context);

    return 0;
}
