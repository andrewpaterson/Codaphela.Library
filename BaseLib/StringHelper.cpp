/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <string.h>
#include <ctype.h>
#include "DataTypes.h"
#include "EscapeCodes.h"
#include "StringHelper.h"


char	gszDigits[17] = { "0123456789abcdef" };
char	gszEmptyString[4] = { '\0', '\0', '\0', '\0' };


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
int StringCompare(const char* ps1, const char* ps2)
{
	int s1;
	int s2;

	if ((ps1 == NULL) && (ps2 == NULL))
	{
		return 0;
	}

	if (ps1 == NULL)
	{
		return -1;
	}
	if (ps2 == NULL)
	{
		return 1;
	}

	do 
	{
		s1 = *ps1++;
		s2 = *ps2++;
		if (s1 == 0)
			break;
	} while (s1 == s2);

	return (s1 < s2) ? -1 : (s1 > s2);
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
int StringInsensitiveCompare(const char* ps1, const char* ps2)
{
	int s1;
	int s2;

    if ((ps1 == NULL) && (ps2 == NULL))
    {
        return 0;
    }

    if (ps1 == NULL)
    {
        return -1;
    }
    if (ps2 == NULL)
    {
        return 1;
    }

	do
	{
		s1 = *ps1++;
		s2 = *ps2++;
		if (s1 == 0)
			break;
		s1 = ToLower(s1);
		s2 = ToLower(s2);
	} while (s1 == s2);

	return (s1 < s2) ? -1 : (s1 > s2);
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
int MemICmp(const void* pv1, const void* pv2, int iLength)
{
    int             i;
    int             c1;
    int             c2;
    int             iDiff;

    if ((pv1 == NULL) && (pv2 == NULL))
    {
        return 0;
    }

    if (pv1 == NULL)
    {
        return -1;
    }
    if (pv2 == NULL)
    {
        return 1;
    }


    if (iLength == 0)
    {
        return 0;
    }

    for (i = 0; i < iLength; i++)
    {
        c1 = tolower(((char*)pv1)[i]);
        c2 = tolower(((char*)pv2)[i]);

        iDiff = c1 - c2;
        if (iDiff != 0)
        {
            return iDiff;
        }
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
int MemCmp(const void* pv1, int iLen1, const void* pv2, int iLen2)
{
	int		iSize;
	int		iResult;

	iSize = iLen1;
	if (iSize == iLen2)
	{
		iResult = memcmp(pv1, pv2, iSize);
	}
	else if (iSize < iLen2)
	{
		iResult = memcmp(pv1, pv2, iSize);
		if (iResult == 0)
		{
			iResult = -1;
		}
	}
	else
	{
		iSize = iLen2;
		iResult = memcmp(pv1, pv2, iSize);
		if (iResult == 0)
		{
			iResult = 1;
		}
	}

	if (iResult < -1)
	{
		iResult = -1;
	}
	else if (iResult > 1)
	{
		iResult = 1;
	}
	return iResult;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
void MemSwp(const void* pv1, const void* pv2, size_t uiLength)
{
	int		i;
	int		iIntLength;
	int		iTemp;
	int		iRemainder;
	char	cTemp;
	int		iCharIndex;
	
	iIntLength = uiLength / sizeof(int);
	iRemainder = uiLength % sizeof(int);
	for (i = 0; i < iIntLength; i++)
	{
		iTemp = ((int*)pv1)[i];
		((int*)pv1)[i] = ((int*)pv2)[i];
		((int*)pv2)[i] = iTemp;
	}

	iCharIndex = iIntLength * sizeof(int);
	for (i = 0; i < iRemainder; i++)
	{
		cTemp = ((char*)pv1)[iCharIndex + i];
		((char*)pv1)[iCharIndex + i] = ((char*)pv2)[iCharIndex + i];
		((char*)pv2)[iCharIndex + i] = cTemp;
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* IntToString(char* szDest, int iDestLength, int iValue, int iBase)
{
	int		iQuotient;
	int     iDigit;
	int     iPos;
	int		bNegative;

	if (iBase != 10)
	{
		return IntToString(szDest, iDestLength, (unsigned int)iValue, iBase);
	}

	bNegative = iValue < 0;
	if (bNegative)
	{
		if (iValue == MIN_INT)
		{
			strcpy(szDest, "-2147483648");
			return szDest;
		}
		iValue *= -1;
	}
	iQuotient = iValue;
	iPos = 0;

	do
	{
		iDigit = iQuotient % iBase;
		iQuotient /= iBase;

		szDest[iPos] = gszDigits[iDigit];
		iPos++;

	} while (iQuotient != 0);

	if (bNegative)
	{
		szDest[iPos] = '-';
		iPos++;
	}

	szDest[iPos] = '\0';

	//Reverse the string.
	StrRev(szDest, iPos);

	return szDest;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* IntToString(char* szDest, int iDestLength, unsigned int iValue, int iBase)
{
	unsigned int	iQuotient;
	unsigned int    iDigit;
	unsigned int    iPos;

	if (iBase < 2 || iBase > 16)
	{
		szDest[0] = 0;
		return szDest;
	}

	iQuotient = iValue;
	iPos = 0;

	do
	{
		iDigit = iQuotient % iBase;
		iQuotient /= iBase;

		szDest[iPos] = gszDigits[iDigit];
		iPos++;

	} while (iQuotient != 0);


	szDest[iPos] = '\0';

	//Reverse the string.
	StrRev(szDest, iPos);

	return szDest;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* IntToString(char* szDest, int iDestLength, uint64 ulliValue, int iBase)
{
	uint64			ulliQuotient;
	unsigned int    iDigit;
	unsigned int    iPos;

	if (iBase < 2 || iBase > 16)
	{
		szDest[0] = 0;
		return szDest;
	}

	ulliQuotient = ulliValue;
	iPos = 0;

	do
	{
		iDigit = (unsigned int)(ulliQuotient % iBase);
		ulliQuotient /= iBase;

		szDest[iPos] = gszDigits[iDigit];
		iPos++;

	} while (ulliQuotient != 0);


	szDest[iPos] = '\0';

	//Reverse the string.
	StrRev(szDest, iPos);

	return szDest;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* IntToString(char* szDest, int iDestLength, int64 lliValue, int iBase)
{
	int64	lliQuotient;
	int		iDigit;
	int		iPos;
	int		bNegative;

	if (iBase != 10)
	{
		return IntToString(szDest, iDestLength, (uint64)lliValue, iBase);
	}

	bNegative = lliValue < 0;
	if (bNegative && (iBase == 10))
	{
		lliValue *= -1;
	}
	lliQuotient = lliValue;
	iPos = 0;

	do
	{
		iDigit = (int)(lliQuotient % iBase);
		lliQuotient /= iBase;

		szDest[iPos] = gszDigits[iDigit];
		iPos++;

	} while (lliQuotient != 0);


	// Only apply negative sign for iBase 10
	if (bNegative && (iBase == 10))
	{
		szDest[iPos] = '-';
		iPos++;
	}

	szDest[iPos] = '\0';

	//Reverse the string.
	StrRev(szDest, iPos);

	return szDest;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* StrRev(char* szString, int iLength)
{
	int  i;
	char c;

	if (iLength <= 1)
	{
		return szString;
	}

	for (i = 0; i < iLength/2; i++)
	{
		c = szString[iLength-1-i];
		szString[iLength-1-i] = szString[i];
		szString[i] = c;
	}
	return szString;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* StrRev(char* szString)
{
	int iLength;

	iLength = (int)strlen(szString);
	return StrRev(szString, iLength);
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* StrIStr(char* ps1, const char* ps2)
{
    int iS1Len;
    int iS2Len;
    int iSearchCount;
    int i;

    if ((ps1 == NULL) || (ps2 == NULL))
    {
        return NULL;
    }

    iS1Len = (int)strlen(ps1);
    iS2Len = (int)strlen(ps2);

    if ((iS1Len == 0) || (iS2Len == 0))
    {
        return NULL;
    }

    if (iS2Len > iS1Len)
    {
        //If the SearchFor string is longer than the string we're searching in then we can't find it.
        return NULL;
    }

    iSearchCount = iS1Len - iS2Len;

    for (i = 0; i <= iSearchCount; i++)
    {
        if (MemICmp(ps2, &ps1[i], iS2Len) == 0)
        {
            return &ps1[i];
        }
    }
    return NULL;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
const char* FindChar(const char* szString, char c, bool bReverse)
{
	int iLen;
	int i;

	if (szString)
	{
		iLen = (int)strlen(szString);

		if (bReverse)
		{
			for (i =  iLen-1; i >= 0; i--)
			{
				if (szString[i] == c)
				{
					return &szString[i];
				}
			}
		}
		else
		{
			for (i =  0; i < iLen; i++)
			{
				if (szString[i] == c)
				{
					return &szString[i];
				}
			}
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int FindCharIndex(char chr, char* str, int index)
{
	int i = 0;

	str = str + index;
	do
	{
		if (str[i] == chr)
		{
			return i;
		}
		i++;
	} while (str[i]);

	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void StrCpySafeStripSurroundingSpaces(char* szDest, char* szSource, size_t uiMaxLength)
{
	uint32	uiSourceLength;
	uint32	uiLeadSpaces = 0;
	uint32	uiLastChar = 0;
	uint32	i;
	char* szFirstCharOfDest = szDest;

	uiSourceLength = strlen(szSource);

	for (i = 0; i < uiSourceLength && szSource[i] == ' '; i++)
	{
		uiLeadSpaces++;
	}

	if (uiSourceLength == uiLeadSpaces)
	{
		*szDest = '\0';
		return;
	}

	for (uiLastChar = uiSourceLength - 1; uiLastChar > 0 && (szSource[uiLastChar] == ' '); uiLastChar--);

	for (i = uiLeadSpaces; i <= uiLastChar; i++)
	{
		*szDest++ = szSource[i];
		if (!uiMaxLength--)
		{
			*szFirstCharOfDest = '\0';
			return;
		}
	}

	*szDest = '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* StrCpySafe(char* szDest, const char* szSource, int iDestLength, int* piSourceLength)
{
	int iLen;

	iLen = (int)strlen(szSource);
	SafeAssign(piSourceLength, iLen);

	if (iDestLength == 0)
	{
		return szDest;
	}

	if (iLen >= iDestLength)
	{
		iLen = iDestLength-1;
	}
	memcpy(szDest, szSource, iLen);
	szDest[iLen] = 0;

	return szDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToLower(char* szString)
{
	int		iLen;	
	int		i;
	char	c;
	
	if (szString)
	{
		iLen = (int)strlen(szString);
		for (i = 0; i < iLen; i++)
		{
			c = szString[i];
			if ((c >= 'A') && (c <= 'Z'))
			{
				szString[i] = c + 'a' - 'A';
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToUpper(char* szString)
{
	int		iLen;	
	int		i;
	char	c;

	if (szString)
	{
		iLen = (int)strlen(szString);
		for (i = 0; i < iLen; i++)
		{
			c = szString[i];
			if ((c >= 'a') && (c <= 'z'))
			{
				szString[i] = c + 'A' - 'a';
			}
		}
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char ToLower(char c)
{
	if ((c >= 'A') && (c <= 'Z'))
	{
		return c + 'a' - 'A';
	}
	return c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char ToUpper(char c)
{
	if ((c >= 'a') && (c <= 'z'))
	{
		return c + 'A' - 'a';
	}
	return c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int StrEmpty(const char* szString)
{
	if (szString == NULL)
	{
		return 1;
	}
	else if (*szString == '\0')
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int StrPrintable(const char* szString)
{
	int				i;
	unsigned char	c;
	int				iPrintable;

	if (StrEmpty(szString))
	{
		return 0;
	}

	iPrintable = 0;
	for (i = 0; ; i++)
	{
		c = szString[i];
		if ((c >= 32) && (c <= 126))
		{
			iPrintable++;
		}
		else if (c == '\0')
		{
			break;
		}
	}

	return iPrintable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int StrPrintable(const char* szString, int iLength)
{
	int				i;
	unsigned char	c;
	int				iPrintable;

	iPrintable = 0;
	for (i = 0; i < iLength; i++)
	{
		c = szString[i];
		if ((c >= 32) && (c <= 126))
		{
			iPrintable++;
		}
	}

	return iPrintable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t StrLen(const char* szString, const char* szLastCharInclusive)
{
	ptrdiff_t i;

	if (szLastCharInclusive)
	{
		i = szLastCharInclusive - szString + 1;
		if (i >= 0)
		{
			return i;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return strlen(szString);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* NullToEmpty(const char* szString)
{
	if (szString == NULL)
	{
		return gszEmptyString;
	}
	else
	{
		return (char*)szString;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* StrCpy(char* szDest, const char* szString, const char* szLastCharInclusive)
{
	size_t	uiLength;

	uiLength = StrLen(szString, szLastCharInclusive);
	memcpy(szDest, szString, uiLength);
	szDest[uiLength] = '\0';

	return szDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char GetHexChar(char c4Bit)
{
	if (c4Bit < 10)
	{
		return '0' + c4Bit;
	}
	else if (c4Bit < 16)
	{
		return 'a' + c4Bit - 10;
	}
	else
	{
		return '?';
	}
}


#define INCREMENT_INDEX_BREAK_ON_MAX(INDEX, MAX) INDEX++; if (INDEX == MAX) { break; }
#define INDEX_RETURN_ON_MAX(INDEX, MAX, DEST) if (INDEX >= MAX) { DEST[iDestLength - 1] = '\0';  return DEST; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FlagsToString(char* szDest, int iDestLength, char* pvMem, int iByteCount)
{
	short int		iIndex;
	char			iBit;
	int				i;
	char			iByte;
	char			iBitIndex;
	unsigned char	iBitMask;
	bool			bAppendedSpace;

	if ((iDestLength == 0) || (iByteCount > 16))
	{
		return szDest;
	}

	iIndex = 0;
	iByte = iByteCount-1;
	iBitIndex = 0;
	iBitMask = 0x80;
	for (i = 0; i < iDestLength; i++)
	{
		iBit = iBitMask & pvMem[iByte];
		if (iBit)
		{
			szDest[iIndex] = '1';
			INCREMENT_INDEX_BREAK_ON_MAX(iIndex, iDestLength);
		}
		else
		{
			szDest[iIndex] = '0';
			INCREMENT_INDEX_BREAK_ON_MAX(iIndex, iDestLength);
		}

		iBitIndex++;
		iBitMask >>= 1;
		bAppendedSpace = false;
		if (iBitIndex == 8)
		{
			iBitMask = 0x80;
			iBitIndex = 0;
			iByte--;
			szDest[iIndex] = ' ';
			bAppendedSpace = true;
			INCREMENT_INDEX_BREAK_ON_MAX(iIndex, iDestLength);

			if (iByte < 0)
			{
				break;
			}
		}
	}


	if ((iIndex == iDestLength) || (bAppendedSpace))
	{
		iIndex--;
	}
	szDest[iIndex] = '\0';
	
	return szDest;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FlagsToString(char* szDest, int iDestLength, int iFlags)
{
	return FlagsToString(szDest, iDestLength, (char*)&iFlags, 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FlagsToString(char* szDest, int iDestLength, short int iFlags)
{
	return FlagsToString(szDest, iDestLength, (char*)&iFlags, 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FlagsToString(char* szDest, int iDestLength, char iFlags)
{
	return FlagsToString(szDest, iDestLength, (char*)&iFlags, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(char* szDest, int iDestLength, char c)
{
	return CharToString(szDest, iDestLength, (unsigned char)c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(char* szDest, int iDestLength, unsigned char c)
{
	char		sz[10];
	short int	iIndex;
	int			iLength;

	if (iDestLength == 0)
	{
		return szDest;
	}

	iIndex = 0;
	szDest[iIndex] = '\'';
	iIndex++;
	INDEX_RETURN_ON_MAX(iIndex, iDestLength, szDest);

	GetEscapeString(c, sz);
	StrCpySafe(&szDest[iIndex], sz, iDestLength - iIndex, &iLength);

	iIndex += iLength;
	INDEX_RETURN_ON_MAX(iIndex, iDestLength, szDest);

	szDest[iIndex] = '\'';
	iIndex++;
	INDEX_RETURN_ON_MAX(iIndex, iDestLength, szDest);

	szDest[iIndex] = '\0';
	return szDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(char* szDest, int iDestLength, char16 c)
{
	char		sz[10];
	short int	iIndex;
	int			iLength;

	if (iDestLength == 0)
	{
		return szDest;
	}

	iIndex = 0;
	szDest[iIndex] = '\'';
	iIndex++;
	INDEX_RETURN_ON_MAX(iIndex, iDestLength, szDest);

	GetEscapeString(c, sz);
	StrCpySafe(&szDest[iIndex], sz, iDestLength - iIndex, &iLength);

	iIndex += iLength;
	INDEX_RETURN_ON_MAX(iIndex, iDestLength, szDest);

	szDest[iIndex] = '\'';
	iIndex++;
	INDEX_RETURN_ON_MAX(iIndex, iDestLength, szDest);

	szDest[iIndex] = '\0';
	return szDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* BoolToString(char* szDest, int iDestLength, bool b)
{
	if (b)
	{
		StrCpySafe(szDest, "true", iDestLength);
	}
	else
	{
		StrCpySafe(szDest, "false", iDestLength);
	}
	return szDest;
}

