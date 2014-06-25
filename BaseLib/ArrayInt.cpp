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
#include <stdlib.h>
#include "ArrayInt.h"
#include "StringHelper.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayInt::AddRemap(int iElementPos, int iRemapNum)
{
	int*	pi;
	int		i;
	int		iNumToAdd;
	int		iMinusOne;

	if (iElementPos >= 0)
	{
		if (miUsedElements > iElementPos)
		{
			pi = (int*)CArrayTemplate<int>::Get(iElementPos);
			(*pi) = iRemapNum;
		}
		else
		{
			iMinusOne = -1;
			iNumToAdd = (iElementPos - miUsedElements);

			for (i = 0; i < iNumToAdd; i++)
			{
				Add(iMinusOne);
			}
			Add(iRemapNum);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayInt::RemoveRemap(int iElementPos)
{
	int*	pi;
	int		i;
	int		iCutDown;

	if (iElementPos < miUsedElements)
	{
		pi = (int*)CArrayTemplate<int>::Get(iElementPos);
		(*pi) = -1;
	}

	iCutDown = miUsedElements;
	for (i = miUsedElements-1; i >= 0; i--)
	{
		pi = (int*)CArrayTemplate<int>::Get(iElementPos);
		if (*pi == -1)
		{
			iCutDown = i;
		}
		else
		{
			break;
		}
	}
	if (iCutDown != miUsedElements)
	{
		SetUsedElements(iCutDown);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayInt::FindUnusedInSorted(void)
{
	int		i;
	int		iValue;

	if (miUsedElements == 0)
	{
		return 0;
	}

	for (i = 0; i < miUsedElements; i++)
	{
		iValue = *Get(i);
		if (iValue != i)
		{
			return i;
		}
	}
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayInt::Dump(void)
{
	//int			i;
	//int			iValue;
	//char		sz[32];


	//EngineOutput("[");
	//for (i = 0; i < miUsedElements; i++)
	//{
	//	iValue = GetValue(i);
	//	IToA(iValue, sz, 10);
	//	EngineOutput(sz);
	//	EngineOutput(" ");
	//}
	//EngineOutput("]\n");
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayInt::WriteArrayInt(CFileWriter* pcFileWriter)
{
	int		iElementSize;

	iElementSize = sizeof(int);
	if (pcFileWriter->WriteData(&iElementSize, sizeof(int))) 
	{ 
		return FALSE; 
	}
	if (!pcFileWriter->WriteData(this, sizeof(CArrayInt))) 
	{ 
		return FALSE; 
	}

	if (NumElements() != 0)
	{
		if (pcFileWriter->WriteData(GetData(), ByteSize())) 
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
BOOL CArrayInt::ReadArrayInt(CFileReader* pcFileReader)
{
	int			iElementSize;

	CheckRead(&iElementSize, sizeof(int));
	if (iElementSize != sizeof(int))
	{
		return FALSE;
	}
	CheckRead(pcArray, sizeof(CArrayInt));

	if (pcArray->NumElements() != 0)
	{
		pcArray->InitFromHeader();
		CheckRead(GetData(), ByteSize());
	}
	return TRUE;
}

