#pragma once

#include <gui/scene_manager.h>

typedef enum {
    MhZ19SceneMain,
    MhZ19SceneAlert,
    MhZ19SceneCount,
} MhZ19Scene;

extern const SceneManagerHandlers mh_z19_scene_handlers;

void mh_z19_scene_main_on_enter(void* context);
bool mh_z19_scene_main_on_event(void* context, SceneManagerEvent event);
void mh_z19_scene_main_on_exit(void* context);

void mh_z19_scene_alert_on_enter(void* context);
bool mh_z19_scene_alert_on_event(void* context, SceneManagerEvent event);
void mh_z19_scene_alert_on_exit(void* context);
