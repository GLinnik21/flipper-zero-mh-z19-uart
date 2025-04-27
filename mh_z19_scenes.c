#include "mh_z19_scenes.h"
#include "mh_z19_app_i.h"

// Scene Main handlers
void mh_z19_scene_main_on_enter(void* context) {
    MhZ19App* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, MhZ19ViewMain);

    // Check if 5V power is available
    if(!app->power_data.is_5V_enabled) {
        scene_manager_next_scene(app->scene_manager, MhZ19SceneAlert);
    }
}

bool mh_z19_scene_main_on_event(void* context, SceneManagerEvent event) {
    MhZ19App* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MhZ19SceneAlert) {
            scene_manager_next_scene(app->scene_manager, MhZ19SceneAlert);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        // Back button pressed, exit application
        view_dispatcher_stop(app->view_dispatcher);
        return true;
    }

    return false;
}

void mh_z19_scene_main_on_exit(void* context) {
    UNUSED(context);
    // Nothing to do on exit
}

// Scene Alert handlers
void mh_z19_scene_alert_on_enter(void* context) {
    MhZ19App* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, MhZ19ViewAlert);
}

bool mh_z19_scene_alert_on_event(void* context, SceneManagerEvent event) {
    MhZ19App* app = context;

    if(event.type == SceneManagerEventTypeBack) {
        // Back button pressed, exit the app when in alert view
        view_dispatcher_stop(app->view_dispatcher);
        return true;
    }

    return false;
}

void mh_z19_scene_alert_on_exit(void* context) {
    UNUSED(context);
    // Nothing to do on exit
}

// Handler arrays
static const AppSceneOnEnterCallback mh_z19_on_enter_handlers[] = {
    mh_z19_scene_main_on_enter,
    mh_z19_scene_alert_on_enter,
};

static const AppSceneOnEventCallback mh_z19_on_event_handlers[] = {
    mh_z19_scene_main_on_event,
    mh_z19_scene_alert_on_event,
};

static const AppSceneOnExitCallback mh_z19_on_exit_handlers[] = {
    mh_z19_scene_main_on_exit,
    mh_z19_scene_alert_on_exit,
};

// Scene manager handler registration
const SceneManagerHandlers mh_z19_scene_handlers = {
    .on_enter_handlers = mh_z19_on_enter_handlers,
    .on_event_handlers = mh_z19_on_event_handlers,
    .on_exit_handlers = mh_z19_on_exit_handlers,
    .scene_num = MhZ19SceneCount,
};
