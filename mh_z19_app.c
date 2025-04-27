
#include <gui/gui.h>
#include <furi_hal_serial.h>

#include "mh_z19_app_i.h"
#include "mh_z19_uart.h"
#include "mh_z19_views.h"
#include "mh_z19_scenes.h"

#include "mh_z19_uart_tools.h"

#define MH_Z19_APP_POLL_INTERVAL_MS (5000U) // 5 seconds

static bool mh_z19_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    MhZ19App* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool mh_z19_app_back_event_callback(void* context) {
    furi_assert(context);
    MhZ19App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void mh_z19_app_tick_event_callback(void* context) {
    furi_assert(context);
    MhZ19App* app = context;

    uint32_t current_time = furi_get_tick();

    // We're removing the warmup period since it doesn't affect sensor readings
    if(app->sensor_data.is_warming_up) {
        app->sensor_data.is_warming_up = false;
        mh_z19_main_view_set_warming_up(app->main_view, false, 0, 0);
        FURI_LOG_I("MH-Z19", "Skipping warmup waiting");
    }

    // Send CO2 read command every interval
    static uint8_t data[9] = {0};
    static uint32_t last_read_time = 0;

    if(current_time - last_read_time > MH_Z19_APP_POLL_INTERVAL_MS) {
        // Only send command if UART is initialized and 5V power is available
        if(app->uart.handle != NULL && app->power_data.is_5V_enabled) {
            mh_z19_uart_read_co2(data);
            furi_hal_serial_tx(app->uart.handle, data, sizeof(data));
            FURI_LOG_D("MH-Z19", "Sent CO2 read command");
        }
        last_read_time = current_time;
    }

    // Update view with current PPM value (warmup check removed)
    mh_z19_main_view_set_data(app->main_view, app->ppm);

    // Check 5V power status and show alert if needed
    mh_z19_app_uart_check_power(&(app->power_data));

    // If we're in the main scene and 5V is disabled, switch to alert
    if(!app->power_data.is_5V_enabled &&
       scene_manager_get_current_scene(app->scene_manager) == MhZ19SceneMain) {
        scene_manager_next_scene(app->scene_manager, MhZ19SceneAlert);
    }
    // If we're in the alert scene and 5V is enabled, return to main
    else if(
        app->power_data.is_5V_enabled &&
        scene_manager_get_current_scene(app->scene_manager) == MhZ19SceneAlert) {
        scene_manager_previous_scene(app->scene_manager);
    }

    scene_manager_handle_tick_event(app->scene_manager);
}

MhZ19App* mh_z19_app_init() {
    MhZ19App* app = (MhZ19App*)malloc(sizeof(MhZ19App));

    // Initialize event queue for UART messages
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Initialize GUI management
    app->gui = furi_record_open(RECORD_GUI);

    // Initialize view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, mh_z19_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, mh_z19_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, mh_z19_app_tick_event_callback, 100);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Initialize scene manager
    app->scene_manager = scene_manager_alloc(&mh_z19_scene_handlers, app);

    // Allocate views
    app->main_view = mh_z19_main_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, MhZ19ViewMain, mh_z19_main_view_get_view(app->main_view));

    app->alert_view = mh_z19_alert_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, MhZ19ViewAlert, mh_z19_alert_view_get_view(app->alert_view));

    // Initialize UART
    app->uart.rx_stream = furi_stream_buffer_alloc(126, 1);
    app->thread_data.mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->thread_data.worker_thread =
        furi_thread_alloc_ex("UARTListenerWorker", 1024, mh_z19_app_uart_listener_worker, app);

    // Initialize sensor data
    app->ppm = 0;
    app->sensor_data.is_warming_up = false; // Skip warmup waiting
    
    // No need to set warming up view since we're skipping it

    // Initialize UART after we've set up the stream buffer
    mh_z19_app_uart_init(app);

    furi_thread_start(app->thread_data.worker_thread);

    return app;
}

void mh_z19_app_free(MhZ19App* app) {
    furi_assert(app);

    // Stop UART
    furi_thread_flags_set(furi_thread_get_id(app->thread_data.worker_thread), WorkerEventStop);
    furi_thread_join(app->thread_data.worker_thread);
    furi_thread_free(app->thread_data.worker_thread);

    furi_mutex_free(app->thread_data.mutex);
    furi_stream_buffer_free(app->uart.rx_stream);
    mh_z19_app_uart_deinit(app);

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, MhZ19ViewMain);
    mh_z19_main_view_free(app->main_view);

    view_dispatcher_remove_view(app->view_dispatcher, MhZ19ViewAlert);
    mh_z19_alert_view_free(app->alert_view);

    // Free scene manager and view dispatcher
    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    // Close GUI record
    furi_record_close(RECORD_GUI);

    // Free message queue
    furi_message_queue_free(app->event_queue);

    free(app);
}

int32_t mh_z19_app(void* p) {
    UNUSED(p);

    MhZ19App* app = mh_z19_app_init();

    // Start with main scene
    scene_manager_next_scene(app->scene_manager, MhZ19SceneMain);

    // Run the view dispatcher
    view_dispatcher_run(app->view_dispatcher);

    // Clean up
    mh_z19_app_free(app);

    return 0;
}
