#ifndef __INT_UTIL_H__
#define __INT_UTIL_H__


int ToShort(int value);
int ToByte(int value);
int ToNybble(int value);
int GetLowByte(int value);
int GetHighByte(int value);
int SetLowByte(int variable, int data);
int SetHighByte(int variable, int data);
void Assert8Bit(int value, char* variable);
void Assert16Bit(int value, char* variable);


#endif // __INT_UTIL_H__

