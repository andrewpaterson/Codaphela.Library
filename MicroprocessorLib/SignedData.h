#ifndef __SIGNED_DATA_H__
#define __SIGNED_DATA_H__
#include "AddressOffset.h"


class CSignedData : public CAddressOffset
{
public:
    int32   GetOffset(CW65C816* pcCPU);
    void    Print(CChars* psz);
};


#endif // __SIGNED_DATA_H__

