#ifndef __SHAREDDEFS_H
#define __SHAREDDEFS_H

#include "main.h"
#include "stm32h7xx_hal.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "string.h"
#include "inttypes.h" 				//inttypes.h includes stdint.h
#include "math.h"

typedef int8_t       									s8;
typedef int16_t      									s16;
typedef int32_t      									s32;
typedef uint8_t      									u8;
typedef uint8_t      									uint_8;
typedef uint16_t     									u16;
typedef uint32_t     									u32;
typedef volatile s8  									vs8;
typedef volatile s16 									vs16;
typedef volatile s32 									vs32;
typedef volatile u8  									vu8;
typedef volatile u16 									vu16;
typedef volatile u32 									vu32;

//#define SDEF_PROD
//#define SDEF_IWDG

#define SDEF_NXP											__attribute__((section(".NetXPoolSection")))
#define SDEF_TXP											__attribute__((section(".ThreadXPoolSection")))

#define SDEF_START_BL									FLASH_BASE
#define SDEF_START_MAIN								(SDEF_START_BL+0x8000)
#define SDEF_FW_META_ADDR							(SDEF_START_MAIN+0x1000)

#define SDEF_BIT_SET(a,b) 						((a) |= (1UL<<(b)))
#define SDEF_BIT_CLEAR(a,b) 					((a) &= ~(1UL<<(b)))
#define SDEF_BIT_FLIP(a,b) 						((a) ^= (1UL<<(b)))
#define SDEF_BIT_CHECK(a,b) 					(!!((a) & (1UL<<(b))))

//#define SDEF_CACHE
//#define SDEF_MPU
#define SDEF_ITM

//#include "globaldefs.h"
//#ifdef SDEF_IWDG
//#include "iwdg.h"
//#endif

#endif
