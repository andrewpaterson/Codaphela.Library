#ifndef __INT_UTIL_H__
#define __INT_UTIL_H__


uint16 ToByte(int value);
uint16 ToNybble(int value);
uint16 GetLowByte(int value);
uint16 GetHighByte(int value);
uint16 SetLowByte(int variable, int data);
uint16 SetHighByte(int variable, int data);
void Assert8Bit(int value, char* variable);
void Assert16Bit(int value, char* variable);


#endif // __INT_UTIL_H__

