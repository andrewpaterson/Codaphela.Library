/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
void CPPBlockSetArray::Init(CPPTokenMemory* pcTokenMemory)
{
	mbRawProcessed = FALSE;
	mpcTokenMemory = pcTokenMemory;
	__CArrayPPBlockSet::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSetArray::RawProcessed(void)
{
	mbRawProcessed = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPBlockSetArray::IsRawProcessed(void)
{
	return mbRawProcessed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPBlockSet* CPPBlockSetArray::Add(int iLine, BOOL bTextBlocks)
{
	CPPBlockSet* pcBlockSet;

	pcBlockSet = __CArrayPPBlockSet::Add();
	pcBlockSet->Init(mpcTokenMemory, iLine, miUsedElements-1, bTextBlocks);
	return pcBlockSet;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPTokenMemory* CPPBlockSetArray::GetTokenMemory(void)
{
	return mpcTokenMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPBlockSetArray::Dump(void)
{
	int				i;
	CPPBlockSet*	pcBlockSet;

	for (i = 0; i < NumElements(); i++)
	{
		pcBlockSet = Get(i);
		pcBlockSet->Dump();
	}
}

