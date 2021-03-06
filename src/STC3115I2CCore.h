#ifndef STC3115_I2C_CORE_FILE_H
#define STC3115_I2C_CORE_FILE_H

#include <Arduino.h>
#include <Wire.h>

class STC3115I2CCore {
public:
    STC3115I2CCore(uint8_t address = 0x70);
    virtual ~STC3115I2CCore();

    bool beginI2C();
    bool readRegister(uint8_t* output, uint8_t reg);
    bool readRegisterRegion(uint8_t* output, uint8_t reg, uint8_t length);
    bool readRegisterInt16(int16_t* output, uint8_t reg);
    bool readRegisterInt(int* output, uint8_t reg);
    bool writeRegister(uint8_t reg, uint8_t data);
    bool writeRegisterInt(uint8_t reg, int data);
    bool writeRegister(uint8_t reg, uint8_t* data, size_t length);
protected:
    uint8_t address;
};

#endif
