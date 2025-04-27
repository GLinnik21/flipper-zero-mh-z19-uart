#include "mh_z19_uart.h"
#include <furi_hal_power.h>
#include <furi_hal.h>

#include "mh_z19_app_i.h"
#include "mh_z19_uart_tools.h"

static void mh_z19_app_uart_power_enable(MhZ19PowerData* power_data) {
    power_data->otg_was_previously_enabled = furi_hal_power_is_otg_enabled();
    furi_hal_power_enable_otg();
    power_data->is_5V_enabled = furi_hal_power_is_otg_enabled() || furi_hal_power_is_charging();
    FURI_LOG_I("MH-Z19", "5V power: %s", power_data->is_5V_enabled ? "ON" : "OFF");
}

static void mh_z19_app_uart_power_restore(MhZ19PowerData* power_data) {
    if(power_data->is_5V_enabled && !power_data->otg_was_previously_enabled) {
        furi_hal_power_disable_otg();
    }
}

void mh_z19_app_uart_check_power(MhZ19PowerData* power_data) {
    bool was_enabled = power_data->is_5V_enabled;
    power_data->is_5V_enabled = furi_hal_power_is_otg_enabled() || furi_hal_power_is_charging();

    if(was_enabled != power_data->is_5V_enabled) {
        FURI_LOG_I("MH-Z19", "5V power changed: %s", power_data->is_5V_enabled ? "ON" : "OFF");
    }
}

void mh_z19_app_uart_init(MhZ19App* app) {
    app->uart.state = MhZ19UartStateWaitStart;

    mh_z19_app_uart_power_enable(&(app->power_data));

    if(app->power_data.is_5V_enabled) {
        app->uart.handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);

        if(app->uart.handle) {
            FURI_LOG_I("MH-Z19", "UART initialized, handle acquired");
            furi_hal_serial_init(app->uart.handle, MH_Z19_BAUDRATE);
            furi_hal_serial_async_rx_start(app->uart.handle, mh_z19_app_uart_callback, app, true);
        } else {
            FURI_LOG_E("MH-Z19", "Failed to acquire UART handle");
        }
    } else {
        FURI_LOG_E("MH-Z19", "Cannot initialize UART: 5V power is OFF");
        app->uart.handle = NULL;
    }
}

void mh_z19_app_uart_deinit(MhZ19App* app) {
    if(app->uart.handle) {
        FURI_LOG_I("MH-Z19", "Shutting down UART");
        furi_hal_serial_async_rx_stop(app->uart.handle);
        furi_hal_serial_deinit(app->uart.handle);
        furi_hal_serial_control_release(app->uart.handle);
        app->uart.handle = NULL;
    }

    mh_z19_app_uart_power_restore(&(app->power_data));
    FURI_LOG_I("MH-Z19", "UART deinitialized, power restored");
}

void mh_z19_app_uart_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    if(context == NULL) {
        FURI_LOG_E("MH-Z19", "UART callback with NULL context");
        return;
    }

    MhZ19App* app = context;

    if(!app->power_data.is_5V_enabled) {
        FURI_LOG_W("MH-Z19", "UART data received but 5V power is OFF");
        return;
    }

    if(event == FuriHalSerialRxEventData && furi_hal_serial_async_rx_available(handle)) {
        uint8_t data = furi_hal_serial_async_rx(handle);
        FURI_LOG_D("MH-Z19", "UART RX: 0x%02X", data);

        switch(app->uart.state) {
        case MhZ19UartStateWaitStart:
            if(data == MH_Z19_START_BYTE) {
                furi_mutex_acquire(app->thread_data.mutex, FuriWaitForever);
                furi_stream_buffer_reset(app->uart.rx_stream);
                furi_mutex_release(app->thread_data.mutex);

                furi_stream_buffer_send(app->uart.rx_stream, &data, 1, 0);
                app->uart.state = MhZ19UartStateCollectPacket;
                FURI_LOG_I("MH-Z19", "Packet start detected (0xFF)");
            }
            break;

        case MhZ19UartStateCollectPacket:
            furi_stream_buffer_send(app->uart.rx_stream, &data, 1, 0);
            size_t byte_count = furi_stream_buffer_bytes_available(app->uart.rx_stream);
            FURI_LOG_D("MH-Z19", "Collecting packet: byte %d = 0x%02X", byte_count, data);

            if(data == MH_Z19_START_BYTE && byte_count != 1) {
                FURI_LOG_W("MH-Z19", "Unexpected start byte in packet, restarting collection");
                furi_mutex_acquire(app->thread_data.mutex, FuriWaitForever);
                furi_stream_buffer_reset(app->uart.rx_stream);
                furi_stream_buffer_send(app->uart.rx_stream, &data, 1, 0);
                furi_mutex_release(app->thread_data.mutex);
            } else if(byte_count == MH_Z19_COMMAND_SIZE) {
                FURI_LOG_I(
                    "MH-Z19", "Complete packet received (%d bytes), processing", byte_count);
                furi_thread_flags_set(
                    furi_thread_get_id(app->thread_data.worker_thread), WorkerEventReserved);
                app->uart.state = MhZ19UartStateWaitStart;
            }
            break;
        }
    } else if(event == FuriHalSerialRxEventIdle) {
        FURI_LOG_D("MH-Z19", "UART idle event");
    } else if(
        event & (FuriHalSerialRxEventFrameError | FuriHalSerialRxEventNoiseError |
                 FuriHalSerialRxEventOverrunError | FuriHalSerialRxEventParityError)) {
        FURI_LOG_E("MH-Z19", "UART error event: 0x%02X", event);
    }
}

int32_t mh_z19_app_uart_listener_worker(void* context) {
    MhZ19App* app = context;
    static uint8_t data[9] = {0};
    static size_t length = 0;

    FURI_LOG_I("MH-Z19", "UART listener worker started");

    while(1) {
        uint32_t flags = furi_thread_flags_wait(
            WorkerEventStop | WorkerEventReserved, FuriFlagWaitAny, FuriWaitForever);

        if(flags & WorkerEventStop) {
            FURI_LOG_I("MH-Z19", "UART listener worker stopping");
            break;
        }

        if(flags & WorkerEventReserved) {
            FURI_LOG_D("MH-Z19", "Processing received packet");

            furi_mutex_acquire(app->thread_data.mutex, FuriWaitForever);
            length = furi_stream_buffer_receive(app->uart.rx_stream, data, MH_Z19_COMMAND_SIZE, 0);
            furi_mutex_release(app->thread_data.mutex);

            FURI_LOG_I("MH-Z19", "Read %d bytes from stream buffer", length);

            if(length == MH_Z19_COMMAND_SIZE) {
                int16_t ppm_value = mh_z19_decode_co2_concentration(data);

                if(ppm_value > 0) {
                    app->ppm = (uint32_t)ppm_value;
                    FURI_LOG_I("MH-Z19", "✅ Valid CO2 reading: %lu ppm", app->ppm);
                } else {
                    const char* error_msg = "";
                    switch(ppm_value) {
                    case -1:
                        error_msg = "Invalid command byte";
                        break;
                    case -2:
                        error_msg = "Checksum mismatch";
                        break;
                    case -3:
                        error_msg = "Invalid start byte";
                        break;
                    default:
                        error_msg = "Unknown decode error";
                        break;
                    }
                    FURI_LOG_E("MH-Z19", "❌ CO2 decode failed: %s (%d)", error_msg, ppm_value);
                }
            } else {
                FURI_LOG_E(
                    "MH-Z19", "❌ Incomplete packet received: %d bytes instead of 9", length);
            }
        }
    }
    return 0;
}
