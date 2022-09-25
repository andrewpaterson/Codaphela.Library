#include <string.h>
#include "BaseLib/LogString.h"
#include "BaseLib/StringHelper.h"
#include "FatCommon.h"


char gszFatSuccess[] = "SUCCESS";

char gszFatBad[] = "BAD_FAT_CODE";

char gaszFatErrors[][34] = {"UNKNOWN_ERROR" ,
							"CANNOT_READ_MEDIA",
							"CANNOT_WRITE_MEDIA",
							"NOT_A_DIRECTORY",
							"INVALID_FILENAME",
							"FILENAME_ALREADY_EXISTS",
							"INVALID_PATH",
							"CORRUPTED_FILE",
							"ILLEGAL_FILENAME",
							"FILENAME_TOO_LONG",
							"NOT_A_FILE",
							"FILE_NOT_FOUND",
							"DIRECTORY_DOES_NOT_EXIST",
							"INSUFFICIENT_DISK_SPACE",
							"FEATURE_NOT_SUPPORTED",
							"OPERATION_IN_PROGRESS",
							"SECTOR_SIZE_NOT_SUPPORTED",
							"LFN_GENERATED",
							"SHORT_LFN_GENERATED",
							"SEEK_FAILED",
							"FILE_NOT_OPENED_FOR_WRITE_ACCESS",
							"INVALID_HANDLE",
							"INVALID_CLUSTER",
							"INVALID_FAT_VOLUME",
							"INVALID_VOLUME_LABEL",
							"INVALID_FORMAT_PARAMETERS",
							"ROOT_DIRECTORY_LIMIT_EXCEEDED",
							"DIRECTORY_LIMIT_EXCEEDED",
							"INVALID_PARAMETERS",
							"FILE_HANDLE_IN_USE",
							"FILE_BUFFER_NOT_SET",
							"MISALIGNED_IO",
							"AWAITING_DATA",
							"BUFFER_TOO_BIG"};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 GetFatClusterFromFatEntry(SFatRawDirectoryEntry* psRawEntry, bool bFat32Volume)
{
	uint32 uiCursorClusterInVolume;

	if (psRawEntry != NULL)
	{
		((uint16*)&uiCursorClusterInVolume)[INT32_WORD0] = psRawEntry->uEntry.sFatRawCommon.uiFirstClusterLowWord;
		if (bFat32Volume)
		{
			((uint16*)&uiCursorClusterInVolume)[INT32_WORD1] = psRawEntry->uEntry.sFatRawCommon.uiFirstClusterHighWord;
		}
		else
		{
			((uint16*)&uiCursorClusterInVolume)[INT32_WORD1] = 0;
		}

		return uiCursorClusterInVolume;
	}
	else
	{
		return 0xFFFFFFFF;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FatCodeToString(EFatCode eCode)
{
	if (eCode == FAT_SUCCESS)
	{
		return gszFatSuccess;
	}

	if (eCode >= FAT_UNKNOWN_ERROR && eCode <= FAT_BUFFER_TOO_BIG)
	{
		return gaszFatErrors[eCode - 0x20];
	}

	return gszFatBad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FatEntryToString(SFatDirectoryEntry* psFatEntry, bool bFat32Volume)
{
	uint32	uiCluster;
	int		iCount;
	char	szTemp[LOG_TO_STRING_MAX_LENGTH];

	if (psFatEntry == NULL)
	{
		return "NULL";
	}
		
	uiCluster = GetFatClusterFromFatEntry(&psFatEntry->raw, bFat32Volume);

	iCount = IncrementLogToStringCount();

	
	memset(gaszLogToStringScratchPad[iCount], 0, LOG_TO_STRING_MAX_LENGTH);
	strcpy(gaszLogToStringScratchPad[iCount], "cluster: ");
	IntToString(szTemp, LOG_TO_STRING_MAX_LENGTH, uiCluster);
	strcat(gaszLogToStringScratchPad[iCount], szTemp);
	strcat(gaszLogToStringScratchPad[iCount], ", sector: ");
	IntToString(szTemp, LOG_TO_STRING_MAX_LENGTH, psFatEntry->uiSectorAddress);
	strcat(gaszLogToStringScratchPad[iCount], szTemp);
	strcat(gaszLogToStringScratchPad[iCount], ":");
	IntToString(szTemp, LOG_TO_STRING_MAX_LENGTH, psFatEntry->uiSectorOffset);
	strcat(gaszLogToStringScratchPad[iCount], szTemp);
	return gaszLogToStringScratchPad[iCount];
}

