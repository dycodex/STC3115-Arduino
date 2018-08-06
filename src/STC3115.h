#ifndef STC3115_DRIVER_COMPONENT_H
#define STC3115_DRIVER_COMPONENT_H

#include <Arduino.h>
#include "STC3115_constants.h"
#include "STC3115_types.h"
#include "STC3115_registers.h"
#include "STC3115I2CCore.h"

#define BATT_CAPACITY 610
#define BATT_RINT 200
#define VMODE MIXED_MODE
#define ALM_EN 0
#define ALM_SOC 10
#define ALM_VBAT 3600
#define RSENSE 50
#define APP_EOC_CURRENT 75
#define APP_CUTOFF_VOLTAGE 3000

class STC3115 : public STC3115I2CCore {
public:

    STC3115(uint8_t address = 0x70);
    virtual ~STC3115();

    bool begin(int batteryCapacity = BATT_CAPACITY, int rSense = RSENSE);
    int getTemperature();
    float getVoltage();
    int getSoC();
    float getSoCPercent();
    int getCurrent();
    int getChargeValue();
    int getOCV();
    int getRemainingTime();
    int getChipID();
    int getStatus();

    void enableDebugging(Stream* stream = NULL);
    void disableDebugging();

    int getRunningCounter();
    bool readBatteryData();
    static int convert(short value, unsigned short factor);

    bool reset();
    bool stop();
    bool powerDown();

    void run();
    bool startPowerSavingMode();
    bool stopPowerSavingMode();

    bool isBatteryDetected();

    STC3115ConfigData config;
protected:
    void initConfig(int battCapacity, int rSense);
    int calculateCRC8RAM(uint8_t* data, size_t length);
    void initRAM();
    bool readRAMData();
    int updateRAMCRC8();
    bool writeRAMData();
    bool startup();
    bool restore();
    void setParamAndRun();

    STC3115BatteryData batteryData;
    STC3115RAMData ramData;

    bool debugEnabled;
    Stream* debugStream;
};

#endif
