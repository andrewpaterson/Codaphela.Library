#include "JavaTokenAnnotation.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenAnnotation::Init(STextPosition* psPosition, char* szAnnotation, int iLength)
{
	CJavaToken::Init(psPosition);
	mszAnnotation = szAnnotation;
	miLength = iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenAnnotation::Kill(void)
{
	mszAnnotation = NULL;
	miLength = 0;
	CJavaToken::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenAnnotation::Print(CChars* pszDest)
{
	pszDest->Append('@');
	pszDest->Append(mszAnnotation, miLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaTokenAnnotation::Is(char* szAnnotation)
{
	int	iLength = StrLen(szAnnotation);

	if (miLength != iLength)
	{
		return false;
	}
	else
	{
		return memcmp(szAnnotation, mszAnnotation, iLength) == 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaTokenAnnotation::GetType(void) { return "Annotation"; }
bool CJavaTokenAnnotation::IsAnnotation(void) { return true; }

