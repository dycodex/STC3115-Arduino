#include "STC3115.h"
#include <Arduino.h>

#define STC3115_DEBUG_PRINT(...) if (debugEnabled && debugStream != NULL) {  debugStream->print(__VA_ARGS__); }
#define STC3115_DEBUG_PRINTLN(...) if (debugEnabled && debugStream != NULL) { debugStream->println(__VA_ARGS__); }

/**
 * @brief Initialize STC3115 I2C driver with given address
 *
 * @param address
 */
STC3115::STC3115(uint8_t address):
 STC3115I2CCore(address),
 debugEnabled(0),
 debugStream(0) {}

STC3115::~STC3115() {}

/**
 * @brief Initialize the STC3115 Gas Gauge chip.
 *
 * @param battCapacity maximum battery capacity
 * @param rSense RSENSE value
 * @return true if the gauge is initialized.
 * @return false if the gauge initialization is failed.
 */
bool STC3115::begin(int battCapacity, int rSense) {
    beginI2C();

    bool retval = true;

    initConfig(battCapacity, rSense);
    readRAMData();
    if (ramData.reg.TestWord != RAM_TESTWORD || calculateCRC8RAM(ramData.db, STC3115_RAM_SIZE) != 0) {
        STC3115_DEBUG_PRINTLN("Invalid RAM data");

        initRAM();
        retval = startup();
    } else {
        uint8_t data;
        readRegister(&data, STC3115_REG_CTRL);

        if ((data & (STC3115_BATFAIL | STC3115_PORDET)) != 0) {
            STC3115_DEBUG_PRINTLN("Fresh start up");
            retval = startup();
        } else {
            STC3115_DEBUG_PRINTLN("Restore from RAM");
            retval = restore();
        }
    }

    ramData.reg.State = STC3115_INIT;
    updateRAMCRC8();
    writeRAMData();

    return retval;
}

/**
 * @brief Get the chip ID from STC3115
 *
 * @return int chip ID of the STC3115
 */
int STC3115::getChipID() {
    uint8_t res = 0;
    bool readResult = readRegister(&res, STC3115_REG_ID);
    STC3115_DEBUG_PRINT("[DBG] CHIP ID READ RESULT: ");
    STC3115_DEBUG_PRINTLN(readResult);

    return static_cast<int>(res);
}

/**
 * @brief Initialize STC3115 RAM data
 *
 */
void STC3115::initRAM() {
    for (int i = 0; i < STC3115_RAM_SIZE; i++) {
        ramData.db[i] = 0;
    }

    ramData.reg.TestWord = RAM_TESTWORD;
    ramData.reg.CCConf = config.CCConf;
    ramData.reg.VMConf = config.VMConf;

    updateRAMCRC8();
}

/**
 * @brief Read STC3115 RAM data and store it to an internal structure
 *
 * @return true
 * @return false
 */
bool STC3115::readRAMData() {
    bool readResult = readRegisterRegion(ramData.db, STC3115_REG_RAM0, STC3115_RAM_SIZE);
    return readResult;
}

/**
 * @brief Update STC3115 RAM CRC8 value
 *
 * @return int RAM CRC8
 */
int STC3115::updateRAMCRC8() {
    int result = calculateCRC8RAM(ramData.db, STC3115_RAM_SIZE - 1);
    ramData.db[STC3115_RAM_SIZE - 1] = result;

    return result;
}

/**
 * @brief Calculate the CRC8 of a buffer.
 *
 * @param data pointer to array of uint8_t
 * @param length length of the array
 * @return int CRC8 of the data
 */
int STC3115::calculateCRC8RAM(uint8_t* data, size_t length) {
    int crc = 0;

    for (int i = 0; i < length; i++) {
        crc ^= data[i];

        for (int j = 0; j < 8; j++) {
            crc <<= 1;
            if (crc & 0x100) {
                crc ^= 7;
            }
        }
    }

    return (crc & 255);
}

/**
 * @brief Write data from internal structure to STC3115 RAM
 *
 * @return true
 * @return false
 */
bool STC3115::writeRAMData() {
    return writeRegister(STC3115_REG_RAM0, ramData.db, STC3115_RAM_SIZE);
}

/**
 * @brief Initialize STC3115 config default values
 *
 */
void STC3115::initConfig(int battCapacity, int rSense) {
    config.VMode = VMODE;
    if (rSense != 0) {
        config.RSense = rSense;
    } else {
        config.RSense = 10;
    }

    config.CCConf = (battCapacity * config.RSense * 250 + 6194) / 12389;

    if (BATT_RINT != 0) {
        config.VMConf = (battCapacity * BATT_RINT * 50 + 24444) / 48889;
    } else {
        config.VMConf = (battCapacity * 200 * 50 + 24444) / 488889;
    }

    for (int i = 0; i < 16; i++) {
       config.OCVOffset[i] = 0;
    }

    config.CNom = battCapacity;
    config.RelaxCurrent = battCapacity / 20;
    config.AlmSOC = ALM_SOC;
    config.AlmVbat = ALM_VBAT;

    batteryData.Presence = 1;
}

/**
 * @brief Get STC3115 status
 *
 * @return int
 */
int STC3115::getStatus() {
    int value = 0;
    uint8_t data[2] = {0};

    int chipId = getChipID();
    STC3115_DEBUG_PRINT("Chip ID: ");
    STC3115_DEBUG_PRINTLN(chipId, HEX);

    if (chipId != STC3115_ID) {
        return -1;
    }

    readRegisterRegion(data, STC3115_REG_MODE, 2);
    value = data[0] | (data[1] << 8);
    value &= 0x7fff;

    return value;
}

/**
 * @brief Write configuration to STC3115 registers
 *
 */
void STC3115::setParamAndRun() {
    writeRegister(STC3115_REG_MODE, STC3115_REGMODE_DEFAULT_STANDBY);

    writeRegister(STC3115_REG_OCVTAB0, config.OCVOffset, STC3115_OCVTAB_SIZE);

    if (config.AlmSOC != 0) {
        writeRegister(STC3115_REG_ALARM_SOC, config.AlmSOC * 2);
    }

    if (config.AlmVbat != 0) {
        int value = static_cast<long>(config.AlmVbat << 9) / VoltageFactor;
        writeRegister(STC3115_REG_ALARM_VOLTAGE, static_cast<uint8_t>(value));
    }

    if (config.RSense != 0) {
        int value = static_cast<long>(config.RelaxCurrent << 9) / (CurrentFactor / config.RSense);
        writeRegister(STC3115_REG_CURRENT_THRES, static_cast<uint8_t>(value));
    }

    if (ramData.reg.CCConf != 0) {
        writeRegisterInt(STC3115_REG_CC_CNF_L, ramData.reg.CCConf);
    }

    if (ramData.reg.VMConf != 0) {
        writeRegisterInt(STC3115_REG_VM_CNF_L, ramData.reg.VMConf);
    }

    writeRegister(STC3115_REG_CTRL, 0x03);
    writeRegister(STC3115_REG_MODE, STC3115_GG_RUN | (STC3115_VMODE * config.VMode) | (STC3115_ALM_ENA * ALM_EN));
}

/**
 * @brief Write SOC data to STC3115 and run
 *
 * @return true
 * @return false
 */
bool STC3115::startup() {
    int HRSOC;
    int ocv, ocvMin;
    int OCVOffset[16] = {0};

    if (getStatus() < 0) {
        return false;
    }

    uint8_t registerDataWord[2] = {0};
    readRegisterRegion(registerDataWord, STC3115_REG_OCV_L, 2);
    ocv = registerDataWord[0] | (registerDataWord[1] << 8);

    ocvMin = 6000 + OCVOffset[0];
    if (ocv < ocvMin) {
        HRSOC = 0;
        writeRegisterInt(STC3115_REG_SOC_L, HRSOC);
        setParamAndRun();
    } else {
        setParamAndRun();
        writeRegisterInt(STC3115_REG_OCV_L, ocv);
    }

    return true;
}

/**
 * @brief Restore SOC value from RAM and run
 *
 * @return true
 * @return false
 */
bool STC3115::restore() {
    if (getStatus() < 0) {
        return false;
    }

    setParamAndRun();
    writeRegisterInt(STC3115_REG_SOC_L, ramData.reg.HRSOC);

    return true;
}

/**
 * @brief Get temperature of the battery
 *
 * @return int8_t
 */
int STC3115::getTemperature() {
    return batteryData.Temperature;
}

/**
 * @brief Get battery voltage
 *
 * @return float
 */
int STC3115::getVoltage() {
    return batteryData.Voltage;
}

/**
 * @brief Get battery current
 *
 * @return float
 */
int STC3115::getCurrent() {
    return batteryData.Current;
}

/**
 * @brief Get SOC of the battery
 *
 * @return int
 */
int STC3115::getSOC() {
    return batteryData.SOC;
}

/**
 * @brief Get the remaining capacity in mAh unit
 *
 * @return int
 */
int STC3115::getChargeValue() {
    return batteryData.ChargeValue;
}

/**
 * @brief Get OCV of the battery.
 *
 * @return int
 */
int STC3115::getOCV() {
    return batteryData.OCV;
}

/**
 * @brief Get the STC3115 conversion counter value
 *
 * @return int conversion counter
 */
int STC3115::getRunningCounter() {
    int data;
    if (!readRegisterInt(&data, STC3115_REG_COUNTER_L)) {
        return -1;
    }

    return data;
}


/**
 * @brief Read battery measurement data in one go.
 *
 * @return true
 * @return false
 */
bool STC3115::readBatteryData() {
    uint8_t data[16];
    bool retVal = true;
    int value;

    retVal = readRegisterRegion(data, 0, 16);
    if (!retVal) {
        STC3115_DEBUG_PRINT("[FAIL]: Return value: ");
        STC3115_DEBUG_PRINTLN(retVal);
        return retVal;
    }

    value = data[3];
    value = (value << 8) + data[2];
    batteryData.HRSOC = value;
    batteryData.SOC = (value * 10 + 256) / 512;
    STC3115_DEBUG_PRINT("[DBG] SOC: ");
    STC3115_DEBUG_PRINTLN(batteryData.SOC);

    value = data[5];
    value = (value << 8) + data[4];
    batteryData.ConvCounter = value;
    STC3115_DEBUG_PRINT("[DBG] ConvCounter: ");
    STC3115_DEBUG_PRINTLN(batteryData.ConvCounter);

    value = data[7];
    value = (value << 8) + data[6];
    value = value & 0x3fff;
    if (value >= 0x2000) {
        value = value - 0x4000;
    }
    batteryData.Current = convert(value, CurrentFactor / config.RSense);
    STC3115_DEBUG_PRINT("[DBG] Current: ");
    STC3115_DEBUG_PRINTLN(batteryData.Current);

    value = data[9];
    value = (value << 8) + data[8];
    value = value & 0x0fff;
    if (value >= 0x0800) {
        value = value - 0x1000;
    }
    batteryData.Voltage = convert(value, VoltageFactor);
    STC3115_DEBUG_PRINT("[DBG] Voltage: ");
    STC3115_DEBUG_PRINTLN(batteryData.Voltage);

    value = data[10];
    if (value >= 0x80) {
        value = value - 0x100;
    }
    batteryData.Temperature = value * 10;
    STC3115_DEBUG_PRINT("[DBG] Temperature: ");
    STC3115_DEBUG_PRINTLN(batteryData.Temperature);

    value = data[14];
    value = (value << 8) | data[13];
    value = value & 0x3fff;
    if (value >= 0x02000) {
        value = value - 0x4000;
    }
    value = convert(value, VoltageFactor);
    value = (value + 2) / 4;
    batteryData.OCV = value;
    STC3115_DEBUG_PRINT("[DBG] OCV: ");
    STC3115_DEBUG_PRINTLN(batteryData.OCV);

    return true;
}

/**
 * @brief Convert measurement data with given factor
 *
 * @param value value to be converted
 * @param factor conversion factor
 * @return int
 */
int STC3115::convert(short value, unsigned short factor) {
    int v = (static_cast<long>(value) * factor) >> 11;
    v = (v + 1) / 2;

    return v;
}

/**
 * @brief Reset gauge.
 *
 * @return true
 * @return false
 */
bool STC3115::reset() {
    bool result;
    ramData.reg.TestWord = 0;
    ramData.reg.State = STC3115_UNINIT;

    result = writeRAMData();
    if (!result) {
        STC3115_DEBUG_PRINTLN("[FAIL] Failed to write RAM data before reset");
        return false;
    }

    result = writeRegister(STC3115_REG_CTRL, STC3115_PORDET);
    return result;
}

/**
 * @brief Power down the gauge.
 *
 * @return true
 * @return false
 */
bool STC3115::powerDown() {
    writeRegister(STC3115_REG_CTRL, 0x01);
    return writeRegister(STC3115_REG_MODE, 0);
}

/**
 * @brief Store last reading data to RAM and then stop the gauge.
 *
 * @return true
 * @return false
 */
bool STC3115::stop() {
    readRAMData();
    ramData.reg.State = STC3115_POWERDN;

    updateRAMCRC8();
    writeRAMData();

    return powerDown();
}

/**
 * @brief Gradually update battery status on the internal structure & RAM. This function should be called inside loop.
 *
 */
void STC3115::run() {
    int status = getStatus();
    if (status < 0) {
        return;
    }

    batteryData.StatusWord = status;

    readRAMData();
    if ((ramData.reg.TestWord != RAM_TESTWORD) || calculateCRC8RAM(ramData.db, STC3115_RAM_SIZE) != 0) {
        initRAM();
        ramData.reg.State = STC3115_INIT;
    }

    if ((batteryData.StatusWord & (static_cast<int>(STC3115_BATFAIL) << 8)) != 0) {
        batteryData.Presence = 0;
        reset();

        return;
    }

    if ((batteryData.StatusWord & STC3115_GG_RUN) == 0) {
        if (ramData.reg.State == STC3115_RUNNING || ramData.reg.State == STC3115_POWERDN) {
            restore();
        } else {
            startup();
        }

        ramData.reg.State = STC3115_INIT;
    }

    if (!readBatteryData()) {
        return;
    }

    if (ramData.reg.State == STC3115_INIT) {
        if (batteryData.ConvCounter > VCOUNT) {
            ramData.reg.State = STC3115_RUNNING;
            batteryData.Presence = 1;
        }
    }

    if (ramData.reg.State != STC3115_RUNNING) {
        batteryData.ChargeValue = config.CNom * batteryData.SOC / MAX_SOC;
        batteryData.Current = 0;
        batteryData.Temperature = 250;
        batteryData.RemTime = -1;
    } else {
        if (batteryData.Voltage < APP_CUTOFF_VOLTAGE) {
            batteryData.SOC = 0;
        } else if (batteryData.Voltage < (APP_CUTOFF_VOLTAGE + VOLTAGE_SECURITY_RANGE)) {
            batteryData.SOC = batteryData.SOC * (batteryData.Voltage - APP_CUTOFF_VOLTAGE) / VOLTAGE_SECURITY_RANGE;
        }

        batteryData.ChargeValue = config.CNom * batteryData.SOC / MAX_SOC;
        if ((batteryData.StatusWord & STC3115_VMODE) == 0) {
            if ((batteryData.StatusWord & STC3115_VMODE) == 0) {
                if (batteryData.Current > APP_EOC_CURRENT && batteryData.SOC > 990) {
                    batteryData.SOC = 990;
                    writeRegisterInt(STC3115_REG_SOC_L, 50688);
                }
            }

            if (batteryData.Current < 0) {
                batteryData.RemTime = (batteryData.RemTime * 4 + batteryData.ChargeValue / batteryData.Current * 60) / 5;
                if (batteryData.RemTime < 0) {
                    batteryData.RemTime = -1;
                }
            } else {
                batteryData.RemTime = -1;
            }
        } else {
            batteryData.Current = 0;
            batteryData.RemTime = -1;
        }

        if (batteryData.SOC > 1000) {
            batteryData.SOC = MAX_SOC;
        }

        if (batteryData.SOC < 0) {
            batteryData.SOC = 0;
        }
    }

    ramData.reg.HRSOC = batteryData.HRSOC;
    ramData.reg.SOC = (batteryData.SOC + 5) / 10;
    updateRAMCRC8();
    writeRAMData();

    if (ramData.reg.State == STC3115_RUNNING) {
        return;
    }
}

/**
 * @brief Start power saving mode.
 *
 * @return true
 * @return false
 */
bool STC3115::startPowerSavingMode() {
    uint8_t mode = 0;
    readRegister(&mode, STC3115_REG_MODE);

    return writeRegister(STC3115_REG_MODE, (mode | STC3115_VMODE));
}


/**
 * @brief Stop power saving mode
 *
 * @return true
 * @return false
 */
bool STC3115::stopPowerSavingMode() {
    uint8_t mode = 0;
    readRegister(&mode, STC3115_REG_MODE);

    if (VMODE != MIXED_MODE) {
        return false;
    }

    return writeRegister(STC3115_REG_MODE, (mode & ~STC3115_VMODE));
}

/**
 * @brief Check whether the battery is detected or not.
 *
 * @return true if the battery is detected
 * @return false otherwise
 */
bool STC3115::isBatteryDetected() {
    return batteryData.Presence == 1;
}

void STC3115::enableDebugging(Stream* stream) {
    this->debugStream = stream;
    this->debugEnabled = true;
}

void STC3115::disableDebugging() {
    this->debugEnabled = false;
    this->debugStream = NULL;
}


