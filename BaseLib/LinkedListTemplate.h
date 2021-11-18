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
#ifndef __LINKED_LIST_TEMPLATE_H__
#define __LINKED_LIST_TEMPLATE_H__
#include "BaseLinkedListBlock.h"
#include "ConstructorCall.h"
#include "SystemAllocator.h"


template<class M>
class CLinkedListTemplate : public CBaseLinkedListBlock, public CPostMalloc<M>
{
private:
	unsigned int	muiDataSize;

public:
	void			Init(void);
	void			Init(CMallocator* pcMalloc);
	void			Kill(void);

	M*				InsertAfterTail(void);
	M*				InsertBeforeHead(void);
	M*				InsertBeforeNode(M* psPos);
	M*				InsertAfterNode(M* psPos); 
	M*				Add(void);

	M*				GetHead(void);
	M*				GetTail(void);
	M*				GetNext(M* pvData);
	M*				GetPrev(M* pvData);

	void			Remove(M* pvData);
	BOOL			SafeRemove(M* pvData);

	int				ByteSize(void);

	BOOL			Write(CFileWriter* pcFileWriter);
	BOOL			Read(CFileReader* pcFileReader);

	void			InsertDetachedAfterTail(M* pvData);

protected:	
	SLLNode*		AllocateDetached(void);
	SLLNode*		DataGetNode(M* pvData);

	BOOL			WriteHeader(CFileWriter* pcFileWriter);
	BOOL			WriteData(CFileWriter* pcFileWriter);
	BOOL			ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, int* piNumElements);
	BOOL			ReadData(CFileReader* pcFileReader, int iNumElements);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CLinkedListTemplate<M>::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CLinkedListTemplate<M>::Init(CMallocator* pcMalloc)
{
	CBaseLinkedListBlock::Init(pcMalloc, sizeof(SLLNode));
	muiDataSize = sizeof(M);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CLinkedListTemplate<M>::Kill(void)
{
	CBaseLinkedListBlock::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::GetHead(void)
{
	return (M*)CBaseLinkedListBlock::GetHead();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::GetTail(void)
{
	return (M*)CBaseLinkedListBlock::GetTail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::GetNext(M* pvData)
{
	return (M*)CBaseLinkedListBlock::GetNext(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::GetPrev(M* pvData)
{
	return (M*)CBaseLinkedListBlock::GetPrev(pvData);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CLinkedListTemplate<M>::Remove(M* pvData)
{
	CBaseLinkedListBlock::Remove(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::SafeRemove(M* pvData)
{
	return (M*)CBaseLinkedListBlock::SafeRemove(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CLinkedListTemplate<M>::ByteSize(void)
{
	int		iSize;
	M*	pvNode;

	iSize = 0;

	pvNode = GetHead();
	while (pvNode)
	{
		iSize += GetNodeSize(pvNode);
		pvNode = GetNext(pvNode);
	}

	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::InsertAfterTail(void)
{
	SLLNode*	psNode;

	psNode = AllocateDetached();
	return (M*)CBaseLinkedListBlock::InsertDetachedAfterTail(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::Add(void)
{
	return InsertAfterTail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::InsertBeforeHead(void)
{
	SLLNode* psNode;

	psNode = AllocateDetached();
	return (M*)CBaseLinkedListBlock::InsertDetachedBeforeHead(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::InsertBeforeNode(M* psPos)
{
	SLLNode* psNode;
	SLLNode* psNodePos;

	psNodePos = DataGetNode(psPos);
	psNode = AllocateDetached();
	return (M*)CBaseLinkedListBlock::InsertDetachedBeforeNode(psNode, psNodePos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::InsertAfterNode(M* psPos)
{
	SLLNode* psNode;
	SLLNode* psNodePos;

	psNodePos = DataGetNode(psPos);
	psNode = AllocateDetached();
	return (M*)CBaseLinkedListBlock::InsertDetachedAfterNode(psNode, psNodePos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SLLNode* CLinkedListTemplate<M>::AllocateDetached(void)
{
	return CBaseLinkedListBlock::AllocateDetached(muiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SLLNode* CLinkedListTemplate<M>::DataGetNode(M* pvData)
{
	return (SLLNode*)CBaseLinkedListBlock::DataGetNode(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CLinkedListTemplate<M>::InsertDetachedAfterTail(M* pvData)
{
	SLLNode* psNode;

	psNode = DataGetNode(pvData);
	CBaseLinkedListBlock::InsertDetachedAfterTail(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::Write(CFileWriter* pcFileWriter)
{
	BOOL	bResult;

	bResult = gcMallocators.Write(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = WriteData(pcFileWriter);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::WriteHeader(CFileWriter* pcFileWriter)
{
	SLinkedListTemplateDesc	sHeader;
	int						iNumElements;

	iNumElements = NumElements();
	sHeader.Init(iNumElements, muiNodeSize, muiDataSize);

	if (!pcFileWriter->WriteData(&sHeader, sizeof(SLinkedListTemplateDesc)))
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::WriteData(CFileWriter* pcFileWriter)
{
	M*		pvData;

	pvData = GetHead();
	while (pvData)
	{
		if (!pcFileWriter->WriteData(pvData, muiDataSize))
		{
			return FALSE;
		}

		pvData = GetNext(pvData);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	int				iNumElements;
	BOOL			bResult;
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.Read(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}

	bResult = ReadHeader(pcFileReader, pcMalloc, &iNumElements);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = ReadData(pcFileReader, iNumElements);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, int* piNumElements)
{
	SLinkedListTemplateDesc	sDesc;

	if (!pcFileReader->ReadData(&sDesc, sizeof(SLinkedListTemplateDesc)))
	{
		return FALSE;
	}

	if (sizeof(SLLNode) != sDesc.uiNodeSize)
	{
		return FALSE;
	}
	if (sizeof(M) != sDesc.uiDataSize)
	{
		return FALSE;
	}

	Init(pcMalloc);
	*piNumElements = sDesc.iNumElements;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::ReadData(CFileReader* pcFileReader, int iNumElements)
{
	int				i;
	M*				pvData;

	for (i = 0; i < iNumElements; i++)
	{
		pvData = InsertAfterTail();
		if (!pcFileReader->ReadData(pvData, muiDataSize))
		{
			return FALSE;
		}
	}
	return TRUE;
}


#endif // !__LINKED_LIST_TEMPLATE_H__

