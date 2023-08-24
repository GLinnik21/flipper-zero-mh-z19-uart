#pragma once

#include <sys/types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MH_Z19_BAUDRATE (9600U)
#define MH_Z19_COMMAND_SIZE (9U)
#define MH_Z19_START_BYTE (0xFFU)

typedef enum {
    MhZ19UartCommandCO2Concentraion = 0x86,
    MhZ19UartCommandCalibrateZERO = 0x87,
    MhZ19UartCommandCalibrateSPAN = 0x88,
    MhZ19UartCommandOnOffAutoCalibration = 0x79,
    MhZ19UartCommandSwitchDetectionRange = 0x99,
} MhZ19UartCommand;

typedef enum {
    MhZ19DetectionRange2000ppm = 2000,
    MhZ19DetectionRange5000ppm = 5000
} MhZ19DetectionRange;

/**
 * @brief Calculate checksum for a given packet.
 * 
 * @param packet Pointer to the packet of size 9 bytes.
 * @return uint8_t Calculated checksum.
 */
uint8_t mh_z19_uart_checksum(const uint8_t* packet);

/**
 * @brief Prepare an arbitrary command to be sent to the sensor.
 * 
 * This function allows you to send a custom command to the sensor. Ensure you understand the
 * sensor's protocol and command structure before using this function.
 * 
 * @param cmd The command byte to be sent.
 * @param data Pointer to the data bytes (up to 6 bytes) to be included in the command.
 * @param data_length Length of the data to be included (should be <= 6).
 * @param command Pointer to the command buffer of size 9 bytes where the final command will be stored.
 */
void mh_z19_uart_send_arbitrary_command(
    uint8_t cmd,
    const uint8_t* data,
    uint8_t data_length,
    uint8_t* command);

/**
 * @brief Prepare command to read CO2 concentration.
 * 
 * @param command Pointer to the command buffer of size 9 bytes.
 */
void mh_z19_uart_read_co2(uint8_t* command);

/**
 * @brief Prepare command to calibrate zero point.
 * 
 * @note: ZERO point is 400PPM. Ensure the sensor had been worked under 400PPM for over 20 minutes.
 * 
 * @param command Pointer to the command buffer of size 9 bytes.
 */
void mh_z19_uart_calibrate_zero(uint8_t* command);

/**
 * @brief Prepare command to calibrate span point.
 * 
 * @note: Please do ZERO calibration before span calibration.
 * Ensure the sensor worked under a certain level of CO2 for over 20 minutes.
 * Suggest using 2000ppm as span, at least 1000ppm.
 * 
 * @param value Span calibration value.
 * @param command Pointer to the command buffer of size 9 bytes.
 */
void mh_z19_uart_calibrate_span(uint16_t value, uint8_t* command);

/**
 * @brief Prepare command to enable or disable auto-calibration.
 * 
 * @note: Enabled by default by the manufacturer.
 * 
 * @param enable Boolean value to enable (true) or disable (false) auto-calibration.
 * @param command Pointer to the command buffer of size 9 bytes.
 */
void mh_z19_uart_set_autocalibration(bool enable, uint8_t* command);

/**
 * @brief Prepare command to switch detection range.
 * 
 * @param range Detection range value (either 2000ppm or 5000ppm).
 * @param command Pointer to the command buffer of size 9 bytes.
 */
void mh_z19_uart_switch_detection_range(MhZ19DetectionRange range, uint8_t* command);

/**
 * @brief Decode CO2 concentration from received data.
 * 
 * @param data Pointer to the received data of size 9 bytes.
 * @return int16_t Decoded CO2 concentration. Returns -1 for invalid command and -2 for invalid checksum.
 */
int16_t mh_z19_decode_co2_concentration(const uint8_t* data);

#ifdef __cplusplus
}
#endif