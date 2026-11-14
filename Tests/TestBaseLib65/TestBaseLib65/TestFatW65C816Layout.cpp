#include <stdio.h>
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/Chars.h"
#include "Assembler65Lib/Asm65Memory.h"
#include "MicroprocessorLib/FastW65C816.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestTarget65.h"
#include "TestProject65.h"


//The FAT assembly (BaseLib/Fat*W65C816.asm) reads and writes the FAT classes' members, so it needs their W65C816
//layout.  BaseLib/FatW65C816.inc has it, worked out here by the W65C816 compiler itself: offsetof, sizeof and the
//constants the assembly uses are compiled into an array (filled in by the start up: offsetof isn't a constant
//initialiser to the compiler), which is run and read.  A change to the classes changes the generated file
//(Output/<target>/FatW65C816.inc), which then no longer matches the one in BaseLib: copy it over, and check the
//assembly that uses what moved.
#define TEST_FAT_W65C816_COMMON		".." _FS_ ".." _FS_ ".." _FS_ "Libraries"
#define TEST_FAT_W65C816_LAYOUT		TEST_FAT_W65C816_COMMON _FS_ "BaseLib" _FS_ "FatW65C816.inc"
#define TEST_FAT_W65C816_LIBC65		".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65"


struct STestFatW65C816Value
{
	const char*	szName;			//The name in FatW65C816.inc.
	const char*	szValue;		//A C++ expression the W65C816 compiler works out.
};


#define FAT_W65C816_MEMBER(c, m)	{ #c "_" #m, "offsetof(" #c ", " #m ")" }
#define FAT_W65C816_FIELD(n, c, m)	{ n, "offsetof(" #c ", " #m ")" }
#define FAT_W65C816_SIZE(c)			{ "SIZEOF_" #c, "sizeof(" #c ")" }
#define FAT_W65C816_CONSTANT(c)		{ #c, "(" #c ")" }


static const STestFatW65C816Value gasTestFatW65C816Values[] =
{
	FAT_W65C816_MEMBER(CFatVolume, muiID),
	FAT_W65C816_MEMBER(CFatVolume, muiFatSize),
	FAT_W65C816_MEMBER(CFatVolume, muiRootCluster),
	FAT_W65C816_MEMBER(CFatVolume, muiRootSector),
	FAT_W65C816_MEMBER(CFatVolume, muiFirstDataSector),
	FAT_W65C816_MEMBER(CFatVolume, muiHiddenSectors),
	FAT_W65C816_MEMBER(CFatVolume, muiNoOfSectors),
	FAT_W65C816_MEMBER(CFatVolume, muiNoOfDataSectors),
	FAT_W65C816_MEMBER(CFatVolume, muiNoOfClusters),
	FAT_W65C816_MEMBER(CFatVolume, muiNoOfReservedSectors),
	FAT_W65C816_MEMBER(CFatVolume, muiNextFreeCluster),
	FAT_W65C816_MEMBER(CFatVolume, muiTotalFreeClusters),
	FAT_W65C816_MEMBER(CFatVolume, muiFileSystemInfoSector),
	FAT_W65C816_MEMBER(CFatVolume, muiBytesPerCluster),
	FAT_W65C816_MEMBER(CFatVolume, muiRootDirectorySectors),
	FAT_W65C816_MEMBER(CFatVolume, muiBytesPerSector),
	FAT_W65C816_MEMBER(CFatVolume, muiNoOfSectorsPerCluster),
	FAT_W65C816_MEMBER(CFatVolume, muiBytesPerSectorShift),
	FAT_W65C816_MEMBER(CFatVolume, muiBytesPerClusterShift),
	FAT_W65C816_MEMBER(CFatVolume, muiSectorsPerClusterShift),
	FAT_W65C816_MEMBER(CFatVolume, mbUseLongFilenames),
	FAT_W65C816_MEMBER(CFatVolume, meFileSystem),
	FAT_W65C816_MEMBER(CFatVolume, muiNoOfFatTables),
	FAT_W65C816_MEMBER(CFatVolume, mszLabel),
	FAT_W65C816_MEMBER(CFatVolume, mpcDevice),
	FAT_W65C816_MEMBER(CFatVolume, mcSectorCache),
	FAT_W65C816_MEMBER(CFatVolume, msBPB),
	FAT_W65C816_SIZE(CFatVolume),
	FAT_W65C816_SIZE(EFatFileSystemType),
	FAT_W65C816_SIZE(EFatCode),
	FAT_W65C816_SIZE(CChars),

	FAT_W65C816_MEMBER(SFatCache, pvCachedSector),
	FAT_W65C816_MEMBER(SFatCache, uiTimeStamp),
	FAT_W65C816_SIZE(SFatCache),

	FAT_W65C816_MEMBER(CFatSectorCache, muiCreationStamp),
	FAT_W65C816_MEMBER(CFatSectorCache, muiInfoSector),
	FAT_W65C816_MEMBER(CFatSectorCache, muiLocks),
	FAT_W65C816_MEMBER(CFatSectorCache, mbDirty),
	FAT_W65C816_SIZE(CFatSectorCache),

	FAT_W65C816_MEMBER(CFatInfoSectorCache, mllcCachedSectors),
	FAT_W65C816_MEMBER(CFatInfoSectorCache, muiSectorSize),
	FAT_W65C816_MEMBER(CFatInfoSectorCache, muiMinimumUnlockedCaches),
	FAT_W65C816_MEMBER(CFatInfoSectorCache, muiCaches),
	FAT_W65C816_MEMBER(CFatInfoSectorCache, mpcDrive),
	FAT_W65C816_MEMBER(CFatInfoSectorCache, muiCreationStamp),
	FAT_W65C816_SIZE(CFatInfoSectorCache),
	FAT_W65C816_SIZE(CLinkedListBlock),
	FAT_W65C816_FIELD("CLinkedListBlock_mpsHead", CLinkedListBlock, mcList.mpsHead),
	FAT_W65C816_FIELD("CLinkedListBlock_mpsTail", CLinkedListBlock, mcList.mpsTail),
	FAT_W65C816_MEMBER(CLinkedListBlock, muiNodeSize),
	FAT_W65C816_MEMBER(SLLNode, psNext),
	FAT_W65C816_MEMBER(SLLNode, psPrev),
	FAT_W65C816_SIZE(SLLBlockNode),

	FAT_W65C816_MEMBER(SClusterCache, uiCluster),
	FAT_W65C816_MEMBER(SClusterCache, uiClusterFirstSector),
	FAT_W65C816_MEMBER(SClusterCache, pvCache),
	FAT_W65C816_MEMBER(SClusterCache, pbCachedSectors),
	FAT_W65C816_MEMBER(SClusterCache, pbDirtySectors),
	FAT_W65C816_SIZE(SClusterCache),

	FAT_W65C816_MEMBER(CFatClusterCache, msCluster),
	FAT_W65C816_MEMBER(CFatClusterCache, muiClusterSize),
	FAT_W65C816_MEMBER(CFatClusterCache, muiSectorSize),
	FAT_W65C816_MEMBER(CFatClusterCache, muiSectorsPerCluster),
	FAT_W65C816_MEMBER(CFatClusterCache, muiBytesForBits),
	FAT_W65C816_MEMBER(CFatClusterCache, muiSectorSizeShift),
	FAT_W65C816_MEMBER(CFatClusterCache, mpcDrive),
	FAT_W65C816_SIZE(CFatClusterCache),

	FAT_W65C816_MEMBER(SFatFile, sDirectoryEntry),
	FAT_W65C816_MEMBER(SFatFile, uiFileSize),
	FAT_W65C816_MEMBER(SFatFile, uiFilePosition),
	FAT_W65C816_MEMBER(SFatFile, uiFilePositionCluster),
	FAT_W65C816_MEMBER(SFatFile, bBusy),
	FAT_W65C816_MEMBER(SFatFile, uiMagic),
	FAT_W65C816_MEMBER(SFatFile, uiAccessFlags),
	FAT_W65C816_SIZE(SFatFile),

	FAT_W65C816_MEMBER(CFatFile, msFile),
	FAT_W65C816_MEMBER(CFatFile, mpcVolume),
	FAT_W65C816_MEMBER(CFatFile, mcCache),
	FAT_W65C816_MEMBER(CFatFile, mszName),
	FAT_W65C816_SIZE(CFatFile),

	FAT_W65C816_MEMBER(CFileDrive, muiMaxSector),
	FAT_W65C816_MEMBER(CFileDrive, muiSectorSize),
	FAT_W65C816_SIZE(CFileDrive),

	FAT_W65C816_MEMBER(SFatDirectoryEntry, szName),
	FAT_W65C816_MEMBER(SFatDirectoryEntry, uiAttributes),
	FAT_W65C816_MEMBER(SFatDirectoryEntry, uiSize),
	FAT_W65C816_MEMBER(SFatDirectoryEntry, uiSectorAddress),
	FAT_W65C816_MEMBER(SFatDirectoryEntry, uiSectorOffset),
	FAT_W65C816_MEMBER(SFatDirectoryEntry, sRaw),
	FAT_W65C816_SIZE(SFatDirectoryEntry),

	FAT_W65C816_FIELD("RAW_szShortName", SFatRawDirectoryEntry, uEntry.sFatRawCommon.szShortName),
	FAT_W65C816_FIELD("RAW_uiAttributes", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiAttributes),
	FAT_W65C816_FIELD("RAW_uiReserved", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiReserved),
	FAT_W65C816_FIELD("RAW_uiCreateTimeTenths", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiCreateTimeTenths),
	FAT_W65C816_FIELD("RAW_uiCreateTime", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiCreateTime),
	FAT_W65C816_FIELD("RAW_uiCreateDate", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiCreateDate),
	FAT_W65C816_FIELD("RAW_uiAccessDate", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiAccessDate),
	FAT_W65C816_FIELD("RAW_uiFirstClusterHighWord", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiFirstClusterHighWord),
	FAT_W65C816_FIELD("RAW_uiModifyTime", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiModifyTime),
	FAT_W65C816_FIELD("RAW_uiModifyDate", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiModifyDate),
	FAT_W65C816_FIELD("RAW_uiFirstClusterLowWord", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiFirstClusterLowWord),
	FAT_W65C816_FIELD("RAW_uiSize", SFatRawDirectoryEntry, uEntry.sFatRawCommon.uiSize),
	FAT_W65C816_FIELD("LFN_uiSequence", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiSequence),
	FAT_W65C816_FIELD("LFN_auiChars1", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.auiChars1),
	FAT_W65C816_FIELD("LFN_uiAttributes", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiAttributes),
	FAT_W65C816_FIELD("LFN_uiType", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiType),
	FAT_W65C816_FIELD("LFN_uiChecksum", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiChecksum),
	FAT_W65C816_FIELD("LFN_auiChars2", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.auiChars2),
	FAT_W65C816_FIELD("LFN_uiFirstCluster", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.uiFirstCluster),
	FAT_W65C816_FIELD("LFN_auiChars3", SFatRawDirectoryEntry, uEntry.sFatRawLongFileName.auiChars3),
	FAT_W65C816_SIZE(SFatRawDirectoryEntry),

	FAT_W65C816_MEMBER(SFatFileSystemInfo, uiTrailSignature),
	FAT_W65C816_MEMBER(SFatFileSystemInfo, uiNextFreeCluster),
	FAT_W65C816_MEMBER(SFatFileSystemInfo, uiNumFreeClusters),
	FAT_W65C816_MEMBER(SFatFileSystemInfo, uiStructSignature),
	FAT_W65C816_MEMBER(SFatFileSystemInfo, uiLeadSignature),
	FAT_W65C816_SIZE(SFatFileSystemInfo),

	FAT_W65C816_MEMBER(SFatPartitionEntry, uiStatus),
	FAT_W65C816_MEMBER(SFatPartitionEntry, uiPartitionType),
	FAT_W65C816_MEMBER(SFatPartitionEntry, uiLBAFirstSector),
	FAT_W65C816_MEMBER(SFatPartitionEntry, uiTotalSectors),
	FAT_W65C816_SIZE(SFatPartitionEntry),

	FAT_W65C816_FIELD("BPB_uiBytsPerSector", SFatBIOSParameterBlock, uiBytsPerSector),
	FAT_W65C816_FIELD("BPB_uiSectorsPerCluster", SFatBIOSParameterBlock, uiSectorsPerCluster),
	FAT_W65C816_FIELD("BPB_uiReservedSectorCount", SFatBIOSParameterBlock, uiReservedSectorCount),
	FAT_W65C816_FIELD("BPB_uiNumFileAllocationTables", SFatBIOSParameterBlock, uiNumFileAllocationTables),
	FAT_W65C816_FIELD("BPB_uiRootEntryCount", SFatBIOSParameterBlock, uiRootEntryCount),
	FAT_W65C816_FIELD("BPB_uiTotalSectorsFat16", SFatBIOSParameterBlock, uiTotalSectorsFat16),
	FAT_W65C816_FIELD("BPB_uiMediaType", SFatBIOSParameterBlock, uiMediaType),
	FAT_W65C816_FIELD("BPB_uiFATSzFat16", SFatBIOSParameterBlock, uiFATSzFat16),
	FAT_W65C816_FIELD("BPB_uiNumHiddenSectors", SFatBIOSParameterBlock, uiNumHiddenSectors),
	FAT_W65C816_FIELD("BPB_uiTotalSectorsFat32", SFatBIOSParameterBlock, uiTotalSectorsFat32),
	FAT_W65C816_FIELD("BPB16_uiVolumeID", SFatBIOSParameterBlock, uFatEx.sFat16.uiVolumeID),
	FAT_W65C816_FIELD("BPB16_szVolumeLabel", SFatBIOSParameterBlock, uFatEx.sFat16.szVolumeLabel),
	FAT_W65C816_FIELD("BPB32_uiFATSzFat32", SFatBIOSParameterBlock, uFatEx.sFat32.uiFATSzFat32),
	FAT_W65C816_FIELD("BPB32_uiRootCluster", SFatBIOSParameterBlock, uFatEx.sFat32.uiRootCluster),
	FAT_W65C816_FIELD("BPB32_uiFileSystemInformation", SFatBIOSParameterBlock, uFatEx.sFat32.uiFileSystemInformation),
	FAT_W65C816_FIELD("BPB32_uiVolumeID", SFatBIOSParameterBlock, uFatEx.sFat32.uiVolumeID),
	FAT_W65C816_FIELD("BPB32_szVolumeLabel", SFatBIOSParameterBlock, uFatEx.sFat32.szVolumeLabel),
	FAT_W65C816_SIZE(SFatBIOSParameterBlock),

	FAT_W65C816_MEMBER(SFatQueryState, uiAttributes),
	FAT_W65C816_MEMBER(SFatQueryState, uiCurrentSector),
	FAT_W65C816_MEMBER(SFatQueryState, uiCurrentCluster),
	FAT_W65C816_MEMBER(SFatQueryState, psCurrentEntryRaw),
	FAT_W65C816_MEMBER(SFatQueryState, psFirstEntryRaw),
	FAT_W65C816_MEMBER(SFatQueryState, auiLongFilename),
	FAT_W65C816_MEMBER(SFatQueryState, uiSequence),
	FAT_W65C816_MEMBER(SFatQueryState, uiChecksum),
	FAT_W65C816_MEMBER(SFatQueryState, sBuffer),
	FAT_W65C816_MEMBER(SFatQueryState, uiLockedSectors),
	FAT_W65C816_MEMBER(SFatQueryState, sEndEntry),
	FAT_W65C816_SIZE(SFatQueryState),

	FAT_W65C816_MEMBER(SFatFileSystemQuery, sCurrentEntry),
	FAT_W65C816_MEMBER(SFatFileSystemQuery, sQuery),
	FAT_W65C816_SIZE(SFatFileSystemQuery),

	FAT_W65C816_CONSTANT(FAT_MAX_PATH),
	FAT_W65C816_CONSTANT(FAT_MAX_FILENAME),
	FAT_W65C816_CONSTANT(FAT_MAX_LFN_ENTRIES),
	FAT_W65C816_CONSTANT(FAT_MAX_SECTOR_QUERY_LOCKS),
	FAT_W65C816_CONSTANT(FAT_FIRST_LFN_ENTRY),
	FAT_W65C816_CONSTANT(FAT_MAX_DIRECTORY_ENTRIES),
	FAT_W65C816_CONSTANT(FAT_LONG_NAME_END_WCHAR),
	FAT_W65C816_CONSTANT(MAX_SECTOR_LENGTH),
	FAT_W65C816_CONSTANT(NO_SECTOR_CACHED),
	FAT_W65C816_CONSTANT(NO_CLUSTER_CACHED),

	FAT_W65C816_CONSTANT(FAT_ATTR_READ_ONLY),
	FAT_W65C816_CONSTANT(FAT_ATTR_HIDDEN),
	FAT_W65C816_CONSTANT(FAT_ATTR_SYSTEM),
	FAT_W65C816_CONSTANT(FAT_ATTR_VOLUME_ID),
	FAT_W65C816_CONSTANT(FAT_ATTR_DIRECTORY),
	FAT_W65C816_CONSTANT(FAT_ATTR_ARCHIVE),
	FAT_W65C816_CONSTANT(FAT_ATTR_LONG_NAME),
	FAT_W65C816_CONSTANT(FAT_FILE_ACCESS_READ),
	FAT_W65C816_CONSTANT(FAT_FILE_ACCESS_WRITE),
	FAT_W65C816_CONSTANT(FAT_FILE_ACCESS_APPEND),
	FAT_W65C816_CONSTANT(FAT_FILE_ACCESS_OVERWRITE),
	FAT_W65C816_CONSTANT(FAT_FILE_ACCESS_CREATE),
	FAT_W65C816_CONSTANT(FAT_SEEK_START),
	FAT_W65C816_CONSTANT(FAT_SEEK_CURRENT),
	FAT_W65C816_CONSTANT(FAT_SEEK_END),
	FAT_W65C816_CONSTANT(FAT_LOWERCASE_EXTENSION),
	FAT_W65C816_CONSTANT(FAT_LOWERCASE_BASENAME),

	FAT_W65C816_CONSTANT(FAT_SUCCESS),
	FAT_W65C816_CONSTANT(FAT_UNKNOWN_ERROR),
	FAT_W65C816_CONSTANT(FAT_CANNOT_READ_MEDIA),
	FAT_W65C816_CONSTANT(FAT_CANNOT_WRITE_MEDIA),
	FAT_W65C816_CONSTANT(FAT_NOT_A_DIRECTORY),
	FAT_W65C816_CONSTANT(FAT_INVALID_FILENAME),
	FAT_W65C816_CONSTANT(FAT_FILENAME_ALREADY_EXISTS),
	FAT_W65C816_CONSTANT(FAT_INVALID_PATH),
	FAT_W65C816_CONSTANT(FAT_CORRUPTED_FILE),
	FAT_W65C816_CONSTANT(FAT_ILLEGAL_FILENAME),
	FAT_W65C816_CONSTANT(FAT_FILENAME_TOO_LONG),
	FAT_W65C816_CONSTANT(FAT_NOT_A_FILE),
	FAT_W65C816_CONSTANT(FAT_FILE_NOT_FOUND),
	FAT_W65C816_CONSTANT(FAT_DIRECTORY_DOES_NOT_EXIST),
	FAT_W65C816_CONSTANT(FAT_INSUFFICIENT_DISK_SPACE),
	FAT_W65C816_CONSTANT(FAT_FEATURE_NOT_SUPPORTED),
	FAT_W65C816_CONSTANT(FAT_OP_IN_PROGRESS),
	FAT_W65C816_CONSTANT(FAT_SECTOR_SIZE_NOT_SUPPORTED),
	FAT_W65C816_CONSTANT(FAT_LFN_GENERATED),
	FAT_W65C816_CONSTANT(FAT_SHORT_LFN_GENERATED),
	FAT_W65C816_CONSTANT(FAT_SEEK_FAILED),
	FAT_W65C816_CONSTANT(FAT_FILE_NOT_OPENED_FOR_WRITE_ACCESS),
	FAT_W65C816_CONSTANT(FAT_INVALID_HANDLE),
	FAT_W65C816_CONSTANT(FAT_INVALID_CLUSTER),
	FAT_W65C816_CONSTANT(FAT_INVALID_FAT_VOLUME),
	FAT_W65C816_CONSTANT(FAT_INVALID_VOLUME_LABEL),
	FAT_W65C816_CONSTANT(FAT_INVALID_FORMAT_PARAMETERS),
	FAT_W65C816_CONSTANT(FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED),
	FAT_W65C816_CONSTANT(FAT_DIRECTORY_LIMIT_EXCEEDED),
	FAT_W65C816_CONSTANT(FAT_INVALID_PARAMETERS),
	FAT_W65C816_CONSTANT(FAT_FILE_HANDLE_IN_USE),
	FAT_W65C816_CONSTANT(FAT_INVALID_FAT_CACHE),
	FAT_W65C816_CONSTANT(FAT_NO_CACHE_LOCK),
	FAT_W65C816_CONSTANT(FAT_SHORT_NAME_FOUND),
	FAT_W65C816_CONSTANT(FAT_NOT_ALL_CLUSTERS_ALLOCATED),

	FAT_W65C816_CONSTANT(FAT_FS_TYPE_UNSPECIFIED),
	FAT_W65C816_CONSTANT(FAT_FS_TYPE_FAT12),
	FAT_W65C816_CONSTANT(FAT_FS_TYPE_FAT16),
	FAT_W65C816_CONSTANT(FAT_FS_TYPE_FAT32),
	FAT_W65C816_CONSTANT(FAT16_EOC),
	FAT_W65C816_CONSTANT(FAT32_EOC),
	FAT_W65C816_CONSTANT(FAT16_BAD_CLUSTER),
	FAT_W65C816_CONSTANT(FAT32_BAD_CLUSTER),
	FAT_W65C816_CONSTANT(FAT16_CLUSTER_MASK),
	FAT_W65C816_CONSTANT(FAT32_CLUSTER_MASK),
	FAT_W65C816_CONSTANT(FAT12_MAX_CLUSTERS),
	FAT_W65C816_CONSTANT(FAT16_MAX_CLUSTERS),
	FAT_W65C816_CONSTANT(FAT_OPEN_HANDLE_MAGIC),
	FAT_W65C816_CONSTANT(FAT_KANJI_SKIP_ENTRY),
	FAT_W65C816_CONSTANT(FAT_DELETED_ENTRY),
	{ NULL, NULL }
};


//////////////////////////////////////////////////////////////////////////
//
// The W65C816 source that works the values out: an array of longs.
//
//////////////////////////////////////////////////////////////////////////
static void TestFatW65C816LayoutSource(CChars* pszSource)
{
	size	i;

	pszSource->Init();
	pszSource->Append("#include <stddef.h>\n");
	pszSource->Append("#include \"BaseLib/FatVolume.h\"\n");
	pszSource->Append("#include \"BaseLib/FatFile.h\"\n");
	pszSource->Append("extern \"C\" const unsigned long gauiFatW65C816Layout[] =\n{\n");
	for (i = 0; gasTestFatW65C816Values[i].szName; i++)
	{
		pszSource->Append("\t(unsigned long)")->Append(gasTestFatW65C816Values[i].szValue)->Append(",\n");
	}
	pszSource->Append("};\nint main(void)\n{\n\treturn (int)gauiFatW65C816Layout[0];\n}\n");
}


//////////////////////////////////////////////////////////////////////////
//
// FatW65C816.inc's text from the values in memory at uiAddress.
//
//////////////////////////////////////////////////////////////////////////
static void TestFatW65C816LayoutText(CAsm65Memory* pcMemory, uint32 uiAddress, CChars* pszText)
{
	size	i;
	uint32	uiValue;
	char	szHex[16];

	pszText->Init();
	pszText->Append("; The W65C816 layout of BaseLib's FAT classes and the FAT constants, for the FAT assembly (*816.asm).\n");
	pszText->Append("; Generated by TestBaseLib65 (TestFatW65C816Layout.cpp) with the W65C816 compiler: don't edit it, copy\n");
	pszText->Append("; TestBaseLib65's Output/<target>/FatW65C816.inc over it when the classes change.\n\n");
	for (i = 0; gasTestFatW65C816Values[i].szName; i++)
	{
		uiValue = (uint32)pcMemory->GetByte(uiAddress + (uint32)i * 4) | ((uint32)pcMemory->GetByte(uiAddress + (uint32)i * 4 + 1) << 8) |
				  ((uint32)pcMemory->GetByte(uiAddress + (uint32)i * 4 + 2) << 16) | ((uint32)pcMemory->GetByte(uiAddress + (uint32)i * 4 + 3) << 24);
		sprintf(szHex, "0x%X", uiValue);
		pszText->Append(gasTestFatW65C816Values[i].szName)->Append(" = ")->Append(szHex)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFatW65C816Layout(void)
{
	CCPP65Builder	cBuilder;
	CAsm65Memory	cMemory;
	CFastW65C816	cMPU;
	CFileUtil		cFileUtil;
	CTextFile		cExpected;
	CChars			szSource;
	CChars			szText;
	CChars			szCommon;
	CChars			szLibc65;
	CChars			szOutput;
	int64			lliAddress;
	FILE*			pcFile;
	bool			bLinked;

	BeginTests();

	szCommon.Init(TEST_FAT_W65C816_COMMON);
	cFileUtil.FullPath(&szCommon);
	szLibc65.Init(TEST_FAT_W65C816_LIBC65);
	cFileUtil.FullPath(&szLibc65);

	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.AddIncludeDirectory(szLibc65.Text(), false, true);
	cBuilder.AddIncludeDirectory(szCommon.Text(), true, false);
	AssertTrue(cBuilder.AddStartUp());
	TestFatW65C816LayoutSource(&szSource);
	bLinked = cBuilder.LinkWith("FatW65C816Layout.cpp", szSource.Text());
	AssertString("", cBuilder.GetErrors());
	AssertTrue(bLinked);
	AssertTrue(cBuilder.GetSymbol("_gauiFatW65C816Layout", &lliAddress));
	if (bLinked)
	{
		cMemory.Init(0x1000000);
		cMemory.Load(cBuilder.GetOutput());
		cMemory.InitFast65816(&cMPU);
		cMPU.Reset();
		cMPU.Run(10000000);
		AssertTrue(cMPU.IsStopped());
		TestFatW65C816LayoutText(&cMemory, (uint32)lliAddress, &szText);
		cMPU.Kill();
		cMemory.Kill();

		//In the run's own Output/<target>: a 65C64 run and a W65C816 run can go at once.
		szOutput.Init("Output" _FS_);
		szOutput.Append(TestTarget65Name(geTestProject65Target));
		cFileUtil.TouchDir(szOutput.Text());
		szOutput.Append(_FS_ "FatW65C816.inc");
		pcFile = fopen(szOutput.Text(), "wb");
		szOutput.Kill();
		if (pcFile)
		{
			fwrite(szText.Text(), 1, szText.Length(), pcFile);
			fclose(pcFile);
		}

		cExpected.Init();
		AssertTrue(cExpected.Read(TEST_FAT_W65C816_LAYOUT));
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
