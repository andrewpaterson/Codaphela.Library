#ifndef __STACK_POINTER_H__
#define __STACK_POINTER_H__
#include "AddressOffset.h"


enum EStackPointerMode
{
	SPM_Page,			// S.  In emulation mode the stack is in page 1.
	SPM_PagePlusOne,	// S+1 for 6502 pulls.  In emulation mode it wraps within page 1.
	SPM_Linear,			// All 16 bits of S, used by the instructions in datasheet caveat 7.1.
};


class CStackPointer : public CAddressOffset
{
protected:
	EStackPointerMode	meMode;

public:
    void    Init(void);
    void    Init(EStackPointerMode eMode);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __STACK_POINTER_H__

