#ifndef __SIGNED_DATA_LOW_H__
#define __SIGNED_DATA_LOW_H__
#include "W65AddressOffset.h"


class CW65SignedDataLow : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __SIGNED_DATA_LOW_H__

