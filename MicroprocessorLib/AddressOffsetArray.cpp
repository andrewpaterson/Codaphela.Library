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
	CAddressOffset* pcOffset;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOffset = __CAddressOffsetArray::GetPtr(i);
		SafeKill(pcOffset);
	}

	__CAddressOffsetArray::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAddressOffsetArray::Print(CChars* psz)
{
	size                uiNumOffsets;
	size                i;
	CAddressOffset* pcOffset;
	bool                bFirst;

	bFirst = true;
	uiNumOffsets = NumElements();
	for (i = 0; i < uiNumOffsets; i++)
	{
		pcOffset = GetPtr(i);
		if (!bFirst && !(pcOffset->IsConstantOffset()))
		{
			if (psz->GetChar(psz->Length() - 1) != ',')
			{
				psz->Append("+");
			}
		}
		pcOffset->Print(psz);

		bFirst = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CalculateAddressWrappingFromAddressOffsets(CAddress* pcDest, CW65C816* pcCPU, CAddressOffsetArray* pacAddressOffsets)
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CalculateAddressSpanningFromAddressOffsets(CAddress* pcDest, CW65C816* pcCPU, CAddressOffsetArray* pacAddressOffsets)
{
	CAddressOffset* pcOffset;
	uint8				uiBank;
	int32				iOffset;
	size				i;
	size				uiNumElements;

	uiBank = 0;
	iOffset = 0;
	uiNumElements = pacAddressOffsets->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOffset = pacAddressOffsets->GetPtr(i);
		iOffset += pcOffset->GetOffset(pcCPU);
		uiBank += pcOffset->GetBank(pcCPU);
	}
	uiBank += (uint8)(iOffset >> 16);
	pcDest->Init(uiBank, (uint16)iOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAddressOffsetArray* Address(CAddressOffset* pcOffset, ...)
{
	va_list		            vaMarker;
	size		            iCount;
	CAddressOffsetArray*	papcOffsets;

	papcOffsets = NewMalloc<CAddressOffsetArray>();
	papcOffsets->Init();

	if (pcOffset)
	{
		iCount = 0;

		va_start(vaMarker, pcOffset);
		while (pcOffset)
		{
			if (iCount > 10)
			{
				LOG_ERROR("Address Offsets have no terminal NULL.");
				return NULL;
			}

			papcOffsets->Add(pcOffset);

			iCount++;
			pcOffset = va_arg(vaMarker, CAddressOffset*);
		}
		va_end(vaMarker);
	}

	return papcOffsets;
}

