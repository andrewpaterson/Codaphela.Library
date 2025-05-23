#ifndef __ABSOUTE_ADDRESS_H__
#define __ABSOUTE_ADDRESS_H__
#include "AddressOffset.h"


class CAbsoluteAddress : public CAddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __ABSOUTE_ADDRESS_H__

