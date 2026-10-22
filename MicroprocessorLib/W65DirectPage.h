#ifndef __DIRECT_PAGE_H__
#define __DIRECT_PAGE_H__
#include "W65AddressOffset.h"


class CW65DirectPage : public CW65AddressOffset
{
protected:
	bool	mbPageWrap;

public:
    void    Init(void);
    void    Init(bool bPageWrap);
    void    Kill(void) override;

    int32   GetOffset(CW65CCore* pcCPU);
    bool    IsPageWrapped(CW65CCore* pcCPU) override;
    void    Print(CChars* psz);
};


#endif // __DIRECT_PAGE_H__

