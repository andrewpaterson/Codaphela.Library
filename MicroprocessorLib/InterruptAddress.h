#ifndef __INTERRUPT_ADDRESS_H__
#define __INTERRUPT_ADDRESS_H__
#include "InterruptVector.h"
#include "AddressOffset.h"


class CInterruptAddress : public CAddressOffset
{
private:
    CInterruptVector*   mpcInterruptVector;

public:
    void    Init(CInterruptVector* pcinterruptVector);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};



#endif // __INTERRUPT_ADDRESS_H__

