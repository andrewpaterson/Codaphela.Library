#ifndef __COMPOUND_ADDRESS_H__
#define __COMPOUND_ADDRESS_H__
#include "AddressOffsetArray.h"


class CCompoundAddress : public CAddressOffset
{
private:
    CAddressOffsetArray     mapcAddressOffsets;
	
public:
    void                    Init(void);
    void                    Kill(void) override;

    CAddressOffsetArray*    GetAddressOffsets(void);

    int32                   GetOffset(CW65C816* pcCPU) override;
    uint8                   GetBank(CW65C816* pcCPU);
    void                    Print(CChars* psz) override;
};


CCompoundAddress* Span(CAddressOffset* pcOffset, ...);


#endif // __COMPOUND_ADDRESS_H__

