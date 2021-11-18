/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "EnumeratorEnumerator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEnumeratorVoid* CEnumeratorEnumerator::GetEnumerator(char* szType)
{
	CEnumeratorVoid*	pcVoidEnumerator;

	__CEnumeratorEnumerator::Get(szType, &pcVoidEnumerator);
	return pcVoidEnumerator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEnumeratorVoid* CEnumeratorEnumerator::GetEnumerator(char* szType, int iTypeLen)
{
	CEnumeratorVoid*	pcVoidEnumerator;

	__CEnumeratorEnumerator::Get(szType, iTypeLen, &pcVoidEnumerator);
	return pcVoidEnumerator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEnumeratorVoid* CEnumeratorEnumerator::GetEnumerator(int iType)
{
	CEnumeratorVoid*	pcVoidEnumerator;

	__CEnumeratorEnumerator::GetWithID(iType, &pcVoidEnumerator, NULL);
	return pcVoidEnumerator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEnumeratorEnumerator::GetTypeName(int iType)
{
	char*	psz;

	__CEnumeratorEnumerator::GetWithID(iType, NULL, &psz);
	return psz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEnumeratorEnumerator::AddType(char* szType)
{
	CEnumeratorVoid		cVoidEnumerator;
	int					iID;

	iID = __CEnumeratorEnumerator::Get(szType, NULL);
	if (iID == -1)
	{
		cVoidEnumerator.Init();
		return  __CEnumeratorEnumerator::Add(szType, &cVoidEnumerator, 0, -1, FALSE);
	}
	return iID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEnumeratorEnumerator::AddType(char* szType, int iTypeLen)
{
	CEnumeratorVoid		cVoidEnumerator;
	int					iID;

	iID = __CEnumeratorEnumerator::Get(szType, NULL);
	if (iID == -1)
	{
		cVoidEnumerator.Init();
		return  __CEnumeratorEnumerator::Add(szType, iTypeLen, &cVoidEnumerator, 0, -1, FALSE);
	}
	return iID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEnumeratorEnumerator::Add(int iType, char* szText)
{
	CEnumeratorVoid*	pcVoidEnumerator;

	pcVoidEnumerator = GetEnumerator(iType);
	if (!pcVoidEnumerator)
	{
		return -1;
	}
	return pcVoidEnumerator->Add(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEnumeratorEnumerator::Add(char* szType, char* szText)
{
	int		iID;
	CEnumeratorVoid*	pcVoidEnumerator;

	//Perfectly safe to call this every time and not slow to get it by its ID.
	iID = AddType(szType);
	pcVoidEnumerator = GetEnumerator(iID);

	pcVoidEnumerator->Add(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEnumeratorEnumerator::Get(int iType, int iID)
{
	CEnumeratorVoid*	pcVoidEnumerator;
	
	pcVoidEnumerator = GetEnumerator(iType);
	return pcVoidEnumerator->Get(iID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEnumeratorEnumerator::Kill(void)
{
	SEnumeratorIterator		sIterator;
	CEnumeratorVoid*		pcVoidEnumerator;
	
	__CEnumeratorEnumerator::StartIteration(&sIterator, NULL, NULL, &pcVoidEnumerator);
	while (sIterator.bValid)
	{
		pcVoidEnumerator->Kill();
		__CEnumeratorEnumerator::Iterate(&sIterator, NULL, NULL, &pcVoidEnumerator);
	}
	__CEnumeratorEnumerator::Kill();
}

