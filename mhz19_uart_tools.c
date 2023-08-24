#include "mhz19_uart_tools.h"

#include <string.h>

#define MH_Z19_SENSOR_BYTE (0x01U)

uint8_t mhz19_uart_checksum(const uint8_t* packet) {
    uint8_t checksum = 0x00;
    for(uint8_t i = 1; i < 8; i++) {
        checksum += packet[i];
    }
    return 0xFF - checksum + 1;
}

void mhz19_uart_send_arbitrary_command(
    uint8_t cmd,
    const uint8_t* data,
    uint8_t data_length,
    uint8_t* command) {
    if(data_length > 6) {
        // Error: Data length exceeds the maximum allowed size.
        return;
    }

    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = cmd;

    for(uint8_t i = 0; i < data_length; i++) {
        command[3 + i] = data[i];
    }

    command[8] = mhz19_uart_checksum(command);
}

void mhz19_uart_read_co2(uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandCO2Concentraion;
    command[8] = mhz19_uart_checksum(command);
}

void mhz19_uart_calibrate_zero(uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandCalibrateZERO;
    command[8] = mhz19_uart_checksum(command);
}

void mhz19_uart_calibrate_span(uint16_t value, uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandCalibrateSPAN;
    command[3] = (uint8_t)(value >> 8);
    command[4] = (uint8_t)value;
    command[8] = mhz19_uart_checksum(command);
}

void mhz19_uart_set_autocalibration(bool enable, uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandOnOffAutoCalibration;
    command[3] = enable ? 0xA0 : 0x00;
    command[8] = mhz19_uart_checksum(command);
}

void mhz19_uart_switch_detection_range(MhZ19DetectionRange range, uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandSwitchDetectionRange;
    command[6] = (uint8_t)(range >> 8);
    command[7] = (uint8_t)range;
    command[8] = mhz19_uart_checksum(command);
}

int16_t mhz19_decode_co2_concentration(const uint8_t* data) {
    if(data[1] != MhZ19UartCommandCO2Concentraion) {
        return -1; // Invalid command
    }
    if(data[8] != mhz19_uart_checksum(data)) {
        return -2; // Invalid checksum
    }
    return (data[2] << 8) | data[3];
}
