#pragma once

#include <gui/scene_manager.h>

extern const SceneManagerHandlers mh_z19_scene_handlers;

// For MhZ19AppSceneSetup
void mh_z19_scene_setup_on_enter(void* context);
bool mh_z19_scene_setup_on_event(void* context, SceneManagerEvent event);
void mh_z19_scene_setup_on_exit(void* context);

// For MhZ19AppSceneMain
void mh_z19_scene_main_on_enter(void* context);
bool mh_z19_scene_main_on_event(void* context, SceneManagerEvent event);
void mh_z19_scene_main_on_exit(void* context);

// For MhZ19AppSceneMenu
void mh_z19_scene_menu_on_enter(void* context);
bool mh_z19_scene_menu_on_event(void* context, SceneManagerEvent event);
void mh_z19_scene_menu_on_exit(void* context);

typedef enum MhZ19AppScene {
    MhZ19AppSceneSetup,
    MhZ19AppSceneMain,
    MhZ19AppSceneMenu,
    MhZ19AppSceneCount,
} MhZ19AppScene;
