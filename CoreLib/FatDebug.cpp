#include "BaseLib/Chars.h"
#include "BaseLib/StringHelper.h"
#include "FatVolume.h"
#include "FatTime.h"
#include "FatDebug.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintInterestingFATClusters(CFatVolume* pcVolume)
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
	for (uiClusterIndex = 0; uiClusterIndex < pcVolume->GetNoOfClusters(); uiClusterIndex++)
	{
		pcVolume->CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiSector);
		eResult = pcVolume->FatReadFatSector(uiSector);
		if (eResult != FAT_SUCCESS)
		{
			break;
		}

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
void PrintRootDirectory(CFatVolume* pcVolume)
{
	EFatCode				eResult;
	uint32					uiCluster;
	uint32					uiSector;
	SFatRawDirectoryEntry*	psEntry;
	uint32					uiSectorCount;
	CChars					sz;
	char					szShortName[13];
	
	uiCluster = pcVolume->GetRootCluster();
	uiSector = pcVolume->GetRootSector();

	eResult = pcVolume->FatReadFatSector(uiSector);
	if (eResult != FAT_SUCCESS)
	{
		return;
	}

	sz.Init();
	uiSectorCount = 0;
	memset(szShortName, '\0', 13);
	psEntry = (SFatRawDirectoryEntry*)pcVolume->mauiFatSharedBuffer;
	for (;;)
	{
		if (((uintptr_t)psEntry - (uintptr_t)pcVolume->mauiFatSharedBuffer) == pcVolume->GetSectorSize() - 0x20)
		{
			if (uiSectorCount == pcVolume->GetNoOfSectorsPerCluster() - 1)
			{
				if (pcVolume->GetNextClusterEntry(uiCluster, &uiCluster) != FAT_SUCCESS)
				{
					sz.DumpKill();
					return;
				}
				if (pcVolume->FatIsEOFEntry(uiCluster))
				{
					sz.DumpKill();
					return;
				}
				uiSector = pcVolume->CalculateFirstSectorOfCluster(uiCluster); // +psQuery->uiCurrentSector;
				uiSectorCount = 0;
			}
			else
			{
				uiSectorCount++;
				uiSector++;
			}
			eResult = pcVolume->FatReadFatSector(uiSector);
			psEntry = (SFatRawDirectoryEntry*)pcVolume->mauiFatSharedBuffer;
		}

		if (!(psEntry->uEntry.sFatRawCommon.szShortName[0] == FAT_DELETED_ENTRY))
		{
			if (!((psEntry->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_LONG_NAME) == FAT_ATTR_LONG_NAME))
			{
				if (psEntry->uEntry.sFatRawCommon.szShortName[0] != '\0')
				{
					pcVolume->FatGetShortNameFromEntry((uint8*)szShortName, psEntry->uEntry.sFatRawCommon.szShortName);

					sz.Append("---------------------------------------");
					sz.AppendNewLine();
					sz.Append("Short name: ");
					sz.Append(szShortName);
					sz.AppendNewLine();

					sz.Append("Cluster:    ");
					sz.Append(GetFirstClusterFromFatEntry(psEntry, pcVolume->GetFileSystemType() == FAT_FS_TYPE_FAT32));
					sz.AppendNewLine();

					sz.Append("Size:       ");
					sz.Append(psEntry->uEntry.sFatRawCommon.uiSize);
					sz.AppendNewLine();

					sz.Append("Attributes: ");
					PrintAttributes(&sz, psEntry->uEntry.sFatRawCommon.uiAttributes);
					sz.AppendNewLine();

					sz.Append("Created:    ");
					sz.Append(FatDecodeDateTime(psEntry->uEntry.sFatRawCommon.uiCreateDate, psEntry->uEntry.sFatRawCommon.uiCreateTime));
					sz.AppendNewLine();

					sz.Append("Accessed:   ");
					sz.Append(FatDecodeDateTime(psEntry->uEntry.sFatRawCommon.uiAccessDate, 0));
					sz.AppendNewLine();

					sz.Append("Modified:   ");
					sz.Append(FatDecodeDateTime(psEntry->uEntry.sFatRawCommon.uiModifyDate, psEntry->uEntry.sFatRawCommon.uiModifyTime));
					sz.AppendNewLine();
				}
			}
			else
			{
				sz.Append("---------------------------------------");
				sz.AppendNewLine();

				sz.Append("Sequence:     ");
				sz.Append(psEntry->uEntry.sFatRawLongFileName.uiSequence);
				sz.AppendNewLine();

				sz.Append("Long name[1]: ");
				PrintLongNamePart(&sz, psEntry->uEntry.sFatRawLongFileName.auiChars1, 10);
				sz.AppendNewLine();

				sz.Append("Long name[2]: ");
				PrintLongNamePart(&sz, psEntry->uEntry.sFatRawLongFileName.auiChars2, 12);
				sz.AppendNewLine();

				sz.Append("Long name[3]: ");
				PrintLongNamePart(&sz, psEntry->uEntry.sFatRawLongFileName.auiChars3, 4);
				sz.AppendNewLine();

				sz.Append("Cluster:      ");
				sz.Append(psEntry->uEntry.sFatRawLongFileName.uiFirstCluster);
				sz.AppendNewLine();

				sz.Append("Attributes:   ");
				PrintAttributes(&sz, psEntry->uEntry.sFatRawLongFileName.uiAttributes);
				sz.AppendNewLine();

				sz.Append("Checksum:     ");
				sz.Append(psEntry->uEntry.sFatRawLongFileName.uiChecksum);
				sz.AppendNewLine();
			}
		}

		psEntry++;
	}
}

