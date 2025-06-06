#include "BaseLib/IntegerHelper.h"
#include "Address.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool AreOffsetsOnDifferentPages(uint16 iOffsetFirst, uint16 iOffsetSecond)
{
    uint16 pageOfFirst = iOffsetFirst / PAGE_SIZE_BYTES;
    uint16 pageOfSecond = iOffsetSecond / PAGE_SIZE_BYTES;
    return pageOfFirst != pageOfSecond;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddress::Init(CAddress* pcAddress)
{
    Init(pcAddress->GetBank(), pcAddress->GetOffset());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddress::Init(void)
{
    Init(0x00, 0x0000);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddress::Init(uint8 uiBank, uint16 uiOffset)
{
    muiBank = uiBank;
    muiOffset = uiOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CAddress::GetBank(void)
{
    return muiBank;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CAddress::GetOffset(void)
{
    return muiOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CAddress::GetOffsetLow(void)
{
    return (uint8)muiOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CAddress::GetOffsetHigh(void)
{
    return (uint8)((muiOffset & 0xff00) >> 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAddress* CAddress::Offset(int16 uiOffset)
{
    if (uiOffset != 0)
    {
        muiOffset = muiOffset + uiOffset;
    }
    return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddress::Print(CChars* psz)
{
    psz->Append(HexByteToString((uint8)muiBank));
    psz->Append(":");
    psz->Append(HexShortToString((uint16)muiOffset));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddress::SetOffset(uint16 uiOffset)
{
    muiOffset = uiOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddress::SetBank(uint8 uiBank)
{
    muiBank = uiBank;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddress::SetOffsetLow(uint8 uiOffsetLow)
{
    muiOffset = SetLowByte(muiOffset, uiOffsetLow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddress::SetOffsetHigh(uint8 uiOffsetHigh)
{
    muiOffset = SetHighByte(muiOffset, uiOffsetHigh);
}


