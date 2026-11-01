#ifndef __DIRECT_OFFSET_H__
#define __DIRECT_OFFSET_H__
#include "W65AddressOffset.h"


class CW65DirectOffset : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU);
    void    Print(CChars* psz);
};


#endif // __DIRECT_OFFSET_H__