#ifndef __ADDRESS_H__
#define __ADDRESS_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/HexString.h"
#include "BaseLib/IntegerHelper.h"


#define BANK_SIZE_BYTES 0x10000
#define PAGE_SIZE_BYTES 0x100


class CAddress
{
protected:
    uint8    muiBank;
    uint16   muiOffset;

public:
    void        Init(CAddress* pcAddress);
    void        Init(void);
    void        Init(uint8 uiBank, uint16 uiOffset);

    uint8       GetBank(void);
    uint16      GetOffset(void);
    uint8       GetOffsetLow(void);
    uint8       GetOffsetHigh(void);
    CAddress*   Offset(int16 iOffset);
    void        Print(CChars* psz);
    void        SetOffset(uint16 uiOffset);
    void        SetBank(uint8 uiBank);
    void        SetOffsetLow(uint8 uiOffsetLow);
    void        SetOffsetHigh(uint8 uiOffsetHigh);
};


bool AreOffsetsOnDifferentPages(uint16 iOffsetFirst, uint16 iOffsetSecond);


#endif // __ADDRESS_H__

