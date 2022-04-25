#include "JavaToken.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaToken::Init(void)
{
	mpcNext = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaToken::Kill(void)
{
	mpcNext = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaToken::SetNext(CJavaToken* pcNext)
{
	mpcNext = pcNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* CJavaToken::GetNext(void)
{
	return mpcNext;
}

