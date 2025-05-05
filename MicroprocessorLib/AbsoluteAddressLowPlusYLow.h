#ifndef __ABSOLUTE_ADDRESS_LOW_PLUS_Y_LOW_H__
#define __ABSOLUTE_ADDRESS_LOW_PLUS_Y_LOW_H__
#include "AddressOffset.h"


class CAbsoluteAddressLowPlusYLow : public CAddressOffset
{
public:
    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __ABSOLUTE_ADDRESS_LOW_PLUS_Y_LOW_H__

