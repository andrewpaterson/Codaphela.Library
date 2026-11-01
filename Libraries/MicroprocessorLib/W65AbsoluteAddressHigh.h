#ifndef __ABSOLUTE_ADDRESS_HIGH_H__
#define __ABSOLUTE_ADDRESS_HIGH_H__
#include "W65AddressOffset.h"


class CW65AbsoluteAddressHigh : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __ABSOLUTE_ADDRESS_HIGH_H__

