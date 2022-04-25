#include "JavaCharacter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaCharacter::Init(char c)
{
	mc = c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaCharacter::Init(char16 c)
{
	mc = c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaCharacter::Kill(void)
{
	mc = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaCharacter::Print(CChars* pszDest)
{
	pszDest->Append('\'');
	if ((mc >= 32) && (mc <= 255))
	{
		pszDest->Append((char)mc);
	}
	else
	{
		pszDest->Append("[?]");
	}
	pszDest->Append('\'');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaCharacter::IsCharacter(void)
{
	return TRUE;
}

