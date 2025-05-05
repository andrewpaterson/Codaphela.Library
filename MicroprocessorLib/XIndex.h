#ifndef __X_INDEX_H__
#define __X_INDEX_H__
#include "AddressOffset.h"


class CXIndex : public CAddressOffset
{
public:
    int32   GetOffset(CW65C816* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __X_INDEX_H__

