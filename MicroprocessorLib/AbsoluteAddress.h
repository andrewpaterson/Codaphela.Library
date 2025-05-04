#ifndef __ABSOUTE_ADDRESS_H__
#define __ABSOUTE_ADDRESS_H__
#include "AddressOffset.h"


class CAbsoluteAddress : public CAddressOffset
{
public:
    uint16  GetOffset(CW65C816* pcCPU);
    void    Print(CChars* psz);
};


#endif // __ABSOUTE_ADDRESS_H__

