#pragma once

#include "mh_z19_app_i.h"

void uart_callback(UartIrqEvent event, uint8_t data, void* context);
int32_t uart_listener_worker(void* context);
void uart_init(MhZ19App* app);
void uart_deinit(MhZ19App* app);
