#ifndef __SIGNED_DATA_H__
#define __SIGNED_DATA_H__
#include "W65AddressOffset.h"


class CW65SignedData : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU);
    void    Print(CChars* psz);
};


#endif // __SIGNED_DATA_H__

