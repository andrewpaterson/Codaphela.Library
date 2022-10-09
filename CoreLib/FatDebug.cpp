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
fatEntry GetDunnoClusterMarker(CFatVolume* pcVolume)
{
	switch (pcVolume->GetFileSystemType())
	{
	case FAT_FS_TYPE_FAT12:
		return 0x0FF8;

	case FAT_FS_TYPE_FAT16:
		return 0xFFF8;

	case FAT_FS_TYPE_FAT32:
		return 0x0FFFFFF8;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode AllFATClustersEntriesSame(bool* pbAllEntriesSame, CFatVolume* pcVolume, uint32 uiStartClusterIndex, uint32 uiEndClusterIndex)
{
	EFatCode	eResult;
	uint32		uiOffsetInSector;
	fatEntry	uiEntry;
	uint16		uiRow;
	fatEntry	uiClusterIndex;
	uint32		uiSector;
	fatEntry	uiPreviousEntry;
	bool		bFirst;

	*pbAllEntriesSame = true;
	bFirst = true;

	uiRow = 0;
	for (uiClusterIndex = uiStartClusterIndex; uiClusterIndex < uiEndClusterIndex; uiClusterIndex++)
	{
		pcVolume->CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiSector);

		eResult = pcVolume->ReadFatEntry(uiOffsetInSector, uiClusterIndex, uiSector, &uiEntry);
		RETURN_ON_FAT_FAILURE(eResult)

		if (bFirst)
		{
			bFirst = false;
			uiPreviousEntry = uiEntry;
		}
		else
		{
			if (uiEntry != uiPreviousEntry)
			{
				*pbAllEntriesSame = false;
				break;
			}
		}

	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode PrintFATClusters(CChars* psz, CFatVolume* pcVolume, uint32 uiStartClusterIndex, uint32 uiEndClusterIndex)
{
	EFatCode	eResult;
	uint32		uiOffsetInSector;
	uint32		uiSector;
	fatEntry	uiEntry;
	uint16		uiRow;
	CChars		szNumber;
	fatEntry	uiClusterIndex;
	uint32		uiPreviousSector;
	fatEntry	uiPreviousEntry;
	uint32		uiEOC;
	uint32		uiDunno;
	uint32		uiBAD;
	bool		bFirst;

	uiPreviousSector = 0;
	bFirst = true;

	uiEOC = pcVolume->GetEndOfClusterMarker();
	uiDunno = GetDunnoClusterMarker(pcVolume);
	uiBAD = pcVolume->GetBadClusterMarker();

	szNumber.Init();
	uiRow = 0;
	for (uiClusterIndex = uiStartClusterIndex; uiClusterIndex < uiEndClusterIndex; uiClusterIndex++)
	{
		pcVolume->CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiSector);

		eResult = pcVolume->ReadFatEntry(uiOffsetInSector, uiClusterIndex, uiSector, &uiEntry);
		RETURN_ON_FAT_FAILURE(eResult)

		if (bFirst || (uiPreviousSector != uiSector))
		{
			bFirst = false;

			if (psz->EndsWith(", "))
			{
				psz->RemoveFromEnd(2);
			}
			if (!psz->EndsWith('\n'))
			{
				psz->AppendNewLine();
			}

			psz->Clear();
			psz->Append("-------------------------------------------------------------------- Sector (");
			psz->Append(uiSector);
			psz->Append(") --------------------------------------------------------------------");
			psz->AppendNewLine();
			uiPreviousSector = uiSector;
			uiRow = 0;

			uiPreviousEntry = uiEntry;
		}

		szNumber.Append((int)uiClusterIndex, 16);
		psz->RightAlign(szNumber.Text(), ' ', 5);
		szNumber.Clear();
		psz->Append(" -> ");

		if (uiEntry == uiEOC)
		{
			psz->Append(" EOC");
		}
		else if (uiEntry == uiDunno)
		{
			psz->Append("FFF8");
		}
		else if (uiEntry == uiBAD)
		{
			psz->Append(" BAD");
		}
		else
		{
			szNumber.Append((int)uiEntry, 16);
			psz->RightAlign(szNumber.Text(), ' ', 4);
			szNumber.Clear();
		}
		uiRow++;
		if (uiRow == 16)
		{
			psz->AppendNewLine();
			uiRow = 0;
		}
		else
		{
			psz->Append(", ");
		}
	}

	szNumber.Kill();
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode PrintInterestingFATClusters(CChars* psz, CFatVolume* pcVolume)
{
	EFatCode	eResult;
	uint16		uiRow;
	CChars		szNumber;
	fatEntry	uiClusterIndex;
	CChars		szSector;
	bool		bAllEntriesSame;
	uint32		uiOffsetInSector;
	uint32		uiSector;
	uint32		uiPreviousSector;
	uint32		uiFirstCluster;
	bool		bFirst;

	psz->Init();
	szNumber.Init();
	szSector.Init();
	uiRow = 0;
	uiPreviousSector = 0;
	uiFirstCluster = 0;
	bFirst = true;

	for (uiClusterIndex = 0; uiClusterIndex < pcVolume->GetNumClusters(); uiClusterIndex ++)
	{
		pcVolume->CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiSector);

		if (uiSector != uiPreviousSector)
		{
			uiPreviousSector = uiSector;
			if (!bFirst)
			{
				eResult = AllFATClustersEntriesSame(&bAllEntriesSame, pcVolume, uiFirstCluster, uiClusterIndex);
				RETURN_ON_FAT_FAILURE(eResult)

				if (!bAllEntriesSame)
				{
					eResult = PrintFATClusters(psz, pcVolume, uiFirstCluster, uiClusterIndex);
					RETURN_ON_FAT_FAILURE(eResult)
				}
			}
			else
			{
				bFirst = false;
			}
			uiFirstCluster = uiClusterIndex;
		}
	}

	szSector.Kill();
	szNumber.Kill();
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpInterestingFATClusters(CFatVolume* pcVolume)
{
	CChars		sz;

	sz.Init();
	PrintInterestingFATClusters(&sz, pcVolume);
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

