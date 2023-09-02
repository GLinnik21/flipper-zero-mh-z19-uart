#include "mh_z19_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const mh_z19_scene_on_enter_handlers[])(void*) = {
#include "mh_z19_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const mh_z19_scene_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "mh_z19_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const mh_z19_scene_on_exit_handlers[])(void* context) = {
#include "mh_z19_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure

const SceneManagerHandlers mh_z19_scene_handlers = {
    .on_enter_handlers = mh_z19_scene_on_enter_handlers,
    .on_event_handlers = mh_z19_scene_on_event_handlers,
    .on_exit_handlers = mh_z19_scene_on_exit_handlers,
    .scene_num = MhZ19SceneCount};
