#ifndef __INT_UTIL_H__
#define __INT_UTIL_H__
#include "BaseLib/PrimitiveTypes.h"


uint8 ToByte(uint16 uiValue);
uint8 ToNybble(uint16 uiValue);
uint8 GetLowByte(uint16 uiValue);
uint8 GetHighByte(uint16 uiValue);
uint16 SetLowByte(uint16 uiVariable, uint8 uiData);
uint16 SetHighByte(uint16 uiVariable, uint8 uiData);
void Assert8Bit(uint16 uiValue, char* pszVariable);
void Assert16Bit(uint16 uiValue, char* pszVariable);


#endif // __INT_UTIL_H__

