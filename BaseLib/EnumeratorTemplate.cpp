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
#include "EnumeratorTemplate.h"
#include "StringHelper.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int AlphabeticalComparisionCallbackCaseSensitive(const void* pvNode1, const void* pvNode2)
{
	SENode* psNode1;
	SENode*	psNode2;

	psNode1 = (SENode*)pvNode1;
	psNode2 = (SENode*)pvNode2;

	if ((!psNode1->szName) || (!psNode2->szName))
	{
		return 0;
	}

	return strcmp(psNode1->szName, psNode2->szName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int AlphabeticalComparisionCallbackCaseInsensitive(const void* pvNode1, const  void* pvNode2)
{
	SENode* psNode1;
	SENode*	psNode2;

	psNode1 = (SENode*)pvNode1;
	psNode2 = (SENode*)pvNode2;

	if ((!psNode1->szName) || (!psNode2->szName))
	{
		return 0;
	}

	return StrICmp(psNode1->szName, psNode2->szName);
}

