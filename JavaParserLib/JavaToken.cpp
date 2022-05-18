#include "JavaToken.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaToken::Init(STextPosition* psPosition)
{
	msPos = *psPosition;
	mpcNext = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaToken::Kill(void)
{
	msPos.Clear();
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
STextPosition* CJavaToken::GetPosition(void)
{
	return &msPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* CJavaToken::GetNext(void) {	return mpcNext; }
BOOL CJavaToken::IsSeparator(void) { return FALSE; }
BOOL CJavaToken::IsComment(void) { return FALSE; }
BOOL CJavaToken::IsKeyword(void) { return FALSE; }
BOOL CJavaToken::IsIdentifier(void) { return FALSE; }
BOOL CJavaToken::IsScope(void) { return FALSE; }
BOOL CJavaToken::IsAmbiguous(void) { return FALSE; }
BOOL CJavaToken::IsOperator(void) { return FALSE; }
BOOL CJavaToken::IsLiteral(void) { return FALSE; }
BOOL CJavaToken::IsAnnotation(void) { return FALSE; }

