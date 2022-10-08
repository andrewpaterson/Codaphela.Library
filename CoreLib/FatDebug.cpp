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
	uint32		uiEOC;
	
	uiPreviousSector = 0;
	bAllEntriesSame = true;
	
	uiEOC = pcVolume->GetEndOfClusterMarker();
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
			szSector.Append("-------------------------------------------------------------------- Sector (");
			szSector.Append(uiSector);
			szSector.Append(") --------------------------------------------------------------------");
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

		szNumber.Append((int)uiClusterIndex, 16);
		szSector.RightAlign(szNumber.Text(), ' ', 5);
		szNumber.Clear();
		szSector.Append(" -> ");

		if (uiEOC != uiEntry)
		{
			szNumber.Append((int)uiEntry, 16);
			szSector.RightAlign(szNumber.Text(), ' ', 4);
			szNumber.Clear();
		}
		else
		{
			szSector.Append(" EOC");
		}
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

	szSector.Kill();
	szNumber.Kill();
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
	SFatCache				sCache;
	
	uiCluster = pcVolume->GetRootCluster();
	uiSector = pcVolume->GetRootSector();

	sCache = pcVolume->ReadSector(uiSector);
	if (!sCache.IsValid())
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	uiSectorCount = 0;
	memset(szShortName, '\0', 13);
	psEntry = (SFatRawDirectoryEntry*)sCache.Get();
	psFirstEntry = (SFatRawDirectoryEntry*)sCache.Get();

	for (;;)
	{
		eResult = pcVolume->ValidateFatCache(sCache);
		RETURN_ON_FAT_FAILURE(eResult)

		if (((uintptr_t)psEntry - (uintptr_t)psFirstEntry) == pcVolume->GetSectorSize())
		{
			if (uiSectorCount == pcVolume->NumSectorsPerCluster() - 1)
			{
				eResult = pcVolume->GetNextClusterEntry(uiCluster, &uiCluster);
				RETURN_ON_FAT_FAILURE(eResult)

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
			sCache = pcVolume->ReadSector(uiSector);
			if (!sCache.IsValid())
			{
				return FAT_CANNOT_READ_MEDIA;
			}
			psEntry = (SFatRawDirectoryEntry*)sCache.Get();
			psFirstEntry = (SFatRawDirectoryEntry*)sCache.Get();
		}

		eResult = pcVolume->ValidateFatCache(sCache);
		RETURN_ON_FAT_FAILURE(eResult)

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
	SFatDirectoryEntry*		psFatDirectoryEntry;
	EFatCode				eResult;
	char					szNewPath[FAT_MAX_PATH];

	sQuery.Init();
	eResult = pcVolume->FindFirstFATEntry(szPath, 0, &psFatDirectoryEntry, &sQuery);
	for (;;)
	{
		if (eResult != FAT_SUCCESS)
		{
			break;
		}

		if (StrEmpty((char*)psFatDirectoryEntry->name))
		{
			eResult = FAT_SUCCESS;
			break;
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
					break;
				}
				paszDirectories->Add(szNewPath);
			}
		}

		eResult = pcVolume->FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
		if (eResult != FAT_SUCCESS)
		{
			break;
		}
	}

	sQuery.Kill(pcVolume->GetSectorCache());
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode RecurseFindFatFilenames(CFatVolume* pcVolume, char* szPath, CArrayChars* paszFiles, int iDepth, int iMaxDepth)
{
	SFatFileSystemQuery		sQuery;
	SFatDirectoryEntry*		psFatDirectoryEntry;
	EFatCode				eResult;
	char					szNewPath[FAT_MAX_PATH];

	sQuery.Init();
	eResult = pcVolume->FindFirstFATEntry(szPath, 0, &psFatDirectoryEntry, &sQuery);
	for (;;)
	{
		if (eResult != FAT_SUCCESS)
		{
			break;
		}

		if (StrEmpty((char*)psFatDirectoryEntry->name))
		{
			eResult = FAT_SUCCESS;
			break;
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
						break;
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

	sQuery.Kill(pcVolume->GetSectorCache());
	return eResult;
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

