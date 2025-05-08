#ifndef __PROGRAM_COUNTER_H__
#define __PROGRAM_COUNTER_H__
#include "AddressOffset.h"


class CProgramCounter : public CAddressOffset
{
public:
    void    Init(void);
    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __PROGRAM_COUNTER_H__

