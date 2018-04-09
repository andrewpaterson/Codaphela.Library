/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
	mpcMalloc = pcMalloc;
	miNumBits = 0;
	aSmall.ui[0] = 0;
	aSmall.ui[1] = 0;
	aSmall.ui[2] = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Kill(void)
{
	if (IsLarge())
	{
		Free(aLarge.mpvData);
	}
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Zero(void)
{
	int	iAllocatedBytes;
	
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
void CArrayBit::GrowBy(int iNumBits)
{
	int		iOldByteSize;
	int		iNewByteSize;
	int		iTransitionSize;
	void*	pvData;

	iOldByteSize = ByteSize(miNumBits);
	iNewByteSize = ByteSize(miNumBits + iNumBits);

	iTransitionSize = sizeof(unsigned int)*3;
	if ((iNewByteSize <= iTransitionSize) || (iOldByteSize == iNewByteSize))
	{
		miNumBits += iNumBits;
		return;
	}

	if ((iOldByteSize <= iTransitionSize) && (iNewByteSize > iTransitionSize))
	{
		pvData = Malloc(iNewByteSize);
		memcpy_fast(pvData, GetData(), iNewByteSize);
		aLarge.mpvData = pvData;
		miNumBits += iNumBits;
		return;
	}

	miNumBits += iNumBits;
	aLarge.mpvData = Realloc(aLarge.mpvData, iNewByteSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBit::Get(int iBit)
{
	return GetBit(iBit, GetData());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Set(int iBit, BOOL bBit)
{
	SetBit(iBit, GetData(), bBit);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Add(BOOL bBit)
{
	GrowBy(1);
	Set(miNumBits-1, bBit);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Get(void* pvDest, int iDestOffset, int iOffset, int iNumBits)
{
	CopyBits(pvDest, iDestOffset, GetData(), iOffset, iNumBits, 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Set(int iOffset, void* pvSource, int iSourceOffset, int iNumBits)
{
	CopyBits(GetData(), iOffset, pvSource, iSourceOffset, iNumBits, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Add(void* pvSource, int iSourceOffset, int iNumBits)
{
	int		iOldNumBits;

	iOldNumBits = miNumBits;
	GrowBy(iNumBits);
	Set(iOldNumBits, pvSource, iSourceOffset, iNumBits);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBit::Read(CFileReader* pcFileReader)
{
	int				iNumBits;
	BOOL			bResult;
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}

	Init(pcMalloc);
	bResult = pcFileReader->ReadInt(&iNumBits);
	if (!bResult)
	{
		return FALSE;
	}

	GrowBy(iNumBits);
	return pcFileReader->ReadBits(GetData(), ByteSize(iNumBits));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBit::Write(CFileWriter* pcFileWriter)
{
	BOOL	bResult;

	bResult = gcMallocators.WriteMallocator(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = pcFileWriter->WriteInt(miNumBits);
	if (!bResult)
	{
		return FALSE;
	}

	return pcFileWriter->WriteBits(GetData(), ByteSize(miNumBits));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBit::NumElements(void)
{
	return miNumBits;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBit::IsLarge(void)
{
	return miNumBits > sizeof(unsigned int)*3*8;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBit::GetData(void)
{
	if (IsLarge())
	{
		return aLarge.mpvData;
	}
	else
	{
		return aSmall.ui;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBit::ByteSize(int iNumBits)
{
	return (iNumBits + 7) / 8;
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBit::Malloc(size_t tSize)
{
	return malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Free(void* pv)
{
	SafeFree(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBit::Realloc(void* pv, size_t tSize)
{
	pv = realloc(pv, tSize);
	return pv;
}
