/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include "PointerFunctions.h"
#include "IntegerHelper.h"
#include "GlobalMemory.h"
#include "ArrayBit.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Init(CMallocator* pcMalloc)
{
	CMalloc::Init(pcMalloc);
	miNumBits = 0;
	u.aSmall.ui[0] = 0;
	u.aSmall.ui[1] = 0;
    u.aSmall.ui[2] = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Kill(void)
{
	if (IsLarge())
	{
		Free(u.aLarge.mpvData);
	}
	CMalloc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Zero(void)
{
	size	iAllocatedBytes;
	
	iAllocatedBytes = ByteSize(miNumBits);
	memset(GetData(), 0, iAllocatedBytes);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::ReInit(void)
{
	CMallocator* pcMalloc;

	pcMalloc = mpcMalloc;

	Kill();
	Init(pcMalloc);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::GrowBy(size iNumBits)
{
	size	uiOldByteSize;
	size	uiNewByteSize;
	size	uiTransitionSize;
	void*	pvData;

	uiOldByteSize = ByteSize(miNumBits);
	uiNewByteSize = ByteSize(miNumBits + iNumBits);

	uiTransitionSize = sizeof(uint32)*3;
	if ((uiNewByteSize <= uiTransitionSize) || (uiOldByteSize == uiNewByteSize))
	{
		miNumBits += iNumBits;
		return;
	}

	if ((uiOldByteSize <= uiTransitionSize) && (uiNewByteSize > uiTransitionSize))
	{
		pvData = Malloc(uiNewByteSize);
		memcpy_fast(pvData, GetData(), uiNewByteSize);
        u.aLarge.mpvData = pvData;
		miNumBits += iNumBits;
		return;
	}

	miNumBits += iNumBits;
    u.aLarge.mpvData = Realloc(u.aLarge.mpvData, uiNewByteSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayBit::Get(size iBit)
{
	return GetBit(iBit, GetData());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Set(size iBit, bool bBit)
{
	SetBit(iBit, GetData(), bBit);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Add(bool bBit)
{
	GrowBy(1);
	Set(miNumBits-1, bBit);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Get(void* pvDest, size iDestOffset, size iOffset, size iNumBits)
{
	CopyBits(pvDest, iDestOffset, GetData(), iOffset, iNumBits, 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Set(size iOffset, void* pvSource, size iSourceOffset, size iNumBits)
{
	CopyBits(GetData(), iOffset, pvSource, iSourceOffset, iNumBits, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Add(void* pvSource, size iSourceOffset, size iNumBits)
{
	size		iOldNumBits;

	iOldNumBits = miNumBits;
	GrowBy(iNumBits);
	Set(iOldNumBits, pvSource, iSourceOffset, iNumBits);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayBit::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	size			iNumBits;
	bool			bResult;
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.Read(pcFileReader);
	if (pcMalloc == NULL)
	{
		return false;
	}

	Init(pcMalloc);
	bResult = pcFileReader->ReadSize(&iNumBits);
	if (!bResult)
	{
		return false;
	}

	GrowBy(iNumBits);
	return pcFileReader->ReadBits(GetData(), ByteSize(iNumBits));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayBit::Write(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = gcMallocators.Write(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return false;
	}

	bResult = pcFileWriter->WriteSize(miNumBits);
	if (!bResult)
	{
		return false;
	}

	return pcFileWriter->WriteBits(GetData(), ByteSize(miNumBits));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayBit::NumElements(void)
{
	return miNumBits;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayBit::IsLarge(void)
{
	return miNumBits > sizeof(uint32)*3*8;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBit::GetData(void)
{
	if (IsLarge())
	{
		return u.aLarge.mpvData;
	}
	else
	{
		return u.aSmall.ui;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayBit::ByteSize(size iNumBits)
{
	return (iNumBits + 7) / 8;
}

