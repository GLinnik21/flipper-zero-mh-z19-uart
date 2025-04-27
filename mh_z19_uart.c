#include "mh_z19_uart.h"
#include <furi_hal_power.h>
#include <furi_hal.h>

#include "mh_z19_app_i.h"
#include "mh_z19_uart_tools.h"

static void mh_z19_app_uart_power_enable(MhZ19PowerData* power_data) {
    power_data->otg_was_previously_enabled = furi_hal_power_is_otg_enabled();
    furi_hal_power_enable_otg();
    power_data->is_5V_enabled = furi_hal_power_is_otg_enabled() || furi_hal_power_is_charging();
}

static void mh_z19_app_uart_power_restore(MhZ19PowerData* power_data) {
    if(power_data->is_5V_enabled && !power_data->otg_was_previously_enabled) {
        furi_hal_power_disable_otg();
    }
}

void mh_z19_app_uart_init(MhZ19App* app) {
    app->uart.state = MhZ19UartStateWaitStart;

    mh_z19_app_uart_power_enable(&(app->power_data));

    // Get handle for the standard USART port (USART1 = FuriHalSerialIdUsart)
    app->uart.handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);

    if(app->uart.handle) {
        // Initialize serial with MH-Z19 baudrate
        furi_hal_serial_init(app->uart.handle, MH_Z19_BAUDRATE);

        // Start async RX with our callback
        furi_hal_serial_async_rx_start(app->uart.handle, mh_z19_app_uart_callback, app, true);
    }
}

void mh_z19_app_uart_deinit(MhZ19App* app) {
    if(app->uart.handle) {
        // Stop async RX
        furi_hal_serial_async_rx_stop(app->uart.handle);

        // Deinitialize serial
        furi_hal_serial_deinit(app->uart.handle);

        // Release the serial handle
        furi_hal_serial_control_release(app->uart.handle);
        app->uart.handle = NULL;
    }

    mh_z19_app_uart_power_restore(&(app->power_data));
}

void mh_z19_app_uart_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    furi_assert(context);
    MhZ19App* app = context;

    if(event == FuriHalSerialRxEventData && furi_hal_serial_async_rx_available(handle)) {
        uint8_t data = furi_hal_serial_async_rx(handle);

        switch(app->uart.state) {
        case MhZ19UartStateWaitStart:
            if(data == MH_Z19_START_BYTE) {
                furi_stream_buffer_send(app->uart.rx_stream, &data, 1, 0);
                app->uart.state = MhZ19UartStateCollectPacket;
            }
            break;

        case MhZ19UartStateCollectPacket:
            furi_stream_buffer_send(app->uart.rx_stream, &data, 1, 0);

            static size_t byte_count = 0;
            furi_mutex_acquire(app->thread_data.mutex, FuriWaitForever);
            furi_mutex_release(app->thread_data.mutex);
            byte_count = furi_stream_buffer_bytes_available(app->uart.rx_stream);

            if(data == MH_Z19_START_BYTE && byte_count != 1) {
                furi_stream_buffer_reset(app->uart.rx_stream);
                furi_stream_buffer_send(app->uart.rx_stream, &data, 1, 0);
            } else if(byte_count == MH_Z19_COMMAND_SIZE) {
                furi_thread_flags_set(
                    furi_thread_get_id(app->thread_data.worker_thread), WorkerEventReserved);
                app->uart.state = MhZ19UartStateWaitStart;
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
            furi_mutex_acquire(app->thread_data.mutex, FuriWaitForever);
            length = furi_stream_buffer_receive(app->uart.rx_stream, data, MH_Z19_COMMAND_SIZE, 0);
            if(length == MH_Z19_COMMAND_SIZE) {
                app->ppm = mh_z19_decode_co2_concentration(data);
            }
            furi_mutex_release(app->thread_data.mutex);
        }
    }
    return 0;
}
