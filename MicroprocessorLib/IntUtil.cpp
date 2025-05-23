#include "BaseLib/Logger.h"
#include "IntUtil.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 ToByte(uint16 uiValue)
{
    return (uint8)uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 ToNybble(uint16 uiValue)
{
    return uiValue & 0xf;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 GetLowByte(uint16 uiValue)
{
    return (uint8)uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 GetHighByte(uint16 uiValue)
{
    return (uint8)((uiValue & 0xFF00) >> 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 SetLowByte(uint16 uiVariable, uint8 uiData)
{
    return (uiVariable & 0xFF00) | uiData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 SetHighByte(uint16 uiVariable, uint8 uiData)
{
    return (uiVariable & 0xFF) | (uiData << 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Assert8Bit(uint16 uiValue, char* pszVariable)
{
    if (uiValue > 0xFF)
    {
        gcLogger.Error2(__METHOD__, " ", pszVariable,"uiValue [0x", IntToString(uiValue, 16), + "] must in the range 0...0xFF.", NULL);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Assert16Bit(uint16 uiValue, char* pszVariable)
{
    if (uiValue > 0xFFFF)
    {
        gcLogger.Error2(__METHOD__, " ", pszVariable, "uiValue [0x", IntToString(uiValue, 16), +"] must in the range 0...0xFFFF.", NULL);
    }
}

