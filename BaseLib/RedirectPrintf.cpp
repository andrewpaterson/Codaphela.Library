#include <stdio.h>
#include "Chars.h"
#include "StackMemory.h"
#include "DebugOutput.h"
#include "RedirectPrintf.h"


PrintFunction	gPrintFunction = NULL;


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool IsPrintFSpecifier(const char c)
{
	if (c == 'd' ||
		c == 'i' ||
		c == 'u' ||
		c == 'o' ||
		c == 'x' ||
		c == 'X' ||
		c == 'f' ||
		c == 'F' ||
		c == 'e' ||
		c == 'E' ||
		c == 'g' ||
		c == 'G' ||
		c == 'a' ||
		c == 'A' ||
		c == 'c' ||
		c == 's' ||
		c == 'p' ||
		c == 'n' ||
		c == '%')
	{
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int FindPrintFParameters(const char* szUnformatted, char** paszParameters)
{
	int		i;
	char	c;
	int		iFormattedCount;
	int		iUnformattedCount;
	bool	bInSpecifier;
	int		iParameterLength;
	char*	aszParameters;
	bool	bInUnformatted;
	int		iLength;
	char*	szCurrent;
	bool	bAppendTerminator;

	bInUnformatted = false;
	bInSpecifier = false;
	iParameterLength = 0;
	iFormattedCount = 0;
	iUnformattedCount = 0;
	iLength = 0;

	for (i = 0;; i++)
	{
		c = szUnformatted[i];
		if (c == '\0')
		{
			if (bInSpecifier || bInSpecifier)
			{
				iParameterLength++;
			}
			break;
		}
		else if (c == '%')
		{
			iFormattedCount++;
			iParameterLength++;  //Leading 0x7C indicating a formatted parameter.
			bInSpecifier = true;

			if (bInUnformatted)
			{
				bInUnformatted = false;
				iParameterLength++;  //Trailing '\0'.
			}
		}
		else if (bInSpecifier)
		{
			if (IsPrintFSpecifier(c))
			{
				iParameterLength++;  //Trailing '\0'.
				bInSpecifier = false;
			}
		}
		else if (!bInUnformatted && !bInSpecifier)
		{
			iUnformattedCount++;
			iParameterLength++;  //Leading 0x7D indicating an unformatted parameter.
			bInUnformatted = true;
		}
		iParameterLength++;
		iLength++;
	}

	aszParameters = (char*)malloc(iParameterLength + 1);
	iParameterLength = 0;
	bInUnformatted = false;
	bInSpecifier = false;
	bAppendTerminator = false;

	for (i = 0; i < iLength; i++)
	{
		c = szUnformatted[i];
		if (c == '%')
		{
			if (bInUnformatted)
			{
				bInUnformatted = false;
				aszParameters[iParameterLength] = '\0';
				iParameterLength++;
			}

			aszParameters[iParameterLength] = 0x7c;
			iParameterLength++;
			szCurrent = &aszParameters[iParameterLength];
			bInSpecifier = true;

		}
		else if (bInSpecifier)
		{
			if (IsPrintFSpecifier(c))
			{
				bAppendTerminator = true;
				bInSpecifier = false;
			}
		}
		else if (!bInUnformatted && !bInSpecifier)
		{
			aszParameters[iParameterLength] = 0x7d;
			iParameterLength++;
			szCurrent = &aszParameters[iParameterLength];
			bInUnformatted = true;
		}
		aszParameters[iParameterLength] = c;
		iParameterLength++;

		if (bAppendTerminator)
		{
			aszParameters[iParameterLength] = '\0';
			iParameterLength++;
			bAppendTerminator = false;
		}
	}

	if (bInSpecifier || bInUnformatted)
	{
		aszParameters[iParameterLength] = '\0';
		iParameterLength++;
	}

	*paszParameters = aszParameters;
	return iFormattedCount + iUnformattedCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int eprintf(const char* szUnformatted, ...)
{
	va_list				vaMarker;
	const char*			szValue;
	CChars				sz;
	int					iParameterCount;
	int					i;
	int					iLength;
	char*				aszParameters;
	char*				szParameter;
	size				iFormattedeLength;
	CStackMemory<1 KB>	cStack;
	char*				szFormattedParameter;

	aszParameters = NULL;
	sz.Init();
	iParameterCount = FindPrintFParameters(szUnformatted, &aszParameters);

	if (iParameterCount == 0)
	{
		SafeFree(aszParameters);
		return 0;
	}

	szParameter = aszParameters;
	va_start(vaMarker, szUnformatted);
	for (i = 0; i < iParameterCount; i++)
	{
		if (*szParameter == 0x7d)
		{
			szParameter++;
			sz.Append(szParameter);
		}
		else if (*szParameter == 0x7c)
		{
			szParameter++;
			szValue = (const char*)va_arg(vaMarker, const char*);
			iFormattedeLength = snprintf(NULL, 0, szParameter, szValue);
			szFormattedParameter = (char*)cStack.Init(iFormattedeLength);
			sprintf(szFormattedParameter, szParameter, szValue);
			sz.Append(szFormattedParameter);
			cStack.Kill();
		}


		iLength = strlen(szParameter);
		szParameter = szParameter + iLength + 1;
	}
	va_end(vaMarker);

	if (gPrintFunction == NULL)
	{
		EngineOutput(sz.Text());
	}
	else
	{
		gPrintFunction(sz.Text());
	}

	iLength = sz.Length();
	sz.Kill();

	SafeFree(aszParameters);

	return iLength;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SetPrintFunction(PrintFunction pPrintFunction)
{
	gPrintFunction = pPrintFunction;
}