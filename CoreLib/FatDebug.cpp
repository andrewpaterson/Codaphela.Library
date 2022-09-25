#include "BaseLib/Chars.h"
#include "FatVolume.h"
#include "FatDebug.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintFATCluster(CFatVolume* pcVolume)
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

	sz.Dump();
}

