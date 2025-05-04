#ifndef __INT_UTIL_H__
#define __INT_UTIL_H__


int toShort(int value);
int toByte(int value);
int toNybble(int value);
int getLowByte(int value);
int getHighByte(int value);
int setLowByte(int variable, int data);
int setHighByte(int variable, int data);
void assert8Bit(int value, char* variable);
void assert16Bit(int value, char* variable);


#endif // __INT_UTIL_H__

