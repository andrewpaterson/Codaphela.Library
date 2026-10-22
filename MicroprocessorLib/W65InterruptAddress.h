#ifndef __INTERRUPT_ADDRESS_H__
#define __INTERRUPT_ADDRESS_H__
#include "W65InterruptVector.h"
#include "W65AddressOffset.h"


class CW65InterruptAddress : public CW65AddressOffset
{
private:
    CW65InterruptVector*   mpcInterruptVector;

public:
    void    Init(CW65InterruptVector* pcinterruptVector);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};



#endif // __INTERRUPT_ADDRESS_H__

