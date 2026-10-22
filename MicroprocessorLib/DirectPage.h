#ifndef __DIRECT_PAGE_H__
#define __DIRECT_PAGE_H__
#include "AddressOffset.h"


class CDirectPage : public CAddressOffset
{
protected:
	bool	mbPageWrap;

public:
    void    Init(void);
    void    Init(bool bPageWrap);
    void    Kill(void) override;

    int32   GetOffset(CW65C816* pcCPU);
    bool    IsPageWrapped(CW65C816* pcCPU) override;
    void    Print(CChars* psz);
};


#endif // __DIRECT_PAGE_H__

