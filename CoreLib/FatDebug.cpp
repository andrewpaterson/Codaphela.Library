#include "BaseLib/IntegerHelper.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/StringHelper.h"
#include "FatVolume.h"
#include "FatTime.h"
#include "FatDebug.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpInterestingFATClusters(CFatVolume* pcVolume)
{
	EFatCode	eResult;
	uint32		uiOffsetInSector;
	fatEntry	uiEntry;
	CChars		sz;
	uint16		uiRow;
	CChars		szNumber;
	fatEntry	uiClusterIndex;
	uint32		uiSector;
	uint32		uiPreviousSector;
	CChars		szSector;
	fatEntry	uiPreviousEntry;
	bool		bAllEntriesSame;
	
	uiPreviousSector = 0;
	bAllEntriesSame = true;
	
	sz.Init();
	szNumber.Init();
	szSector.Init();
	uiRow = 0;
	for (uiClusterIndex = 0; uiClusterIndex < pcVolume->GetNumClusters(); uiClusterIndex++)
	{
		pcVolume->CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiSector);

		eResult = pcVolume->ReadFatEntry(uiOffsetInSector, uiClusterIndex, uiSector, &uiEntry);
		if (eResult != FAT_SUCCESS)
		{
			break;
		}

		if (uiPreviousSector != uiSector)
		{
			if (szSector.EndsWith(", "))
			{
				szSector.RemoveFromEnd(2);
			}
			if (!szSector.EndsWith('\n'))
			{
				szSector.AppendNewLine();
			}

			if (!bAllEntriesSame)
			{
				sz.Append(szSector);
			}

			szSector.Clear();
			szSector.Append("-------------------------------------------------------------------- ");
			szSector.Append(uiSector);
			szSector.Append(" --------------------------------------------------------------------");
			szSector.AppendNewLine();
			uiPreviousSector = uiSector;
			uiRow = 0;

			uiPreviousEntry = uiEntry;
			bAllEntriesSame = true;
		}
		else
		{
			if (uiEntry != uiPreviousEntry)
			{
				bAllEntriesSame = false;
			}
		}

		szNumber.Append((int)uiEntry, 16);
		szSector.RightAlign(szNumber.Text(), ' ', 10);
		szNumber.Clear();
		uiRow++;
		if (uiRow == 16)
		{
			szSector.AppendNewLine();
			uiRow = 0;
		}
		else
		{
			szSector.Append(", ");
		}
	}

	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintAttributes(CChars* psz, uint8 uiAttributes)
{
	int		i;
	char* szAttribute;

	if (uiAttributes == FAT_ATTR_LONG_NAME)
	{
		psz->Append(FatAttributeToString(uiAttributes));
	}
	else
	{
		for (i = FAT_ATTR_READ_ONLY; i <= FAT_ATTR_ARCHIVE; i <<= 1)
		{
			szAttribute = FatAttributeToString(uiAttributes & i);
			if (!StrEmpty(szAttribute))
			{
				psz->Append(szAttribute);
				psz->Append(", ");
			}
		}

		if (psz->EndsWith(", "))
		{
			psz->RemoveFromEnd(2);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintLongNamePart(CChars* psz, uint8* puiChars, uint8 uiLength)
{
	int		i;
	char	c;

	for (i = 0; i < uiLength; i += 2)
	{
		c = puiChars[i];
		if (c != 0)
		{
			psz->Append(c);
		}
		else
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode PrintRootDirectoryEntries(CChars* psz, CFatVolume* pcVolume, bool bPrintTimes)
{
	EFatCode				eResult;
	uint32					uiCluster;
	uint32					uiSector;
	SFatRawDirectoryEntry*	psEntry;
	SFatRawDirectoryEntry*	psFirstEntry;
	uint32					uiSectorCount;
	char					szShortName[13];
	uint8*					pvSector;
	
	uiCluster = pcVolume->GetRootCluster();
	uiSector = pcVolume->GetRootSector();

	pvSector = pcVolume->ReadInfoSector(uiSector);
	if (pvSector == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}


	uiSectorCount = 0;
	memset(szShortName, '\0', 13);
	psEntry = (SFatRawDirectoryEntry*)pvSector;
	psFirstEntry = (SFatRawDirectoryEntry*)pvSector;

	for (;;)
	{
		if (((uintptr_t)psEntry - (uintptr_t)psFirstEntry) == pcVolume->GetSectorSize())
		{
			if (uiSectorCount == pcVolume->NumSectorsPerCluster() - 1)
			{
				eResult = pcVolume->GetNextClusterEntry(uiCluster, &uiCluster);
				RETURN_ON_FAT_FAILURE(eResult);

				if (pcVolume->FatIsEOFEntry(uiCluster))
				{
					return FAT_SUCCESS;
				}
				uiSector = pcVolume->CalculateFirstSectorOfCluster(uiCluster); // +psQuery->uiCurrentSector;
				uiSectorCount = 0;
			}
			else
			{
				uiSectorCount++;
				uiSector++;
			}
			pvSector = pcVolume->ReadInfoSector(uiSector);
			if (pvSector == NULL)
			{
				return FAT_CANNOT_READ_MEDIA;
			}
			psEntry = (SFatRawDirectoryEntry*)pvSector;
			psFirstEntry = (SFatRawDirectoryEntry*)pvSector;
		}

		if (!(psEntry->uEntry.sFatRawCommon.szShortName[0] == FAT_DELETED_ENTRY))
		{
			if (!((psEntry->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_LONG_NAME) == FAT_ATTR_LONG_NAME))
			{
				if (psEntry->uEntry.sFatRawCommon.szShortName[0] != '\0')
				{
					ConvertFATShortInternalNameInto8Dot3Format((uint8*)szShortName, psEntry->uEntry.sFatRawCommon.szShortName);

					psz->Append("---------------------------------------");
					psz->AppendNewLine();
					psz->Append("Short name: ");
					psz->Append(szShortName);
					psz->AppendNewLine();

					psz->Append("Cluster:    ");
					psz->Append(GetFirstClusterFromFatEntry(psEntry, pcVolume->GetFileSystemType() == FAT_FS_TYPE_FAT32));
					psz->AppendNewLine();

					psz->Append("Size:       ");
					psz->Append(psEntry->uEntry.sFatRawCommon.uiSize);
					psz->AppendNewLine();

					psz->Append("Attributes: ");
					PrintAttributes(psz, psEntry->uEntry.sFatRawCommon.uiAttributes);
					psz->AppendNewLine();

					if (bPrintTimes)
					{
						psz->Append("Created:    ");
						psz->Append(FatDecodeDateTime(psEntry->uEntry.sFatRawCommon.uiCreateDate, psEntry->uEntry.sFatRawCommon.uiCreateTime));
						psz->AppendNewLine();

						psz->Append("Accessed:   ");
						psz->Append(FatDecodeDateTime(psEntry->uEntry.sFatRawCommon.uiAccessDate, 0));
						psz->AppendNewLine();

						psz->Append("Modified:   ");
						psz->Append(FatDecodeDateTime(psEntry->uEntry.sFatRawCommon.uiModifyDate, psEntry->uEntry.sFatRawCommon.uiModifyTime));
						psz->AppendNewLine();
					}
				}
			}
			else
			{
				psz->Append("---------------------------------------");
				psz->AppendNewLine();

				psz->Append("Sequence:     ");
				psz->Append(psEntry->uEntry.sFatRawLongFileName.uiSequence);
				psz->AppendNewLine();

				psz->Append("Long name[1]: ");
				PrintLongNamePart(psz, psEntry->uEntry.sFatRawLongFileName.auiChars1, 10);
				psz->AppendNewLine();

				psz->Append("Long name[2]: ");
				PrintLongNamePart(psz, psEntry->uEntry.sFatRawLongFileName.auiChars2, 12);
				psz->AppendNewLine();

				psz->Append("Long name[3]: ");
				PrintLongNamePart(psz, psEntry->uEntry.sFatRawLongFileName.auiChars3, 4);
				psz->AppendNewLine();

				psz->Append("Cluster:      ");
				psz->Append(psEntry->uEntry.sFatRawLongFileName.uiFirstCluster);
				psz->AppendNewLine();

				psz->Append("Attributes:   ");
				PrintAttributes(psz, psEntry->uEntry.sFatRawLongFileName.uiAttributes);
				psz->AppendNewLine();

				psz->Append("Checksum:     ");
				psz->Append(psEntry->uEntry.sFatRawLongFileName.uiChecksum);
				psz->AppendNewLine();
			}
		}

		psEntry++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintBiosParameterBlock(CChars* psz, CFatVolume* pcVolume)
{
	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpRootDirectoryEntries(CFatVolume* pcVolume, bool bPrintTime)
{
	CChars		sz;
	EFatCode	eResult;

	sz.Init();
	eResult = PrintRootDirectoryEntries(&sz, pcVolume, bPrintTime);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode RecurseFindFatDirectories(CFatVolume* pcVolume, char* szPath, CArrayChars* paszDirectories)
{
	SFatFileSystemQuery		sQuery;
	SFatDirectoryEntry* psFatDirectoryEntry;
	EFatCode				eResult;
	char					szNewPath[FAT_MAX_PATH];

	memset(&sQuery, 0, sizeof(SFatFileSystemQuery));
	eResult = pcVolume->FindFirstFATEntry(szPath, 0, &psFatDirectoryEntry, &sQuery);
	for (;;)
	{
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		if (StrEmpty((char*)psFatDirectoryEntry->name))
		{
			return FAT_SUCCESS;
		}

		memset(szNewPath, 0, FAT_MAX_PATH);
		strcpy(szNewPath, szPath);
		strcat(szNewPath, "\\");
		strcat(szNewPath, (char*)psFatDirectoryEntry->name);

		if (FixBool(psFatDirectoryEntry->attributes & FAT_ATTR_DIRECTORY))
		{
			if (!((strcmp((char*)psFatDirectoryEntry->name, ".") == 0) || (strcmp((char*)psFatDirectoryEntry->name, "..") == 0)))
			{
				eResult = RecurseFindFatDirectories(pcVolume, szNewPath, paszDirectories);
				if (eResult != FAT_SUCCESS)
				{
					return eResult;
				}
				paszDirectories->Add(szNewPath);
			}
		}

		eResult = pcVolume->FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode RecurseFindFatFilenames(CFatVolume* pcVolume, char* szPath, CArrayChars* paszFiles, int iDepth, int iMaxDepth)
{
	SFatFileSystemQuery		sQuery;
	SFatDirectoryEntry* psFatDirectoryEntry;
	EFatCode				eResult;
	char					szNewPath[FAT_MAX_PATH];

	memset(&sQuery, 0, sizeof(SFatFileSystemQuery));
	eResult = pcVolume->FindFirstFATEntry(szPath, 0, &psFatDirectoryEntry, &sQuery);
	for (;;)
	{
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		if (StrEmpty((char*)psFatDirectoryEntry->name))
		{
			return FAT_SUCCESS;
		}

		memset(szNewPath, 0, FAT_MAX_PATH);
		strcpy(szNewPath, szPath);
		strcat(szNewPath, "\\");
		strcat(szNewPath, (char*)psFatDirectoryEntry->name);

		if (FixBool(psFatDirectoryEntry->attributes & FAT_ATTR_DIRECTORY))
		{
			if ((iDepth < iMaxDepth) || (iMaxDepth == -1))
			{
				if (!((strcmp((char*)psFatDirectoryEntry->name, ".") == 0) || (strcmp((char*)psFatDirectoryEntry->name, "..") == 0)))
				{
					eResult = RecurseFindFatFilenames(pcVolume, szNewPath, paszFiles, iDepth + 1, iMaxDepth);
					if (eResult != FAT_SUCCESS)
					{
						return eResult;
					}
				}
			}
		}

		if (FixBool(psFatDirectoryEntry->attributes & FAT_ATTR_ARCHIVE))
		{
			paszFiles->Add(szNewPath);
		}

		eResult = pcVolume->FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode RecurseFindFatFilenames(CFatVolume* pcVolume, char* szPath, CArrayChars* paszFiles)
{
	return RecurseFindFatFilenames(pcVolume, szPath, paszFiles, 0, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpAllFatFilenames(CFatVolume* pcVolume)
{
	CChars	sz;

	sz.Init();
	PrintAllFatFilenames(&sz, pcVolume);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintAllFatFilenames(CChars* psz, CFatVolume* pcVolume)
{
	CArrayChars				aszFileNames;

	aszFileNames.Init();
	RecurseFindFatFilenames(pcVolume, "", &aszFileNames);

	aszFileNames.Print(psz);
	aszFileNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpRootFatFilenames(CFatVolume* pcVolume)
{
	CChars	sz;

	sz.Init();
	PrintRootFatFilenames(&sz, pcVolume);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintRootFatFilenames(CChars* psz, CFatVolume* pcVolume)
{
	CArrayChars				aszFileNames;

	aszFileNames.Init();
	RecurseFindFatFilenames(pcVolume, "", &aszFileNames, 0, 0);

	aszFileNames.Print(psz);
	aszFileNames.Kill();
}

