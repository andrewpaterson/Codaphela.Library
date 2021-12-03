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
#include "ArrayCBlockSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCBlockSet::Init(CPPTokens* pcTokenMemory)
{
	mbRawProcessed = FALSE;
	mpcTokenMemory = pcTokenMemory;
	__CArrayCBlockSet::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCBlockSet::RawProcessed(void)
{
	mbRawProcessed = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCBlockSet::IsRawProcessed(void)
{
	return mbRawProcessed;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPBlockSet* CArrayCBlockSet::Add(int iLine, BOOL bTextBlocks)
{
	CPPBlockSet* pcBlockSet;

	pcBlockSet = __CArrayCBlockSet::Add();
	pcBlockSet->Init(mpcTokenMemory, iLine, miUsedElements-1, bTextBlocks);
	return pcBlockSet;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCBlockSet::Dump(void)
{
	int				i;
	CPPBlockSet*	pcBlockSet;

	for (i = 0; i < NumElements(); i++)
	{
		pcBlockSet = Get(i);
		pcBlockSet->Dump();
	}
}

