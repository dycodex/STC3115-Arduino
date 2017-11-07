#include "STC3115.h"
#include <Arduino.h>

/**
 * STC3115 I2C interface class
 */
STC3115::STC3115(uint8_t address): STC3115I2CCore(address) {
}

STC3115::~STC3115() {}

bool STC3115::begin(uint8_t vmode) {
    uint8_t usedVmode = 0;
    // VMODE = 0 (mixed mode)
    // CLR_VM_ADJ = 0
    // CLR_CC_ADJ = 0
    // ALRM_ENA = 1 (enable alarm)
    // GG_RUN = 0
    // FORCE_CC = 0
    // FORCE_VM = 0
    uint8_t mode = usedVmode | 1 << 3;
    Serial.print("[init] REG_MODE value: ");
    Serial.println(mode);

    if (!beginI2C()) {
        Serial.println("[init] Failed initializing I2C");
        return false;
    }

    return writeRegister(STC3115_REG_MODE, mode);
}

int8_t STC3115::getTemperature() {
    uint8_t temp = 0;
    if (!readRegister(&temp, STC3115_REG_TEMPERATURE)) {
        return 0;
    }

    return static_cast<int8_t>(temp);
}

float STC3115::getVoltage() {
    uint8_t voltage = 0;
    if (!readRegister(&voltage, STC3115_REG_VOLTAGE_L)) {
        return 0.0;
    }

    return static_cast<float>(voltage); // / 2.2; // millivolt
}

float STC3115::getCurrent() {
    uint8_t current = 0;
    if (!readRegister(&current, STC3115_REG_CURRENT_L)) {
        return 0.0;
    }

    return static_cast<float>(current); // / 5.88 ; // microvolt
}
