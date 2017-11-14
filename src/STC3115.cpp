#include "STC3115.h"
#include <Arduino.h>

/**
 * @brief Initialize STC3115 I2C driver with given address
 *
 * @param address
 */
STC3115::STC3115(uint8_t address): STC3115I2CCore(address) {
}

STC3115::~STC3115() {}

/**
 * @brief Configure STC3115
 *
 * @return true when the STC3115 is configured successfully
 * @return false when STC3115 is failed to be configured
 */
bool STC3115::begin() {
    bool retval = true;
    initConfig();

    // TODO: handle read RAM data failure
    readRAMData();

    if (ramData.reg.TestWord != RAM_TESTWORD || calculateCRC8RAM(ramData.db, STC3115_RAM_SIZE) != 0) {
        // handle invalid RAM data
        initRAM();
        retval = startup();
    } else {
        uint8_t data;
        readRegister(&data, STC3115_REG_CTRL);

        if (data & (STC3115_BATFAIL | STC3115_PORDET) != 0) {
            retval = startup();
        } else {
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
    readRegister(&res, STC3115_REG_ID);

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
void STC3115::initConfig() {
    config.VMode = VMODE;
    if (RSENSE != 0) {
        config.RSense = RSENSE;
    } else {
        config.RSense = 10;
    }

    config.CCConf = (BATT_CAPACITY * config.RSense * 250 + 6194) / 12389;

    if (BATT_RINT != 0) {
        config.VMConf = (BATT_CAPACITY * BATT_RINT * 50 + 24444) / 48889;
    } else {
        config.VMConf = (BATT_CAPACITY * 200 * 50 + 24444) / 488889;
    }

    for (int i = 0; i < 16; i++) {
       config.OCVOffset[i] = 0;
    }

    config.CNom = BATT_CAPACITY;
    config.RelaxCurrent = BATT_CAPACITY / 20;
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

    if (getChipID() != STC3115_ID) {
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
int8_t STC3115::getTemperature() {
    uint8_t temp = 0;
    if (!readRegister(&temp, STC3115_REG_TEMPERATURE)) {
        return 0;
    }

    return static_cast<int8_t>(temp);
}

/**
 * @brief Get battery voltage
 *
 * @return float
 */
float STC3115::getVoltage() {
    uint8_t voltage = 0;
    if (!readRegister(&voltage, STC3115_REG_VOLTAGE_L)) {
        return 0.0;
    }

    return static_cast<float>(voltage);
}

/**
 * @brief Get battery current
 *
 * @return float
 */
float STC3115::getCurrent() {
    uint8_t current = 0;
    if (!readRegister(&current, STC3115_REG_CURRENT_L)) {
        return 0.0;
    }

    return static_cast<float>(current);
}
