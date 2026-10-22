#ifndef __CONSTANT_OFFSET_H__
#define __CONSTANT_OFFSET_H__
#include "BaseLib/Logger.h"
#include "AddressOffset.h"


class CConstantOffset : public CAddressOffset
{
private:
    int32   miOffset;
    bool    mbOnlyIfMemory16Bit;

public:
    void    Init(int32 iOffset);
    void    Init(int32 iOffset, bool bOnlyIfMemory16Bit);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
    bool    IsConstantOffset(void);
};


#endif // __CONSTANT_OFFSET_H__

