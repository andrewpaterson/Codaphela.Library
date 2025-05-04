#ifndef __ADDRESS_H__
#define __ADDRESS_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/HexString.h"
#include "IntUtil.h"
#include "ReflectiveData.h"


#define BANK_SIZE_BYTES 0x10000
#define PAGE_SIZE_BYTES 0x100


class CAddress : public CReflectiveData
{
protected:
    uint8    miBank;
    uint16   miOffset;

public:
    void        Init(CAddress* pcAddress);
    void        Init(void);
    void        Init(int iBank, int iOffset);
    int         GetBank(void);
    int         GetOffset(void);
    CAddress*   Offset(int iOffset, bool bWrapOffset);
    void        Print(CChars* psz);
    void        SetOffset(int iOffset);
    void        SetBank(int iBank);
    void        SetOffsetLow(int iOffsetLow);
    void        SetOffsetHigh(int iOffsetHigh);
};


bool AreOffsetsOnDifferentPages(int iOffsetFirst, int iOffsetSecond);


#endif // __ADDRESS_H__

