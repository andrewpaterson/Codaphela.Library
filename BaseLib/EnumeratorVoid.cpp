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
#include "EnumeratorVoid.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEnumeratorVoid::Add(char* szName, int iNum)
{
	return PrivateAddGetNode(szName, NULL, 0, 0, iNum, FALSE, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEnumeratorVoid::Add(char* szName)
{
	return PrivateAddGetNode(szName, NULL, 0, 0, -1, FALSE, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEnumeratorVoid::Get(char* szName)
{
	return __CEnumeratorTemplate<void>::Get(szName, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEnumeratorVoid::Get(char* szName, int iNameLen)
{
	return __CEnumeratorTemplate<void>::Get(szName, iNameLen, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEnumeratorVoid::Get(int iID)
{
	char*	szName;

	__CEnumeratorTemplate<void>::GetWithID(iID, NULL, &szName);
	return szName;
}

