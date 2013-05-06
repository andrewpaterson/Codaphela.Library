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
#include "ArrayBit.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBit::Init(void)
{
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
	if (Large())
	{
		free(aLarge.mpvData);
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
	Kill();
	Init();
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
		pvData = malloc(iNewByteSize);
		memcpy_fast(pvData, GetData(), iNewByteSize);
		aLarge.mpvData = pvData;
		miNumBits += iNumBits;
		return;
	}

	miNumBits += iNumBits;
	aLarge.mpvData = realloc(aLarge.mpvData, iNewByteSize);
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
void CArrayBit::Remove(int iOffset, int iNumBits)
{
	//Implement me later...
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBit::Load(CFileReader* pcFile)
{
	int		iNumBits;

	Init();
	ReturnOnFalse(pcFile->ReadInt(&iNumBits));
	GrowBy(iNumBits);
	return pcFile->ReadBits(GetData(), ByteSize(iNumBits));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBit::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt(miNumBits));
	return pcFile->WriteBits(GetData(), ByteSize(miNumBits));
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
BOOL CArrayBit::Large(void)
{
	return miNumBits > sizeof(unsigned int)*3*8;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBit::GetData(void)
{
	if (Large())
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

