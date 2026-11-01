/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/Numbers.h"
#include "PPTokenMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Init(void)
{
	mcStack.Init(4 KB);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Kill(void)
{
	//The tokens own nothing: the stack's memory is all there is.
	mcStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Mark(CStackMarkExtended* psMark)
{
	mcStack.Mark(psMark);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Rollback(CStackMarkExtended* psMark)
{
	mcStack.Rollback(psMark);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPPTokenMemory::StartIteration(SPPTokenIterator* psIter)
{
	CPPToken*		pcPPToken;
	int				iNumStacks;
	CMemoryStack*	pcStack;
	int				iNumElements;

	iNumStacks = (int)mcStack.NumStacks();
	psIter->iStack = 0;
	psIter->iElement = 0;

	for (; psIter->iStack < iNumStacks; )
	{
		pcStack = mcStack.GetStack(psIter->iStack);
		psIter->pvStackData = pcStack->GetData();
		iNumElements = (int)pcStack->NumElements();
		if (psIter->iElement < iNumElements)
		{
			pcPPToken = (CPPToken*)psIter->pvStackData;
			psIter->pvStackData = RemapSinglePointer(psIter->pvStackData, pcPPToken->ClassSize());
			psIter->iElement++;
			return pcPPToken;
		}
		psIter->iStack++;
		psIter->iElement = 0;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPPTokenMemory::Iterate(SPPTokenIterator* psIter)
{
	CPPToken*		pcPPToken;
	CMemoryStack*	pcStack;
	int				iNumElements;

	for (;;)
	{
		pcStack = mcStack.GetStack(psIter->iStack);
		iNumElements = (int)pcStack->NumElements();
		for (; psIter->iElement < iNumElements; )
		{
			pcPPToken = (CPPToken*)psIter->pvStackData;
			psIter->pvStackData = RemapSinglePointer(psIter->pvStackData, pcPPToken->ClassSize());
			psIter->iElement++;
			return pcPPToken;
		}
		psIter->iStack++;
		psIter->iElement = 0;
		pcStack = mcStack.GetStack(psIter->iStack);
		if (pcStack)
		{
			psIter->pvStackData = pcStack->GetData();
		}
		else
		{
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CPPTokenMemory::Size(void)
{
	return mcStack.GetUsedMemory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPTokenMemory::Print(CChars* psz)
{
	SPPTokenIterator	sIter;
	CPPToken*			pcPPToken;

	pcPPToken = StartIteration(&sIter);
	while (pcPPToken)
	{
		pcPPToken->Print(psz);
		pcPPToken = Iterate(&sIter);
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}

