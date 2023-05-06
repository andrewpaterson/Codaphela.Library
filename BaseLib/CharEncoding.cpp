#include "StringHelper.h"
#include "CharEncoding.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECharEncoding CCharEncoding::ParseEncoding(char* szEncodingString)
{
	if (StrEmpty(szEncodingString))
	{
		return CE_NoSet;
	}

	if (StringInsensitiveCompare(szEncodingString, "US-ASCII") == 0)
	{
		return CE_US_ASCII;
	}
	else if (StringInsensitiveCompare(szEncodingString, "UTF-8") == 0)
	{
		return CE_UTF_8;
	}
	else if (StringInsensitiveCompare(szEncodingString, "UTF-16") == 0)
	{
		return CE_UTF_16;
	}
	else if (StringInsensitiveCompare(szEncodingString, "ISO-8859-1") == 0)
	{
		return CE_ISO_8859_1;
	}
	else
	{
		return CE_Unkown;
	}
}

