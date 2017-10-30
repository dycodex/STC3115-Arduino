#include "STC3115.h"

STC3115I2C::STC3115I2C(uint8_t address):
address(address) {

}

STC3115I2C::~STC3115I2C() {
}

bool STC3115I2C::beginI2C() {
    bool result = true;
    Wire.beginTransmission(address);

    result = Wire.endTransmission() == 0;

    return result;
}

bool STC3115I2C::readRegister(uint8_t* output, uint8_t reg) {
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

bool STC3115I2C::readRegisterRegion(uint8_t* output, uint_t reg, uint8_t length) {
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

bool STC3115I2C::readRegisterInt16(int16_t* output, uint8_t reg) {
    uint8_t buffer[2] = {0};
    bool status = readRegisterRegion(buffer, reg, 2);
    *output = static_cast<int16_t>(buffer[0]) | static_cast<int16_t>(buffer[1]) << 8;

    return status;
}

bool STC3115I2C::writeRegister(uint8_t reg, uint8_t data) {
    bool returnValue = true;

    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(data);
    if (Wire.endTransmission != 0) {
        returnValue = false;
    }

    return returnValue;
}
