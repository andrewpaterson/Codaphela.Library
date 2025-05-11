#ifndef __DIRECT_OFFSET_H__
#define __DIRECT_OFFSET_H__
#include "AddressOffset.h"


class CDirectOffset : public CAddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU);
    void    Print(CChars* psz);
};


#endif // __DIRECT_OFFSET_H__