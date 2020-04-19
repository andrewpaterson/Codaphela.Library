/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "StringHelper.h"


char	gszEmptyString[] = "";


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
int StrICmp(const char* ps1, const char* ps2)
{
    int             iS1Len;
    int             iS2Len;
    int             iLength;
    int             iResult;

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

    iS1Len = (int)strlen(ps1);
    iS2Len = (int)strlen(ps2);

    if ((iS1Len == 0) && (iS2Len == 0))
    {
        return 0;
    }
	else if (iS1Len == 0)
	{
		return -1;
	}
	else if (iS2Len == 0)
	{
		return 1;
	}

    if (iS1Len < iS2Len)
    {
        iLength = iS1Len;
    }
    else
    {
        iLength = iS2Len;
    }

    iResult = MemICmp(ps1, ps2, iLength);

    if (iResult != 0)
    {
        return iResult;
    }

    if (iS1Len < iS2Len)
    {
        return -1;
    }
    else if (iS1Len > iS2Len)
    {
        return 1;
    }
    return 0;
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
char* IToA(int iValue, char* szResult, int iBase)
{
	char	szDigits[] = "0123456789abcdef";
	int		iQuotient;
	int     iDigit;
	int     iPos;
	int		bNegative;

	if (iBase != 10)
	{
		return IToA((unsigned int)iValue, szResult, iBase);
	}

	bNegative = iValue < 0;
	if (bNegative)
	{
		if (iValue == MIN_INT)
		{
			strcpy(szResult, "-2147483648");
			return szResult;
		}
		iValue *= -1;
	}
	iQuotient = iValue;
	iPos = 0;

	do
	{
		iDigit = iQuotient % iBase;
		iQuotient /= iBase;

		szResult[iPos] = szDigits[iDigit];
		iPos++;

	} while (iQuotient != 0);


	// Only apply negative sign for iBase 10
	if (bNegative)
	{
		szResult[iPos] = '-';
		iPos++;
	}

	szResult[iPos] = '\0';

	//Reverse the string.
	StrRev(szResult, iPos);

	return szResult;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* IToA(unsigned int iValue, char* szResult, int iBase)
{
	char			szDigits[] = "0123456789abcdef";
	unsigned int	iQuotient;
	unsigned int    iDigit;
	unsigned int    iPos;

	if (iBase < 2 || iBase > 16)
	{
		szResult[0] = 0;
		return szResult;
	}

	iQuotient = iValue;
	iPos = 0;

	do
	{
		iDigit = iQuotient % iBase;
		iQuotient /= iBase;

		szResult[iPos] = szDigits[iDigit];
		iPos++;

	} while (iQuotient != 0);


	szResult[iPos] = '\0';

	//Reverse the string.
	StrRev(szResult, iPos);

	return szResult;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* IToA(unsigned long long int ulliValue, char* szResult, int iBase)
{
	char					szDigits[] = "0123456789abcdef";
	unsigned long long int	ulliQuotient;
	unsigned int    		iDigit;
	unsigned int    		iPos;

	if (iBase < 2 || iBase > 16)
	{
		szResult[0] = 0;
		return szResult;
	}

	ulliQuotient = ulliValue;
	iPos = 0;

	do
	{
		iDigit = (unsigned int)(ulliQuotient % iBase);
		ulliQuotient /= iBase;

		szResult[iPos] = szDigits[iDigit];
		iPos++;

	} while (ulliQuotient != 0);


	szResult[iPos] = '\0';

	//Reverse the string.
	StrRev(szResult, iPos);

	return szResult;
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
char* IToA(long long int lliValue, char* szResult, int iBase)
{
	char	szDigits[] = "0123456789abcdef";
	long long int		lliQuotient;
	int     iDigit;
	int     iPos;
	int		bNegative;

	if (iBase != 10)
	{
		return IToA((unsigned long long int)lliValue, szResult, iBase);
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

		szResult[iPos] = szDigits[iDigit];
		iPos++;

	} while (lliQuotient != 0);


	// Only apply negative sign for iBase 10
	if (bNegative && (iBase == 10))
	{
		szResult[iPos] = '-';
		iPos++;
	}

	szResult[iPos] = '\0';

	//Reverse the string.
	StrRev(szResult, iPos);

	return szResult;
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
const char* FindChar(const char* szString, char c, BOOL bReverse)
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
void StrCpySafe(char* szDest, const char* szSource, int iDestLength)
{
	int iLen;	

	iLen = (int)strlen(szSource);
	if (iLen >= iDestLength)
	{
		iLen = iDestLength-1;
	}
	memcpy(szDest, szSource, iLen);
	szDest[iLen] = 0;
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
	else if (szString[0] == '\0')
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

