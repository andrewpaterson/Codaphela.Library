/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __C_P_P_SYNTAX_MEMORY_H__
#define __C_P_P_SYNTAX_MEMORY_H__
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/MemoryStackExtended.h"
#include "CPPSyntax.h"


//Nodes are allocated together and freed together when the tree is killed.
class CCPPSyntaxMemory
{
protected:
	CMemoryStackExtended	mcStack;
	CMallocator*			mpcNodes;		//The nodes are allocated by it (an arena), or in mcStack if NULL.
	CArraySyntax			mapcSyntaxes;
	CCPPSyntaxTree*			mpcTree;

public:
	void							Init(CCPPSyntaxTree* pcTree, CMallocator* pcSyntaxes = NULL, CMallocator* pcNodes = NULL);
	void							Kill(void);
	size							NumSyntaxes(void);
	void							Rollback(size uiSyntaxes);

	template<class M>	M*			Create(CCPPSyntax* pcParent);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CCPPSyntaxMemory::Create(CCPPSyntax* pcParent)
{
	M*	pcSyntax;

	pcSyntax = New<M>((M*)(mpcNodes ? mpcNodes->Malloc(sizeof(M)) : mcStack.Add(sizeof(M))));
	pcSyntax->Init(mpcTree, pcParent);
	mapcSyntaxes.Add(pcSyntax);
	return pcSyntax;
}


#endif // __C_P_P_SYNTAX_MEMORY_H__

