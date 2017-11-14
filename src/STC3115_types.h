#ifndef STC3115_TYPES_H
#define STC3115_TYPES_H

#include <stdint.h>
#include "STC3115_constants.h"

/**
 * @brief STC3115 configuration structure
 *
 */
typedef struct {
    int VMode;
    int AlmSOC;
    int AlmVbat;
    int CCConf;
    int VMConf;
    int CNom;
    int RSense;
    int RelaxCurrent;
    uint8_t OCVOffset[16];
} STC3115ConfigData;

/**
 * @brief STC3115 battery measurement data structure
 *
 */
typedef struct {
    int StatusWord;
    int HRSOC;
    int SOC;
    int Voltage;
    int Current;
    int Temperature;
    int ConvCounter;
    int OCV;
    int Presence;
    int ChargeValue;
    int RemTime;
} STC3115BatteryData;

/**
 * @brief STC3115 RAM data internal structure
 *
 */
typedef union {
    uint8_t db[STC3115_RAM_SIZE];
    struct {
        short TestWord;
        short HRSOC;
        short CCConf;
        short VMConf;
        char SOC;
        char State;
        char unused1;
        char unused2;
        char unused3;
        char unused4;
        char unused5;
        char CRC;
    } reg;
} STC3115RAMData;

#endif
