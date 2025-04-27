#include "mh_z19_uart_tools.h"

#include <furi.h>
#include <string.h>

#define MH_Z19_SENSOR_BYTE (0x01U)

uint8_t mh_z19_uart_checksum(const uint8_t* packet) {
    uint8_t checksum = 0x00;
    for(uint8_t i = 1; i < 8; i++) {
        checksum += packet[i];
    }
    return 0xFF - checksum + 1;
}

void mh_z19_uart_send_arbitrary_command(
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

    command[8] = mh_z19_uart_checksum(command);
}

void mh_z19_uart_read_co2(uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandCO2Concentraion;
    command[8] = mh_z19_uart_checksum(command);
}

void mh_z19_uart_calibrate_zero(uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandCalibrateZERO;
    command[8] = mh_z19_uart_checksum(command);
}

void mh_z19_uart_calibrate_span(uint16_t value, uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandCalibrateSPAN;
    command[3] = (uint8_t)(value >> 8);
    command[4] = (uint8_t)value;
    command[8] = mh_z19_uart_checksum(command);
}

void mh_z19_uart_set_autocalibration(bool enable, uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandOnOffAutoCalibration;
    command[3] = enable ? 0xA0 : 0x00;
    command[8] = mh_z19_uart_checksum(command);
}

void mh_z19_uart_switch_detection_range(MhZ19DetectionRange range, uint8_t* command) {
    memset(command, 0, 9);
    command[0] = MH_Z19_START_BYTE;
    command[1] = MH_Z19_SENSOR_BYTE;
    command[2] = MhZ19UartCommandSwitchDetectionRange;
    command[6] = (uint8_t)(range >> 8);
    command[7] = (uint8_t)range;
    command[8] = mh_z19_uart_checksum(command);
}

int16_t mh_z19_decode_co2_concentration(const uint8_t* data) {
    // Debug the received packet
    FURI_LOG_I(
        "MH-Z19",
        "Received: %02X %02X %02X %02X %02X %02X %02X %02X %02X", 
        data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
    
    // Check for valid packet format
    if(data[0] != MH_Z19_START_BYTE) {
        FURI_LOG_E("MH-Z19", "Invalid start byte: %02X, expected %02X", data[0], MH_Z19_START_BYTE);
        return -3; // Invalid start byte
    }
    
    // For CO2 reading responses, byte 1 should be the command code 0x86
    if(data[1] != MhZ19UartCommandCO2Concentraion) {
        FURI_LOG_E("MH-Z19", "Invalid command byte: %02X, expected %02X", data[1], MhZ19UartCommandCO2Concentraion);
        return -1; // Invalid command
    }
    
    // Calculate and verify checksum
    uint8_t calculated_checksum = mh_z19_uart_checksum(data);
    if(data[8] != calculated_checksum) {
        FURI_LOG_E("MH-Z19", "Checksum mismatch: %02X != %02X", data[8], calculated_checksum);
        return -2; // Invalid checksum
    }
    
    // According to MH-Z19 datasheet, response format for CO2 reading is:
    // Byte 0: 0xFF (start byte)
    // Byte 1: 0x86 (command - same as request)
    // Byte 2: High byte of CO2 value
    // Byte 3: Low byte of CO2 value
    // Byte 4-7: Other data
    // Byte 8: Checksum
    
    // CO2 value is in high byte (data[2]) and low byte (data[3])
    uint16_t co2_value = (data[2] << 8) | data[3];
    FURI_LOG_I("MH-Z19", "Decoded CO2: %d ppm", co2_value);
    
    return co2_value;
}
