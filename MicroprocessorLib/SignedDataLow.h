#ifndef __SIGNED_DATA_LOW_H__
#define __SIGNED_DATA_LOW_H__
#include "AddressOffset.h"


class CSignedDataLow : public CAddressOffset
{
public:
    void    Init(void);
    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __SIGNED_DATA_LOW_H__

