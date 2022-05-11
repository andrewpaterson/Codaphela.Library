#include "BaseLib/EscapeCodes.h"
#include "JavaCharacter.h"
#include "JavaString.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaString::Init(char* sz, int iLength)
{
	CJavaLiteral::Init(JLT_String);
	msz = sz;
	meType = JST_string8;
	miLength = iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaString::Init(char16* sz, int iLength)
{
	msz = sz;
	meType = JST_string16;
	miLength = iLength;
	CJavaLiteral::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaString::Kill(void)
{
	msz = NULL;
	miLength = 0;
	meType = JST_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaString::GetType(void)
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
		return CJavaLiteral::GetType();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaString::Print(CChars* pszDest)
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
BOOL CJavaString::Is(char* szString)
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
BOOL CJavaString::IsString(void) { return TRUE; }

