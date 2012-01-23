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
#ifndef __ENUMERATOR_BLOCK_H__
#define __ENUMERATOR_BLOCK_H__
#include "EnumeratorTemplate.h"


//////////////////////////////////////////////////////////////////////////
//
//		!!!!!!!!!! CAUTION: Data size for node not stored. !!!!!!!!!!!
//
//////////////////////////////////////////////////////////////////////////


class CEnumeratorBlock : public CEnumeratorTemplate<void>
{
public:
	int		AddGetNode(char* szName, void* pvData, int iDataSize, int iKeySize, int iNum, BOOL bReplace, SENode** pcThisNode);
	int		Add(char* szName, void* pvData, int iDataSize, int iKeySize, int iNum, BOOL bReplace = TRUE);
	void*	Add(char* szName, int iDataSize = 0);
	void*	Add(char* szName, int iDataSize, int iNum);
};


#endif // __ENUMERATOR_BLOCK_H__

