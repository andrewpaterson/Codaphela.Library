#include "BaseLib/Chars.h"
#include "BaseLib/StringHelper.h"
#include "FatVolume.h"
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

					sz.Append(szShortName);
					sz.AppendNewLine();
					PrintAttributes(&sz, psEntry->uEntry.sFatRawCommon.uiAttributes);
					sz.AppendNewLine();

					//uint8	uiReserved;
					//uint8	uiCreateTimeTenths;
					//uint16	uiCreateTime;
					//uint16	uiCreateDate;
					//uint16	uiAccessDate;
					//uint16	uiFirstClusterHighWord;
					//uint16	uiModifyTime;
					//uint16	uiModifyDate;
					//uint16	uiFirstClusterLowWord;
					//uint32	uiSize;
				}

			}
		}

		psEntry++;
	}
}

