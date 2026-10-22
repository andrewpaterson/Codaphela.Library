#ifndef __NEW_PROGRAM_COUNTER_H__
#define __NEW_PROGRAM_COUNTER_H__
#include "W65AddressOffset.h"


class CW65NewProgramCounter : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __NEW_PROGRAM_COUNTER_H__

