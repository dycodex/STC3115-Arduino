#ifndef STC3115_DRIVER_COMPONENT_H
#define STC3115_DRIVER_COMPONENT_H

#include <stdint.h>
#include <Wire.h>

class STC3115I2C {
public:
    STC3115I2C(uint8_t address);
    ~STC3115I2C();

    bool beginI2C();
    bool readRegister(uint8_t* output, uint8_t reg);
    bool readRegisterRegion(uint8_t* output, uint8_t reg, uint8_t length);
    bool readRegisterInt16(int16_t* output, uint8_t reg);
    bool writeRegister(uint8_t reg, uint8_t data);
protected:
    uint8_t address;
};

#endif