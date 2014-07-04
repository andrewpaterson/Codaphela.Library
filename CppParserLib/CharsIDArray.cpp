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
#include "CharsIDArray.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsIDArray::Init(int iChunkSize)
{
	mcArray.Init(iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsIDArray::Kill(void)
{
	int		i;
	CCharsID*	pcChars;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcChars = mcArray.Get(i);
		pcChars->Kill();
	}
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CCharsIDArray::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCharsID* CCharsIDArray::Add(CCharsID* pcChars)
{
	CCharsID*		pcChars2;

	pcChars2 = mcArray.Add();
	pcChars2->Init(*pcChars);
	return pcChars2;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCharsID* CCharsIDArray::Add(char* szString)
{
	CCharsID*		pcChars2;

	pcChars2 = mcArray.Add();
	pcChars2->Init(szString);
	return pcChars2;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCharsID* CCharsIDArray::Add(char* szString, int iStartInclusive, int iEndExclusive)
{
	CCharsID*		pcChars2;

	pcChars2 = mcArray.Add();
	pcChars2->Init(szString, iStartInclusive, iEndExclusive);
	return pcChars2;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCharsID* CCharsIDArray::Add(void)
{
	CCharsID*		pcChars;

	pcChars = mcArray.Add();
	pcChars->Init();
	return pcChars;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsIDArray::Remove(CCharsID* pcChars)
{
	Remove(pcChars->Text());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsIDArray::Remove(char* szString)
{
	int			i;
	CCharsID*		pcChars2;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcChars2 = mcArray.Get(i);
		if (pcChars2->Equals(szString))
		{
			Remove(i);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsIDArray::Remove(int iIndex, BOOL bPreserveOrder)
{
	CCharsID*		pcChars;

	pcChars = mcArray.Get(iIndex);
	pcChars->Kill();
	mcArray.RemoveAt(iIndex, bPreserveOrder);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCharsID* CCharsIDArray::Get(int iIndex)
{
	return mcArray.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CCharsIDArray::GetText(int iIndex)
{
	CCharsID*		pcChars;

	pcChars = mcArray.SafeGet(iIndex);
	if (pcChars)
	{
		return pcChars->Text();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CCharsIDArray::GetIndex(char* szStart, int iLen)
{
	int			i;
	CCharsID*		psz;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psz = mcArray.Get(i);
		if (psz->Equals(szStart, iLen))
		{
			return i;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsIDArray::Copy(CCharsIDArray* pcSource)
{
	int		i;
	CCharsID*	psz;

	for (i = 0; i < pcSource->NumElements(); i++)
	{
		psz = pcSource->Get(i);
		Add(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CCharsIDArray::Equals(CCharsIDArray* pcOther)
{
	int		i;
	CCharsID*	pszThis;
	CCharsID*	pszOther;

	if (mcArray.NumElements() != pcOther->mcArray.NumElements())
	{
		return FALSE;
	}

	for (i = 0; i < NumElements(); i++)
	{
		pszThis = Get(i);
		pszOther = pcOther->Get(i);
		if (!pszThis->Equals(*pszOther))
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsIDArray::Dump(void)
{
	int			i;
	CCharsID*		psz;
	CCharsID		sz;

	sz.Init(1024);
	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psz = mcArray.Get(i);
		sz.Append(psz);
		sz.AppendNewLine();
	}
	sz.Dump();
	sz.Kill();
}

