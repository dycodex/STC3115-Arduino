#ifndef STC3115_CONSTANTS_H
#define STC3115_CONSTANTS_H

#define STC3115_VMODE   	0x01
#define STC3115_CLR_VM_ADJ  0x02
#define STC3115_CLR_CC_ADJ  0x04
#define STC3115_ALM_ENA		0x08
#define STC3115_GG_RUN		0x10
#define STC3115_FORCE_CC	0x20
#define STC3115_FORCE_VM	0x40
#define STC3115_REGMODE_DEFAULT_STANDBY   	0x09

#define STC3115_GG_RST		0x02
#define STC3115_GG_VM		0x04
#define STC3115_BATFAIL		0x08
#define STC3115_PORDET		0x10
#define STC3115_ALM_SOC		0x20
#define STC3115_ALM_VOLT	0x40

#define STC3115_ID          0x14
#define STC3115_RAM_SIZE    16
#define STC3115_OCVTAB_SIZE 16
#define VCOUNT				4
#define VM_MODE 			1
#define CC_MODE 			0
#define MIXED_MODE			0
#define MAX_HRSOC          	51200
#define MAX_SOC            	1000
#define STC3115_OK 					0
#define VoltageFactor  		9011
#define CurrentFactor		24084
#define VOLTAGE_SECURITY_RANGE 200

#define RAM_TESTWORD 		0x53A9
#define STC3115_UNINIT    0
#define STC3115_INIT     'I'
#define STC3115_RUNNING  'R'
#define STC3115_POWERDN  'D'

#endif
