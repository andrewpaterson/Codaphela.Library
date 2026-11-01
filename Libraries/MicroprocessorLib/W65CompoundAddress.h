#ifndef __COMPOUND_ADDRESS_H__
#define __COMPOUND_ADDRESS_H__
#include "W65AddressOffsetArray.h"


class CW65CompoundAddress : public CW65AddressOffset
{
private:
    CW65AddressOffsetArray     mapcAddressOffsets;
	
public:
    void                    Init(void);
    void                    Kill(void) override;

    CW65AddressOffsetArray*    GetAddressOffsets(void);

    int32                   GetOffset(CW65CCore* pcCPU) override;
    uint8                   GetBank(CW65CCore* pcCPU) override;
    void                    Print(CChars* psz) override;
};


CW65CompoundAddress* Span(CW65AddressOffset* pcOffset, ...);


#endif // __COMPOUND_ADDRESS_H__

