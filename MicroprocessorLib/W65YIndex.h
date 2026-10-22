#ifndef __Y_INDEX_H__
#define __Y_INDEX_H__
#include "W65AddressOffset.h"


class CW65YIndex : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __Y_INDEX_H__

