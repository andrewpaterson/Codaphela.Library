#ifndef __PROGRAM_COUNTER_H__
#define __PROGRAM_COUNTER_H__
#include "W65AddressOffset.h"


class CW65ProgramCounter : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    bool    IsProgramCounter(void) override;
    void    Print(CChars* psz) override;
};


#endif // __PROGRAM_COUNTER_H__

