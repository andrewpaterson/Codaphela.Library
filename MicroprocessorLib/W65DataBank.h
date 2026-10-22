#ifndef __DATA_BANK_H__
#define __DATA_BANK_H__
#include "W65AddressOffset.h"


class CW65DataBank : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    uint8   GetBank(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __DATA_BANK_H__

