#pragma once

#include <furi.h>
#include <furi_hal_serial.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include "mh_z19_app.h"
#include "mh_z19_scenes.h"
#include "mh_z19_views.h"

typedef enum MhZ19UartState {
    MhZ19UartStateWaitStart,
    MhZ19UartStateCollectPacket,
} MhZ19UartState;

typedef struct MhZ19Uart {
    FuriHalSerialHandle* handle;
    MhZ19UartState state;
    FuriStreamBuffer* rx_stream;
} MhZ19Uart;

typedef struct MhZ19ThreadData {
    FuriMutex* mutex;
    FuriThread* worker_thread;
} MhZ19ThreadData;

typedef struct MhZ19PowerData {
    bool otg_was_previously_enabled;
    bool is_5V_enabled;
} MhZ19PowerData;

typedef struct MhZ19SensorData {
    bool is_warming_up; // Kept for compatibility but no longer used for waiting
} MhZ19SensorData;

typedef enum {
    MhZ19ViewMain,
    MhZ19ViewAlert,
} MhZ19View;

struct MhZ19App {
    // Scene management
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    // Views
    MhZ19MainView* main_view;
    MhZ19AlertView* alert_view;

    // UART and sensor data
    FuriMessageQueue* event_queue;
    uint32_t ppm;
    MhZ19Uart uart;
    MhZ19ThreadData thread_data;
    MhZ19PowerData power_data;
    MhZ19SensorData sensor_data;

    // Gui data (will be removed as we use view_dispatcher)
    Gui* gui;
};

typedef enum MhZ19WorkerEventFlags {
    WorkerEventReserved = (1 << 0),
    WorkerEventStop = (1 << 1),
} MhZ19WorkerEventFlags;

MhZ19App* mh_z19_app_init();
void mh_z19_app_free(MhZ19App* app);
int32_t mh_z19_app(void* p);
