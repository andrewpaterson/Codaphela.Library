#ifndef __NEW_PROGRAM_COUNTER_H__
#define __NEW_PROGRAM_COUNTER_H__
#include "AddressOffset.h"


class CNewProgramCounter : public CAddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __NEW_PROGRAM_COUNTER_H__

