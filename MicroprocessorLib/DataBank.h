#ifndef __DATA_BANK_H__
#define __DATA_BANK_H__
#include "AddressOffset.h"


class CDataBank : public CAddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU) override;
    uint8   GetBank(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __DATA_BANK_H__

