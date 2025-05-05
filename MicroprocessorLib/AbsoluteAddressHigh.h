#ifndef __ABSOLUTE_ADDRESS_HIGH_H__
#define __ABSOLUTE_ADDRESS_HIGH_H__
#include "AddressOffset.h"


class CAbsoluteAddressHigh : public CAddressOffset
{
public:
    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __ABSOLUTE_ADDRESS_HIGH_H__

