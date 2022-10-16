#include "BaseLib/StringHelper.h"
#include "FatFilenameHelper.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char GetFatLongNameForEntry(uint16* puiDest, char* szSource)
{
	int i;

	for (i = 0; i < (int)strlen((char*)szSource); i++)
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
char CompareFatLongName(uint16* puiName1, uint16* puiName2)
{
	int		i;
	char	c1, c2;

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
EFatCode MatchesFatFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16* puiTargetFileLongName, char* szCurrentLevelPath, SFatQueryState* psQuery)
{
	bool bLongFilename = false;
	bool bMatch;

	if (GetFatShortNameForEntry(szConstructedShortFileName, szCurrentLevelPath, 1) == FAT_INVALID_FILENAME)
	{
		if (GetFatLongNameForEntry(puiTargetFileLongName, szCurrentLevelPath) == FAT_INVALID_FILENAME)
		{
			return FAT_INVALID_FILENAME;
		}
		bLongFilename = true;
		bMatch = CompareFatLongName(puiTargetFileLongName, psQuery->auiLongFilename) || CompareFatShortName(szConstructedShortFileName, (char*)psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
	}
	else
	{
		bMatch = CompareFatShortName(szConstructedShortFileName, (char*)psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
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
	char		szName[13];
	bool		bUppercase = false;
	uint16		uiDotIndex;
	uint16		uiLength;
	uint16		i;

	if (!bLFNDisabled)
	{
		uint8	c;
		bool	bLFN = false;

		uiLength = (uint16)strlen(szSource);
		uiDotIndex = FindCharIndex('.', szSource, 0);

		if (uiDotIndex < 0 && uiLength > 8)
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

			c = uiDotIndex + 1;
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

	if (*szName == 0 || strlen(szName) > 12)
	{
		return FAT_INVALID_FILENAME;
	}

	uiDotIndex = (uintptr_t)strchr(szName, (int)'.');

	if (uiDotIndex)
	{
		uiDotIndex -= (uintptr_t)szName - 1;
	}

	uiLength = (uint16)strlen(szName);

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

