#ifdef STC3115_I2C_CORE_FILE_H
#define STC3115_I2C_CORE_FILE_H

#include <Arduino.h>
#include <Wire.h>

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

#endif
