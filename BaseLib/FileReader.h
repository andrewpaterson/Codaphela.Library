/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __FILE_READER_H__
#define __FILE_READER_H__
#include "ArrayTemplate.h"
#include "ArrayInt.h"
#include "ArrayBlock.h"
#include "ArraySimple.h"
#include "LinkListTemplate.h"
#include "LinkListBlock.h"
#include "TreeTemplate.h"
#include "TreeBlock.h"
#include "EnumeratorTemplate.h"
#include "EnumeratorBlock.h"
#include "ErrorTypes.h"
#include "AbstractFile.h"


#define CheckRead(pvData, iSize) 	if (Read(pvData, iSize, 1) != 1) { return FALSE; }


class CFileReader
{
public:
	//The Read.
	virtual				filePos	Read(void* pvDest, filePos iSize, filePos iCount) =0;

	//General
						BOOL	ReadData(void* pvData, filePos iDataSize);

	//Chunking
						int		FindFirstChunkWithName(char* szName);
						int		FindFirstChunkWithName(int iName);
						int		FindNextChunkWithName(void);
						BOOL	ReadChunkBegin(int iChunkNum);  //ChunkIndex not ChunkName
						BOOL	ReadChunkEnd(void);
						BOOL	ReadChunkEnd(int iNumChunksToEnd);

	//Strings.
						BOOL	ReadStringLength(int* piLength);
						BOOL	ReadStringChars(char* szString, int iLength);
						BOOL	ReadString(CChars* szString);  //Call this one if the string was saved from a CChars
						BOOL	ReadString(CChars* szString, BOOL bDoesntMatter);  //Or this one if the string was saved from a char*

	//Arrays.
	template<class M>	BOOL	ReadArrayTemplate(CArrayTemplate<M>* pcArray);
	template<class M>	BOOL	ReadArrayTemplateHeader(CArrayTemplate<M>* pcArray);
						BOOL	ReadArrayInt(CArrayInt* pcArray);
						BOOL	ReadArrayUnknown(CArrayBlock* pcArray);
	template<class M>	BOOL	ReadArraySimple(CArraySimple<M>* pcArray);

	//LinkList.
	template<class M>	BOOL	ReadLinkListTemplate(CLinkListTemplate<M>* pcLinkList);
						BOOL	ReadLinkListBlock(CLinkListBlock* pcLinkList);

	//Trees.
	template<class M>	BOOL	ReadTreeTemplate(CTreeTemplate<M>* pcTree);
						BOOL	ReadTreeUnknown(CTreeBlock* pcTree);

	//Enumerator.		
	template<class M>	BOOL	ReadEnumeratorTemplate(CEnumeratorTemplate<M>* pcEnumerator);
						BOOL	ReadEnumeratorBlock(CEnumeratorBlock* pcEnumerator);

	//Helpers.		
						BOOL	ReadInt(int* pi);
						BOOL	ReadLong(long long int* plli);
						BOOL	ReadFloat(float* pf);
						BOOL	ReadDouble(double* pf);
						BOOL	ReadChar(char* pc);
						BOOL	ReadBool(BOOL* pb);
						BOOL	ReadShort(short int* pi);
						BOOL	ReadBits(void* pvData, int iByteLength);
						BOOL	ReadIntArray(int* pai, int iLength);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileReader::ReadArrayTemplate(CArrayTemplate<M>* pcArray)
{
	ReturnOnFalse(ReadArrayTemplateHeader(pcArray));
	if (pcArray->NumElements() != 0)
	{
		CheckRead(pcArray->GetData(), pcArray->ByteSize());
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileReader::ReadArrayTemplateHeader(CArrayTemplate<M>* pcArray)
{
	SArrayTemplateHeader	sHeader;

	CheckRead(&sHeader, sizeof(SArrayTemplateHeader));
	if (sHeader.miElementSize != sizeof(M))
	{
		return FALSE;
	}
	pcArray->Init(&sHeader);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileReader::ReadLinkListTemplate(CLinkListTemplate<M>* pcLinkList)
{
	M*			pvData;
	int			iNumElements;
	int			iElementSize;
	int			i;

	CheckRead(&iElementSize, sizeof(int));
	if (iElementSize != sizeof(M))
	{
		return FALSE;
	}
	CheckRead(pcLinkList, sizeof(CLinkListTemplate<M>));
	iNumElements = pcLinkList->miNumElements;
	pcLinkList->Init();
	for (i = 0; i < iNumElements; i++)
	{
		pvData = pcLinkList->InsertAfterTail();
		CheckRead(pvData, sizeof(M));
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileReader::ReadTreeTemplate(CTreeTemplate<M>* pcTree)
{
	M*			pvData[2];
	int			iElementSize;
	int			i;
	int			iPathSize[2];
	int			aiPath[2][1024];
	int			iNumElements;
	int			iPathNum;
	int			iOldPath;

	CheckRead(&iElementSize, sizeof(int));
	if (iElementSize != sizeof(M))
	{
		return FALSE;
	}

	CheckRead(pcTree, sizeof(CTreeTemplate<M>));
	iNumElements = pcTree->miNumElements;
	iPathNum = 0;
	iOldPath = 0;
	pcTree->Init();

	for (i = 0; i < iNumElements; i++)
	{
		CheckRead(&iPathSize[iPathNum], sizeof(int));
		if (iPathSize[iPathNum] != 0)
		{
			CheckRead(aiPath[iPathNum], sizeof(int) * iPathSize[iPathNum]);
		}
		pvData[iPathNum] = pcTree->InsertOnPath(aiPath[iPathNum], iPathSize[iPathNum], aiPath[iOldPath], iPathSize[iOldPath], pvData[iOldPath]);
		CheckRead(pvData[iPathNum], sizeof(M));

		iOldPath = iPathNum;
		iPathNum++;
		iPathNum = iPathNum % 2;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>	
BOOL CFileReader::ReadEnumeratorTemplate(CEnumeratorTemplate<M>* pcEnumerator)
{
	return ReadEnumeratorBlock(pcEnumerator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileReader::ReadArraySimple(CArraySimple<M>* pcArray)
{
	CheckRead(pcArray, sizeof(CArraySimple<M>));

	if (pcArray->NumElements() != 0)
	{
		pcArray->InitFromHeader();
		CheckRead(pcArray->GetData(), pcArray->ByteSize());
	}
	return TRUE;
}


#endif // __FILE_READER_H__

