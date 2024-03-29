#include "BaseLib/EscapeCodes.h"
#include "JavaTokenCharacter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenCharacter::Init(STextPosition* psPosition, char c)
{
	CJavaTokenLiteral::Init(psPosition, JLT_Character);
	mc = c;
	meType = JCT_char8;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenCharacter::Init(STextPosition* psPosition, char16 c)
{
	CJavaTokenLiteral::Init(psPosition, JLT_Character);
	mc = c;
	meType = JCT_char16;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenCharacter::Kill(void)
{
	meType = JCT_Unknown;
	mc = -1;
	CJavaTokenLiteral::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaTokenCharacter::GetType(void)
{
	if (meType == JCT_char8)
	{
		return "Literal (char8)";
	}
	else if (meType == JCT_char16)
	{
		return "Literal (char16)";
	}
	else
	{
		return CJavaTokenLiteral::GetType();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaTokenCharacter::Is(char8 c)
{
	if (meType == JCT_char8)
	{
		return (char)mc == c;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaTokenCharacter::Is(char16 c)
{
	if (meType == JCT_char16)
	{
		return mc == c;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenCharacter::Print(CChars* pszDest)
{
	char	sz[10];

	pszDest->Append('\'');
	if (mc <= 255)
	{
		pszDest->Append(GetEscapeString((char)mc, sz));
	}
	else
	{
		pszDest->Append(GetEscapeString(mc, sz));
	}
	pszDest->Append('\'');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaTokenCharacter::IsCharacter(void)
{
	return true;
}


