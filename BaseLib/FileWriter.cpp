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
#include "FileWriter.h"
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteData(void* pvData, filePos iDataSize)
{
	CheckWrite(pvData, iDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteArrayInt(CArrayInt* pcArray)
{
	int		iElementSize;

	iElementSize = sizeof(int);
	CheckWrite(&iElementSize, sizeof(int));
	CheckWrite(pcArray, sizeof(CArrayInt));

	if (pcArray->NumElements() != 0)
	{
		CheckWrite(pcArray->GetData(), pcArray->ByteSize());
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteString(char* szString)
{
	int		iStrLen;

	iStrLen = (int)strlen(szString)+1;
	CheckWrite(&iStrLen, sizeof(int));
	CheckWrite(szString, iStrLen);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteString(CChars* szString)
{
	return WriteArrayTemplate(&szString->mcText);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteArrayUnknown(CArrayBlock* pcArray)
{
	CheckWrite(pcArray, sizeof(CArrayBlock));

	if (pcArray->NumElements() != 0)
	{
		CheckWrite(pcArray->GetData(), pcArray->ByteSize());
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteLinkListBlock(CLinkListBlock* pcLinkList)
{
	void*			pvData;
	int				i;
	SUnknownType	sType;

	CheckWrite(pcLinkList, sizeof(CLinkListBlock));
	pvData = pcLinkList->GetHead();
	for (i = 0; i < pcLinkList->miNumElements; i++)
	{
		pcLinkList->GetNodeTypeAndSize(pvData, &sType);
		CheckWrite(&sType, sizeof(SUnknownType));
		CheckWrite(pvData, sType.miSize);
		pvData = pcLinkList->GetNext(pvData);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteTreeUnknown(CTreeBlock* pcTree)
{
	void*			pvData;
	int				iPathSize;
	int				aiPath[1024];
	SUnknownType	sType;

	CheckWrite(pcTree, sizeof(CTreeBlock));

	if (pcTree->miNumElements != 0)
	{
		pvData = pcTree->StartTraversal();
		while (pvData != NULL)
		{
			iPathSize = pcTree->GetPathTo(aiPath, pvData);
			if (iPathSize >= 1024)
			{
				return FALSE; 
			}
			pcTree->GetNodeTypeAndSize(pvData, &sType);
			CheckWrite(&sType, sizeof(SUnknownType));
			CheckWrite(&iPathSize, sizeof(int));
			CheckWrite(aiPath, sizeof(int) * iPathSize);
			CheckWrite(pvData, sType.miSize);
			pvData = pcTree->TraverseFrom(pvData);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteEnumeratorBlock(CEnumeratorBlock* pcEnumerator)
{
	SENode*		psNode;

	CheckWrite(&pcEnumerator->mbCaseSensitive, sizeof(BOOL));
	ReturnOnFalse(WriteArrayTemplate(&pcEnumerator->mcIDArray));
	ReturnOnFalse(WriteLinkListTemplate(&pcEnumerator->mcList));

	psNode = pcEnumerator->mcList.GetHead();
	while (psNode)
	{
		ReturnOnFalse(WriteString(psNode->szName));
		if (psNode->iDataSize)
		{
			ReturnOnFalse(WriteData(psNode->pvData, psNode->iDataSize));
		}
		psNode = pcEnumerator->mcList.GetNext(psNode);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteInt(int i)
{
	CheckWrite(&i, sizeof(int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteFloat(float f)
{
	CheckWrite(&f, sizeof(float));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteDouble(double f)
{
	CheckWrite(&f, sizeof(double));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteChar(char c)
{
	CheckWrite(&c, sizeof(char));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteBool(BOOL b)
{
	CheckWrite(&b, sizeof(BOOL));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteShort(short int i)
{
	CheckWrite(&i, sizeof(short int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteLong(long long int i)
{
	CheckWrite(&i, sizeof(long long int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteBits(void* pvData, int iByteLength)
{
	CheckWrite(pvData, iByteLength);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteIntArray(int* pai, int iLength)
{
	CheckWrite(pai, iLength * sizeof(int));
	return TRUE;
}

