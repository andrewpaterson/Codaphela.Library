#ifndef __STACK_POINTER_H__
#define __STACK_POINTER_H__
#include "AddressOffset.h"


class CStackPointer : public CAddressOffset
{
public:
    void    Init(void);
    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __STACK_POINTER_H__

