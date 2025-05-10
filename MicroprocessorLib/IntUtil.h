#ifndef __INT_UTIL_H__
#define __INT_UTIL_H__
#include "BaseLib/PrimitiveTypes.h"


uint8 ToByte(int value);
uint8 ToNybble(int value);
uint8 GetLowByte(int value);
uint8 GetHighByte(int value);
uint16 SetLowByte(int variable, uint8 data);
uint16 SetHighByte(int variable, uint8 data);
void Assert8Bit(uint16 value, char* variable);
void Assert16Bit(uint16 value, char* variable);


#endif // __INT_UTIL_H__

