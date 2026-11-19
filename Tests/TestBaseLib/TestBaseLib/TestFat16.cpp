#include <string.h>
#include <stdlib.h>
#include "BaseLib/MemoryDrive.h"
#include "BaseLib/Fat32.h"
#include "TestLib/Assert.h"


//An 8MB drive (16384 sectors of 512 bytes: it fits in the W65C816's memory) formatted FAT16 with 1KB clusters: 16K
//clusters, a fixed 512 entry root directory.  FAT12 isn't supported.
#define TEST_FAT16_SECTORS				16384
#define TEST_FAT16_SECTORS_PER_CLUSTER	2
#define TEST_FAT16_BIG					70000			//Bytes: 69 clusters.
#define TEST_FAT16_ROOT_ENTRIES			512


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFat16Fill(uint8* puiData, uint32 uiLength, uint32 uiSeed)
{
	uint32	i;

	for (i = 0; i < uiLength; i++)
	{
		puiData[i] = (uint8)((i * 7 + uiSeed + (i >> 9)) & 0xFF);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Writes uiLength bytes of the seed's pattern as szName (created).
//
//////////////////////////////////////////////////////////////////////////
static void TestFat16Write(CFatVolume* pcVolume, const char* szName, uint32 uiLength, uint32 uiSeed)
{
	CFatFile	cFile;
	uint8*		puiData;

	puiData = (uint8*)malloc(uiLength);
	TestFat16Fill(puiData, uiLength, uiSeed);
	cFile.Init(pcVolume);
	AssertInt(FAT_SUCCESS, cFile.Open(szName, FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE));
	AssertInt(FAT_SUCCESS, cFile.Write(puiData, uiLength));
	AssertInt(uiLength, cFile.Size());
	AssertInt(FAT_SUCCESS, cFile.Close());
	free(puiData);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads szName and checks it is uiLength bytes of the seed's pattern.
//
//////////////////////////////////////////////////////////////////////////
static void TestFat16Read(CFatVolume* pcVolume, const char* szName, uint32 uiLength, uint32 uiSeed)
{
	CFatFile	cFile;
	uint8*		puiData;
	uint8*		puiExpected;
	lsize		uiRead;

	puiData = (uint8*)malloc(uiLength + 1);
	puiExpected = (uint8*)malloc(uiLength);
	TestFat16Fill(puiExpected, uiLength, uiSeed);
	cFile.Init(pcVolume);
	AssertInt(FAT_SUCCESS, cFile.Open(szName, FAT_FILE_ACCESS_READ));
	AssertInt(uiLength, cFile.Size());
	uiRead = 0;
	AssertInt(FAT_SUCCESS, cFile.Read(puiData, uiLength, &uiRead));
	AssertInt(uiLength, (uint32)uiRead);
	AssertInt(0, memcmp(puiData, puiExpected, uiLength));

	//Seeking into the middle (into a later cluster) and reading on.
	if (uiLength > 5000)
	{
		AssertInt(FAT_SUCCESS, cFile.Seek(5000, FAT_SEEK_START));
		AssertInt(FAT_SUCCESS, cFile.Read(puiData, 100, &uiRead));
		AssertInt(0, memcmp(puiData, &puiExpected[5000], 100));
	}
	AssertInt(FAT_SUCCESS, cFile.Close());
	free(puiExpected);
	free(puiData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TestFat16Exists(CFatVolume* pcVolume, const char* szName)
{
	SFatDirectoryEntry	sEntry;

	AssertInt(FAT_SUCCESS, pcVolume->GetFileEntry(szName, &sEntry));
	return sEntry.szName[0] != '\0';
}


//////////////////////////////////////////////////////////////////////////
//
// A FAT16 volume: formatted, mounted, files and directories written,
// listed, renamed and deleted, the root directory filled, remounted.
//
//////////////////////////////////////////////////////////////////////////
void TestFat16Volume(void)
{
	CMemoryDrive			cDrive;
	CFatVolume				cVolume;
	SFatFileSystemQuery		sQuery;
	SFatDirectoryEntry*		psEntry;
	SFatDirectoryEntry*		psNewEntry;
	EFatCode				eResult;
	uint32					uiFreeClusters;
	int						iFiles;
	int						iDirectories;
	char					szName[16];
	int						i;

	cDrive.Init((uint64)TEST_FAT16_SECTORS * 512, 512);
	AssertInt(FAT_SUCCESS, FatFormat(FAT_FS_TYPE_FAT16, "Sixteen", TEST_FAT16_SECTORS_PER_CLUSTER, &cDrive));
	AssertInt(FAT_SUCCESS, cVolume.Mount(&cDrive));
	AssertInt((int)FAT_FS_TYPE_FAT16, (int)cVolume.GetFileSystemType());
	AssertString("Sixteen", cVolume.GetLabel());
	AssertInt(TEST_FAT16_SECTORS_PER_CLUSTER, (int)cVolume.NumSectorsPerCluster());
	AssertTrue(cVolume.GetNumClusters() > FAT12_MAX_CLUSTERS);
	AssertTrue(cVolume.GetNumClusters() <= FAT16_MAX_CLUSTERS);
	AssertInt((uint32)0, cVolume.GetRootCluster());
	AssertInt((uint32)FAT16_EOC, cVolume.GetEndOfClusterMarker());
	uiFreeClusters = cVolume.GetTotalFreeClusters();

	//Files in the root and in a directory, one of many clusters with a long name.
	TestFat16Write(&cVolume, "\\Hello.txt", 3000, 1);
	AssertInt(FAT_SUCCESS, cVolume.CreateDirectory("\\Sub Directory"));
	TestFat16Write(&cVolume, "\\Sub Directory\\A Rather Long File Name.data", TEST_FAT16_BIG, 2);
	TestFat16Read(&cVolume, "\\Hello.txt", 3000, 1);
	TestFat16Read(&cVolume, "\\Sub Directory\\A Rather Long File Name.data", TEST_FAT16_BIG, 2);

	//The root lists the file and the directory.
	iFiles = 0;
	iDirectories = 0;
	sQuery.Init();
	eResult = cVolume.FindFirstFATEntry(NULL, 0, &psEntry, &sQuery);
	while ((eResult == FAT_SUCCESS) && (psEntry->szName[0] != '\0'))
	{
		if (psEntry->uiAttributes & FAT_ATTR_DIRECTORY)
		{
			AssertString("Sub Directory", (char*)psEntry->szName);
			iDirectories++;
		}
		else
		{
			AssertString("Hello.txt", (char*)psEntry->szName);
			AssertInt((uint32)3000, psEntry->uiSize);
			iFiles++;
		}
		eResult = cVolume.FindNextFATEntry(&psEntry, &sQuery);
	}
	sQuery.Kill(cVolume.GetSectorCache());
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(1, iFiles);
	AssertInt(1, iDirectories);

	//The directory starts with "." (itself) and ".." (the root: cluster 0).
	sQuery.Init();
	AssertInt(FAT_SUCCESS, cVolume.FindFirstFATEntry("\\Sub Directory", 0, &psEntry, &sQuery));
	AssertString(".", (char*)psEntry->szName);
	AssertTrue(GetFirstClusterFromFatEntry(&psEntry->sRaw, false) >= 2);
	AssertInt(FAT_SUCCESS, cVolume.FindNextFATEntry(&psEntry, &sQuery));
	AssertString("..", (char*)psEntry->szName);
	AssertInt((uint32)0, GetFirstClusterFromFatEntry(&psEntry->sRaw, false));
	AssertInt(FAT_SUCCESS, cVolume.FindNextFATEntry(&psEntry, &sQuery));
	AssertString("A Rather Long File Name.data", (char*)psEntry->szName);
	sQuery.Kill(cVolume.GetSectorCache());

	//Renamed and deleted: the clusters come back.
	AssertInt(FAT_SUCCESS, cVolume.Rename("\\Hello.txt", "\\Goodbye.txt"));
	AssertFalse(TestFat16Exists(&cVolume, "\\Hello.txt"));
	AssertTrue(TestFat16Exists(&cVolume, "\\Goodbye.txt"));
	TestFat16Read(&cVolume, "\\Goodbye.txt", 3000, 1);
	AssertInt(FAT_SUCCESS, cVolume.Delete("\\Sub Directory\\A Rather Long File Name.data"));
	AssertFalse(TestFat16Exists(&cVolume, "\\Sub Directory\\A Rather Long File Name.data"));
	AssertInt(FAT_SUCCESS, cVolume.Delete("\\Goodbye.txt"));
	AssertInt(uiFreeClusters - 1, cVolume.GetTotalFreeClusters());

	//The root directory is a fixed run of sectors: it fills up (the directory and the volume label take two entries).
	psNewEntry = (SFatDirectoryEntry*)malloc(sizeof(SFatDirectoryEntry));
	for (i = 0; i < TEST_FAT16_ROOT_ENTRIES; i++)
	{
		strcpy(szName, "\\F");
		szName[2] = (char)('A' + (i / 26 / 26) % 26);
		szName[3] = (char)('A' + (i / 26) % 26);
		szName[4] = (char)('A' + i % 26);
		szName[5] = '\0';
		eResult = cVolume.CreateFATEntry(NULL, &szName[1], FAT_ATTR_ARCHIVE, 0, psNewEntry);
		if (eResult != FAT_SUCCESS)
		{
			break;
		}
	}
	free(psNewEntry);
	AssertInt(FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED, eResult);
	AssertTrue(i > TEST_FAT16_ROOT_ENTRIES - 4);
	AssertTrue(TestFat16Exists(&cVolume, "\\FAAA"));

	//It all stays after an unmount.
	AssertInt(FAT_SUCCESS, cVolume.Unmount());
	AssertInt(FAT_SUCCESS, cVolume.Mount(&cDrive));
	AssertInt((int)FAT_FS_TYPE_FAT16, (int)cVolume.GetFileSystemType());
	AssertTrue(TestFat16Exists(&cVolume, "\\Sub Directory"));
	AssertTrue(TestFat16Exists(&cVolume, "\\FAAB"));
	AssertFalse(TestFat16Exists(&cVolume, "\\Goodbye.txt"));
	AssertInt(FAT_SUCCESS, cVolume.Unmount());

	cDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// FAT12 isn't supported: it isn't formatted (asked for, or for a drive too
// small for FAT16), and a FAT12 sized volume isn't mounted.
//
//////////////////////////////////////////////////////////////////////////
void TestFat16NoFat12(void)
{
	CMemoryDrive	cDrive;
	CFatVolume		cVolume;

	cDrive.Init((uint64)4096 * 512, 512);
	AssertInt(FAT_INVALID_FORMAT_PARAMETERS, FatFormat(FAT_FS_TYPE_FAT12, "Twelve", 1, &cDrive));
	AssertInt(FAT_INVALID_FORMAT_PARAMETERS, FatFormat(FAT_FS_TYPE_UNSPECIFIED, "Twelve", 0, &cDrive));
	AssertInt(FAT_INVALID_FORMAT_PARAMETERS, FatFormat(FAT_FS_TYPE_FAT16, "Twelve", 1, &cDrive));
	AssertInt(FAT_INVALID_FAT_VOLUME, cVolume.Mount(&cDrive));
	cVolume.Unmount();
	cDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat16(void)
{
	BeginTests();

	TestFat16Volume();
	TestFat16NoFat12();

	TestStatistics();
}
