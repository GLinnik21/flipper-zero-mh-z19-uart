#pragma once

#include <furi.h>
#include <furi_hal_uart.h>
#include <gui/gui.h>

typedef enum HelloWorldUartState {
    HelloWorldUartStateWaitStart,
    HelloWorldUartStateCollectPacket,
} HelloWorldUartState;

typedef struct HelloWorldContext {
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    Gui* gui;
    FuriHalUartId uart_channel;
    HelloWorldUartState uart_state;
    FuriStreamBuffer* rx_stream;
    FuriMutex* mutex;
    FuriThread* worker_thread;
    uint32_t ppm;
} HelloWorldContext;

typedef enum WorkerEventFlags {
    WorkerEventReserved = (1 << 0),
    WorkerEventStop = (1 << 1),
} WorkerEventFlags;

HelloWorldContext* hello_world_app_context_init();
void hello_world_app_context_free(HelloWorldContext* context);
int32_t hello_world_app(void* p);
