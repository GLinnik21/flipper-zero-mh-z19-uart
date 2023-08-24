#include "hello_world_uart.h"
#include <furi_hal_uart.h>
#include <furi_hal_console.h>

#include "mh_z19_uart_tools.h"

void uart_init(HelloWorldContext* context) {
    context->uart_state = HelloWorldUartStateWaitStart;
    furi_hal_console_disable();
    furi_hal_uart_deinit((context->uart_channel = FuriHalUartIdUSART1));
    furi_hal_uart_init(context->uart_channel, MH_Z19_BAUDRATE);
    furi_hal_uart_set_irq_cb(context->uart_channel, uart_callback, context);
}

void uart_deinit(HelloWorldContext* context) {
    furi_hal_console_enable();
    furi_hal_uart_deinit(context->uart_channel);
}

void uart_callback(UartIrqEvent event, uint8_t data, void* context) {
    furi_assert(context);
    HelloWorldContext* app = context;

    if(event == UartIrqEventRXNE) {
        switch(app->uart_state) {
        case HelloWorldUartStateWaitStart:
            if(data == MH_Z19_START_BYTE) {
                furi_stream_buffer_send(app->rx_stream, &data, 1, 0);
                app->uart_state = HelloWorldUartStateCollectPacket;
            }
            break;

        case HelloWorldUartStateCollectPacket:
            furi_stream_buffer_send(app->rx_stream, &data, 1, 0);

            static size_t byte_count = 0;
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            byte_count = furi_stream_buffer_bytes_available(app->rx_stream);
            furi_mutex_release(app->mutex);

            if(data == MH_Z19_START_BYTE && byte_count != 1) {
                furi_stream_buffer_reset(app->rx_stream);
                furi_stream_buffer_send(app->rx_stream, &data, 1, 0);
            } else if(byte_count == MH_Z19_COMMAND_SIZE) {
                furi_thread_flags_set(furi_thread_get_id(app->worker_thread), WorkerEventReserved);
                app->uart_state = HelloWorldUartStateWaitStart;
            }
            break;
        }
    }
}

int32_t uart_listener_worker(void* context) {
    HelloWorldContext* app = context;
    static uint8_t data[9] = {0};
    static size_t length = 0;

    while(1) {
        uint32_t flags = furi_thread_flags_wait(
            WorkerEventStop | WorkerEventReserved, FuriFlagWaitAny, FuriWaitForever);
        if(flags & WorkerEventStop) {
            break;
        }
        if(flags & WorkerEventReserved) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            length = furi_stream_buffer_receive(app->rx_stream, data, MH_Z19_COMMAND_SIZE, 0);
            if(length == MH_Z19_COMMAND_SIZE) {
                app->ppm = mh_z19_decode_co2_concentration(data);
            }
            furi_mutex_release(app->mutex);
        }
    }
    return 0;
}
