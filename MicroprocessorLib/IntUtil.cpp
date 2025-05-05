#include "BaseLib/Logger.h"
#include "IntUtil.h"


int ToShort(int value)
{
    return value & 0xffff;
}


int ToByte(int value)
{
    return value & 0xff;
}


int ToNybble(int value)
{
    return value & 0xf;
}


int GetLowByte(int value)
{
    return ToByte(value);
}


int GetHighByte(int value)
{
    return ToByte((value & 0xFF00) >> 8);
}


int SetLowByte(int variable, int data)
{
    return (variable & 0xFF00) | ToByte(data);
}


int SetHighByte(int variable, int data)
{
    return (variable & 0xFF) | (ToByte(data) << 8);
}


void Assert8Bit(int value, char* variable)
{
    if ((value < 0) || (value > 0xFF))
    {
        gcLogger.Error2(__METHOD__, " ", variable ,"value [0x", IntToString(value, 16), + "] must in the range 0...0xFF.", NULL);
    }
}


void Assert16Bit(int value, char* variable)
{
    if ((value < 0) || (value > 0xFFFF))
    {
        gcLogger.Error2(__METHOD__, " ", variable, "value [0x", IntToString(value, 16), +"] must in the range 0...0xFFFF.", NULL);
    }
}

