#include <string.h>
#include <stdio.h>
#include "ArrayElementNotFound.h"
#include "StringHelper.h"
#include "FatFilenameHelper.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode GetFatLongNameForEntry(uint16* puiDest, char* szSource)
{
	size	i;
	size	uiLength;

	uiLength = strlen(szSource);
	for (i = 0; i < uiLength; i++)
	{
		puiDest[i] = (uint16)szSource[i];
	}
	puiDest[i] = 0x0;

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CompareFatShortName(char* szName1, char* szName2)
{
	return memcmp(szName1, szName2, 11) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareFatLongName(uint16* puiName1, uint16* puiName2)
{
	size	i;
	char	c1;
	char	c2;;

	for (i = 0; i <= FAT_MAX_FILENAME; i++)
	{
		c1 = toupper((char)puiName1[i]);
		c2 = toupper((char)puiName2[i]);
		if (c1 != c2)
		{
			return 0;
		}

		if (c1 == '\0')
		{
			return 1;
		}
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode MatchesFatFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16* puiTargetFileLongName, char* szCurrentLevelPath, uint16* auiLongFilename, uint8* auiShortName)
{
	bool bLongFilename;
	bool bMatch;

	bLongFilename = false;
	if (GetFatShortNameForEntry(szConstructedShortFileName, szCurrentLevelPath, 1) == FAT_INVALID_FILENAME)
	{
		if (GetFatLongNameForEntry(puiTargetFileLongName, szCurrentLevelPath) == FAT_INVALID_FILENAME)
		{
			return FAT_INVALID_FILENAME;
		}
		bLongFilename = true;
		bMatch = CompareFatLongName(puiTargetFileLongName, auiLongFilename) || CompareFatShortName(szConstructedShortFileName, (char*)auiShortName);
	}
	else
	{
		bMatch = CompareFatShortName(szConstructedShortFileName, (char*)auiShortName);
	}

	*pbUsingLFN = bLongFilename;
	*pbMatch = bMatch;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode GetFatShortNameForEntry(char* szDest, char* szSource, bool bLFNDisabled)
{
	char	szName[13];
	bool	bUppercase;
	size	uiDotIndex;
	size	uiLength;
	size	i;
	uint8	c;
	bool	bLFN;

	bUppercase = false;
	if (!bLFNDisabled)
	{
		bLFN = false;

		uiLength = strlen(szSource);
		uiDotIndex = FindCharIndex('.', szSource, 0);

		if (uiDotIndex == ARRAY_ELEMENT_NOT_FOUND && uiLength > 8)
		{
			bLFN = true;
		}

		if (uiDotIndex >= 0)
		{
			if (uiDotIndex > 7 || (uiLength - uiDotIndex) > 4)
			{
				bLFN = true;
			}
			if (uiDotIndex >= 0)
			{
				if (FindCharIndex('.', (char*)szSource, 1) >= 0)
				{
					bLFN = true;
				}
			}
		}
		else
		{
			if (uiLength > 8)
			{
				bLFN = true;
			}
		}

		if (!bLFN)
		{
			for (i = 0; i < uiLength; i++)
			{
				if (szSource[i] == ' ' || szSource[i] != toupper(szSource[i]))
				{
					bLFN = true;
					break;
				}
			}
		}
		if (bLFN)
		{
			uiDotIndex = uiLength;
			for (i = uiLength - 1; i; i--)
			{
				if (szSource[i] == '.')
				{
					uiDotIndex = i;
					break;
				}
			}

			c = 0;
			for (i = 0; i < 8; i++)
			{
				while (c < uiDotIndex)
				{
					if (szSource[c] == ' ' || szSource[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < uiDotIndex)
				{
					szName[i] = toupper(szSource[c++]);
				}
				else
				{
					szName[i] = ' ';
				}
			}

			c = (uint8)(uiDotIndex + 1);
			for (i = 8; i < 11; i++)
			{
				while (c < uiLength)
				{
					if (szSource[c] == ' ' || szSource[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < uiLength)
				{
					szName[i] = toupper(szSource[c++]);
				}
				else
				{
					szName[i] = ' ';
				}
			}

			for (i = 0; i < 11; i++)
			{
				*szDest++ = szName[i];
			}

			return FAT_LFN_GENERATED;
		}
	}

	StrCpySafeStripSurroundingSpaces(szName, (char*)szSource, 12);

	uiLength = strlen(szName);
	if (uiLength == 0 || uiLength > 12)
	{
		return FAT_INVALID_FILENAME;
	}

	uiDotIndex = FindCharIndex('.', szName, 0);

	if (uiDotIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		uiDotIndex = 0;
	}

	if ((uiLength > 9 &&
		(uiDotIndex == 0 || (uiDotIndex) > 9)) ||
		(uiDotIndex > 0 && (uiLength - uiDotIndex) > 5))
	{
		return FAT_INVALID_FILENAME;
	}

	for (i = 0; i < 8; i++)
	{
		if (uiDotIndex == 0)
		{
			if (i < uiLength)
			{
				if (bLFNDisabled && (szName[i] != toupper(szName[i])))
					bUppercase = true;

				*szDest++ = toupper(szName[i]);
			}
			else
			{
				*szDest++ = ' ';
			}
		}
		else
		{
			if (i < uiDotIndex - 1)
			{
				if (bLFNDisabled && (szName[i] != toupper(szName[i])))
				{
					bUppercase = true;
				}

				*szDest++ = toupper(szName[i]);
			}
			else
			{
				*szDest++ = ' ';
			}
		}
	}

	if (uiDotIndex == 0)
	{
		for (i = 0; i < 3; i++)
		{
			*szDest++ = ' ';
		}
	}
	else
	{
		for (i = uiDotIndex; i < uiDotIndex + 3; i++)
		{
			if (i < uiLength)
			{
				if (bLFNDisabled && (szName[i] != toupper(szName[i])))
					bUppercase = true;
				*szDest++ = toupper(szName[i]);
			}
			else
			{
				*szDest++ = ' ';
			}
		}
	}

	return bUppercase ? FAT_INVALID_FILENAME : FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ConvertFATShortInternalNameInto8Dot3Format(uint8* puiDest, const uint8* puiSource)
{
	if (*puiSource != FAT_KANJI_SKIP_ENTRY)
	{
		*puiDest++ = *puiSource++;
	}
	else
	{
		*puiDest++ = FAT_DELETED_ENTRY;
	}

	// if there's a second character...
	if (*puiSource != ' ')
	{
		*puiDest++ = *puiSource++;
		if (*puiSource != ' ')
		{
			*puiDest++ = *puiSource++;
			if (*puiSource != ' ')
			{
				*puiDest++ = *puiSource++;
				if (*puiSource != ' ')
				{
					*puiDest++ = *puiSource++;
					if (*puiSource != ' ')
					{
						*puiDest++ = *puiSource++;
						if (*puiSource != ' ')
						{
							*puiDest++ = *puiSource++;
							if (*puiSource != ' ')
							{
								*puiDest++ = *puiSource++;
							}
							else
							{
								puiSource++;
							}
						}
						else
						{
							puiSource += 0x2;
						}
					}
					else
					{
						puiSource += 0x3;
					}
				}
				else
				{
					puiSource += 0x4;
				}
			}
			else
			{
				puiSource += 0x5;
			}
		}
		else
		{
			puiSource += 0x6;
		}
	}
	else
	{
		puiSource += 0x7;
	}

	if (*puiSource != ' ')
	{
		*puiDest++ = '.';
		*puiDest++ = *puiSource++;
		if (*puiSource != ' ')
		{
			*puiDest++ = *puiSource++;
			if (*puiSource != ' ')
			{
				*puiDest++ = *puiSource;
			}
		}
	}
	*puiDest = '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ConstructFatLongFileNameFromShortName(uint16* auiLongFilename, char* szShortName, bool bLowercaseBase, bool bLowercaseExtension)
{
	size	i;
	size	uiResult;

	i = 0;
	for (uiResult = 0; uiResult < 8; uiResult++)
	{
		if (szShortName[uiResult] != ' ')
		{
			if (bLowercaseBase)
			{
				auiLongFilename[i] = tolower(szShortName[uiResult]);
			}
			else
			{
				auiLongFilename[i] = szShortName[uiResult];
			}
			i++;
		}
	}

	if (szShortName[8] != ' ')
	{
		auiLongFilename[i++] = '.';

		for (uiResult = 8; uiResult < 11; uiResult++)
		{
			if (szShortName[uiResult] != 0x20)
			{
				if (bLowercaseExtension)
				{
					auiLongFilename[i] = tolower(szShortName[uiResult]);
				}
				else
				{
					auiLongFilename[i] = szShortName[uiResult];
				}
				i++;
			}
		}
	}

	auiLongFilename[i] = 0x0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* StepPathOverSlash(char* szPathName)
{
	if (szPathName)
	{
		if (szPathName[0] == '\\')
		{
			szPathName++;
		}
	}
	return szPathName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* StepPathToSlash(char* szPathName)
{
	while (*(--szPathName) != '\\');
	return szPathName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyLongFilenameIntoString(char* szDestination, uint16* auiSource)
{
	size	uiIndex;

	if (szDestination[0] != '\0')
	{
		if (auiSource[0] != 0)
		{
			for (uiIndex = 0; uiIndex < 256; uiIndex++)
			{
				szDestination[uiIndex] = (char)auiSource[uiIndex];
				if (auiSource[uiIndex] == 0)
				{
					break;
				}
			}
		}
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsIllegalFilenameCharacter(char c)
{
	size	uiIndex;

	if (c <= 0x1F)
	{
		return true;
	}

	for (uiIndex = 0; uiIndex < ILLEGAL_CHARS_COUNT; uiIndex++)
	{
		if (c == ILLEGAL_CHARS[uiIndex] && c != '.')
		{
			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsIllegalFilename(char* szName, uint16 uiLength)
{
	size		uiCharIndex;

	uiCharIndex = FindCharIndex('.', szName, 0);

	if (uiCharIndex == 0 || uiCharIndex == (uiLength - 1))
	{
		return true;
	}

	for (uiCharIndex = 0x0; uiCharIndex < uiLength; uiCharIndex++)
	{
		if (IsIllegalFilenameCharacter(szName[uiCharIndex]))
		{
			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void GenerateShortNameWithSuffix(uint16 uiNameSuffix, uint8* szShortName)
{
	char	szNameSuffix[6];
	size	i;
	size	c;
	size	uiSuffixLength;

	sprintf(szNameSuffix, "~%i", uiNameSuffix);
	uiSuffixLength = strlen(szNameSuffix);

	for (i = 0; i < 8 - uiSuffixLength; i++)
	{
		if (szShortName[i] == ' ')
		{
			break;
		}
	}

	for (c = 0; c < uiSuffixLength; c++)
	{
		szShortName[i] = szNameSuffix[c];
		i++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ParsePathAndFilename(char* szPath, char* szPathPart, char** pszFilenamePart)
{
	*pszFilenamePart = szPath + strlen(szPath);

	while (*--(*pszFilenamePart) != '\\' && (*pszFilenamePart) != szPath);

	while (szPath != *pszFilenamePart)
	{
		*szPathPart++ = *szPath++;
	}
	*szPathPart = 0;
	(*pszFilenamePart)++;
}

