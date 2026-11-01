#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/FileDrive.h"
#include "TestFat32Common.h"
#include "BaseLib/Fat32.h"
#include "TestLib/Assert.h"


//Random file system operations (creates, deletes, renames, overwrites, appends, truncates, directories, remounts) on
//a FAT32 volume, checked against a model of what it should hold after every one: each directory's listing, every
//file's cluster chain (its length from the file's size, no cluster in two chains) and, every so often, every file's
//contents, the clusters no chain owns and the volume's free cluster count.  The drive is as big as FAT32 needs but
//only keeps the sectors written.

#define FAT_STRESS_MAX_BYTES		(4 MB)
#define FAT_STRESS_MAX_FILES		120
#define FAT_STRESS_MAX_DIRECTORIES	12
#define FAT_STRESS_MAX_FILE_SIZE	24000
#define FAT_STRESS_FULL_CHECK		16

#ifdef __LIBC65__
#define FAT_STRESS_OPERATIONS		120
#else
#define FAT_STRESS_OPERATIONS		1500
#endif


struct SFatStressFile
{
	char	szPath[FAT_MAX_PATH + 1];
	uint8*	puiData;
	uint32	uiSize;
	uint32	uiDirectory;
};


struct SFatStressDirectory
{
	char	szPath[FAT_MAX_PATH + 1];
};


struct SFatStress
{
	CFatTestDrive			cDrive;
	CFatVolume				cVolume;
	SFatStressFile			asFiles[FAT_STRESS_MAX_FILES];
	uint32					uiFiles;
	SFatStressDirectory		asDirectories[FAT_STRESS_MAX_DIRECTORIES];
	uint32					uiDirectories;
	uint32					uiSeed;
	uint32					uiClusters;
	uint8*					puiClusterOwners;	//A bit per cluster, set for clusters in a chain.
	uint8*					puiBuffer;
	uint32					uiOperation;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint32 FatStressRandom(SFatStress* ps, uint32 uiRange)
{
	ps->uiSeed = ps->uiSeed * 1103515245 + 12345;
	return (ps->uiSeed >> 8) % uiRange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressFill(SFatStress* ps, uint8* puiData, uint32 uiSize)
{
	uint32	i;

	for (i = 0; i < uiSize; i++)
	{
		puiData[i] = (uint8)FatStressRandom(ps, 256);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szName (in uiDirectory) is taken by a file or a directory, ignoring case as FAT does.
//
//////////////////////////////////////////////////////////////////////////
static bool FatStressNameUsed(SFatStress* ps, uint32 uiDirectory, const char* szName)
{
	char	szPath[FAT_MAX_PATH + 1];
	uint32	i;

	sprintf(szPath, "%s\\%s", ps->asDirectories[uiDirectory].szPath, szName);
	for (i = 0; i < ps->uiFiles; i++)
	{
		if (StringInsensitiveCompare(ps->asFiles[i].szPath, szPath) == 0)
		{
			return true;
		}
	}
	for (i = 0; i < ps->uiDirectories; i++)
	{
		if (StringInsensitiveCompare(ps->asDirectories[i].szPath, szPath) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A new name in uiDirectory: an upper case 8.3 name, a short long name or one up to the longest the path allows.
//
//////////////////////////////////////////////////////////////////////////
static void FatStressNewPath(SFatStress* ps, uint32 uiDirectory, uint32 uiMaxLength, char* szPath)
{
	static const char	szShort[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	static const char	szLong[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -_.()!#$%&'@^{}~";  //BaseLib does not allow + , ; = [ ] (legal in long names).
	char				szName[FAT_MAX_FILENAME + 1];
	uint32				uiLength;
	uint32				uiExtension;
	uint32				uiKind;
	uint32				uiLimit;
	uint32				i;

	uiLimit = FAT_MAX_PATH - 1 - (uint32)strlen(ps->asDirectories[uiDirectory].szPath) - 1;
	if (uiLimit > uiMaxLength)
	{
		uiLimit = uiMaxLength;
	}
	do
	{
		uiKind = FatStressRandom(ps, 10);
		if ((uiKind < 3) || (uiLimit < 12))
		{
			uiLength = 1 + FatStressRandom(ps, 8);
			for (i = 0; i < uiLength; i++)
			{
				szName[i] = szShort[FatStressRandom(ps, sizeof(szShort) - 1)];
			}
			uiExtension = FatStressRandom(ps, 4);
			if (uiExtension > 0)
			{
				szName[uiLength++] = '.';
				for (i = 0; i < uiExtension; i++)
				{
					szName[uiLength++] = szShort[FatStressRandom(ps, sizeof(szShort) - 1)];
				}
			}
		}
		else
		{
			uiLength = 1 + FatStressRandom(ps, (uiKind < 8) ? 30 : uiLimit);
			for (i = 0; i < uiLength; i++)
			{
				szName[i] = szLong[FatStressRandom(ps, sizeof(szLong) - 1)];
			}

			//Windows drops leading spaces and trailing spaces and dots, and . and .. are taken.
			while ((uiLength > 0) && ((szName[uiLength - 1] == ' ') || (szName[uiLength - 1] == '.')))
			{
				uiLength--;
			}
			if (uiLength > 0)
			{
				szName[0] = 'x';
			}
		}
		szName[uiLength] = '\0';
	}
	while ((uiLength == 0) || FatStressNameUsed(ps, uiDirectory, szName));
	sprintf(szPath, "%s\\%s", ps->asDirectories[uiDirectory].szPath, szName);
}


//The checks below run over every cluster and every directory entry after every operation.  So that each check counts
//as one test, they only Assert when something is wrong and return whether everything was right; FatStressRun
//Asserts that.

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool FatStressMarkCluster(SFatStress* ps, uint32 uiCluster)
{
	bool	bOwned;

	if (uiCluster < 2)
	{
		AssertTrue(uiCluster >= 2);
		return false;
	}
	if (uiCluster >= ps->uiClusters + 2)
	{
		AssertTrue(uiCluster < ps->uiClusters + 2);
		return false;
	}
	bOwned = FixBool(ps->puiClusterOwners[uiCluster >> 3] & ((uint32)1 << (uiCluster & 7)));
	if (bOwned)
	{
		AssertFalse(bOwned);
		return false;
	}
	ps->puiClusterOwners[uiCluster >> 3] |= (uint8)(1 << (uiCluster & 7));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the chain's clusters as owned (each once) and sets how many there are.
//
//////////////////////////////////////////////////////////////////////////
static bool FatStressMarkChain(SFatStress* ps, uint32 uiCluster, uint32* puiCount)
{
	EFatCode	eResult;
	fatEntry	uiNext;

	*puiCount = 0;
	while ((uiCluster != 0) && !ps->cVolume.FatIsEOFEntry(uiCluster))
	{
		if (!FatStressMarkCluster(ps, uiCluster))
		{
			return false;
		}
		(*puiCount)++;
		if (*puiCount > ps->uiClusters)
		{
			AssertTrue(*puiCount <= ps->uiClusters);
			return false;
		}
		eResult = ps->cVolume.GetNextClusterEntry(uiCluster, &uiNext);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			return false;
		}
		uiCluster = uiNext;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The directory lists what the model says it holds (and its . and .. if it is not the root), with the right sizes
// and chains.
//
//////////////////////////////////////////////////////////////////////////
static bool FatStressCheckDirectory(SFatStress* ps, uint32 uiDirectory)
{
	SFatDirectoryEntry*		psEntry;
	SFatFileSystemQuery		sQuery;
	EFatCode				eResult;
	char					szPath[FAT_MAX_PATH + 1];
	uint32					uiExpected;
	uint32					uiFound;
	uint32					uiDots;
	uint32					uiClusters;
	uint32					uiFileClusters;
	uint32					uiClusterSize;
	uint32					uiExpectedDots;
	uint32					i;
	bool					bFound;
	bool					bResult;

	uiClusterSize = ps->cVolume.GetClusterSize();
	uiExpected = 0;
	for (i = 0; i < ps->uiFiles; i++)
	{
		uiExpected += (ps->asFiles[i].uiDirectory == uiDirectory) ? 1 : 0;
	}
	for (i = 1; i < ps->uiDirectories; i++)
	{
		strcpy(szPath, ps->asDirectories[i].szPath);
		*strrchr(szPath, '\\') = '\0';
		uiExpected += (strcmp(szPath, ps->asDirectories[uiDirectory].szPath) == 0) ? 1 : 0;
	}

	uiFound = 0;
	uiDots = 0;
	bResult = true;
	sQuery.Init();
	eResult = ps->cVolume.FindFirstFATEntry((uiDirectory == 0) ? "\\" : ps->asDirectories[uiDirectory].szPath, 0, &psEntry, &sQuery);
	while (bResult && (eResult == FAT_SUCCESS) && (psEntry->szName[0] != '\0'))
	{
		if ((strcmp((char*)psEntry->szName, ".") == 0) || (strcmp((char*)psEntry->szName, "..") == 0))
		{
			uiDots++;
		}
		else
		{
			uiFound++;
			sprintf(szPath, "%s\\%s", ps->asDirectories[uiDirectory].szPath, (char*)psEntry->szName);
			bFound = false;
			if (psEntry->uiAttributes & FAT_ATTR_DIRECTORY)
			{
				for (i = 1; i < ps->uiDirectories; i++)
				{
					if (strcmp(ps->asDirectories[i].szPath, szPath) == 0)
					{
						bFound = true;
						break;
					}
				}
				if (!bFound)
				{
					AssertTrue(bFound);
					bResult = false;
				}
				else if (!FatStressMarkChain(ps, GetFirstClusterFromFatEntry(&psEntry->sRaw, true), &uiClusters))
				{
					bResult = false;
				}
				else if (uiClusters == 0)
				{
					AssertTrue(uiClusters > 0);
					bResult = false;
				}
			}
			else
			{
				for (i = 0; i < ps->uiFiles; i++)
				{
					if (strcmp(ps->asFiles[i].szPath, szPath) == 0)
					{
						bFound = true;
						break;
					}
				}
				if (!bFound)
				{
					AssertString("A file in the model", szPath);
					bResult = false;
				}
				else if (ps->asFiles[i].uiSize != psEntry->uiSize)
				{
					AssertInt(ps->asFiles[i].uiSize, psEntry->uiSize);
					bResult = false;
				}
				else if (!FatStressMarkChain(ps, GetFirstClusterFromFatEntry(&psEntry->sRaw, true), &uiClusters))
				{
					bResult = false;
				}
				else
				{
					uiFileClusters = (ps->asFiles[i].uiSize + uiClusterSize - 1) / uiClusterSize;
					if (uiFileClusters != uiClusters)
					{
						AssertInt(uiFileClusters, uiClusters);
						bResult = false;
					}
				}
			}
		}
		if (bResult)
		{
			eResult = ps->cVolume.FindNextFATEntry(&psEntry, &sQuery);
		}
	}
	sQuery.Kill(ps->cVolume.GetSectorCache());
	if (!bResult)
	{
		return false;
	}
	if (eResult != FAT_SUCCESS)
	{
		AssertInt(FAT_SUCCESS, eResult);
		return false;
	}
	if (uiExpected != uiFound)
	{
		AssertInt(uiExpected, uiFound);
		return false;
	}
	uiExpectedDots = (uiDirectory == 0) ? 0 : 2;
	if (uiExpectedDots != uiDots)
	{
		AssertInt(uiExpectedDots, uiDots);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool FatStressCheckContents(SFatStress* ps, SFatStressFile* psFile)
{
	CFatFile	cFatFile;
	EFatCode	eResult;
	uint32		uiDone;
	uint32		uiLength;
	lsize		uiBytesRead;
	bool		bResult;

	cFatFile.Init(&ps->cVolume);
	eResult = cFatFile.Open(psFile->szPath, FAT_FILE_ACCESS_READ);
	if (eResult != FAT_SUCCESS)
	{
		AssertInt(FAT_SUCCESS, eResult);
		return false;
	}

	bResult = true;
	if (psFile->uiSize != cFatFile.Size())
	{
		AssertInt(psFile->uiSize, cFatFile.Size());
		bResult = false;
	}
	for (uiDone = 0; bResult && (uiDone < psFile->uiSize); uiDone += uiLength)
	{
		uiLength = psFile->uiSize - uiDone;
		if (uiLength > 4096)
		{
			uiLength = 4096;
		}
		eResult = cFatFile.Read(ps->puiBuffer, uiLength, &uiBytesRead);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bResult = false;
		}
		else if (uiBytesRead != uiLength)
		{
			AssertLSize(uiLength, uiBytesRead);
			bResult = false;
		}
		else if (memcmp(&psFile->puiData[uiDone], ps->puiBuffer, uiLength) != 0)
		{
			AssertMemory(&psFile->puiData[uiDone], ps->puiBuffer, uiLength);
			bResult = false;
		}
	}
	if (bResult)
	{
		eResult = cFatFile.Read(ps->puiBuffer, 1, &uiBytesRead);
		if (uiBytesRead != 0)
		{
			AssertLSize(0, uiBytesRead);
			bResult = false;
		}
	}
	eResult = cFatFile.Close();
	if (eResult != FAT_SUCCESS)
	{
		AssertInt(FAT_SUCCESS, eResult);
		bResult = false;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Every directory, and (when bFull) every file's contents, no cluster outside the chains in use and the free count.
//
//////////////////////////////////////////////////////////////////////////
static bool FatStressCheck(SFatStress* ps, bool bFull)
{
	EFatCode	eResult;
	fatEntry	uiEntry;
	uint32		uiOwned;
	uint32		uiRootClusters;
	uint32		uiFree;
	uint32		i;

	memset(ps->puiClusterOwners, 0, (ps->uiClusters + 2 + 7) / 8);
	if (!FatStressMarkChain(ps, ps->cVolume.GetRootCluster(), &uiRootClusters))
	{
		return false;
	}
	for (i = 0; i < ps->uiDirectories; i++)
	{
		if (!FatStressCheckDirectory(ps, i))
		{
			return false;
		}
	}
	if (!bFull)
	{
		return true;
	}

	for (i = 0; i < ps->uiFiles; i++)
	{
		if (!FatStressCheckContents(ps, &ps->asFiles[i]))
		{
			return false;
		}
	}

	uiOwned = 0;
	for (i = 2; i < ps->uiClusters + 2; i++)
	{
		eResult = ps->cVolume.GetNextClusterEntry(i, &uiEntry);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			return false;
		}
		if (ps->puiClusterOwners[i >> 3] & (1 << (i & 7)))
		{
			uiOwned++;
		}
		else if ((uiEntry & 0x0FFFFFFF) != 0)
		{
			//A cluster no chain owns is free.
			AssertInt((uint32)0, (uint32)(uiEntry & 0x0FFFFFFF));
			return false;
		}
	}
	uiFree = ps->cVolume.GetNumClusters() - uiOwned;
	if (uiFree != ps->cVolume.GetTotalFreeClusters())
	{
		AssertInt(uiFree, ps->cVolume.GetTotalFreeClusters());
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The clusters the files use (and those a write of uiMoreBytes would add) fit in the memory set aside for them.
//
//////////////////////////////////////////////////////////////////////////
static bool FatStressHasRoom(SFatStress* ps, uint32 uiMoreBytes)
{
	uint32	uiClusterSize;
	uint32	uiClusters;
	uint32	i;

	uiClusterSize = ps->cVolume.GetClusterSize();
	uiClusters = (uiMoreBytes + uiClusterSize - 1) / uiClusterSize + 1;
	for (i = 0; i < ps->uiFiles; i++)
	{
		uiClusters += (ps->asFiles[i].uiSize + uiClusterSize - 1) / uiClusterSize;
	}
	return uiClusters * uiClusterSize < FAT_STRESS_MAX_BYTES;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressWrite(SFatStress* ps, SFatStressFile* psFile, uint32 uiStart, uint32 uiLength, uint8 uiAccess)
{
	CFatFile	cFatFile;
	EFatCode	eResult;
	uint32		uiDone;
	uint32		uiChunk;

	cFatFile.Init(&ps->cVolume);
	eResult = cFatFile.Open(psFile->szPath, uiAccess);
	AssertInt(FAT_SUCCESS, eResult);
	if (!(uiAccess & FAT_FILE_ACCESS_APPEND))
	{
		eResult = cFatFile.Seek(uiStart, FAT_SEEK_START);
		AssertInt(FAT_SUCCESS, eResult);
	}
	AssertInt(uiStart, cFatFile.Tell());
	for (uiDone = 0; uiDone < uiLength; uiDone += uiChunk)
	{
		uiChunk = 1 + FatStressRandom(ps, 3000);
		if (uiChunk > uiLength - uiDone)
		{
			uiChunk = uiLength - uiDone;
		}
		eResult = cFatFile.Write(&psFile->puiData[uiStart + uiDone], uiChunk);
		AssertInt(FAT_SUCCESS, eResult);
	}
	AssertInt(psFile->uiSize, cFatFile.Size());
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressCreate(SFatStress* ps)
{
	SFatStressFile*	psFile;
	uint32			uiSize;

	uiSize = (FatStressRandom(ps, 4) == 0) ? 0 : FatStressRandom(ps, FatStressRandom(ps, 2) ? 3000 : FAT_STRESS_MAX_FILE_SIZE);
	if ((ps->uiFiles == FAT_STRESS_MAX_FILES) || !FatStressHasRoom(ps, uiSize))
	{
		return;
	}
	psFile = &ps->asFiles[ps->uiFiles];
	psFile->uiDirectory = FatStressRandom(ps, ps->uiDirectories);
	FatStressNewPath(ps, psFile->uiDirectory, FAT_MAX_FILENAME, psFile->szPath);
	psFile->uiSize = uiSize;
	psFile->puiData = (uint8*)malloc(FAT_STRESS_MAX_FILE_SIZE);
	FatStressFill(ps, psFile->puiData, uiSize);
	ps->uiFiles++;
	FatStressWrite(ps, psFile, 0, uiSize, FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressDelete(SFatStress* ps, uint32 uiFile)
{
	EFatCode	eResult;

	eResult = ps->cVolume.Delete(ps->asFiles[uiFile].szPath);
	AssertInt(FAT_SUCCESS, eResult);
	free(ps->asFiles[uiFile].puiData);
	ps->uiFiles--;
	ps->asFiles[uiFile] = ps->asFiles[ps->uiFiles];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressRename(SFatStress* ps, uint32 uiFile)
{
	SFatStressFile*	psFile;
	EFatCode		eResult;
	char			szNewPath[FAT_MAX_PATH + 1];

	psFile = &ps->asFiles[uiFile];
	FatStressNewPath(ps, psFile->uiDirectory, FAT_MAX_FILENAME, szNewPath);
	eResult = ps->cVolume.Rename(psFile->szPath, szNewPath);
	AssertInt(FAT_SUCCESS, eResult);
	strcpy(psFile->szPath, szNewPath);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressOverwrite(SFatStress* ps, uint32 uiFile)
{
	SFatStressFile*	psFile;
	uint32			uiStart;
	uint32			uiLength;

	psFile = &ps->asFiles[uiFile];
	if (psFile->uiSize == 0)
	{
		return;
	}
	uiStart = FatStressRandom(ps, psFile->uiSize);
	uiLength = 1 + FatStressRandom(ps, psFile->uiSize - uiStart);
	FatStressFill(ps, &psFile->puiData[uiStart], uiLength);
	FatStressWrite(ps, psFile, uiStart, uiLength, FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE);
}


//////////////////////////////////////////////////////////////////////////
//
// Appends, or writes over the end and on past it.
//
//////////////////////////////////////////////////////////////////////////
static void FatStressExtend(SFatStress* ps, uint32 uiFile, bool bAppend)
{
	SFatStressFile*	psFile;
	uint32			uiStart;
	uint32			uiLength;
	uint32			uiOldSize;

	psFile = &ps->asFiles[uiFile];
	uiOldSize = psFile->uiSize;
	uiStart = (bAppend || (uiOldSize == 0)) ? uiOldSize : uiOldSize - FatStressRandom(ps, (uiOldSize < 2000) ? uiOldSize : 2000);
	uiLength = FatStressRandom(ps, FAT_STRESS_MAX_FILE_SIZE - uiStart);
	if ((uiStart + uiLength <= uiOldSize) || !FatStressHasRoom(ps, uiStart + uiLength - uiOldSize))
	{
		return;
	}
	FatStressFill(ps, &psFile->puiData[uiStart], uiLength);
	psFile->uiSize = uiStart + uiLength;
	FatStressWrite(ps, psFile, uiStart, uiLength, bAppend ? FAT_FILE_ACCESS_APPEND : (FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressTruncate(SFatStress* ps, uint32 uiFile)
{
	SFatStressFile*	psFile;
	CFatFile		cFatFile;
	EFatCode		eResult;
	uint32			uiSize;

	psFile = &ps->asFiles[uiFile];
	if (psFile->uiSize == 0)
	{
		return;
	}
	uiSize = FatStressRandom(ps, psFile->uiSize);
	cFatFile.Init(&ps->cVolume);
	eResult = cFatFile.Open(psFile->szPath, FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cFatFile.Seek(uiSize, FAT_SEEK_START);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cFatFile.Truncate();
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(uiSize, cFatFile.Size());
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);
	psFile->uiSize = uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
// Small reads after seeks.
//
//////////////////////////////////////////////////////////////////////////
static void FatStressSeekAndRead(SFatStress* ps, uint32 uiFile)
{
	SFatStressFile*	psFile;
	CFatFile		cFatFile;
	EFatCode		eResult;
	uint32			uiStart;
	uint32			uiLength;
	uint32			i;
	lsize			uiBytesRead;

	psFile = &ps->asFiles[uiFile];
	if (psFile->uiSize == 0)
	{
		return;
	}
	cFatFile.Init(&ps->cVolume);
	eResult = cFatFile.Open(psFile->szPath, FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	for (i = 0; i < 8; i++)
	{
		uiStart = FatStressRandom(ps, psFile->uiSize);
		uiLength = 1 + FatStressRandom(ps, (psFile->uiSize - uiStart < 4096) ? psFile->uiSize - uiStart : 4096);
		eResult = cFatFile.Seek(uiStart, FAT_SEEK_START);
		AssertInt(FAT_SUCCESS, eResult);
		eResult = cFatFile.Read(ps->puiBuffer, uiLength, &uiBytesRead);
		AssertInt(FAT_SUCCESS, eResult);
		AssertLSize(uiLength, uiBytesRead);
		AssertMemory(&psFile->puiData[uiStart], ps->puiBuffer, uiLength);
	}
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressCreateDirectory(SFatStress* ps)
{
	SFatStressDirectory*	psDirectory;
	EFatCode				eResult;
	uint32					uiParent;

	if ((ps->uiDirectories == FAT_STRESS_MAX_DIRECTORIES) || !FatStressHasRoom(ps, 0))
	{
		return;
	}
	uiParent = FatStressRandom(ps, ps->uiDirectories);
	if (strlen(ps->asDirectories[uiParent].szPath) > 120)
	{
		return;
	}
	psDirectory = &ps->asDirectories[ps->uiDirectories];
	FatStressNewPath(ps, uiParent, 40, psDirectory->szPath);
	eResult = ps->cVolume.CreateDirectory(psDirectory->szPath);
	AssertInt(FAT_SUCCESS, eResult);
	ps->uiDirectories++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressRemount(SFatStress* ps)
{
	EFatCode	eResult;

	eResult = ps->cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);
	eResult = ps->cVolume.Mount(&ps->cDrive);
	AssertInt(FAT_SUCCESS, eResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressOperation(SFatStress* ps)
{
	uint32	uiKind;
	uint32	uiFile;

	uiKind = FatStressRandom(ps, 100);
	uiFile = (ps->uiFiles > 0) ? FatStressRandom(ps, ps->uiFiles) : 0;
	if ((uiKind < 30) || (ps->uiFiles == 0))
	{
		FatStressCreate(ps);
	}
	else if (uiKind < 45)
	{
		FatStressDelete(ps, uiFile);
	}
	else if (uiKind < 57)
	{
		FatStressRename(ps, uiFile);
	}
	else if (uiKind < 67)
	{
		FatStressOverwrite(ps, uiFile);
	}
	else if (uiKind < 73)
	{
		FatStressExtend(ps, uiFile, true);
	}
	else if (uiKind < 78)
	{
		FatStressExtend(ps, uiFile, false);
	}
	else if (uiKind < 84)
	{
		FatStressTruncate(ps, uiFile);
	}
	else if (uiKind < 94)
	{
		FatStressSeekAndRead(ps, uiFile);
	}
	else if (uiKind < 98)
	{
		FatStressCreateDirectory(ps);
	}
	else
	{
		FatStressRemount(ps);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatStressRun(uint8 uiSectorsPerCluster, uint64 uiSectors, uint32 uiSeed)
{
	SFatStress*		ps;
	EFatCode		eResult;
	uint32			i;

	ps = new SFatStress();
	ps->uiSeed = uiSeed;
	ps->cDrive.Init(uiSectors);
	eResult = FatFormat(FAT_FS_TYPE_FAT32, "Stress", uiSectorsPerCluster, &ps->cDrive);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = ps->cVolume.Mount(&ps->cDrive);
	AssertInt(FAT_SUCCESS, eResult);

	ps->uiClusters = ps->cVolume.GetNumClusters();
	ps->puiClusterOwners = (uint8*)malloc((ps->uiClusters + 2 + 7) / 8);
	ps->puiBuffer = (uint8*)malloc(4096);
	ps->asDirectories[0].szPath[0] = '\0';
	ps->uiDirectories = 1;
	AssertTrue(FatStressCheck(ps, true));

	for (ps->uiOperation = 0; ps->uiOperation < FAT_STRESS_OPERATIONS; ps->uiOperation++)
	{
		FatStressOperation(ps);
		AssertTrue(FatStressCheck(ps, ((ps->uiOperation + 1) % FAT_STRESS_FULL_CHECK) == 0));
	}
	FatStressRemount(ps);
	AssertTrue(FatStressCheck(ps, true));

	eResult = ps->cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);
	for (i = 0; i < ps->uiFiles; i++)
	{
		free(ps->asFiles[i].puiData);
	}
	free(ps->puiBuffer);
	free(ps->puiClusterOwners);
	ps->cDrive.Kill();
	delete ps;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFatStress(void)
{
	TypeConverterInit();
	BeginTests();

	//FAT32 needs at least 65525 clusters: 512 byte clusters (33MB) and 4KB clusters (260MB).
	FatStressRun(1, 65600 + 4096, 1);
	FatStressRun(8, 65600 * 8 + 4096, 2);
	FatStressRun(1, 65600 + 4096, 3);

	TestStatistics();
	TypeConverterKill();
}

