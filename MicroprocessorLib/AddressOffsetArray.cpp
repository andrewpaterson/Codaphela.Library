#include "Address.h"
#include "AddressOffsetArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddressOffsetArray::Kill(void)
{
	size				i;
	size				uiNumElements;
	CAddressOffset*		pcAddressOffset;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcAddressOffset = __CAddressOffsetArray::GetPtr(i);
		pcAddressOffset->Kill();
	}

	__CAddressOffsetArray::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CalculateAddressFromAddressOffsets(CAddress* pcDest, CW65C816* pcCPU, CAddressOffsetArray* pacAddressOffsets)
{
	CAddressOffset*		pcOffset;
	uint8				uiBank;
	uint16				uiOffset;
	size				i;
	size				uiNumElements;

	uiBank = 0;
	uiOffset = 0;
	uiNumElements = pacAddressOffsets->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOffset = pacAddressOffsets->GetPtr(i);
		uiOffset += pcOffset->GetOffset(pcCPU);
		uiBank += pcOffset->GetBank(pcCPU);
	}
	pcDest->Init(uiBank, uiOffset);
}

