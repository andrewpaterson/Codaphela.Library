#include <stdio.h>
#include <string.h>
#include <time.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/Fat32.h"
#include "BaseLib/FatFilenameHelper.h"
#include "BaseLib/FatTime.h"
#include "TestLib/Assert.h"
#include "TestFat32Common.h"


//A test for each bug the FAT32 stress test (TestFatStress.cpp) found, the smallest way it shows.


//////////////////////////////////////////////////////////////////////////
//
// A FAT32 volume (formatted and mounted) on a drive with 65600 clusters of uiSectorsPerCluster sectors.
//
//////////////////////////////////////////////////////////////////////////
static void FatBugVolume(CFatTestDrive* pcDrive, CFatVolume* pcVolume, uint8 uiSectorsPerCluster)
{
	pcDrive->Init((uint64)65600 * uiSectorsPerCluster + 4096);
	AssertInt(FAT_SUCCESS, FatFormat(FAT_FS_TYPE_FAT32, "Bugs", uiSectorsPerCluster, pcDrive));
	AssertInt(FAT_SUCCESS, pcVolume->Mount(pcDrive));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatBugKill(CFatTestDrive* pcDrive, CFatVolume* pcVolume)
{
	AssertInt(FAT_SUCCESS, pcVolume->Unmount());
	pcDrive->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatBugFill(uint8* puiData, uint32 uiSize, uint8 uiSeed)
{
	uint32	i;

	for (i = 0; i < uiSize; i++)
	{
		puiData[i] = (uint8)(i * 7 + uiSeed + (i >> 8));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void FatBugCreate(CFatVolume* pcVolume, const char* szPath, uint8* puiData, uint32 uiSize)
{
	CFatFile	cFile;

	cFile.Init(pcVolume);
	AssertInt(FAT_SUCCESS, cFile.Open(szPath, FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE));
	if (uiSize > 0)
	{
		AssertInt(FAT_SUCCESS, cFile.Write(puiData, uiSize));
	}
	AssertInt(FAT_SUCCESS, cFile.Close());
}


//////////////////////////////////////////////////////////////////////////
//
// The file holds exactly uiSize bytes of puiData.
//
//////////////////////////////////////////////////////////////////////////
static void FatBugCheckFile(CFatVolume* pcVolume, const char* szPath, uint8* puiData, uint32 uiSize)
{
	CFatFile	cFile;
	uint8		auiRead[4096];
	lsize		uiBytesRead;

	cFile.Init(pcVolume);
	AssertInt(FAT_SUCCESS, cFile.Open(szPath, FAT_FILE_ACCESS_READ));
	AssertInt(uiSize, cFile.Size());
	AssertInt(FAT_SUCCESS, cFile.Read(auiRead, uiSize, &uiBytesRead));
	AssertLSize(uiSize, uiBytesRead);
	AssertMemory(puiData, auiRead, uiSize);
	AssertInt(FAT_SUCCESS, cFile.Close());
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szDirectory's listing has szName (and its size in *puiSize).  Returns how many entries the listing has
// (but . and ..) in *puiCount.
//
//////////////////////////////////////////////////////////////////////////
static bool FatBugListed(CFatVolume* pcVolume, const char* szDirectory, const char* szName, uint32* puiSize, uint32* puiCount)
{
	SFatDirectoryEntry*		psEntry;
	SFatFileSystemQuery		sQuery;
	EFatCode				eResult;
	bool					bFound;

	bFound = false;
	*puiCount = 0;
	sQuery.Init();
	eResult = pcVolume->FindFirstFATEntry(szDirectory, 0, &psEntry, &sQuery);
	while ((eResult == FAT_SUCCESS) && (psEntry->szName[0] != '\0'))
	{
		if ((strcmp((char*)psEntry->szName, ".") != 0) && (strcmp((char*)psEntry->szName, "..") != 0))
		{
			(*puiCount)++;
		}
		if (strcmp((char*)psEntry->szName, szName) == 0)
		{
			bFound = true;
			*puiSize = psEntry->uiSize;
		}
		eResult = pcVolume->FindNextFATEntry(&psEntry, &sQuery);
	}
	sQuery.Kill(pcVolume->GetSectorCache());
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
// The long name checksum of a short name (as FAT stores it, 11 characters).
//
//////////////////////////////////////////////////////////////////////////
static uint8 FatBugChecksum(const char* szShortName)
{
	uint8	uiSum;
	int		i;

	uiSum = 0;
	for (i = 0; i < 11; i++)
	{
		uiSum = (uint8)(((uiSum & 1) << 7) + (uiSum >> 1) + (uint8)szShortName[i]);
	}
	return uiSum;
}


//////////////////////////////////////////////////////////////////////////
//
// The checksum of the short name of the file at szPath.
//
//////////////////////////////////////////////////////////////////////////
static uint8 FatBugFileChecksum(CFatVolume* pcVolume, const char* szPath)
{
	SFatDirectoryEntry	sEntry;

	AssertInt(FAT_SUCCESS, pcVolume->GetFileEntry(szPath, &sEntry));
	return FatBugChecksum((char*)sEntry.sRaw.uEntry.sFatRawCommon.szShortName);
}


//////////////////////////////////////////////////////////////////////////
//
// The number of clusters in the chain from uiCluster.
//
//////////////////////////////////////////////////////////////////////////
static uint32 FatBugChainLength(CFatVolume* pcVolume, uint32 uiCluster)
{
	fatEntry	uiNext;
	uint32		uiCount;

	uiCount = 0;
	while ((uiCluster != 0) && !pcVolume->FatIsEOFEntry(uiCluster) && (uiCount < 100000))
	{
		uiCount++;
		AssertInt(FAT_SUCCESS, pcVolume->GetNextClusterEntry(uiCluster, &uiNext));
		uiCluster = uiNext;
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// A read that ended at the end of a cluster left the file on that cluster, so the next read read it again.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugReadEndsOnClusterBoundary(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	CFatFile		cFile;
	uint8			auiData[2048];
	uint8			auiRead[512];
	lsize			uiBytesRead;
	uint32			i;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 2048, 1);
	FatBugCreate(&cVolume, "\\Four clusters.bin", auiData, 2048);

	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open("\\Four clusters.bin", FAT_FILE_ACCESS_READ));
	for (i = 0; i < 4; i++)
	{
		AssertInt(FAT_SUCCESS, cFile.Read(auiRead, 512, &uiBytesRead));
		AssertLSize(512, uiBytesRead);
		AssertMemory(&auiData[i * 512], auiRead, 512);
	}
	AssertInt(FAT_SUCCESS, cFile.Close());
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A write inside the file that ended at the end of a cluster left the file on that cluster, so the next write wrote
// over it.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugWriteEndsOnClusterBoundary(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	CFatFile		cFile;
	uint8			auiData[2048];

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 2048, 2);
	FatBugCreate(&cVolume, "\\Rewritten.bin", auiData, 2048);

	FatBugFill(auiData, 1024, 3);
	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open("\\Rewritten.bin", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE));
	AssertInt(FAT_SUCCESS, cFile.Write(auiData, 512));
	AssertInt(FAT_SUCCESS, cFile.Write(&auiData[512], 512));
	AssertInt(FAT_SUCCESS, cFile.Close());

	FatBugCheckFile(&cVolume, "\\Rewritten.bin", auiData, 2048);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A write of part of a sector read the sector's old bytes into the wrong place in the cluster cache (and not at all
// when the write started at the sector's start), so the rest of the sector was written as zeros.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugPartialSectorWrite(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	CFatFile		cFile;
	uint8			auiData[3000];

	FatBugVolume(&cDrive, &cVolume, 8);
	FatBugFill(auiData, 3000, 4);
	FatBugCreate(&cVolume, "\\Patched.bin", auiData, 3000);

	//In the cluster's third sector, and at the start of its first.
	memset(&auiData[1100], 0xAA, 10);
	memset(&auiData[0], 0xBB, 10);
	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open("\\Patched.bin", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE));
	AssertInt(FAT_SUCCESS, cFile.Seek(1100, FAT_SEEK_START));
	AssertInt(FAT_SUCCESS, cFile.Write(&auiData[1100], 10));
	AssertInt(FAT_SUCCESS, cFile.Seek(0, FAT_SEEK_START));
	AssertInt(FAT_SUCCESS, cFile.Write(&auiData[0], 10));
	AssertInt(FAT_SUCCESS, cFile.Close());

	FatBugCheckFile(&cVolume, "\\Patched.bin", auiData, 3000);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A query that came to the end of a directory (its last cluster full) zeroed
// the directory's last entry in the sector cache to say so: the next file
// made in the directory took that entry, and the file there was lost.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugQueryEndErasesLastEntry(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	uint8			auiData[100];
	char			szPath[64];
	uint32			uiSize;
	uint32			uiCount;
	int				i;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 100, 5);

	//. and .. (2 entries), Source (2) and four files (3 each) fill the 16 entries of the directory's cluster.
	AssertInt(FAT_SUCCESS, cVolume.CreateDirectory("\\Dir"));
	AssertInt(FAT_SUCCESS, cVolume.CreateDirectory("\\Dir\\Source"));
	for (i = 0; i < 4; i++)
	{
		sprintf(szPath, "\\Dir\\File Number %d.txt", i);
		FatBugCreate(&cVolume, szPath, auiData, 100);
	}

	//The listing comes to the end of the directory, then another file is made.
	AssertTrue(FatBugListed(&cVolume, "\\Dir", "File Number 3.txt", &uiSize, &uiCount));
	AssertInt((uint32)5, uiCount);
	FatBugCreate(&cVolume, "\\Dir\\File Number 4.txt", auiData, 100);

	AssertTrue(FatBugListed(&cVolume, "\\Dir", "File Number 3.txt", &uiSize, &uiCount));
	AssertInt((uint32)6, uiCount);
	for (i = 0; i < 5; i++)
	{
		sprintf(szPath, "\\Dir\\File Number %d.txt", i);
		FatBugCheckFile(&cVolume, szPath, auiData, 100);
	}
	FatBugKill(&cDrive, &cVolume);
}




//////////////////////////////////////////////////////////////////////////
//
// The entries for a long name that went into deleted entries over the end of
// a cluster were written from the wrong place: the sector where the free
// entries ended (not where they started), and a sector past the earlier
// cluster's last.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugLongNameOverClusters(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	uint8			auiData[100];
	char			szName[256];
	char			szPath[300];
	uint32			uiSize;
	uint32			uiCount;
	int				i;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 100, 6);
	AssertInt(FAT_SUCCESS, cVolume.CreateDirectory("\\Dir"));
	FatBugCreate(&cVolume, "\\Dir\\First.txt", auiData, 10);

	//250 characters: 20 long name entries and the short one, from the first cluster into the second, then deleted.
	for (i = 0; i < 250; i++)
	{
		szName[i] = (char)('a' + (i % 26));
	}
	szName[250] = '\0';
	sprintf(szPath, "\\Dir\\%s", szName);
	FatBugCreate(&cVolume, szPath, auiData, 20);
	FatBugCreate(&cVolume, "\\Dir\\Last.txt", auiData, 30);
	AssertInt(FAT_SUCCESS, cVolume.Delete(szPath));

	//A name of 18 entries goes in the deleted ones, over the end of the first cluster.
	szName[220] = '\0';
	szName[0] = 'X';
	sprintf(szPath, "\\Dir\\%s", szName);
	FatBugCreate(&cVolume, szPath, auiData, 100);

	AssertTrue(FatBugListed(&cVolume, "\\Dir", szName, &uiSize, &uiCount));
	AssertInt((uint32)100, uiSize);
	AssertInt((uint32)3, uiCount);
	FatBugCheckFile(&cVolume, szPath, auiData, 100);
	FatBugCheckFile(&cVolume, "\\Dir\\First.txt", auiData, 10);
	FatBugCheckFile(&cVolume, "\\Dir\\Last.txt", auiData, 30);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// Names of every length that fills a long name entry exactly and one
// either side, up to 255 (all 20 entries, the 20th holding 9 characters),
// come back from a query as they were made (SetQueryLongFilenamePart copies
// each entry's 13 characters at once).  In the root: a path is at most 260.
//
//////////////////////////////////////////////////////////////////////////
void TestFatLongNameLengths(void)
{
	static const int	aiLengths[] = { 1, 12, 13, 14, 25, 26, 27, 38, 39, 40, 246, 247, 248, 254, 255 };
	CFatTestDrive		cDrive;
	CFatVolume			cVolume;
	uint8				auiData[16];
	static char			aszNames[15][256];
	char				szPath[300];
	uint32				uiSize;
	uint32				uiCount;
	int					i;
	int					j;
	int					iFiles;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 16, 9);
	iFiles = (int)(sizeof(aiLengths) / sizeof(int));
	for (i = 0; i < iFiles; i++)
	{
		for (j = 0; j < aiLengths[i]; j++)
		{
			aszNames[i][j] = (char)(((j % 3) == 2) ? ('0' + (j % 10)) : ('a' + ((j + i) % 26)));
		}
		aszNames[i][aiLengths[i]] = '\0';
		sprintf(szPath, "\\%s", aszNames[i]);
		FatBugCreate(&cVolume, szPath, auiData, (uint32)i);
	}
	for (i = 0; i < iFiles; i++)
	{
		AssertTrue(FatBugListed(&cVolume, "\\", aszNames[i], &uiSize, &uiCount));
		AssertInt((uint32)i, uiSize);
		AssertInt((uint32)iFiles, uiCount);
		sprintf(szPath, "\\%s", aszNames[i]);
		FatBugCheckFile(&cVolume, szPath, auiData, (uint32)i);
	}
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A directory entry's times are decoded from its raw date and time when
// asked for: 2024-02-29 13:45:58 is that local time, 1980-01-01 00:00:00
// (the earliest FAT date) is too, and a new file's create time is the one
// its raw entry holds.
//
//////////////////////////////////////////////////////////////////////////
void TestFatDirectoryEntryTimes(void)
{
	CFatTestDrive		cDrive;
	CFatVolume			cVolume;
	SFatDirectoryEntry	sEntry;
	uint8				auiData[16];
	time_t				tTime;
	struct tm*			psTime;

	memset(&sEntry, 0, sizeof(SFatDirectoryEntry));
	sEntry.sRaw.uEntry.sFatRawCommon.uiModifyDate = (uint16)(((2024 - 1980) << 9) | (2 << 5) | 29);
	sEntry.sRaw.uEntry.sFatRawCommon.uiModifyTime = (uint16)((13 << 11) | (45 << 5) | (58 >> 1));
	tTime = sEntry.GetModifyTime();
	psTime = localtime(&tTime);
	AssertInt(124, psTime->tm_year);
	AssertInt(1, psTime->tm_mon);
	AssertInt(29, psTime->tm_mday);
	AssertInt(13, psTime->tm_hour);
	AssertInt(45, psTime->tm_min);
	AssertInt(58, psTime->tm_sec);

	sEntry.sRaw.uEntry.sFatRawCommon.uiAccessDate = (uint16)((0 << 9) | (1 << 5) | 1);
	tTime = sEntry.GetAccessTime();
	psTime = localtime(&tTime);
	AssertInt(80, psTime->tm_year);
	AssertInt(0, psTime->tm_mon);
	AssertInt(1, psTime->tm_mday);
	AssertInt(0, psTime->tm_hour);

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 16, 4);
	FatBugCreate(&cVolume, "\\Timed.txt", auiData, 16);
	AssertInt(FAT_SUCCESS, cVolume.GetFileEntry("\\Timed.txt", &sEntry));
	AssertTrue(sEntry.GetCreateTime() == (time_t)FatDecodeDateTime(sEntry.sRaw.uEntry.sFatRawCommon.uiCreateDate, sEntry.sRaw.uEntry.sFatRawCommon.uiCreateTime));
	AssertTrue(sEntry.GetModifyTime() == (time_t)FatDecodeDateTime(sEntry.sRaw.uEntry.sFatRawCommon.uiModifyDate, sEntry.sRaw.uEntry.sFatRawCommon.uiModifyTime));
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A boot sector whose bytes per sector isn't a power of two isn't mounted
// (sector offsets are shifted and masked); put back to 512 it is.
//
//////////////////////////////////////////////////////////////////////////
void TestFatMountSectorSizePowerOfTwo(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	uint8			auiSector[512];

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugKill(&cDrive, &cVolume);

	cDrive.Init((uint64)65600 + 4096);
	AssertInt(FAT_SUCCESS, FatFormat(FAT_FS_TYPE_FAT32, "Odd", 1, &cDrive));
	AssertTrue(cDrive.Read(0, auiSector));
	AssertInt(512, auiSector[11] | (auiSector[12] << 8));
	auiSector[11] = 0x00;
	auiSector[12] = 0x03;
	AssertTrue(cDrive.Write(0, auiSector));
	AssertFalse(cVolume.Mount(&cDrive) == FAT_SUCCESS);
	AssertInt(FAT_SUCCESS, cVolume.Unmount());

	auiSector[11] = 0x00;
	auiSector[12] = 0x02;
	AssertTrue(cDrive.Write(0, auiSector));
	AssertInt(FAT_SUCCESS, cVolume.Mount(&cDrive));
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A broken long name entry with a sequence past the 20th (30) is not a long
// name: its characters went past the end of the query's name (and over the
// query).  The file is listed by its short name and still reads.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugLongNameSequenceTooHigh(void)
{
	CFatTestDrive		cDrive;
	CFatVolume			cVolume;
	SFatDirectoryEntry	sEntry;
	uint8				auiData[40];
	uint8				auiSector[512];
	char				szShortName[13];
	uint32				uiSector;
	uint32				uiSize;
	uint32				uiCount;
	uint16				uiOffset;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 40, 2);
	FatBugCreate(&cVolume, "\\Long nm.text", auiData, 40);
	AssertInt(FAT_SUCCESS, cVolume.GetFileEntry("\\Long nm.text", &sEntry));
	uiSector = sEntry.uiSectorAddress;
	uiOffset = sEntry.uiSectorOffset;
	strcpy(szShortName, (char*)sEntry.szName);
	AssertInt(FAT_SUCCESS, cVolume.Unmount());

	//The long name's one entry is just before the short one (in the same sector: it isn't the sector's first).
	AssertTrue(uiOffset >= 32);
	AssertTrue(cDrive.Read(uiSector, auiSector));
	AssertInt(FAT_FIRST_LFN_ENTRY | 1, auiSector[uiOffset - 32]);
	auiSector[uiOffset - 32] = FAT_FIRST_LFN_ENTRY | 30;
	AssertTrue(cDrive.Write(uiSector, auiSector));

	AssertInt(FAT_SUCCESS, cVolume.Mount(&cDrive));
	AssertFalse(FatBugListed(&cVolume, "\\", "Long nm.text", &uiSize, &uiCount));
	AssertTrue(FatBugListed(&cVolume, "\\", szShortName, &uiSize, &uiCount));
	AssertInt((uint32)40, uiSize);
	AssertInt((uint32)1, uiCount);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A query kept every sector it had been in locked (and making a short name
// queried the directory again for each suffix tried, locking it again): a
// directory of more than 32 sectors could not be read to its end.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugBigDirectory(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	uint8			auiData[10];
	char			szPath[64];
	uint32			uiSize;
	uint32			uiCount;
	int				i;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 10, 15);

	//200 files of 3 entries: 38 sectors.
	for (i = 0; i < 200; i++)
	{
		sprintf(szPath, "\\Many files number %d.txt", i);
		FatBugCreate(&cVolume, szPath, auiData, 10);
	}
	AssertTrue(FatBugListed(&cVolume, "\\", "Many files number 199.txt", &uiSize, &uiCount));
	AssertInt((uint32)200, uiCount);
	FatBugCheckFile(&cVolume, "\\Many files number 0.txt", auiData, 10);
	FatBugCheckFile(&cVolume, "\\Many files number 199.txt", auiData, 10);
	FatBugKill(&cDrive, &cVolume);
}




//////////////////////////////////////////////////////////////////////////
//
// Writing a name's entries over the end of a cluster of more than one sector kept the old cluster's first sector,
// so the entries after it went sectors and clusters astray.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugLongNamesInClustersOfSectors(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	uint8			auiData[100];
	char			szName[128];
	char			szPath[140];
	uint32			uiSize;
	uint32			uiCount;
	int				i;
	int				j;

	FatBugVolume(&cDrive, &cVolume, 2);
	FatBugFill(auiData, 100, 7);
	for (i = 0; i < 12; i++)
	{
		for (j = 0; j < 100; j++)
		{
			szName[j] = (char)('a' + ((i + j) % 26));
		}
		sprintf(&szName[100], "%d", i);
		sprintf(szPath, "\\%s", szName);
		FatBugCreate(&cVolume, szPath, auiData, 100);
	}
	for (i = 0; i < 12; i++)
	{
		for (j = 0; j < 100; j++)
		{
			szName[j] = (char)('a' + ((i + j) % 26));
		}
		sprintf(&szName[100], "%d", i);
		sprintf(szPath, "\\%s", szName);
		AssertTrue(FatBugListed(&cVolume, "\\", szName, &uiSize, &uiCount));
		FatBugCheckFile(&cVolume, szPath, auiData, 100);
	}
	AssertInt((uint32)12, uiCount);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A name that needs no long name (upper case 8.3) had no entries written (and its sector buffer was used before it
// was read).
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugShortNameOnly(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	uint8			auiData[100];
	uint32			uiSize;
	uint32			uiCount;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 100, 8);
	FatBugCreate(&cVolume, "\\ABC.TXT", auiData, 100);
	AssertTrue(FatBugListed(&cVolume, "\\", "ABC.TXT", &uiSize, &uiCount));
	AssertInt((uint32)100, uiSize);
	FatBugCheckFile(&cVolume, "\\ABC.TXT", auiData, 100);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A long name (no short name could be made from it) was also compared as a short name: with the short name that
// was never written, whatever was in its buffer.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugLongNameComparedAsShort(void)
{
	char		szShortName[13];
	uint16		auiLongName[FAT_MAX_PATH + 1];
	uint16		auiEntryLongName[FAT_MAX_FILENAME + 1];
	char		szPath[] = "A name much too long to be a short name.txt";
	uint8		auiEntryShortName[] = "DOCUMENTTXT";
	bool		bMatch;
	bool		bLongName;

	//The buffer holds the short name of what was looked for before.
	memcpy(szShortName, "DOCUMENTTXT", 12);
	auiEntryLongName[0] = 0;
	AssertInt(FAT_SUCCESS, MatchesFatFileName(&bMatch, &bLongName, szShortName, auiLongName, szPath, auiEntryLongName, auiEntryShortName));
	AssertTrue(bLongName);
	AssertFalse(bMatch);
}


//////////////////////////////////////////////////////////////////////////
//
// SetTotalFreeClusters assigned its parameter to itself: the free cluster count never changed.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugFreeClusterCount(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	uint8			auiData[4096];
	uint32			uiFree;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 4096, 9);
	uiFree = cVolume.GetTotalFreeClusters();
	FatBugCreate(&cVolume, "\\Eight clusters.bin", auiData, 4096);
	AssertInt(uiFree - 8, cVolume.GetTotalFreeClusters());
	AssertInt(FAT_SUCCESS, cVolume.Delete("\\Eight clusters.bin"));
	AssertInt(uiFree, cVolume.GetTotalFreeClusters());
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// Creates files (numbered from *piNext on) until one's short name has the
// checksum of another's still there.  Returns the new one's number, and the
// other's in *piOther.
//
//////////////////////////////////////////////////////////////////////////
static int FatBugCreateTwins(CFatVolume* pcVolume, uint8* auiChecksums, bool* abExists, int* piNext, int* piOther)
{
	uint8	auiData[10];
	char	szPath[64];
	int		i;
	int		j;

	memset(auiData, 1, 10);
	for (i = *piNext; i < 128; i++)
	{
		sprintf(szPath, "\\Checksum twin number %d.txt", i);
		FatBugCreate(pcVolume, szPath, auiData, 10);
		auiChecksums[i] = FatBugFileChecksum(pcVolume, szPath);
		abExists[i] = true;
		for (j = 0; j < i; j++)
		{
			if (abExists[j] && (auiChecksums[j] == auiChecksums[i]))
			{
				*piNext = i + 1;
				*piOther = j;
				return i;
			}
		}
	}
	*piNext = i;
	*piOther = -1;
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Delete and Rename deleted every long name entry in the directory with the
// file's checksum: other files' long names too.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugChecksumTwins(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	uint8			auiChecksums[128];
	bool			abExists[128];
	char			szTwin[64];
	char			szOther[64];
	uint32			uiSize;
	uint32			uiCount;
	int				iNext;
	int				iTwin;
	int				iOther;

	FatBugVolume(&cDrive, &cVolume, 1);
	memset(abExists, 0, sizeof(abExists));
	iNext = 0;

	iTwin = FatBugCreateTwins(&cVolume, auiChecksums, abExists, &iNext, &iOther);
	AssertTrue(iTwin != -1);
	sprintf(szTwin, "\\Checksum twin number %d.txt", iTwin);
	sprintf(szOther, "Checksum twin number %d.txt", iOther);
	AssertInt(FAT_SUCCESS, cVolume.Delete(szTwin));
	abExists[iTwin] = false;
	AssertTrue(FatBugListed(&cVolume, "\\", szOther, &uiSize, &uiCount));
	AssertInt((uint32)(iNext - 1), uiCount);

	iTwin = FatBugCreateTwins(&cVolume, auiChecksums, abExists, &iNext, &iOther);
	AssertTrue(iTwin != -1);
	sprintf(szTwin, "\\Checksum twin number %d.txt", iTwin);
	sprintf(szOther, "Checksum twin number %d.txt", iOther);
	AssertInt(FAT_SUCCESS, cVolume.Rename(szTwin, "\\Renamed twin.txt"));
	AssertTrue(FatBugListed(&cVolume, "\\", szOther, &uiSize, &uiCount));
	AssertTrue(FatBugListed(&cVolume, "\\", "Renamed twin.txt", &uiSize, &uiCount));
	AssertInt((uint32)(iNext - 1), uiCount);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// Truncating at the end of a cluster kept the cluster after it.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugTruncateOnClusterBoundary(void)
{
	CFatTestDrive		cDrive;
	CFatVolume			cVolume;
	CFatFile			cFile;
	SFatDirectoryEntry	sEntry;
	uint8				auiData[2048];
	uint32				uiFree;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 2048, 11);
	FatBugCreate(&cVolume, "\\Truncated.bin", auiData, 2048);
	uiFree = cVolume.GetTotalFreeClusters();

	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open("\\Truncated.bin", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE));
	AssertInt(FAT_SUCCESS, cFile.Seek(1024, FAT_SEEK_START));
	AssertInt(FAT_SUCCESS, cFile.Truncate());
	AssertInt(FAT_SUCCESS, cFile.Close());

	AssertInt(FAT_SUCCESS, cVolume.GetFileEntry("\\Truncated.bin", &sEntry));
	AssertInt((uint32)1024, sEntry.uiSize);
	AssertInt((uint32)2, FatBugChainLength(&cVolume, GetFirstClusterFromFatEntry(&sEntry.sRaw, true)));
	AssertInt(uiFree + 2, cVolume.GetTotalFreeClusters());
	FatBugCheckFile(&cVolume, "\\Truncated.bin", auiData, 1024);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A read at the end of a file that fills its last cluster read a whole cluster past the end.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugReadPastEndOfFullCluster(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	CFatFile		cFile;
	uint8			auiData[1024];
	uint8			auiRead[1024];
	lsize			uiBytesRead;

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 1024, 12);
	FatBugCreate(&cVolume, "\\Two clusters.bin", auiData, 1024);

	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open("\\Two clusters.bin", FAT_FILE_ACCESS_READ));
	AssertInt(FAT_SUCCESS, cFile.Read(auiRead, 1024, &uiBytesRead));
	AssertLSize(1024, uiBytesRead);
	AssertInt(FAT_SUCCESS, cFile.Read(auiRead, 1, &uiBytesRead));
	AssertLSize(0, uiBytesRead);
	AssertInt(FAT_SUCCESS, cFile.Close());
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// A query kept a long name after the short name entry it belonged to: a short name only entry after it with the
// same checksum was listed (and found) by the other file's long name.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugLongNameKeptForNextEntry(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	uint8			auiData[100];
	char			szShortName[12];
	char			szPath[32];
	uint8			uiChecksum;
	uint32			uiSize;
	uint32			uiCount;
	int32			i;  //Counts past 32767.

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 100, 13);
	FatBugCreate(&cVolume, "\\A long named file.txt", auiData, 50);
	uiChecksum = FatBugFileChecksum(&cVolume, "\\A long named file.txt");

	//An upper case 8.3 name with the same checksum.
	for (i = 0; i < 100000; i++)
	{
		sprintf(szShortName, "X%07d", i);
		memcpy(&szShortName[8], "TXT", 3);
		if (FatBugChecksum(szShortName) == uiChecksum)
		{
			break;
		}
	}
	AssertTrue(i < 100000);
	sprintf(szPath, "\\X%07d.TXT", i);
	FatBugCreate(&cVolume, szPath, auiData, 100);

	AssertTrue(FatBugListed(&cVolume, "\\", &szPath[1], &uiSize, &uiCount));
	AssertInt((uint32)100, uiSize);
	AssertTrue(FatBugListed(&cVolume, "\\", "A long named file.txt", &uiSize, &uiCount));
	AssertInt((uint32)50, uiSize);
	AssertInt((uint32)2, uiCount);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
// Opening to append to a file that fills its last cluster (a seek to its end) failed: the seek looked for the
// cluster after the last.
//
//////////////////////////////////////////////////////////////////////////
void TestFatBugAppendToFullCluster(void)
{
	CFatTestDrive	cDrive;
	CFatVolume		cVolume;
	CFatFile		cFile;
	uint8			auiData[1124];

	FatBugVolume(&cDrive, &cVolume, 1);
	FatBugFill(auiData, 1124, 14);
	FatBugCreate(&cVolume, "\\Appended.bin", auiData, 1024);

	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open("\\Appended.bin", FAT_FILE_ACCESS_APPEND));
	AssertInt(FAT_SUCCESS, cFile.Write(&auiData[1024], 100));
	AssertInt(FAT_SUCCESS, cFile.Close());
	FatBugCheckFile(&cVolume, "\\Appended.bin", auiData, 1124);
	FatBugKill(&cDrive, &cVolume);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32Bugs(void)
{
	TypeConverterInit();
	BeginTests();

	TestFatBugReadEndsOnClusterBoundary();
	TestFatBugWriteEndsOnClusterBoundary();
	TestFatBugPartialSectorWrite();
	TestFatBugQueryEndErasesLastEntry();
	TestFatBugLongNameOverClusters();
	TestFatBugLongNamesInClustersOfSectors();
	TestFatBugShortNameOnly();
	TestFatBugLongNameComparedAsShort();
	TestFatBugFreeClusterCount();
	TestFatBugChecksumTwins();
	TestFatBugTruncateOnClusterBoundary();
	TestFatBugReadPastEndOfFullCluster();
	TestFatBugLongNameKeptForNextEntry();
	TestFatBugAppendToFullCluster();
	TestFatBugBigDirectory();
	TestFatLongNameLengths();
	TestFatDirectoryEntryTimes();
	TestFatMountSectorSizePowerOfTwo();
	TestFatBugLongNameSequenceTooHigh();

	TestStatistics();
	TypeConverterKill();
}

