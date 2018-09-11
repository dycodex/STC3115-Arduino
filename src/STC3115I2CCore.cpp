#include "STC3115I2CCore.h"

/**
 * @brief Initialize STC3115 I2C driver and assign the address
 *
 * @param address
 */
STC3115I2CCore::STC3115I2CCore(uint8_t address):
address(address) {
}

STC3115I2CCore::~STC3115I2CCore() {
}

/**
 * @brief Initialize I2C and check whether the address is available or not
 *
 * @return true
 * @return false
 */
bool STC3115I2CCore::beginI2C() {
//    Wire.begin();

    //bool result = true;
    Wire.beginTransmission(address);

    //result = Wire.endTransmission() == 0;
    //return result;

    uint8_t result = Wire.endTransmission(false);

    return (result == 7);
}

/**
 * @brief Read an unsigned byte from a register and return the read status.
 *
 * @param output pointer to the variable that will hold the result
 * @param reg register address
 * @return true
 * @return false
 */
bool STC3115I2CCore::readRegister(uint8_t* output, uint8_t reg) {
    uint8_t result = 0;
    uint8_t numBytes = 1;
    bool returnValue = true;

    Wire.beginTransmission(address);
    Wire.write(reg);

    if (Wire.endTransmission() != 0) {
        returnValue = false;
    }

    Wire.requestFrom(address, numBytes);
    while (Wire.available()) {
        result = Wire.read();
    }

    *output = result;
    return returnValue;
}

/**
 * @brief Read unsigned bytes from register range
 *
 * @param output array that will hold the read result
 * @param reg register to start reading
 * @param length length of the bytes
 * @return true
 * @return false
 */
bool STC3115I2CCore::readRegisterRegion(uint8_t* output, uint8_t reg, uint8_t length) {
    bool returnValue = true;
    uint8_t counter = 0;
    uint8_t temp = 0;

    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        returnValue = false;
    } else {
        Wire.requestFrom(address, length);
        while (Wire.available() && (counter < length)) {
            temp = Wire.read();
            *output = temp;
            output++;
            counter++;
        }
    }

    return returnValue;
}

/**
 * @brief Read 2 bytes of data and convert it to a signed integer
 *
 * @param output pointer to the variable that will hold the result
 * @param reg register address
 * @return true
 * @return false
 */
bool STC3115I2CCore::readRegisterInt16(int16_t* output, uint8_t reg) {
    uint8_t buffer[2] = {0};
    bool status = readRegisterRegion(buffer, reg, 2);
    *output = static_cast<int16_t>(buffer[0]) | static_cast<int16_t>(buffer[1]) << 8;

    return status;
}

/**
 * @brief Write unsigned byte data to a register
 *
 * @param reg register address
 * @param data data to be written
 * @return true
 * @return false
 */
bool STC3115I2CCore::writeRegister(uint8_t reg, uint8_t data) {
    bool returnValue = true;

    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(data);
    if (Wire.endTransmission() != 0) {
        returnValue = false;
    }

    return returnValue;
}

/**
 * @brief Write signed integer data to a register
 *
 * @param reg register address
 * @param data data to be written
 * @return true
 * @return false
 */
bool STC3115I2CCore::writeRegisterInt(uint8_t reg, int data) {
    bool returnValue = true;
    uint8_t msb = (data >> 8) & 0xFF;
    uint8_t lsb = data & 0xFF;

    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(lsb);
    Wire.write(msb);
    if (Wire.endTransmission() != 0) {
        returnValue = false;
    }

    return returnValue;
}

/**
 * @brief Read signed integer value from a register
 *
 * @param output pointer to a variable that will hold the result
 * @param reg register address
 * @return true
 * @return false
 */
bool STC3115I2CCore::readRegisterInt(int* output, uint8_t reg) {
    uint8_t buffer[2] = {0};
    bool status = readRegisterRegion(buffer, reg, 2);
    *output = static_cast<int>(buffer[0]) | static_cast<int>(buffer[1]) << 8;

    return status;
}

/**
 * @brief Write array of unsigned bytes to a register
 *
 * @param reg register address
 * @param data array of unsigned bytes
 * @param length length of the array
 * @return true
 * @return false
 */
bool STC3115I2CCore::writeRegister(uint8_t reg, uint8_t* data, size_t length) {
    bool returnValue = true;
    Wire.beginTransmission(address);
    Wire.write(reg);

    for (size_t i = 0; i < length; i++) {
        Wire.write(data[i]);
    }

    if (Wire.endTransmission() != 0) {
        returnValue = false;
    }

    return returnValue;
}
