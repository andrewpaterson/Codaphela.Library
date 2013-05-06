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
#include "ASCIINode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIINode::Init(CASCIINode* pcParent, int iParentLetter)
{
	int		i;

	mpcParent = pcParent;
	miParentLetter = iParentLetter;

	for (i = 0; i < ASCII_NODE_MAX_CHARS; i++)
	{
		 mapcChildren[i] = NULL;
		 maiWordEnds[i] = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIINode::IsEmpty(void)
{
	int		i;

	for (i = 0; i < ASCII_NODE_MAX_CHARS; i++)
	{
		if (maiWordEnds[i] != -1)
		{
			return FALSE;
		}
	}
	return TRUE;
}

