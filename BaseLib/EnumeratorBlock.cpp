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
#include "EnumeratorBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CEnumeratorBlock::AddGetNode(char* szName, void* pvData, int iDataSize, int iKeySize, int iNum, BOOL bReplace, SENode** pcThisNode)
{
	int			iID;
	SENode*		psNode;

	if (pcThisNode)
	{
		iID = PrivateAddGetNode(szName, pvData, iDataSize, iKeySize, iNum, bReplace, pcThisNode);
		psNode = (*pcThisNode);
	}
	else
	{
		iID = PrivateAddGetNode(szName, pvData, iDataSize, iKeySize, iNum, bReplace, &psNode);
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
int CEnumeratorBlock::Add(char* szName, void* pvData, int iDataSize, int iKeySize, int iNum, BOOL bReplace)
{
	return AddGetNode(szName, pvData, iDataSize, iKeySize, iNum, bReplace, NULL);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CEnumeratorBlock::Add(char* szName, int iDataSize)
{
	return Add(szName, iDataSize, -1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CEnumeratorBlock::Add(char* szName, int iDataSize, int iNum)
{
	SENode*		psNode;

	AddGetNode(szName, NULL, iDataSize, 0, iNum, TRUE, &psNode);
	return psNode->pvData;
}

