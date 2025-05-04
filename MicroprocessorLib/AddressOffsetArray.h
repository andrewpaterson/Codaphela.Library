#ifndef __ADDRESS_OFFSET_ARRAY_H__
#define __ADDRESS_OFFSET_ARRAY_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "AddressOffset.h"


typedef CArrayTemplatePtr<CAddressOffset> __CAddressOffsetArray;


class CAddressOffsetArray : public __CAddressOffsetArray
{
public:
	void Kill(void);
};


void CalculateAddressFromAddressOffsets(CAddress* pcDest, CW65C816* pcCPU, CAddressOffsetArray* pacAddressOffsets);


#endif // __ADDRESS_OFFSET_ARRAY_H__


