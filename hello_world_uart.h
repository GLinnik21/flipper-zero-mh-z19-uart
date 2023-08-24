#pragma once

#include "hello_world.h"

void uart_callback(UartIrqEvent event, uint8_t data, void* context);
int32_t uart_listener_worker(void* context);
void uart_init(HelloWorldContext* context);
void uart_deinit(HelloWorldContext* context);
