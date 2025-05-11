#ifndef __ADDRESS_BANK_H__
#define __ADDRESS_BANK_H__
#include "AddressOffset.h"


class CAddressBank : public CAddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU) override;
    uint8   GetBank(CW65C816* pcCPU);
    void    Print(CChars* psz) override;
};


#endif // __ADDRESS_BANK_H__

