#pragma once

#include <furi_hal_uart.h>
#include "mh_z19_app.h"

void mh_z19_app_uart_callback(UartIrqEvent event, uint8_t data, void* context);
int32_t mh_z19_app_uart_listener_worker(void* context);
void mh_z19_app_uart_init(MhZ19App* app);
void mh_z19_app_uart_deinit(MhZ19App* app);
