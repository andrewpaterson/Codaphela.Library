#ifndef __Y_INDEX_H__
#define __Y_INDEX_H__
#include "AddressOffset.h"


class CYIndex : public CAddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __Y_INDEX_H__

