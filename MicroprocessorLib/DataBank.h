#ifndef __DATA_BANK_H__
#define __DATA_BANK_H__
#include "AddressOffset.h"


class CDataBank : public CAddressOffset
{
public:
    int32   GetOffset(CW65C816* pcCPU) override;
    uint8   GetBank(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __DATA_BANK_H__

