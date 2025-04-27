#pragma once

#include <gui/view.h>

typedef struct MhZ19App MhZ19App;

typedef struct MhZ19MainView MhZ19MainView;
typedef struct MhZ19AlertView MhZ19AlertView;

MhZ19MainView* mh_z19_main_view_alloc();
void mh_z19_main_view_free(MhZ19MainView* main_view);
View* mh_z19_main_view_get_view(MhZ19MainView* main_view);
void mh_z19_main_view_set_data(MhZ19MainView* main_view, uint32_t ppm);
void mh_z19_main_view_set_warming_up(
    MhZ19MainView* main_view, bool is_warming_up, uint32_t elapsed_time_ms, uint32_t warmup_duration_ms);

MhZ19AlertView* mh_z19_alert_view_alloc();
void mh_z19_alert_view_free(MhZ19AlertView* alert_view);
View* mh_z19_alert_view_get_view(MhZ19AlertView* alert_view);
