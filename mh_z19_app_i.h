#pragma once

#include <furi.h>
#include <furi_hal_uart.h>
#include <gui/gui.h>

#include "mh_z19_app.h"

typedef enum MhZ19UartState {
    MhZ19UartStateWaitStart,
    MhZ19UartStateCollectPacket,
} MhZ19UartState;

struct MhZ19App {
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    Gui* gui;
    FuriHalUartId uart_channel;
    MhZ19UartState uart_state;
    FuriStreamBuffer* rx_stream;
    FuriMutex* mutex;
    FuriThread* worker_thread;
    uint32_t ppm;
};

typedef enum MhZ19WorkerEventFlags {
    WorkerEventReserved = (1 << 0),
    WorkerEventStop = (1 << 1),
} MhZ19WorkerEventFlags;

MhZ19App* mh_z19_app_context_init();
void mh_z19_app_context_free(MhZ19App* context);
int32_t mh_z19_app(void* p);
