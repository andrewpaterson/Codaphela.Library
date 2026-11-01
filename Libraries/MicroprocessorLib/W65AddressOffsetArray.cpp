#include "W65Address.h"
#include "W65AddressOffsetArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65AddressOffsetArray::Kill(void)
{
	size				i;
	size				uiNumElements;
	CW65AddressOffset* pcOffset;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOffset = __CW65AddressOffsetArray::GetPtr(i);
		SafeKill(pcOffset);
	}

	__CW65AddressOffsetArray::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65AddressOffsetArray::Print(CChars* psz)
{
	size                uiNumOffsets;
	size                i;
	CW65AddressOffset* pcOffset;
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
void CalculateAddressWrappingFromAddressOffsets(CW65Address* pcDest, CW65CCore* pcCPU, CW65AddressOffsetArray* pacAddressOffsets)
{
	CW65AddressOffset*		pcOffset;
	uint8				uiBank;
	uint16				uiOffset;
	uint16				uiPage;
	bool				bPageWrapped;
	size				i;
	size				uiNumElements;

	uiBank = 0;
	uiOffset = 0;
	uiPage = 0;
	bPageWrapped = false;
	uiNumElements = pacAddressOffsets->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOffset = pacAddressOffsets->GetPtr(i);
		if (pcOffset->CanPageWrap() && pcOffset->IsPageWrapped(pcCPU))
		{
			bPageWrapped = true;
			uiPage += pcOffset->GetOffset(pcCPU);
		}
		else
		{
			uiOffset += pcOffset->GetOffset(pcCPU);
		}
		if (pcOffset->HasBank())
		{
			uiBank += pcOffset->GetBank(pcCPU);
		}
	}

	if (bPageWrapped)
	{
		uiOffset = uiPage + GetLowByte(uiOffset);
	}
	else
	{
		uiOffset += uiPage;
	}
	pcDest->Init(uiBank, uiOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CalculateAddressSpanningFromAddressOffsets(CW65Address* pcDest, CW65CCore* pcCPU, CW65AddressOffsetArray* pacAddressOffsets)
{
	CW65AddressOffset* pcOffset;
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
CW65AddressOffsetArray* Address(CW65AddressOffset* pcOffset, ...)
{
	va_list		            vaMarker;
	size		            iCount;
	CW65AddressOffsetArray*	papcOffsets;

	papcOffsets = NewMalloc<CW65AddressOffsetArray>();
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
			pcOffset = va_arg(vaMarker, CW65AddressOffset*);
		}
		va_end(vaMarker);
	}

	return papcOffsets;
}

