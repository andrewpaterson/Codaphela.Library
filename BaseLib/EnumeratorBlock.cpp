/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "EnumeratorBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CEnumeratorBlock::AddGetNode(char* szName, void* pvData, size uiDataSize, size uiKeySize, size iNum, bool bReplace, SENode** pcThisNode)
{
	size			iID;
	SENode*		psNode;

	if (pcThisNode)
	{
		iID = PrivateAddGetNode(szName, pvData, uiDataSize, uiKeySize, iNum, bReplace, pcThisNode);
		psNode = (*pcThisNode);
	}
	else
	{
		iID = PrivateAddGetNode(szName, pvData, uiDataSize, uiKeySize, iNum, bReplace, &psNode);
	}
	//If name with key exists and can't be replaced.
	if (psNode == NULL)
	{
		return -1;
	}

	return iID;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CEnumeratorBlock::Add(char* szName, void* pvData, size uiDataSize, size uiKeySize, size iNum, bool bReplace)
{
	return AddGetNode(szName, pvData, uiDataSize, uiKeySize, iNum, bReplace, NULL);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CEnumeratorBlock::Add(char* szName, size uiDataSize)
{
	return Add(szName, uiDataSize, -1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CEnumeratorBlock::Add(char* szName, size uiDataSize, size iNum)
{
	SENode*		psNode;

	AddGetNode(szName, NULL, uiDataSize, 0, iNum, true, &psNode);
	return psNode->pvData;
}

