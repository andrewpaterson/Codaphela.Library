#ifndef __ADDRESS_OFFSET_ARRAY_H__
#define __ADDRESS_OFFSET_ARRAY_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "W65AddressOffset.h"


typedef CArrayTemplatePtr<CW65AddressOffset> __CW65AddressOffsetArray;


class CW65AddressOffsetArray : public __CW65AddressOffsetArray
{
public:
	void Kill(void);

	void Print(CChars* psz);
};


void					CalculateAddressWrappingFromAddressOffsets(CW65Address* pcDest, CW65CCore* pcCPU, CW65AddressOffsetArray* pacAddressOffsets);
void					CalculateAddressSpanningFromAddressOffsets(CW65Address* pcDest, CW65CCore* pcCPU, CW65AddressOffsetArray* pacAddressOffsets);
CW65AddressOffsetArray*	Address(CW65AddressOffset* pcOffset, ...);


#endif // __ADDRESS_OFFSET_ARRAY_H__

