#ifndef STC3115_DRIVER_COMPONENT_H
#define STC3115_DRIVER_COMPONENT_H

#include <stdint.h>
#include <Wire.h>
#include "STC3115_registers.h"

class STC3115I2CCore {
public:
    STC3115I2CCore(uint8_t address = 0x70);
    ~STC3115I2CCore();

    bool beginI2C();
    bool readRegister(uint8_t* output, uint8_t reg);
    bool readRegisterRegion(uint8_t* output, uint8_t reg, uint8_t length);
    bool readRegisterInt16(int16_t* output, uint8_t reg);
    bool writeRegister(uint8_t reg, uint8_t data);
protected:
    uint8_t address;
};

class STC3115 : public STC3115I2CCore {
public:
    STC3115(uint8_t address = 0x70);
    ~STC3115();

    bool begin(uint8_t vmode = 0);
    int8_t getTemperature();
    float getVoltage();
    float getCurrent();
};

#endif
