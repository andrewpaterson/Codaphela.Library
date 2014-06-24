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
#include "FileReader.h"
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadData(void* pvData, filePos iDataSize)
{
	CheckRead(pvData, iDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadArrayInt(CArrayInt* pcArray)
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
		CheckRead(pcArray->GetData(), pcArray->ByteSize());
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadStringLength(int* piLength)
{
	CheckRead(piLength, sizeof(int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadStringChars(char* szString, int iLength)
{
	CheckRead(szString, iLength);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadString(CChars* szString, BOOL bDoesntMatter)
{
	int	iLength;

	ReturnOnFalse(ReadStringLength(&iLength));
	if (iLength == 0)
	{
		szString->Init();
		return TRUE;
	}
	else if (iLength > 0)
	{
		szString->Init('@', iLength-1);
		CheckRead(szString->Text(), iLength);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadString(CChars* szString)
{
	return ReadArrayTemplate(&szString->mcText);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadArrayUnknown(CArrayBlock* pcArray)
{
	CheckRead(pcArray, sizeof(CArrayBlock));

	if (pcArray->NumElements() != 0)
	{
		pcArray->InitFromHeader();
		CheckRead(pcArray->GetData(), pcArray->ByteSize());
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadLinkListBlock(CLinkListBlock* pcLinkList)
{
	int				iNumElements;
	int				i;
	SUnknownType	sType;
	void*			pvData;

	CheckRead(pcLinkList, sizeof(CLinkListBlock));
	iNumElements = pcLinkList->miNumElements;
	pcLinkList->Init();
	for (i = 0; i < iNumElements; i++)
	{
		CheckRead(&sType, sizeof(SUnknownType));
		pvData = pcLinkList->InsertAfterTail(sType.miSize, sType.miType);
		CheckRead(pvData, sType.miSize);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadTreeUnknown(CTreeBlock* pcTree)
{
	void*			pvData;
	int				i;
	int				iPathSize;
	int				aiPath[1024];
	int				iNumElements;
	SUnknownType	sType;

	CheckRead(pcTree, sizeof(CTreeBlock));
	iNumElements = pcTree->NumElements();
	pcTree->Init();

	for (i = 0; i < iNumElements; i++)
	{
		CheckRead(&sType, sizeof(SUnknownType));
		CheckRead(&iPathSize, sizeof(int));
		CheckRead(aiPath, sizeof(int) * iPathSize);
		pvData = pcTree->InsertOnPath(aiPath, iPathSize, sType.miSize, sType.miType);
		CheckRead(pvData, sType.miSize);
	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadEnumeratorBlock(CEnumeratorBlock* pcEnumerator)
{
	SENode*		psNode;
	int			iReadSize;

	CheckRead(&pcEnumerator->mbCaseSensitive, sizeof(BOOL));
	ReturnOnFalse(ReadArrayTemplate(&pcEnumerator->mcIDArray));
	ReturnOnFalse(ReadLinkListTemplate(&pcEnumerator->mcList));

	psNode = pcEnumerator->mcList.GetHead();
	while (psNode)
	{
		//Actually this is copied from ReadString because I need the string length first.
		CheckRead(&iReadSize, sizeof(int));

		pcEnumerator->AllocateNodeData(psNode, iReadSize);
		CheckRead(psNode->szName, iReadSize);
		if (psNode->iDataSize)
		{
			ReturnOnFalse(ReadData(psNode->pvData, psNode->iDataSize));
		}
		psNode = pcEnumerator->mcList.GetNext(psNode);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadInt(int* pi)
{
	CheckRead(pi, sizeof(int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadLong(long long int* plli)
{
	CheckRead(plli, sizeof(long long int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadFloat(float* pf)
{
	CheckRead(pf, sizeof(float));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadDouble(double* pf)
{
	CheckRead(pf, sizeof(double));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadChar(char* pc)
{
	CheckRead(pc, sizeof(char));
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadChar(unsigned char* pc)
{
	CheckRead(pc, sizeof(unsigned char));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadBool(BOOL* pb)
{
	CheckRead(pb, sizeof(BOOL));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadShort(short int* pi)
{
	CheckRead(pi, sizeof(short int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadShort(unsigned short int* pi)
{
	CheckRead(pi, sizeof(unsigned short int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadBits(void* pvData, int iByteLength)
{
	CheckRead(pvData, iByteLength);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadIntArray(int* pai, int iLength)
{
	CheckRead(pai, iLength * sizeof(int));
	return TRUE;
}
