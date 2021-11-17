#ifndef __INDEX_STRING_TEMPLATE_H__
#define __INDEX_STRING_TEMPLATE_H__
#include "IndexBlock.h"


template<class D>
class CIndexStringTemplate : public CIndexBlock
{
public:
	D*		Get(char* szKey, char* szLastCharInclusive = NULL);
	D*		Get(const char* szKey, char* szLastCharInclusive = NULL);

	D*		Put(char* szKey, char* szLastCharInclusive = NULL);
	BOOL	Put(char* szKey, D* psData, char* szLastCharInclusive = NULL);
	D*		Put(const char* szKey, const char* szLastCharInclusive = NULL);
	BOOL	Put(const char* szKey, D* psData, const char* szLastCharInclusive = NULL);


	BOOL	Remove(char* szKey, char* szLastCharInclusive = NULL);
	BOOL	Remove(const char* szKey, const char* szLastCharInclusive = NULL);

private:
	int		StringLength(char* szKey, char* szLastCharInclusive);
	int		StringLength(const char* szKey, const char* szLastCharInclusive);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexStringTemplate<D>::Get(char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StringLength(szKey, szLastCharInclusive);
	return CIndexBlock::Get(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexStringTemplate<D>::Get(const char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StringLength(szKey, szLastCharInclusive);
	return (D*)CIndexBlock::Get((void*)szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexStringTemplate<D>::Put(char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StringLength(szKey, szLastCharInclusive);
	return (D*)CIndexBlock::Put(szKey, iKeySize, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringTemplate<D>::Put(char* szKey, D* psData, char* szLastCharInclusive)
{
	int iKeySize = StringLength(szKey, szLastCharInclusive);
	return CIndexBlock::Put(szKey, iKeySize, psData, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexStringTemplate<D>::Put(const char* szKey, const char* szLastCharInclusive)
{
	int iKeySize = StringLength(szKey, szLastCharInclusive);
	return (D*)CIndexBlock::Put(szKey, iKeySize, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringTemplate<D>::Put(const char* szKey, D* psData, const char* szLastCharInclusive)
{
	int iKeySize = StringLength(szKey, szLastCharInclusive);
	return CIndexBlock::Put((void*)szKey, iKeySize, psData, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringTemplate<D>::Remove(char* szKey, char* szLastCharInclusive)
{
	int iKeySize = StringLength(szKey, szLastCharInclusive);
	return CIndexBlock::Remove(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CIndexStringTemplate<D>::Remove(const char* szKey, const char* szLastCharInclusive)
{
	int iKeySize = StringLength(szKey, szLastCharInclusive);
	return CIndexBlock::Remove(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
int CIndexStringTemplate<D>::StringLength(char* szKey, char* szLastCharInclusive)
{
	if (szLastCharInclusive)
	{
		return szLastCharInclusive - szKey;
	}
	else
	{
		return strlen(szKey);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
int CIndexStringTemplate<D>::StringLength(const char* szKey, const char* szLastCharInclusive)
{
	if (szLastCharInclusive)
	{
		return szLastCharInclusive - szKey + 1;
	}
	else
	{
		return strlen(szKey);
	}
}


#endif // !__INDEX_STRING_TEMPLATE_H__

