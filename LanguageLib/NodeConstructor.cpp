/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "NodeConstructor.h"
#include <String.h>
#include "BaseLib/Logger.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeConstructor::Init(void)
{
	msNode.eNodeType = NT_Single;
	msNode.eModifier = NM_Normal;
	msNode.eNecessity = NN_Normal;
	msNode.iMinimum = 1;
	msNode.iMaximum = 1;
	msNode.iNumElementsInNode = 0;
	msNode.bFinal = FALSE;

	memset(mszName, 0, MAX_NODE_NAME+1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeConstructor::Kill(void)
{
	//Does Nothing.
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeConstructor::SetName(char* szName)
{
	int		iLen;

	iLen = (int)strlen(szName);

	memset(mszName, 0, MAX_NODE_NAME+1);
	if (iLen >= MAX_NODE_NAME)
	{
		iLen = MAX_NODE_NAME;
	}
	memcpy(mszName, szName, iLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNodeConstructor::ShouldBeAdded(void)
{
	BOOL	bSpecialNode;
	BOOL	bNecessary;

	bSpecialNode = !((msNode.iMinimum == 1) && (msNode.iMaximum == 1) && (msNode.eNodeType == NT_Single) && (msNode.eModifier == NM_Normal));
	bNecessary = ((msNode.eNecessity == NN_Necessary) || (msNode.eNecessity == NN_Empty));

	return bSpecialNode || bNecessary || msNode.bFinal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeConstructor::SetNumElementsInNode(int iNumElements)
{
	msNode.iNumElementsInNode = iNumElements;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNodeConstructor::SetType(EElement eNodeType)
{
	if ((msNode.eNodeType == NT_Single) || (msNode.eNodeType == eNodeType))
	{
		msNode.eNodeType = eNodeType;
		return TRUE;
	}
	else
	{
		gcLogger.Error("Can't change node type");
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeConstructor::SetOrderedIfPossible(void)
{
	if ((msNode.iNumElementsInNode > 0) && (msNode.eNodeType == NT_Single))
	{
		msNode.eNodeType = NT_Ordered;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeConstructor::SetNecessity(EElement eNecessity)
{
	msNode.eNecessity = eNecessity;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeConstructor::SetFinal(BOOL bFinal)
{
	msNode.bFinal = bFinal;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNodeConstructor::TestForCorrectness(void)
{
	if ((msNode.iMinimum < 0) || (msNode.iMaximum < 0))
	{
		gcLogger.Error("Bad Node, iMinimum or iMaximum < 0");
		return FALSE;
	}
	return TRUE;
}

