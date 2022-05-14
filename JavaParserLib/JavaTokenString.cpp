#include "BaseLib/EscapeCodes.h"
#include "JavaTokenCharacter.h"
#include "JavaTokenString.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenString::Init(char* sz, int iLength)
{
	CJavaTokenLiteral::Init(JLT_String);
	msz = sz;
	meType = JST_string8;
	miLength = iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenString::Init(char16* sz, int iLength)
{
	msz = sz;
	meType = JST_string16;
	miLength = iLength;
	CJavaTokenLiteral::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenString::Kill(void)
{
	msz = NULL;
	miLength = 0;
	meType = JST_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaTokenString::GetType(void)
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
void CJavaTokenString::Print(CChars* pszDest)
{
	int		i;
	char16	c16;
	char	c8;
	char	sz[10];

	if (meType == JST_string8)
	{
		pszDest->Append('"');
		for (i = 0; i < miLength; i++)
		{
			c8 = ((char*)msz)[i];
			pszDest->Append(GetEscapeString(c8, sz));
		}
		pszDest->Append('"');
	}
	else if (meType == JST_string16)
	{
		pszDest->Append('"');
		for (i = 0; i < miLength; i++)
		{
			c16 = ((char16*)msz)[i];
			if ((c16 >= 32) && (c16 <= 255))
			{
				pszDest->Append((char)c16);
			}
			else
			{
				pszDest->Append("[?]");
			}

		}
		pszDest->Append('"');
	}
	else
	{
		pszDest->Append("[String?]");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaTokenString::Is(char* szString)
{
	if (meType == JST_string8)
	{
		int	iLength = StrLen(szString);

		if (miLength != iLength)
		{
			return FALSE;
		}
		else
		{
			return memcmp(szString, msz, iLength) == 0;
		}
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaTokenString::IsString(void) { return TRUE; }

