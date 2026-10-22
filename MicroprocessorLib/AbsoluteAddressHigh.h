#ifndef __ABSOLUTE_ADDRESS_HIGH_H__
#define __ABSOLUTE_ADDRESS_HIGH_H__
#include "AddressOffset.h"


class CAbsoluteAddressHigh : public CAddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __ABSOLUTE_ADDRESS_HIGH_H__

