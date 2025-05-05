#ifndef __ABSOUTE_ADDRESS_H__
#define __ABSOUTE_ADDRESS_H__
#include "AddressOffset.h"


class CAbsoluteAddress : public CAddressOffset
{
public:
    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __ABSOUTE_ADDRESS_H__

