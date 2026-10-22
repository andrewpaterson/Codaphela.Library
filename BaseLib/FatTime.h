#ifndef __FAT_TIME_H__
#define __FAT_TIME_H__
#include "PrimitiveTypes.h"


int64 FatDecodeDateTime(uint16 date, uint16 time);
uint16 GetSystemClockDate(void);
uint16 GetSystemClockTime(void);


#endif // __FAT_TIME_H__

