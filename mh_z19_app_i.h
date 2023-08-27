#pragma once

#include <furi.h>
#include <furi_hal_uart.h>
#include <gui/gui.h>

#include "mh_z19_app.h"

typedef enum MhZ19UartState {
    MhZ19UartStateWaitStart,
    MhZ19UartStateCollectPacket,
} MhZ19UartState;

typedef struct MhZ19Uart {
    FuriHalUartId channel;
    MhZ19UartState state;
    FuriStreamBuffer* rx_stream;
} MhZ19Uart;

typedef struct MhZ19ThreadData {
    FuriMutex* mutex;
    FuriThread* worker_thread;
} MhZ19ThreadData;

typedef struct MhZ19GuiData {
    ViewPort* view_port;
    Gui* gui;
} MhZ19GuiData;

typedef struct MhZ19PowerData {
    bool otg_was_previously_enabled;
    bool is_5V_enabled;
} MhZ19PowerData;

struct MhZ19App {
    FuriMessageQueue* event_queue;
    uint32_t ppm;
    MhZ19Uart uart;
    MhZ19ThreadData thread_data;
    MhZ19GuiData gui_data;
    MhZ19PowerData power_data;
};

typedef enum MhZ19WorkerEventFlags {
    WorkerEventReserved = (1 << 0),
    WorkerEventStop = (1 << 1),
} MhZ19WorkerEventFlags;

MhZ19App* mh_z19_app_init();
void mh_z19_app_free(MhZ19App* app);
int32_t mh_z19_app(void* p);
