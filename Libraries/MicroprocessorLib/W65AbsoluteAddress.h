#ifndef __ABSOUTE_ADDRESS_H__
#define __ABSOUTE_ADDRESS_H__
#include "W65AddressOffset.h"


class CW65AbsoluteAddress : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __ABSOUTE_ADDRESS_H__

