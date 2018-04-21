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
#ifndef __LINKED_LIST_TEMPLATE_H__
#define __LINKED_LIST_TEMPLATE_H__
#include "Define.h"
#include "DataMacro.h"
#include "FileIO.h"
#include "LinkedListBlock.h"


template<class M>
class CLinkedListTemplate : public CLinkedListBlock
{
public:
	M* 		InsertAfterTail(void);
	M* 		InsertBeforeHead(void);
	M*		InsertBeforeNode(M* psPos);
	M*		InsertAfterNode(M* psPos);
	M*		Add(void);

	M* 		GetHead(void);
	M* 		GetTail(void);
	M* 		GetNext(M* pvData);
	M* 		GetPrev(M* pvData);
	M*		Get(int iNum);

	int		IndexOf(M* pvData);
	BOOL	IsInList(M* pvData);

	BOOL	Write(CFileWriter* pcFileWriter);
	BOOL	Read(CFileReader* pcFileReader);
};



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::GetHead(void)
{
	return (M*)CLinkedListBlock::GetHead();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::GetTail(void)
{
	return (M*)CLinkedListBlock::GetTail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::GetNext(M* pvData)
{
	return (M*)CLinkedListBlock::GetNext(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::GetPrev(M* pvData)
{
	return (M*)CLinkedListBlock::GetPrev(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::Get(int iNum)
{
	return (M*)CLinkedListBlock::Get(iNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::IsInList(M* pvData)
{
	return CLinkedListBlock::IsInList(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CLinkedListTemplate<M>::IndexOf(M* pvData)
{
	return CLinkedListBlock::IndexOf(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::InsertAfterTail(void)
{
	return (M*)CLinkedListBlock::InsertAfterTail(sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::InsertBeforeHead(void)
{
	return (M*)CLinkedListBlock::InsertBeforeHead(sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::InsertBeforeNode(M* psPos)
{
	return (M*)CLinkedListBlock::InsertBeforeNode(sizeof(M), psPos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::InsertAfterNode(M* psPos)
{
	return (M*)CLinkedListBlock::InsertAfterNode(sizeof(M), psPos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkedListTemplate<M>::Add(void)
{
	return (M*)CLinkedListBlock::InsertAfterTail(sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::Write(CFileWriter* pcFileWriter)
{
	return CLinkedListBlock::Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CLinkedListTemplate<M>::Read(CFileReader* pcFileReader)
{
	return CLinkedListBlock::Read(pcFileReader);
}


#endif // __LINKED_LIST_TEMPLATE_H__

