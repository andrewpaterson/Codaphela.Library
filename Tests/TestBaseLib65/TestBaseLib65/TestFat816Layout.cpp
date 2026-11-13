#include <stdio.h>
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/Chars.h"
#include "Assembler65Lib/Asm65Memory.h"
#include "MicroprocessorLib/FastW65C816.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "TestLib/Assert.h"


//The FAT assembly (BaseLib/*816.asm) reads and writes the FAT classes' members, so it needs their W65C816 layout.
//BaseLib/Fat816.inc has it, worked out here by the W65C816 compiler itself: offsetof, sizeof and the constants the
//assembly uses are compiled into an array (filled in by the start up: offsetof isn't a constant initialiser to the
//compiler), which is run and read.  A change to the classes changes the
//generated file (Output/Fat816.inc), which then no longer matches the one in BaseLib: copy it over, and check the
//assembly that uses what moved.
#define TEST_FAT816_COMMON			".." _FS_ ".." _FS_ ".." _FS_ "Libraries"
#define TEST_FAT816_LAYOUT			TEST_FAT816_COMMON _FS_ "BaseLib" _FS_ "Fat816.inc"
#define TEST_FAT816_LIBC65			".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65"
#define TEST_FAT816_OUTPUT			"Output" _FS_ "Fat816.inc"


struct STestFat816Value
{
	const char*	szName;			//The name in Fat816.inc.
	const char*	szValue;		//A C++ expression the W65C816 compiler works out.
};


#define FAT816_MEMBER(c, m)			{ #c "_" #m, "offsetof(" #c ", " #m ")" }
#define FAT816_FIELD(n, c, m)		{ n, "offsetof(" #c ", " #m ")" }
#define FAT816_SIZE(c)				{ "SIZEOF_" #c, "sizeof(" #c ")" }
#define FAT816_CONSTANT(c)			{ #c, "(" #c ")" }


static const STestFat816Value gasTestFat816Values[] =
{
	FAT816_MEMBER(CFatVolume, muiID),
	FAT816_MEMBER(CFatVolume, muiFatSize),
	FAT816_MEMBER(CFatVolume, muiRootCluster),
	FAT816_MEMBER(CFatVolume, muiRootSector),
	FAT816_MEMBER(CFatVolume, muiFirstDataSector),
	FAT816_MEMBER(CFatVolume, muiHiddenSectors),
	FAT816_MEMBER(CFatVolume, muiNoOfSectors),
	FAT816_MEMBER(CFatVolume, muiNoOfDataSectors),
	FAT816_MEMBER(CFatVolume, muiNoOfClusters),
	FAT816_MEMBER(CFatVolume, muiNoOfReservedSectors),
	FAT816_MEMBER(CFatVolume, muiNextFreeCluster),
	FAT816_MEMBER(CFatVolume, muiTotalFreeClusters),
	FAT816_MEMBER(CFatVolume, muiFileSystemInfoSector),
	FAT816_MEMBER(CFatVolume, muiBytesPerCluster),
	FAT816_MEMBER(CFatVolume, muiRootDirectorySectors),
	FAT816_MEMBER(CFatVolume, muiBytesPerSector),
	FAT816_MEMBER(CFatVolume, muiNoOfSectorsPerCluster),
	FAT816_MEMBER(CFatVolume, muiBytesPerSectorShift),
	FAT816_MEMBER(CFatVolume, muiBytesPerClusterShift),
	FAT816_MEMBER(CFatVolume, muiSectorsPerClusterShift),
	FAT816_MEMBER(CFatVolume, mbUseLongFilenames),
	FAT816_MEMBER(CFatVolume, meFileSystem),
	FAT816_MEMBER(CFatVolume, muiNoOfFatTables),
	FAT816_MEMBER(CFatVolume, mszLabel),
	FAT816_MEMBER(CFatVolume, mpcDevice),
	FAT816_MEMBER(CFatVolume, mcSectorCache),
	FAT816_MEMBER(CFatVolume, msBPB),
	FAT816_SIZE(CFatVolume),
	FAT816_SIZE(EFatFileSystemType),
	FAT816_SIZE(EFatCode),
	FAT816_SIZE(CChars),

	FAT816_MEMBER(SFatCache, pvCachedSector),
	FAT816_MEMBER(SFatCache, uiTimeStamp),
	FAT816_SIZE(SFatCache),

	FAT816_MEMBER(CFatSectorCache, muiCreationStamp),
	FAT816_MEMBER(CFatSectorCache, muiInfoSector),
	FAT816_MEMBER(CFatSectorCache, muiLocks),
	FAT816_MEMBER(CFatSectorCache, mbDirty),
	FAT816_SIZE(CFatSectorCache),

	FAT816_MEMBER(CFatInfoSectorCache, mllcCachedSectors),
	FAT816_MEMBER(CFatInfoSectorCache, muiSectorSize),
	FAT816_MEMBER(CFatInfoSectorCache, muiMinimumUnlockedCaches),
	FAT816_MEMBER(CFatInfoSectorCache, muiCaches),
	FAT816_MEMBER(CFatInfoSectorCache, mpcDrive),
	FAT816_MEMBER(CFatInfoSectorCache, muiCreationStamp),
	FAT816_SIZE(CFatInfoSectorCache),
	FAT816_SIZE(CLinkedListBlock),
	FAT816_FIELD("CLinkedListBlock_mpsHead", CLinkedListBlock, mcList.mpsHead),
	FAT816_FIELD("CLinkedListBlock_mpsTail", CLinkedListBlock, mcList.mpsTail),
	FAT816_MEMBER(CLinkedListBlock, muiNodeSize),
	FAT816_MEMBER(SLLNode, psNext),
	FAT816_MEMBER(SLLNode, psPrev),
	FAT816_SIZE(SLLBlockNode),

	FAT816_MEMBER(SClusterCache, uiCluster),
	FAT816_MEMBER(SClusterCache, uiClusterFirstSector),
	FAT816_MEMBER(SClusterCache, pvCache),
	FAT816_MEMBER(SClusterCache, pbCachedSectors),
	FAT816_MEMBER(SClusterCache, pbDirtySectors),
	FAT816_SIZE(SClusterCache),

	FAT816_MEMBER(CFatClusterCache, msCluster),
	FAT816_MEMBER(CFatClusterCache, muiClusterSize),
	FAT816_MEMBER(CFatClusterCache, muiSectorSize),
	FAT816_MEMBER(CFatClusterCache, muiSectorsPerCluster),
	FAT816_MEMBER(CFatClusterCache, muiBytesForBits),
	FAT816_MEMBER(CFatClusterCache, muiSectorSizeShift),
	FAT816_MEMBER(CFatClusterCache, mpcDrive),
	FAT816_SIZE(CFatClusterCache),

	FAT816_MEMBER(SFatFile, sDirectoryEntry),
	FAT816_MEMBER(SFatFile, uiFileSize),
	FAT816_MEMBER(SFatFile, uiFilePosition),
	FAT816_MEMBER(SFatFile, uiFilePositionCluster),
	FAT816_MEMBER(SFatFile, bBusy),
	FAT816_MEMBER(SFatFile, uiMagic),
	FAT816_MEMBER(SFatFile, uiAccessFlags),
	FAT816_SIZE(SFatFile),

	FAT816_MEMBER(CFatFile, msFile),
	FAT816_MEMBER(CFatFile, mpcVolume),
	FAT816_MEMBER(CFatFile, mcCache),
	FAT816_MEMBER(CFatFile, mszName),
	FAT816_SIZE(CFatFile),

	FAT816_MEMBER(CFileDrive, muiMaxSector),
	FAT816_MEMBER(CFileDrive, muiSectorSize),
	FAT816_SIZE(CFileDrive),

	FAT816_MEMBER(SFatDirectoryEntry, szName),
	FAT816_MEMBER(SFatDirectoryEntry, uiAttributes),
	FAT816_MEMBER(SFatDirectoryEntry, uiSize),
	FAT816_MEMBER(SFatDirectoryEntry, uiSectorAddress),
	FAT816_MEMBER(SFatDirectoryEntry, uiSectorOffset),
	FAT816_MEMBER(SFatDirectoryEntry, sRaw),
	FAT816_SIZE(SFatDirectoryEntry),

	FAT816_FIELD("RAW_szShortName", SFatRawDirectoryEntry, uEntry.sFatRawCommon.szShortName),
	FAT816_FIELD("RAW_uiAttributes", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiAttributes),
	FAT816_FIELD("RAW_uiReserved", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiReserved),
	FAT816_FIELD("RAW_uiCreateTimeTenths", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiCreateTimeTenths),
	FAT816_FIELD("RAW_uiCreateTime", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiCreateTime),
	FAT816_FIELD("RAW_uiCreateDate", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiCreateDate),
	FAT816_FIELD("RAW_uiAccessDate", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiAccessDate),
	FAT816_FIELD("RAW_uiFirstClusterHighWord", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiFirstClusterHighWord),
	FAT816_FIELD("RAW_uiModifyTime", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiModifyTime),
	FAT816_FIELD("RAW_uiModifyDate", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiModifyDate),
	FAT816_FIELD("RAW_uiFirstClusterLowWord", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiFirstClusterLowWord),
	FAT816_FIELD("RAW_uiSize", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiSize),
	FAT816_FIELD("LFN_uiSequence", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiSequence),
	FAT816_FIELD("LFN_auiChars1", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.auiChars1),
	FAT816_FIELD("LFN_uiAttributes", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiAttributes),
	FAT816_FIELD("LFN_uiType", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiType),
	FAT816_FIELD("LFN_uiChecksum", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiChecksum),
	FAT816_FIELD("LFN_auiChars2", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.auiChars2),
	FAT816_FIELD("LFN_uiFirstCluster", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiFirstCluster),
	FAT816_FIELD("LFN_auiChars3", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.auiChars3),
	FAT816_SIZE(SFatRawDirectoryEntry),

	FAT816_MEMBER(SFatFileSystemInfo, uiTrailSignature),
	FAT816_MEMBER(SFatFileSystemInfo, uiNextFreeCluster),
	FAT816_MEMBER(SFatFileSystemInfo, uiNumFreeClusters),
	FAT816_MEMBER(SFatFileSystemInfo, uiStructSignature),
	FAT816_MEMBER(SFatFileSystemInfo, uiLeadSignature),
	FAT816_SIZE(SFatFileSystemInfo),

	FAT816_MEMBER(SFatPartitionEntry, uiStatus),
	FAT816_MEMBER(SFatPartitionEntry, uiPartitionType),
	FAT816_MEMBER(SFatPartitionEntry, uiLBAFirstSector),
	FAT816_MEMBER(SFatPartitionEntry, uiTotalSectors),
	FAT816_SIZE(SFatPartitionEntry),

	FAT816_FIELD("BPB_uiBytsPerSector", SFatBIOSParameterBlock, uiBytsPerSector),
	FAT816_FIELD("BPB_uiSectorsPerCluster", SFatBIOSParameterBlock, uiSectorsPerCluster),
	FAT816_FIELD("BPB_uiReservedSectorCount", SFatBIOSParameterBlock, uiReservedSectorCount),
	FAT816_FIELD("BPB_uiNumFileAllocationTables", SFatBIOSParameterBlock, uiNumFileAllocationTables),
	FAT816_FIELD("BPB_uiRootEntryCount", SFatBIOSParameterBlock, uiRootEntryCount),
	FAT816_FIELD("BPB_uiTotalSectorsFat16", SFatBIOSParameterBlock, uiTotalSectorsFat16),
	FAT816_FIELD("BPB_uiMediaType", SFatBIOSParameterBlock, uiMediaType),
	FAT816_FIELD("BPB_uiFATSzFat16", SFatBIOSParameterBlock, uiFATSzFat16),
	FAT816_FIELD("BPB_uiNumHiddenSectors", SFatBIOSParameterBlock, uiNumHiddenSectors),
	FAT816_FIELD("BPB_uiTotalSectorsFat32", SFatBIOSParameterBlock, uiTotalSectorsFat32),
	FAT816_FIELD("BPB16_uiVolumeID", SFatBIOSParameterBlock, uFatEx.sFat16.uiVolumeID),
	FAT816_FIELD("BPB16_szVolumeLabel", SFatBIOSParameterBlock, uFatEx.sFat16.szVolumeLabel),
	FAT816_FIELD("BPB32_uiFATSzFat32", SFatBIOSParameterBlock, uFatEx.sFat32.uiFATSzFat32),
	FAT816_FIELD("BPB32_uiRootCluster", SFatBIOSParameterBlock, uFatEx.sFat32.uiRootCluster),
	FAT816_FIELD("BPB32_uiFileSystemInformation", SFatBIOSParameterBlock, uFatEx.sFat32.uiFileSystemInformation),
	FAT816_FIELD("BPB32_uiVolumeID", SFatBIOSParameterBlock, uFatEx.sFat32.uiVolumeID),
	FAT816_FIELD("BPB32_szVolumeLabel", SFatBIOSParameterBlock, uFatEx.sFat32.szVolumeLabel),
	FAT816_SIZE(SFatBIOSParameterBlock),

	FAT816_MEMBER(SFatQueryState, uiAttributes),
	FAT816_MEMBER(SFatQueryState, uiCurrentSector),
	FAT816_MEMBER(SFatQueryState, uiCurrentCluster),
	FAT816_MEMBER(SFatQueryState, psCurrentEntryRaw),
	FAT816_MEMBER(SFatQueryState, psFirstEntryRaw),
	FAT816_MEMBER(SFatQueryState, auiLongFilename),
	FAT816_MEMBER(SFatQueryState, uiSequence),
	FAT816_MEMBER(SFatQueryState, uiChecksum),
	FAT816_MEMBER(SFatQueryState, sBuffer),
	FAT816_MEMBER(SFatQueryState, uiLockedSectors),
	FAT816_MEMBER(SFatQueryState, sEndEntry),
	FAT816_SIZE(SFatQueryState),

	FAT816_MEMBER(SFatFileSystemQuery, sCurrentEntry),
	FAT816_MEMBER(SFatFileSystemQuery, sQuery),
	FAT816_SIZE(SFatFileSystemQuery),

	FAT816_CONSTANT(FAT_MAX_PATH),
	FAT816_CONSTANT(FAT_MAX_FILENAME),
	FAT816_CONSTANT(FAT_MAX_LFN_ENTRIES),
	FAT816_CONSTANT(FAT_MAX_SECTOR_QUERY_LOCKS),
	FAT816_CONSTANT(FAT_FIRST_LFN_ENTRY),
	FAT816_CONSTANT(FAT_MAX_DIRECTORY_ENTRIES),
	FAT816_CONSTANT(FAT_LONG_NAME_END_WCHAR),
	FAT816_CONSTANT(MAX_SECTOR_LENGTH),
	FAT816_CONSTANT(NO_SECTOR_CACHED),
	FAT816_CONSTANT(NO_CLUSTER_CACHED),

	FAT816_CONSTANT(FAT_ATTR_READ_ONLY),
	FAT816_CONSTANT(FAT_ATTR_HIDDEN),
	FAT816_CONSTANT(FAT_ATTR_SYSTEM),
	FAT816_CONSTANT(FAT_ATTR_VOLUME_ID),
	FAT816_CONSTANT(FAT_ATTR_DIRECTORY),
	FAT816_CONSTANT(FAT_ATTR_ARCHIVE),
	FAT816_CONSTANT(FAT_ATTR_LONG_NAME),
	FAT816_CONSTANT(FAT_FILE_ACCESS_READ),
	FAT816_CONSTANT(FAT_FILE_ACCESS_WRITE),
	FAT816_CONSTANT(FAT_FILE_ACCESS_APPEND),
	FAT816_CONSTANT(FAT_FILE_ACCESS_OVERWRITE),
	FAT816_CONSTANT(FAT_FILE_ACCESS_CREATE),
	FAT816_CONSTANT(FAT_SEEK_START),
	FAT816_CONSTANT(FAT_SEEK_CURRENT),
	FAT816_CONSTANT(FAT_SEEK_END),
	FAT816_CONSTANT(FAT_LOWERCASE_EXTENSION),
	FAT816_CONSTANT(FAT_LOWERCASE_BASENAME),

	FAT816_CONSTANT(FAT_SUCCESS),
	FAT816_CONSTANT(FAT_UNKNOWN_ERROR),
	FAT816_CONSTANT(FAT_CANNOT_READ_MEDIA),
	FAT816_CONSTANT(FAT_CANNOT_WRITE_MEDIA),
	FAT816_CONSTANT(FAT_NOT_A_DIRECTORY),
	FAT816_CONSTANT(FAT_INVALID_FILENAME),
	FAT816_CONSTANT(FAT_FILENAME_ALREADY_EXISTS),
	FAT816_CONSTANT(FAT_INVALID_PATH),
	FAT816_CONSTANT(FAT_CORRUPTED_FILE),
	FAT816_CONSTANT(FAT_ILLEGAL_FILENAME),
	FAT816_CONSTANT(FAT_FILENAME_TOO_LONG),
	FAT816_CONSTANT(FAT_NOT_A_FILE),
	FAT816_CONSTANT(FAT_FILE_NOT_FOUND),
	FAT816_CONSTANT(FAT_DIRECTORY_DOES_NOT_EXIST),
	FAT816_CONSTANT(FAT_INSUFFICIENT_DISK_SPACE),
	FAT816_CONSTANT(FAT_FEATURE_NOT_SUPPORTED),
	FAT816_CONSTANT(FAT_OP_IN_PROGRESS),
	FAT816_CONSTANT(FAT_SECTOR_SIZE_NOT_SUPPORTED),
	FAT816_CONSTANT(FAT_LFN_GENERATED),
	FAT816_CONSTANT(FAT_SHORT_LFN_GENERATED),
	FAT816_CONSTANT(FAT_SEEK_FAILED),
	FAT816_CONSTANT(FAT_FILE_NOT_OPENED_FOR_WRITE_ACCESS),
	FAT816_CONSTANT(FAT_INVALID_HANDLE),
	FAT816_CONSTANT(FAT_INVALID_CLUSTER),
	FAT816_CONSTANT(FAT_INVALID_FAT_VOLUME),
	FAT816_CONSTANT(FAT_INVALID_VOLUME_LABEL),
	FAT816_CONSTANT(FAT_INVALID_FORMAT_PARAMETERS),
	FAT816_CONSTANT(FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED),
	FAT816_CONSTANT(FAT_DIRECTORY_LIMIT_EXCEEDED),
	FAT816_CONSTANT(FAT_INVALID_PARAMETERS),
	FAT816_CONSTANT(FAT_FILE_HANDLE_IN_USE),
	FAT816_CONSTANT(FAT_INVALID_FAT_CACHE),
	FAT816_CONSTANT(FAT_NO_CACHE_LOCK),
	FAT816_CONSTANT(FAT_SHORT_NAME_FOUND),
	FAT816_CONSTANT(FAT_NOT_ALL_CLUSTERS_ALLOCATED),

	FAT816_CONSTANT(FAT_FS_TYPE_UNSPECIFIED),
	FAT816_CONSTANT(FAT_FS_TYPE_FAT12),
	FAT816_CONSTANT(FAT_FS_TYPE_FAT16),
	FAT816_CONSTANT(FAT_FS_TYPE_FAT32),
	FAT816_CONSTANT(FAT16_EOC),
	FAT816_CONSTANT(FAT32_EOC),
	FAT816_CONSTANT(FAT16_BAD_CLUSTER),
	FAT816_CONSTANT(FAT32_BAD_CLUSTER),
	FAT816_CONSTANT(FAT16_CLUSTER_MASK),
	FAT816_CONSTANT(FAT32_CLUSTER_MASK),
	FAT816_CONSTANT(FAT12_MAX_CLUSTERS),
	FAT816_CONSTANT(FAT16_MAX_CLUSTERS),
	FAT816_CONSTANT(FAT_OPEN_HANDLE_MAGIC),
	FAT816_CONSTANT(FAT_KANJI_SKIP_ENTRY),
	FAT816_CONSTANT(FAT_DELETED_ENTRY),
	{ NULL, NULL }
};


//////////////////////////////////////////////////////////////////////////
//
// The W65C816 source that works the values out: an array of longs.
//
//////////////////////////////////////////////////////////////////////////
static void TestFat816LayoutSource(CChars* pszSource)
{
	size	i;

	pszSource->Init();
	pszSource->Append("#include <stddef.h>\n");
	pszSource->Append("#include \"BaseLib/FatVolume.h\"\n");
	pszSource->Append("#include \"BaseLib/FatFile.h\"\n");
	pszSource->Append("extern \"C\" const unsigned long gauiFat816Layout[] =\n{\n");
	for (i = 0; gasTestFat816Values[i].szName; i++)
	{
		pszSource->Append("\t(unsigned long)")->Append(gasTestFat816Values[i].szValue)->Append(",\n");
	}
	pszSource->Append("};\nint main(void)\n{\n\treturn (int)gauiFat816Layout[0];\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Fat816.inc's text from the values in memory at uiAddress.
//
//////////////////////////////////////////////////////////////////////////
static void TestFat816LayoutText(CAsm65Memory* pcMemory, uint32 uiAddress, CChars* pszText)
{
	size	i;
	uint32	uiValue;
	char	szHex[16];

	pszText->Init();
	pszText->Append("; The W65C816 layout of BaseLib's FAT classes and the FAT constants, for the FAT assembly (*816.asm).\n");
	pszText->Append("; Generated by TestBaseLib65 (TestFat816Layout.cpp) with the W65C816 compiler: don't edit it, copy\n");
	pszText->Append("; TestBaseLib65's Output/Fat816.inc over it when the classes change.\n\n");
	for (i = 0; gasTestFat816Values[i].szName; i++)
	{
		uiValue = (uint32)pcMemory->GetByte(uiAddress + (uint32)i * 4) | ((uint32)pcMemory->GetByte(uiAddress + (uint32)i * 4 + 1) << 8) |
				  ((uint32)pcMemory->GetByte(uiAddress + (uint32)i * 4 + 2) << 16) | ((uint32)pcMemory->GetByte(uiAddress + (uint32)i * 4 + 3) << 24);
		sprintf(szHex, "0x%X", uiValue);
		pszText->Append(gasTestFat816Values[i].szName)->Append(" = ")->Append(szHex)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat816Layout(void)
{
	CCPP65Builder	cBuilder;
	CAsm65Memory	cMemory;
	CFastW65C816		cMPU;
	CFileUtil		cFileUtil;
	CTextFile		cExpected;
	CChars			szSource;
	CChars			szText;
	CChars			szCommon;
	CChars			szLibc65;
	int64			lliAddress;
	FILE*			pcFile;
	bool			bLinked;

	BeginTests();

	szCommon.Init(TEST_FAT816_COMMON);
	cFileUtil.FullPath(&szCommon);
	szLibc65.Init(TEST_FAT816_LIBC65);
	cFileUtil.FullPath(&szLibc65);

	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.AddIncludeDirectory(szLibc65.Text(), false, true);
	cBuilder.AddIncludeDirectory(szCommon.Text(), true, false);
	AssertTrue(cBuilder.AddStartUp());
	TestFat816LayoutSource(&szSource);
	bLinked = cBuilder.LinkWith("Fat816Layout.cpp", szSource.Text());
	AssertString("", cBuilder.GetErrors());
	AssertTrue(bLinked);
	AssertTrue(cBuilder.GetSymbol("_gauiFat816Layout", &lliAddress));
	if (bLinked)
	{
		cMemory.Init(0x1000000);
		cMemory.Load(cBuilder.GetOutput());
		cMemory.InitFast65816(&cMPU);
		cMPU.Reset();
		cMPU.Run(10000000);
		AssertTrue(cMPU.IsStopped());
		TestFat816LayoutText(&cMemory, (uint32)lliAddress, &szText);
		cMPU.Kill();
		cMemory.Kill();

		cFileUtil.MakeDir("Output");
		pcFile = fopen(TEST_FAT816_OUTPUT, "wb");
		if (pcFile)
		{
			fwrite(szText.Text(), 1, szText.Length(), pcFile);
			fclose(pcFile);
		}

		cExpected.Init();
		AssertTrue(cExpected.Read(TEST_FAT816_LAYOUT));
		cExpected.PassifyNewlines();
		AssertString(cExpected.Text(), szText.Text());
		cExpected.Kill();
		szText.Kill();
	}
	szSource.Kill();
	cBuilder.Kill();
	szLibc65.Kill();
	szCommon.Kill();

	TestStatistics();
}
