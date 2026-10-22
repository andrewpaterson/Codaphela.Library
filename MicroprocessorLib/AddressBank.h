#ifndef __ADDRESS_BANK_H__
#define __ADDRESS_BANK_H__
#include "AddressOffset.h"


class CAddressBank : public CAddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    uint8   GetBank(CW65CCore* pcCPU);
    void    Print(CChars* psz) override;
};


#endif // __ADDRESS_BANK_H__

