#include "mh_z19_uart.h"
#include <furi_hal_console.h>
#include <furi_hal_power.h>

#include "mh_z19_app_i.h"
#include "mh_z19_uart_tools.h"

static void mh_z19_app_uart_enable_5V(MhZ19App* app) {
    app->otg_was_previously_enabled = furi_hal_power_is_otg_enabled();
    furi_hal_power_enable_otg();
    app->is_5V_enabled = furi_hal_power_is_otg_enabled() || furi_hal_power_is_charging();
}

static void mh_z19_app_uart_restore_5V_state(MhZ19App* app) {
    if(app->is_5V_enabled && !app->otg_was_previously_enabled) {
        furi_hal_power_disable_otg();
    }
}

void mh_z19_app_uart_init(MhZ19App* app) {
    app->uart_state = MhZ19UartStateWaitStart;
    furi_hal_console_disable();

    mh_z19_app_uart_enable_5V(app);

    furi_hal_uart_deinit((app->uart_channel = FuriHalUartIdUSART1));
    furi_hal_uart_init(app->uart_channel, MH_Z19_BAUDRATE);
    furi_hal_uart_set_irq_cb(app->uart_channel, mh_z19_app_uart_callback, app);
}

void mh_z19_app_uart_deinit(MhZ19App* app) {
    mh_z19_app_uart_restore_5V_state(app);
    furi_hal_console_enable();
    furi_hal_uart_deinit(app->uart_channel);
}

void mh_z19_app_uart_callback(UartIrqEvent event, uint8_t data, void* context) {
    furi_assert(context);
    MhZ19App* app = context;

    if(event == UartIrqEventRXNE) {
        switch(app->uart_state) {
        case MhZ19UartStateWaitStart:
            if(data == MH_Z19_START_BYTE) {
                furi_stream_buffer_send(app->rx_stream, &data, 1, 0);
                app->uart_state = MhZ19UartStateCollectPacket;
            }
            break;

        case MhZ19UartStateCollectPacket:
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
                app->uart_state = MhZ19UartStateWaitStart;
            }
            break;
        }
    }
}

int32_t mh_z19_app_uart_listener_worker(void* context) {
    MhZ19App* app = context;
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
