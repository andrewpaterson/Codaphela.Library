/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "PPBlockSetArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSetArray::Init(void)
{
	mbRawProcessed = false;
	__CArrayPPBlockSet::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSetArray::Kill(void)
{
	int				i;
	int				iNumElements;
	CPPBlockSet*	pcBlockSet;

	iNumElements = NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcBlockSet = Get(i);
		pcBlockSet->Kill();
	}
	__CArrayPPBlockSet::Kill();
	mbRawProcessed = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSetArray::RawProcessed(void)
{
	mbRawProcessed = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPBlockSetArray::IsRawProcessed(void)
{
	return mbRawProcessed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPBlockSet* CPPBlockSetArray::Add(int iLine, bool bTextBlocks)
{
	CPPBlockSet* pcBlockSet;

	pcBlockSet = __CArrayPPBlockSet::Add();
	pcBlockSet->Init(iLine, miUsedElements-1, bTextBlocks);
	return pcBlockSet;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSetArray::Print(CChars* psz)
{
	size			i;
	CPPBlockSet*	pcBlockSet;

	for (i = 0; i < NumElements(); i++)
	{
		pcBlockSet = Get(i);
		pcBlockSet->Print(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSetArray::Dump(void)
{
	CChars			sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSetArray::DumpBlockSets(void)
{
	size			i;
	CPPBlockSet*	pcBlockSet;

	for (i = 0; i < NumElements(); i++)
	{
		pcBlockSet = Get(i);
		pcBlockSet->DumpBlockSet();
	}
}

