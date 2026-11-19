#ifndef __FAT_FILENAME_HELPER_H__
#define __FAT_FILENAME_HELPER_H__
#include <ctype.h>
#include "FatCommon.h"


// performs an ASCII stuff on UTF16 strings
EFatCode GetFatLongNameForEntry(uint16* puiDest, char* szSource) noexcept;
bool CompareFatShortName(char* szName1, char* szName2) noexcept;
int CompareFatLongName(uint16* puiName1, uint16* puiName2) noexcept;
EFatCode GetFatShortNameForEntry(char* szDest, const char* szSource, bool bLFNDisabled) noexcept;
void ConvertFATShortInternalNameInto8Dot3Format(uint8* puiDest, const uint8* puiSource) noexcept;
EFatCode MatchesFatFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16* puiTargetFileLongName, char* szCurrentLevelPath, uint16* auiLongFilename, uint8* auiShortName) noexcept;
void ConstructFatLongFileNameFromShortName(uint16* auiLongFilename, char* szShortName, bool bLowercaseBase, bool bLowercaseExtension) noexcept;
const char* StepPathOverSlash(const char* szPathName) noexcept;
const char* StepPathToSlash(const char* szPathName) noexcept;
void CopyLongFilenameIntoString(char* szDestination, uint16* auiSource) noexcept;
bool IsIllegalFilenameCharacter(char c) noexcept;
bool IsIllegalFilename(const char* szName, uint16 uiLength) noexcept;
void GenerateShortNameWithSuffix(uint16 uiNameSuffix, uint8* szShortName) noexcept;
void ParsePathAndFilename(const char* szPath, char* szPathPart, const char** pszFilenamePart) noexcept;


#endif // __FAT_FILENAME_HELPER_H__

