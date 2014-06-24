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
#ifndef __FILE_WRITER_H__
#define __FILE_WRITER_H__
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


#define CheckWrite(pvData, iSize)	if (Write(pvData, iSize, 1) != 1) { return FALSE; }


class CFileWriter
{
public:
	//General
						BOOL	WriteData(void* pvData, filePos iDataSize);

	//Chunking
						BOOL	WriteChunkBegin(void);
						BOOL	WriteChunkEnd(char* szChunkName);
						BOOL	WriteChunkEnd(int iChunkName);

	//Strings.
						BOOL	WriteString(char* szString);
						BOOL	WriteString(CChars* szString);

	//Arrays.
	template<class M>	BOOL	WriteArrayTemplateHeader(CArrayTemplate<M>* pcArray);
	template<class M>	BOOL	WriteArrayTemplate(CArrayTemplate<M>* pcArray);
						BOOL	WriteArrayInt(CArrayInt* pcArray);
						BOOL	WriteArrayUnknown(CArrayBlock* pcArray);
	template<class M>	BOOL	WriteArraySimple(CArraySimple<M>* pcArray);

	//LinkList.
	template<class M>	BOOL	WriteLinkListTemplate(CLinkListTemplate<M>* pcLinkList);
						BOOL	WriteLinkListBlock(CLinkListBlock* pcLinkList);

	//Trees.
	template<class M>	BOOL	WriteTreeTemplate(CTreeTemplate<M>* pcTree);
						BOOL	WriteTreeUnknown(CTreeBlock* pcTree);

	//Enumerator.		
	template<class M>	BOOL	WriteEnumeratorTemplate(CEnumeratorTemplate<M>* pcEnumerator);
						BOOL	WriteEnumeratorBlock(CEnumeratorBlock* pcEnumerator);

	//Helpers.		
						BOOL	WriteInt(int i);
						BOOL	WriteLong(long long int i);
						BOOL	WriteFloat(float f);
						BOOL	WriteDouble(double f);
						BOOL	WriteChar(char c);
						BOOL	WriteChar(unsigned char c);
						BOOL	WriteBool(BOOL b);
						BOOL	WriteShort(short int i);
						BOOL	WriteShort(unsigned short int i);
						BOOL	WriteBits(void* pvData, int iBitLength);

						BOOL	WriteIntArray(int* pai, int iLength);

protected:
	virtual				filePos	Write(const void* pvSource, filePos iSize, filePos iCount) =0;
};




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileWriter::WriteArrayTemplate(CArrayTemplate<M>* pcArray)
{
	ReturnOnFalse(WriteArrayTemplateHeader(pcArray));

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
template<class M>
BOOL CFileWriter::WriteArrayTemplateHeader(CArrayTemplate<M>* pcArray)
{
	SArrayTemplateHeader	sHeader;

	pcArray->GetHeader(&sHeader);
	CheckWrite(&sHeader, sizeof(SArrayTemplateHeader));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileWriter::WriteLinkListTemplate(CLinkListTemplate<M>* pcLinkList)
{
	M*		pvData;
	int		iElementSize;

	iElementSize = sizeof(M);
	CheckWrite(&iElementSize, sizeof(int));
	CheckWrite(pcLinkList, sizeof(CLinkListTemplate<M>));
	pvData = pcLinkList->GetHead();
	while (pvData)
	{
		CheckWrite(pvData, sizeof(M));
		pvData = pcLinkList->GetNext(pvData);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileWriter::WriteTreeTemplate(CTreeTemplate<M>* pcTree)
{
	M*		pvData;
	int		iElementSize;
	int		iPathSize;
	int		aiPath[1024];

	iElementSize = sizeof(M);
	CheckWrite(&iElementSize, sizeof(int));
	CheckWrite(pcTree, sizeof(CTreeTemplate<M>));

	if (pcTree->NumElements() != 0)
	{
		pvData = pcTree->StartTraversal();
		while (pvData != NULL)
		{
			iPathSize = pcTree->GetPathTo(aiPath, pvData);
			if (iPathSize >= 1024)
			{
				return FALSE; 
			}
			CheckWrite(&iPathSize, sizeof(int));
			if (iPathSize != 0)
			{
				CheckWrite(aiPath, sizeof(int) * iPathSize);
			}
			CheckWrite(pvData, sizeof(M));
			pvData = pcTree->TraverseFrom(pvData);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileWriter::WriteEnumeratorTemplate(CEnumeratorTemplate<M>* pcEnumerator)
{
	return WriteEnumeratorBlock(pcEnumerator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CFileWriter::WriteArraySimple(CArraySimple<M>* pcArray)
{
	int iElementSize;

	iElementSize = sizeof(M);
	CheckWrite(pcArray, sizeof(CArraySimple<M>));

	if (pcArray->NumElements() != 0)
	{
		CheckWrite(pcArray->GetData(), pcArray->ByteSize());
	}
	return TRUE;
}


#endif // __FILE_WRITER_H__

