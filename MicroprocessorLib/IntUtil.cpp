#include "BaseLib/Logger.h"
#include "IntUtil.h"


int toShort(int value)
{
    return value & 0xffff;
}


int toByte(int value)
{
    return value & 0xff;
}


int toNybble(int value)
{
    return value & 0xf;
}


int getLowByte(int value)
{
    return toByte(value);
}


int getHighByte(int value)
{
    return toByte((value & 0xFF00) >> 8);
}


int setLowByte(int variable, int data)
{
    return (variable & 0xFF00) | toByte(data);
}


int setHighByte(int variable, int data)
{
    return (variable & 0xFF) | (toByte(data) << 8);
}


void assert8Bit(int value, char* variable)
{
    if ((value < 0) || (value > 0xFF))
    {
        gcLogger.Error2(__METHOD__, " ", variable ,"value [0x", IntToString(value, 16), + "] must in the range 0...0xFF.", NULL);
    }
}


void assert16Bit(int value, char* variable)
{
    if ((value < 0) || (value > 0xFFFF))
    {
        gcLogger.Error2(__METHOD__, " ", variable, "value [0x", IntToString(value, 16), +"] must in the range 0...0xFFFF.", NULL);
    }
}

