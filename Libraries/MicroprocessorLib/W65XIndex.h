#ifndef __X_INDEX_H__
#define __X_INDEX_H__
#include "W65AddressOffset.h"


class CW65XIndex : public CW65AddressOffset
{
public:
    void    Init(void);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU) override;
    void    Print(CChars* psz) override;
};


#endif // __X_INDEX_H__

