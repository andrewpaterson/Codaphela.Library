#ifndef __NEW_PROGRAM_BANK_H__
#define __NEW_PROGRAM_BANK_H__
#include "W65AddressOffset.h"


class CW65NewProgramBank : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    uint8   GetBank(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};



#endif // __NEW_PROGRAM_BANK_H__

