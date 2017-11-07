#ifndef STC3115_DRIVER_COMPONENT_H
#define STC3115_DRIVER_COMPONENT_H

#include <Arduino.h>
#include "STC3115_registers.h"
#include "STC3115I2CCore.h"

class STC3115 : public STC3115I2CCore {
public:
    STC3115(uint8_t address = 0x70);
    virtual ~STC3115();

    bool begin();
    int8_t getTemperature();
    float getVoltage();
    float getCurrent();
};

#endif
