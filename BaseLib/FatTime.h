#ifndef __FAT_TIME_H__
#define __FAT_TIME_H__
#include <time.h>
#include "PrimitiveTypes.h"


time_t FatDecodeDateTime(uint16 date, uint16 time);
uint16 GetSystemClockDate(void);
uint16 GetSystemClockTime(void);


#endif // !__FAT_TIME_H__

