#include "mh_z19_scene.h"

// Array of on_enter handlers
AppSceneOnEnterCallback mh_z19_scene_on_enter_handlers[MhZ19AppSceneCount] = {
    mh_z19_scene_setup_on_enter,
    mh_z19_scene_main_on_enter,
    mh_z19_scene_menu_on_enter};

// Array of on_event handlers
AppSceneOnEventCallback mh_z19_scene_on_event_handlers[MhZ19AppSceneCount] = {
    mh_z19_scene_setup_on_event,
    mh_z19_scene_main_on_event,
    mh_z19_scene_menu_on_event};

// Array of on_exit handlers
AppSceneOnExitCallback mh_z19_scene_on_exit_handlers[MhZ19AppSceneCount] = {
    mh_z19_scene_setup_on_exit,
    mh_z19_scene_main_on_exit,
    mh_z19_scene_menu_on_exit};

const SceneManagerHandlers mh_z19_scene_handlers = {
    .on_enter_handlers = mh_z19_scene_on_enter_handlers,
    .on_event_handlers = mh_z19_scene_on_event_handlers,
    .on_exit_handlers = mh_z19_scene_on_exit_handlers,
    .scene_num = MhZ19AppSceneCount};
