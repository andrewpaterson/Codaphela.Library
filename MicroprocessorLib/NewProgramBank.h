#ifndef __NEW_PROGRAM_BANK_H__
#define __NEW_PROGRAM_BANK_H__
#include "AddressOffset.h"


class CNewProgramBank : public CAddressOffset
{
public:
    void    Init(void);
    int32   GetOffset(CW65C816* pcCPU) override;
    uint8   GetBank(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};



#endif // __NEW_PROGRAM_BANK_H__

