#ifndef __ABSOLUTE_ADDRESS_LOW_PLUS_Y_LOW_H__
#define __ABSOLUTE_ADDRESS_LOW_PLUS_Y_LOW_H__
#include "W65AddressOffset.h"


class CW65AbsoluteAddressLowPlusYLow : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __ABSOLUTE_ADDRESS_LOW_PLUS_Y_LOW_H__

