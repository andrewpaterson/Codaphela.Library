#ifndef __LIST_TEMPLATE_H__
#define __LIST_TEMPLATE_H__
#include "ListBlock.h"

template<class M>
class CListTemplate : public CListBlock
{
public:
	void	Init(int iChunkSize);
	void	Init(int iChunkSize, int iAlignment);
	void	Init(int iChunkSize, int iAlignment, int iOffset);
	void	Kill(void);

	M*		Add(void);
	M*		Add(M* pvData);
	M* 		AddGetIndex(int* piIndex);

	M*		Get(int iIndex);
	M*		SafeGet(int iIndex);
	int		GetIndex(M* pvElement);
	M*		Tail(void);

	M* 		InsertAt(int iIndex);
	M* 		InsertAt(M* pvData, int iIndex);

	void	Pop(M* pvData);
	M*	 	Push(M* pvElement);
	M*		Push(void);

	BOOL	Contains(M* pvData);

	BOOL	Set(int iIndex, M* pvData);
	BOOL	SafeSet(int iIndex, M* pvData);
};



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CListTemplate<M>::Init(int iChunkSize)
{
	CListBlock::Init(iChunkSize, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CListTemplate<M>::Init(int iChunkSize, int iAlignment)
{
	CListBlock::Init(iChunkSize, sizeof(M), iAlignment);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CListTemplate<M>::Init(int iChunkSize, int iAlignment, int iOffset)
{
	CListBlock::Init(iChunkSize, sizeof(M), iAlignment, iOffset);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CListTemplate<M>::Kill(void)
{
	CListBlock::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::Add(void)
{
	return (M*)CListBlock::Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::Add(M* pvData)
{
	return (M*)CListBlock::Add(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::AddGetIndex(int* piIndex)
{
	return (M*)CListBlock::AddGetIndex(piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::Get(int iIndex)
{
	return (M*)CListBlock::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::SafeGet(int iIndex)
{
	return (M*)CListBlock::SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CListTemplate<M>::GetIndex(M* pvElement)
{
	return CListBlock::GetIndex(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::Tail(void)
{
	return (M*)CListBlock::Tail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::InsertAt(int iIndex)
{
	return (M*)CListBlock::InsertAt(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::InsertAt(M* pvData, int iIndex)
{
	return (M*)CListBlock::InsertAt(pvData, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CListTemplate<M>::Pop(M* pvData)
{
	CListBlock::Pop(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::Push(M* pvElement)
{
	return (M*)CListBlock::Push(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplate<M>::Push(void)
{
	return (M*)CListBlock::Push();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CListTemplate<M>::Contains(M* pvData)
{
	return CListBlock::Contains(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CListTemplate<M>::Set(int iIndex, M* pvData)
{
	return CListBlock::Set(iIndex, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CListTemplate<M>::SafeSet(int iIndex, M* pvData)
{
	return CListBlock::SafeSet(iIndex, pvData);
}


#endif // __LIST_TEMPLATE_H__
