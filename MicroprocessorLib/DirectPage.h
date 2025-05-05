#ifndef __DIRECT_PAGE_H__
#define __DIRECT_PAGE_H__
#include "AddressOffset.h"


class CDirectPage : public CAddressOffset
{
public:
    int32   GetOffset(CW65C816* pcCPU);
    void    Print(CChars* psz);
};


#endif // __DIRECT_PAGE_H__

