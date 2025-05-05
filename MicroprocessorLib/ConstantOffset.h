#ifndef __CONSTANT_OFFSET_H__
#define __CONSTANT_OFFSET_H__
#include "BaseLib/Logger.h"
#include "AddressOffset.h"


class CConstantOffset : public CAddressOffset
{
private:
    int32   miOffset;

public:
    void    Init(int32 iOffset);
    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
    bool    IsConstantOffset(void);
};


#endif // __CONSTANT_OFFSET_H__

