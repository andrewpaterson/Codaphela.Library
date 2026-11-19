#include <string.h>
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/RedirectPrintf.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Fat32.h"
#include "BaseLib/FatDebug.h"
#include "TestLib/Assert.h"
#include "TestLib/Words.h"
#include "TestFat32Common.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32ReadSpecific(void)
{
	CFatTestDrive			cMemoryDrive;
	CDiskFile				cFile;
	lsize					uiLength;
	SFatDirectoryEntry*		psFatDirectoryEntry;
	SFatDirectoryEntry		sFatFileEntry;
	SFatFileSystemQuery		sQuery;
	EFatCode				eResult;
	CFatFile				cFatFile;
	bool					bResult;
	lsize					uiBytesRead;
	static char					auiFileData[44 KB];
	static char					auiExpectedData[44 KB];
	CFatVolume				cVolume;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	AssertSize(sizeof(uint8), sizeof(char));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);


	sQuery.Init();
	eResult = cVolume.FindFirstFATEntry(NULL, 0, &psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("Document.txt", (char*)psFatDirectoryEntry->szName);
	AssertInt(FAT_ATTR_ARCHIVE, psFatDirectoryEntry->uiAttributes);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open((char*)psFatDirectoryEntry->szName, 0);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(12, uiBytesRead);
	auiFileData[12] = '\0';
	AssertString("Mostly Empty", auiFileData);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("Pico", (char*)psFatDirectoryEntry->szName);
	AssertInt(FAT_ATTR_DIRECTORY, psFatDirectoryEntry->uiAttributes);

	eResult = cVolume.FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
	AssertTrue((StrEmpty((char*)psFatDirectoryEntry->szName)));
	sQuery.Kill(cVolume.GetSectorCache());


	sQuery.Init();
	eResult = cVolume.FindFirstFATEntry("\\Pico", 0, &psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString(".", (char*)psFatDirectoryEntry->szName);
	AssertInt(FAT_ATTR_DIRECTORY, psFatDirectoryEntry->uiAttributes);

	eResult = cVolume.FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("..", (char*)psFatDirectoryEntry->szName);
	AssertInt(FAT_ATTR_DIRECTORY, psFatDirectoryEntry->uiAttributes);

	eResult = cVolume.FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString(".gitignore", (char*)psFatDirectoryEntry->szName);
	AssertInt(FAT_ATTR_ARCHIVE, psFatDirectoryEntry->uiAttributes);

	eResult = cVolume.FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("HowDoesItWork", (char*)psFatDirectoryEntry->szName);
	AssertInt(FAT_ATTR_DIRECTORY, psFatDirectoryEntry->uiAttributes);

	eResult = cVolume.FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("LCDBusReader", (char*)psFatDirectoryEntry->szName);
	AssertInt(FAT_ATTR_DIRECTORY, psFatDirectoryEntry->uiAttributes);

	eResult = cVolume.FindNextFATEntry(&psFatDirectoryEntry, &sQuery);
	AssertTrue((StrEmpty((char*)psFatDirectoryEntry->szName)));
	sQuery.Kill(cVolume.GetSectorCache());


	eResult = cVolume.GetFileEntry("\\Pico\\LCDBusReader", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("LCDBUS~1", (char*)sFatFileEntry.szName);
	AssertInt(FAT_ATTR_DIRECTORY, sFatFileEntry.uiAttributes);

	eResult = cVolume.GetFileEntry("\\Pico\\LCDBusReader\\src", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("SRC", (char*)sFatFileEntry.szName);
	AssertInt(FAT_ATTR_DIRECTORY, sFatFileEntry.uiAttributes);

	eResult = cVolume.GetFileEntry("\\Pico\\HowDoesItWork\\build.sh", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("BUILD.SH", (char*)sFatFileEntry.szName);
	AssertInt(FAT_ATTR_ARCHIVE, sFatFileEntry.uiAttributes);

	eResult = cVolume.GetFileEntry("\\Pico\\LCDBusReader\\src\\SDCard.cpp", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("SDCARD.CPP", (char*)sFatFileEntry.szName);
	AssertInt(FAT_ATTR_ARCHIVE, sFatFileEntry.uiAttributes);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open(&sFatFileEntry, FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(44000, uiBytesRead);
	auiFileData[44000] = '\0';

	cFatFile.Close();


	cFile.Init("Input" _FS_ "Fat32" _FS_ "SDCard.cpp");
	bResult = cFile.Open(EFM_Read);
	AssertTrue(bResult);
	uiLength = (lsize)cFile.Size();
	AssertSize(44000, uiLength);
	cFile.Read(auiExpectedData, uiLength, 1);
	cFile.Close();
	cFile.Kill();
	AssertMemory(auiExpectedData, auiFileData, uiLength);


	eResult = cVolume.GetFileEntry("\\Pico\\LCDBusReader\\src\\LCDBusReader.cpp", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("LCDBUS~1.CPP", (char*)sFatFileEntry.szName);
	AssertInt(FAT_ATTR_ARCHIVE, sFatFileEntry.uiAttributes);


	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32ReadDirectoryTree(void)
{
	CFatTestDrive			cMemoryDrive;
	EFatCode				eResult;
	CArrayChars				aszFileNames;
	CChars					sz;
	size					i;
	char*					szFileName;
	SFatDirectoryEntry		sFatFileEntry;
	static char					auiFileData[44 KB];
	CFatFile				cFatFile;
	lsize					uiBytesRead;
	CFatVolume				cVolume;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	aszFileNames.Init();
	RecurseFindFatFilenames(&cVolume, "", &aszFileNames);

	sz.Init();
	aszFileNames.Print(&sz);
	AssertString("\
\\Document.txt\n\
\\Pico\\.gitignore\n\
\\Pico\\HowDoesItWork\\build.sh\n\
\\Pico\\HowDoesItWork\\CMakeLists.txt\n\
\\Pico\\HowDoesItWork\\deploy.sh\n\
\\Pico\\HowDoesItWork\\HowDoesItWork.c\n\
\\Pico\\HowDoesItWork\\pico_sdk_import.cmake\n\
\\Pico\\HowDoesItWork\\.vscode\\.cortex-debug.peripherals.state.json\n\
\\Pico\\HowDoesItWork\\.vscode\\.cortex-debug.registers.state.json\n\
\\Pico\\HowDoesItWork\\.vscode\\c_cpp_properties.json\n\
\\Pico\\HowDoesItWork\\.vscode\\launch.json\n\
\\Pico\\HowDoesItWork\\.vscode\\settings.json\n\
\\Pico\\LCDBusReader\\build.sh\n\
\\Pico\\LCDBusReader\\CMakeLists.txt\n\
\\Pico\\LCDBusReader\\deploy.sh\n\
\\Pico\\LCDBusReader\\pico_sdk_import.cmake\n\
\\Pico\\LCDBusReader\\.vscode\\.cortex-debug.peripherals.state.json\n\
\\Pico\\LCDBusReader\\.vscode\\.cortex-debug.registers.state.json\n\
\\Pico\\LCDBusReader\\.vscode\\c_cpp_properties.json\n\
\\Pico\\LCDBusReader\\.vscode\\launch.json\n\
\\Pico\\LCDBusReader\\.vscode\\settings.json\n\
\\Pico\\LCDBusReader\\src\\GeneralPins.cpp\n\
\\Pico\\LCDBusReader\\src\\GeneralPins.h\n\
\\Pico\\LCDBusReader\\src\\HEXDisplay.cpp\n\
\\Pico\\LCDBusReader\\src\\HEXDisplay.h\n\
\\Pico\\LCDBusReader\\src\\HexToMem.cpp\n\
\\Pico\\LCDBusReader\\src\\HexToMem.h\n\
\\Pico\\LCDBusReader\\src\\LCDBusReader.cpp\n\
\\Pico\\LCDBusReader\\src\\LCDDisplay.cpp\n\
\\Pico\\LCDBusReader\\src\\LCDDisplay.h\n\
\\Pico\\LCDBusReader\\src\\LTC6903.cpp\n\
\\Pico\\LCDBusReader\\src\\LTC6903.h\n\
\\Pico\\LCDBusReader\\src\\SDCard.cpp\n\
\\Pico\\LCDBusReader\\src\\SDCard.h\n\
\\Pico\\LCDBusReader\\src\\SDCardTest.h\n\
\\Pico\\LCDBusReader\\src\\ShiftLCDDisplay.cpp\n\
\\Pico\\LCDBusReader\\src\\ShiftLCDDisplay.h\n\
\\Pico\\LCDBusReader\\src\\ShiftRegister.cpp\n\
\\Pico\\LCDBusReader\\src\\ShiftRegister.h\n\
\\Pico\\LCDBusReader\\src\\SPIComm.cpp\n\
\\Pico\\LCDBusReader\\src\\SPIComm.h\n\
\\Pico\\LCDBusReader\\src\\UARTComm.cpp\n\
\\Pico\\LCDBusReader\\src\\UARTComm.h\n", sz.Text());
	sz.Kill();

	for (i = 0; i < aszFileNames.NumElements(); i++)
	{
		szFileName = aszFileNames.GetText(i);
		eResult = cVolume.GetFileEntry(szFileName, &sFatFileEntry);
		AssertInt(FAT_SUCCESS, eResult);

		cFatFile.Init(&cVolume);
		eResult = cFatFile.Open(szFileName, 0);
		AssertInt(FAT_SUCCESS, eResult);

		eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
		AssertInt(FAT_SUCCESS, eResult);
		AssertSize(cFatFile.Size(), uiBytesRead);
		AssertSize(sFatFileEntry.uiSize, uiBytesRead);

		eResult = cFatFile.Close();
		AssertInt(FAT_SUCCESS, eResult);
	}

	aszFileNames.Kill();

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32Format(void)
{
	CFatTestDrive			cMemoryDrive;
	EFatCode				eResult;
	SFatDirectoryEntry*		psFatDirectoryEntry;
	SFatFileSystemQuery		sQuery;
	CFatVolume				cVolume;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = FatFormat(FAT_FS_TYPE_FAT32, "FAT32", 1, &cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	sQuery.Init();
	eResult = cVolume.FindFirstFATEntry(NULL, 0, &psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("", (char*)psFatDirectoryEntry->szName);
	AssertInt(0, psFatDirectoryEntry->uiAttributes);
	sQuery.Kill(cVolume.GetSectorCache());

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32CreateFileEntries(void)
{
	CFatTestDrive			cMemoryDrive;
	EFatCode				eResult;
	CFatVolume				cVolume;
	SFatDirectoryEntry		sRootDirectoryEntry;
	SFatDirectoryEntry		sFileEntry;
	int						i;
	CChars					szFileName;
	CRandom					cRandom;
	CChars					sz;
	CFatFile				cFatFile;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.GetFileEntry("\\", &sRootDirectoryEntry);

	eResult = cVolume.CreateFATEntry(&sRootDirectoryEntry.sRaw, "Wort Wort Wort.cov", FAT_ATTR_ARCHIVE, 0, &sFileEntry);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Flush();
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintRootFatFilenames(&sz, &cVolume);
	AssertString("\
\\Wort Wort Wort.cov\n\
\\Document.txt\n", sz.Text());
	sz.Kill();

	eResult = cVolume.CreateFATEntry(&sRootDirectoryEntry.sRaw, "Dashwood contempt on mr unlocked resolved provided of of - Stanhill wondered it it welcomed oh - Hundred no prudent he however smiling at an offence - If earnestly extremity he he propriety something admitting convinced ye - Pleasant in to although as.if", FAT_ATTR_ARCHIVE, 0, &sFileEntry);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Flush();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Dashwood contempt on mr unlocked resolved provided of of - Stanhill wondered it it welcomed oh - Hundred no prudent he however smiling at an offence - If earnestly extremity he he propriety something admitting convinced ye - Pleasant in to although as.if", FAT_ATTR_READ_ONLY);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintRootFatFilenames(&sz, &cVolume);
	AssertString("\
\\Wort Wort Wort.cov\n\
\\Document.txt\n\
\\Dashwood contempt on mr unlocked resolved provided of of - Stanhill wondered it it welcomed oh - Hundred no prudent he however smiling at an offence - If earnestly extremity he he propriety something admitting convinced ye - Pleasant in to although as.if\n", sz.Text());
	sz.Kill();

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cRandom.Init(4526);
	eResult = cVolume.GetFileEntry("\\", &sRootDirectoryEntry);
	for (i = 0; i < 10; i++)
	{
		szFileName.Init();
		PrintRandomWords(&szFileName, &cRandom, true, 30);
		szFileName.Append('.');
		PrintRandomWords(&szFileName, &cRandom, false, 1);

		if (szFileName.Length() > FAT_MAX_FILENAME - 1)
		{
			szFileName.SetLength(FAT_MAX_FILENAME - 1);
		}
		
		szFileName.Replace(",", " -");
		eResult = cVolume.CreateFATEntry(&sRootDirectoryEntry.sRaw, szFileName.Text(), FAT_ATTR_ARCHIVE, 0, &sFileEntry);
		AssertInt(FAT_SUCCESS, eResult);
		szFileName.Kill();
	}
	cRandom.Kill();

	eResult = cVolume.Flush();
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintRootFatFilenames(&sz, &cVolume);
	AssertString("\
\\Wort Wort Wort.cov\n\
\\Document.txt\n\
\\Dashwood contempt on mr unlocked resolved provided of of - Stanhill wondered it it welcomed oh - Hundred no prudent he however smiling at an offence - If earnestly extremity he he propriety something admitting convinced ye - Pleasant in to although as.if\n\
\\including sea force second appear all on chicane mage doctor example skill with team strumpet popular option small beer drug animal what kickshaw science raise Levant free hit glim meat them.sea\n\
\\activity bethink oneself of aught spend begin wain land perform opportunity quaggy maid brother mage brimstone sign strumpet very language whence contumely provide hour method happy rack(of a cloud) be driven by the wind base across fruit modern truck.wa\n\
\\timbrel national process get ceil jakes recognize wide mazed cordwainer produce song lay purpose him study candidate forfend otiose energy dot company worry stand drab day ahead kind wonder space.least\n\
\\product thither piepowder machine far rest culture here tocsin win tilt with mrs argue aught third blackguard his egad beautiful stay effort plan green live poor already my travel billion equipage.walk\n\
\\dog cottier similar computer kirtle understand garden experience sex say wrong actually again private thus ween stoup condition beldam horse - coper catch born statement maid fie woman hearken pick leech guy.threat\n\
\\off avaunt relate magazine billion ball indite child subject billion end another image side embouchure water caducity trouble among wherefore quick - the performance already development meeting watchword turnkey work way vote.pm\n\
\\list clout buy nigh continue hearken decade bit describe process north caducity rude portion phone back military inside tax free hold write pest outside from carry taiga kine hereunto just.himself\n\
\\gudgeon host back assume son station spence watchword east whose Lucifer accept nothing behold bruit child owner schoolman service opportunity area put swain rise numbles sore should quickly perchance again.expert\n\
\\friend degree orts letter book at test believe wain shambles full greenwood plan song agree go - cart host attack gentle fire why meat popinjay degree whole month however nice bethink oneself of movie.low\n\
\\fervent watch gage rock wide list song cleanse capital few overset trouble better never prove job sex meet environmental industry best animal during quean aught love apple i gaud learn body.shoulder\n", sz.Text());
	sz.Kill();

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32OpenWriteMode(void)
{
	CFatTestDrive			cMemoryDrive;
	lsize					uiLength;
	EFatCode				eResult;
	CFatFile				cFatFile;
	CFatVolume				cVolume;
	uint64					uiMaxSectorSize;
	SFatFileSystemQuery		sQuery;
	SFatDirectoryEntry*		psFatDirectoryEntry;

	uiLength = 4 MB;
	cMemoryDrive.Init(uiLength, 512);
	
	cMemoryDrive.Erase();
	uiMaxSectorSize = cMemoryDrive.SetMaxSectorForTesting((uint64)(uiLength / 512) * 1024);

	eResult = FatFormat(FAT_FS_TYPE_FAT32, "Fat32", 64, &cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.SetMaxSectorForTesting(uiMaxSectorSize);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());
	AssertInt((uint32)0, cFatFile.CalculateFirstCluster())

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	sQuery.Init();
	eResult = cVolume.FindFirstFATEntry(NULL, 0, &psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	sQuery.Kill(cVolume.GetSectorCache());

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32Write(void)
{
	CFatTestDrive			cMemoryDrive;
	EFatCode				eResult;
	CFatVolume				cVolume;
	CFatFile				cFatFile;
	char					auiFileData[4 KB];
	char					auiFileData2[4 KB];
	int						i;
	lsize					uiBytesRead;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\New File.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());

	for (i = 0; i < 4 KB; i++)
	{
		auiFileData[i] = (char)i;
	}

	eResult = cFatFile.Write((uint8*)auiFileData, 4 KB);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)4096, cFatFile.Size());

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\New File.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)4096, cFatFile.Size());

	eResult = cFatFile.Read((uint8*)auiFileData2, 4 KB, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(0, memcmp(auiFileData, auiFileData2, 4 KB));

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32CreateDirectory(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatVolume		cVolume;
	CArrayChars		aszDirectories;
	CChars			sz;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.CreateDirectory("\\A Directory");

	aszDirectories.Init();
	RecurseFindFatDirectories(&cVolume, "", &aszDirectories);

	sz.Init();
	aszDirectories.Print(&sz);
	aszDirectories.Kill();
	AssertString("\
\\A Directory\n\
\\Pico\\HowDoesItWork\\.vscode\n\
\\Pico\\HowDoesItWork\n\
\\Pico\\LCDBusReader\\.vscode\n\
\\Pico\\LCDBusReader\\src\n\
\\Pico\\LCDBusReader\n\
\\Pico\n", sz.Text());
	sz.Kill();
	
	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32FormatAndCreateDirectory(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatVolume		cVolume;
	CArrayChars		aszDirectories;
	CChars			sz;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = FatFormat(FAT_FS_TYPE_FAT32, "Fat32", 1, &cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.CreateDirectory("\\A Directory");
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cVolume.CreateDirectory("\\A Directory\\Another");
	AssertInt(FAT_SUCCESS, eResult);

	aszDirectories.Init();
	RecurseFindFatDirectories(&cVolume, "", &aszDirectories);

	sz.Init();
	aszDirectories.Print(&sz);
	aszDirectories.Kill();
	AssertString("\
\\A Directory\\Another\n\
\\A Directory\n", sz.Text());
	sz.Kill();

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32CreateFileEntryAndAllocate(void)
{
	CFatTestDrive			cMemoryDrive;
	lsize					uiLength;
	EFatCode				eResult;
	CFatVolume				cVolume;
	uint64					uiMaxSectorSize;
	CFatFile				cFatFile;
	CChars					sz;

	uiLength = 4 MB;
	cMemoryDrive.Init(uiLength, 512);

	cMemoryDrive.Erase();
	uiMaxSectorSize = cMemoryDrive.SetMaxSectorForTesting((uint64)(uiLength / 512) * 1024);

	eResult = FatFormat(FAT_FS_TYPE_FAT32, "Logic", 64, &cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.SetMaxSectorForTesting(uiMaxSectorSize);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());

	cVolume.Flush();
	cFatFile.Flush();

	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0, 16);
	AssertString("\
-------------------------------------------------------------------- Sector (32) --------------------------------------------------------------------\n\
    0 -> FFF8,     1 ->  EOC,     2 ->    0,     3 ->  EOC,     4 ->    0,     5 ->    0,     6 ->    0,     7 ->    0,     8 ->    0,     9 ->    0,     a ->    0,     b ->    0,     c ->    0,     d ->    0,     e ->    0,     f ->    0\n", sz.Text());
	sz.Kill();

	cFatFile.AllocateClusters(64 KB);

	cVolume.Flush();
	cFatFile.Flush();

	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0, 16);
	AssertString("\
-------------------------------------------------------------------- Sector (32) --------------------------------------------------------------------\n\
    0 -> FFF8,     1 ->  EOC,     2 ->    0,     3 ->  EOC,     4 ->    5,     5 ->  EOC,     6 ->    0,     7 ->    0,     8 ->    0,     9 ->    0,     a ->    0,     b ->    0,     c ->    0,     d ->    0,     e ->    0,     f ->    0\n", sz.Text());
	sz.Kill();

	cFatFile.AllocateClusters(128 KB);

	cVolume.Flush();
	cFatFile.Flush();

	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0, 16);
	AssertString("\
-------------------------------------------------------------------- Sector (32) --------------------------------------------------------------------\n\
    0 -> FFF8,     1 ->  EOC,     2 ->    0,     3 ->  EOC,     4 ->    5,     5 ->    6,     6 ->    7,     7 ->  EOC,     8 ->    0,     9 ->    0,     a ->    0,     b ->    0,     c ->    0,     d ->    0,     e ->    0,     f ->    0\n", sz.Text());
	sz.Kill();

	cFatFile.Close();

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* AllocateAndFillMemory(int32 iLength)
{
	int32	i;
	char*	puiFileData;

	puiFileData = (char*)malloc(iLength);

	char c = 33;
	for (i = 0; i < iLength; i++)
	{
		puiFileData[i] = c;
		if (i % 3 == 2)
		{
			c++;
		}
	}
	return puiFileData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32CreateDirectoryAfterWrite(void)
{
	CFatTestDrive		cMemoryDrive;
	EFatCode			eResult;
	CFatVolume			cVolume;
	char*				puiFileData;
	char*				puiFileData2;
	int32				iLength;
	CFatFile			cFatFile;
	lsize				uiBytesRead;
	CChars				sz;
	SFatDirectoryEntry	sFatFileEntry;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\New File.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());

	iLength = 4 KB;
	puiFileData = AllocateAndFillMemory(iLength);

	eResult = cFatFile.Write((uint8*)puiFileData, iLength);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(iLength, cFatFile.Size());

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.CreateDirectory("\\A Directory");
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0x240, 0x260);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  240 ->  EOC,   241 ->  EOC,   242 ->  EOC,   243 ->  EOC,   244 ->  EOC,   245 ->  EOC,   246 ->  EOC,   247 ->  EOC,   248 ->  EOC,   249 ->  24a,   24a ->  EOC,   24b ->  EOC,   24c ->  EOC,   24d ->  EOC,   24e ->  EOC,   24f ->  EOC\n\
  250 ->  EOC,   251 ->  EOC,   252 ->  EOC,   253 ->  EOC,   254 ->  EOC,   255 ->  EOC,   256 ->  EOC,   257 ->  EOC,   258 ->    0,   259 ->    0,   25a ->    0,   25b ->    0,   25c ->    0,   25d ->    0,   25e ->    0,   25f ->    0\n", sz.Text());
	sz.Kill();

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\New File.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(iLength, cFatFile.Size());

	puiFileData2 = (char*)malloc(iLength);
	uiBytesRead = 0;
	cFatFile.Read((uint8*)puiFileData2, iLength, &uiBytesRead);
	AssertSize(iLength, uiBytesRead);
	AssertMemory(puiFileData, puiFileData2, iLength);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.CreateDirectory("\\A Directory\\We must go deeper");
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.GetFileEntry("\\A Directory", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0x257, cVolume.GetEntryCluster(&sFatFileEntry.sRaw));

	eResult = cVolume.GetFileEntry("\\A Directory\\We must go deeper", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0x258, cVolume.GetEntryCluster(&sFatFileEntry.sRaw));

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("A Directory\\Depth Charge.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());

	eResult = cFatFile.Write((uint8*)puiFileData, iLength);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(iLength, cFatFile.Size());

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.GetFileEntry("\\A Directory\\Depth Charge.txt", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0x259, cVolume.GetEntryCluster(&sFatFileEntry.sRaw));

	eResult = cVolume.CreateDirectory("\\A Directory\\Sideways rather is where it is at");
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.GetFileEntry("\\A Directory\\Sideways rather is where it is at", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0x25a, cVolume.GetEntryCluster(&sFatFileEntry.sRaw));

	eResult = cVolume.GetFileEntry("\\A Directory\\We must go deeper", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0x258, cVolume.GetEntryCluster(&sFatFileEntry.sRaw));

	eResult = cVolume.GetFileEntry("\\New File.txt", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0x256, cVolume.GetEntryCluster(&sFatFileEntry.sRaw));

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\A Directory\\Depth Charge.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(iLength, cFatFile.Size());

	uiBytesRead = 0;
	cFatFile.Read((uint8*)puiFileData2, iLength, &uiBytesRead);
	AssertSize(iLength, uiBytesRead);
	AssertMemory(puiFileData, puiFileData2, iLength);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\New File.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(iLength, cFatFile.Size());

	uiBytesRead = 0;
	cFatFile.Read((uint8*)puiFileData2, iLength, &uiBytesRead);
	AssertSize(iLength, uiBytesRead);
	AssertMemory(puiFileData, puiFileData2, iLength);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0x240, 0x260);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  240 ->  EOC,   241 ->  EOC,   242 ->  EOC,   243 ->  EOC,   244 ->  EOC,   245 ->  EOC,   246 ->  EOC,   247 ->  EOC,   248 ->  EOC,   249 ->  24a,   24a ->  EOC,   24b ->  EOC,   24c ->  EOC,   24d ->  EOC,   24e ->  EOC,   24f ->  EOC\n\
  250 ->  EOC,   251 ->  EOC,   252 ->  EOC,   253 ->  EOC,   254 ->  EOC,   255 ->  EOC,   256 ->  EOC,   257 ->  EOC,   258 ->  EOC,   259 ->  EOC,   25a ->  EOC,   25b ->    0,   25c ->    0,   25d ->    0,   25e ->    0,   25f ->    0\n", sz.Text());
	sz.Kill();

	sz.Init();
	PrintFatFilenames(&sz, "\\A Directory", &cVolume);
	AssertString("\\A Directory\\Depth Charge.txt\n", sz.Text())
	sz.Kill();

	sz.Init();
	PrintFatDirectories(&sz, "\\A Directory", &cVolume);
	AssertString("\
\\A Directory\\We must go deeper\n\
\\A Directory\\Sideways rather is where it is at\n", sz.Text());
	sz.Kill();

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	free(puiFileData);
	free(puiFileData2);

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32GreatWrite(void)
{
	CFatTestDrive			cMemoryDrive;
	EFatCode				eResult;
	SFatDirectoryEntry*		psFatDirectoryEntry;
	SFatFileSystemQuery		sQuery;
	CFatVolume				cVolume;
	CFatFile				cFatFile;
	char*					puiFileData;
	char*					puiFileData2;
	lsize					uiBytesRead;
	int32					iLength;
	CChars					sz;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\New File.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());

	sQuery.Init();
	eResult = cVolume.FindFirstFATEntry(NULL, 0, &psFatDirectoryEntry, &sQuery);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("New File.txt", (char*)psFatDirectoryEntry->szName);
	AssertInt(FAT_ATTR_ARCHIVE, psFatDirectoryEntry->uiAttributes);
	sQuery.Kill(cVolume.GetSectorCache());

	iLength = 196 KB;
	puiFileData2 = (char*)malloc(iLength);
	puiFileData = AllocateAndFillMemory(iLength);

	eResult = cFatFile.Write((uint8*)puiFileData, iLength);

	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)(196 KB), cFatFile.Size());

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\New File.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)(196 KB) , cFatFile.Size());

	memset(puiFileData2, 0, iLength);
	eResult = cFatFile.Read((uint8*)puiFileData2, iLength, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(iLength, uiBytesRead);
	AssertInt(0, memcmp(puiFileData, puiFileData2, iLength));

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Open("\\New File.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	memset(puiFileData2, 0, iLength);
	iLength = 196 KB - 64 KB - 1024;
	cFatFile.Seek(64 KB + 1024, FAT_SEEK_START);
	eResult = cFatFile.Read((uint8*)puiFileData2, iLength, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(iLength, uiBytesRead);
	AssertMemory(&(((uint8*)puiFileData)[64 KB + 1024]), puiFileData2, iLength);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	iLength = 196 KB;
	eResult = cVolume.CreateDirectory("\\A Directory");
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cVolume.CreateDirectory("\\A Directory\\Another");
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\A Directory\\Another\\New File Number Two.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());

	eResult = cFatFile.Write((uint8*)puiFileData, iLength);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)(196 KB), cFatFile.Size());

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintInterestingFATClusters(&sz, &cVolume);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  200 ->  EOC,   201 ->  EOC,   202 ->  EOC,   203 ->  EOC,   204 ->  EOC,   205 ->  EOC,   206 ->  EOC,   207 ->  EOC,   208 ->  EOC,   209 ->  EOC,   20a ->  EOC,   20b ->  EOC,   20c ->  EOC,   20d ->  EOC,   20e ->  EOC,   20f ->  EOC\n\
  210 ->  EOC,   211 ->  EOC,   212 ->  EOC,   213 ->  EOC,   214 ->  EOC,   215 ->  EOC,   216 ->  EOC,   217 ->  EOC,   218 ->  EOC,   219 ->  EOC,   21a ->  EOC,   21b ->  EOC,   21c ->  EOC,   21d ->  EOC,   21e ->  EOC,   21f ->  EOC\n\
  220 ->  EOC,   221 ->  EOC,   222 ->  EOC,   223 ->  EOC,   224 ->  EOC,   225 ->  EOC,   226 ->  EOC,   227 ->  EOC,   228 ->  EOC,   229 ->  EOC,   22a ->  EOC,   22b ->  EOC,   22c ->  EOC,   22d ->  EOC,   22e ->  EOC,   22f ->  EOC\n\
  230 ->  EOC,   231 ->  EOC,   232 ->  EOC,   233 ->  EOC,   234 ->  EOC,   235 ->  EOC,   236 ->  EOC,   237 ->  EOC,   238 ->  EOC,   239 ->  EOC,   23a ->  EOC,   23b ->  EOC,   23c ->  EOC,   23d ->  EOC,   23e ->  EOC,   23f ->  EOC\n\
  240 ->  EOC,   241 ->  EOC,   242 ->  EOC,   243 ->  EOC,   244 ->  EOC,   245 ->  EOC,   246 ->  EOC,   247 ->  EOC,   248 ->  EOC,   249 ->  24a,   24a ->  EOC,   24b ->  EOC,   24c ->  EOC,   24d ->  EOC,   24e ->  EOC,   24f ->  EOC\n\
  250 ->  EOC,   251 ->  EOC,   252 ->  EOC,   253 ->  EOC,   254 ->  EOC,   255 ->  EOC,   256 ->  257,   257 ->  258,   258 ->  259,   259 ->  25a,   25a ->  25b,   25b ->  25c,   25c ->  EOC,   25d ->  EOC,   25e ->  EOC,   25f ->  260\n\
  260 ->  261,   261 ->  262,   262 ->  263,   263 ->  264,   264 ->  265,   265 ->  EOC,   266 ->    0,   267 ->    0,   268 ->    0,   269 ->    0,   26a ->    0,   26b ->    0,   26c ->    0,   26d ->    0,   26e ->    0,   26f ->    0\n\
  270 ->    0,   271 ->    0,   272 ->    0,   273 ->    0,   274 ->    0,   275 ->    0,   276 ->    0,   277 ->    0,   278 ->    0,   279 ->    0,   27a ->    0,   27b ->    0,   27c ->    0,   27d ->    0,   27e ->    0,   27f ->    0\n", sz.Text());
	sz.Kill();

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\New File.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)(196 KB), cFatFile.Size());

	memset(puiFileData2, 0, iLength);
	eResult = cFatFile.Read((uint8*)puiFileData2, iLength, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(iLength, uiBytesRead);
	AssertInt(0, memcmp(puiFileData, puiFileData2, iLength));

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);


	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\A Directory\\Another\\New File Number Two.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)(196 KB), cFatFile.Size());

	memset(puiFileData2, 0, iLength);
	eResult = cFatFile.Read((uint8*)puiFileData2, iLength, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(iLength, uiBytesRead);
	AssertInt(0, memcmp(puiFileData, puiFileData2, iLength));

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	free(puiFileData);
	free(puiFileData2);

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32MultipleSmallReads(void)
{
	CFatTestDrive			cMemoryDrive;
	CDiskFile				cFile;
	lsize					uiLength;
	SFatDirectoryEntry		sFatFileEntry;
	EFatCode				eResult;
	CFatFile				cFatFile;
	bool					bResult;
	lsize					uiBytesRead;
	static char					auiFileData[44 KB];
	static char					auiExpectedData[44 KB];
	CFatVolume				cVolume;
	uint32					uiFileSize;
	uint32					uiBytesToRead;
	uint8*					puiFileData;
	int						i;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.GetFileEntry("\\Pico\\LCDBusReader\\src\\SDCard.cpp", &sFatFileEntry);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open(&sFatFileEntry, FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	uiBytesToRead = 1;
	puiFileData = (uint8*)auiFileData;
	uiFileSize = cFatFile.Size();
	for (i = 0;; i++)
	{
		eResult = cFatFile.Read(puiFileData, uiBytesToRead, &uiBytesRead);
		AssertInt(FAT_SUCCESS, eResult);
		AssertSize(uiBytesToRead, uiBytesRead);

		uiFileSize -= uiBytesToRead;
		if (uiFileSize == 0)
		{
			break;
		}
		puiFileData += uiBytesRead;

		uiBytesToRead++;
		if (uiBytesToRead > uiFileSize)
		{
			uiBytesToRead = uiFileSize;
		}
	}
	auiFileData[44000] = '\0';

	cFatFile.Close();

	cFile.Init("Input" _FS_ "Fat32" _FS_ "SDCard.cpp");
	bResult = cFile.Open(EFM_Read);
	AssertTrue(bResult);
	uiLength = (lsize)cFile.Size();
	AssertSize(44000, uiLength);
	cFile.Read(auiExpectedData, uiLength, 1);
	cFile.Close();
	cFile.Kill();

	AssertMemory(auiExpectedData, auiFileData, (int)uiLength);

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32Seek(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	int				i;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	bool bAllSuccess = true;
	for (i = 0; i < (int)cFatFile.Size(); i++)
	{
		eResult = cFatFile.Seek(i, FAT_SEEK_START);
		if (eResult != FAT_SUCCESS)
		{
			bAllSuccess = false;
			break;
		}
	}
	AssertTrue(bAllSuccess);

	cFatFile.Close();

	cVolume.Unmount();
	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32WriteAndSeek(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	int32			i;
	char*			szSource;
	char*			szRead;
	lsize			uiBytesRead;

	szSource = AllocateStringBuffer(196 KB);

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE | FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Write((uint8*)szSource, 196 KB);
	AssertInt(FAT_SUCCESS, eResult);

	bool bAllSuccess = true;
	for (i = 0; i < (int32)cFatFile.Size(); i++)
	{
		eResult = cFatFile.Seek(i, FAT_SEEK_START);
		if (eResult != FAT_SUCCESS)
		{
			bAllSuccess = false;
			break;
		}
	}
	AssertTrue(bAllSuccess);

	szRead = (char*)malloc(196 KB);
	eResult = cFatFile.Seek(0, FAT_SEEK_START);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cFatFile.Read((uint8*)szRead, 196 KB, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertLSize((lsize)(196 KB), uiBytesRead);
	AssertString(szSource, szRead);

	bAllSuccess = true;
	for (i = 1; i < 32769 + 1024; i += 100)
	{
		eResult = cFatFile.Seek(i, FAT_SEEK_START);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccess = false;
			break;
		}
		eResult = cFatFile.Read((uint8*)szRead, 196 KB - i, &uiBytesRead);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccess = false;
			break;
		}
		
		if (uiBytesRead != (lsize)(196 KB - i))
		{
			AssertLSize((lsize)(196 KB - i), uiBytesRead);
			bAllSuccess = false;
			break;
		}
		
		AssertString(&szSource[i], szRead);
	}
	AssertTrue(bAllSuccess);

	cFatFile.Close();

	free(szRead);
	free(szSource);
	cVolume.Unmount();
	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32SeekWriteAndRead1(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	char*			szSource;

	szSource = AllocateStringBuffer(64 KB + 2);

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE | FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Write((uint8*)szSource, 65538);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Seek(65535, FAT_SEEK_START);
	eResult = cFatFile.Write((uint8*)szSource, 3);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Seek(65536, FAT_SEEK_START);
	eResult = cFatFile.Write((uint8*)szSource, 1);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();

	free(szSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32SeekWriteAndRead2(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	int32			i;
	char			szThree[4];
	char			c;
	char			szDest[8 KB];
	lsize			uiBytesRead;
	bool			bAllSuccess;
	
	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE | FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	bAllSuccess = true;
	c = 'A';
	for (i = 0; i < 196 KB; i++)
	{
		szThree[0] = szThree[1] = szThree[2] = c;
		c++;
		if (c == 127)
		{
			c = 'A';
		}
		if ((c == ' ') || (c == '\\') || (c == '"'))
		{
			c++;
		}

		eResult = cFatFile.Write((uint8*)szThree, 3);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccess = false;
			break;
		}


		eResult = cFatFile.Seek(i + 1, FAT_SEEK_START);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccess = false;
			break;
		}

	}
	AssertTrue(bAllSuccess);

	memset(szDest, 0, 8 KB);
	eResult = cFatFile.Seek(0, FAT_SEEK_START);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cFatFile.Read((uint8*)szDest, 8 KB - 1, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32SeekWriteAndRead3(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	int32			i;
	char			szThree[4];
	char			c;
	char			szDest[8 KB];
	lsize			uiBytesRead;
	bool			bAllSuccess;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.CreateDirectory("\\Directory");

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Directory\\File.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE | FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Directory\\File.txt", FAT_FILE_ACCESS_WRITE | FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(cFatFile.Size(), (uint32)0);

	bAllSuccess = true;
	c = 'A';
	for (i = 0; i < 196 KB; i++)
	{
		szThree[0] = szThree[1] = szThree[2] = c;
		c++;
		if (c == 127)
		{
			c = 'A';
		}
		if ((c == ' ') || (c == '\\') || (c == '"'))
		{
			c++;
		}

		eResult = cFatFile.Write((uint8*)szThree, 3);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccess = false;
			break;
		}


		eResult = cFatFile.Seek(i + 1, FAT_SEEK_START);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccess = false;
			break;
		}

	}
	AssertTrue(bAllSuccess);

	memset(szDest, 0, 8 KB);
	eResult = cFatFile.Seek(0, FAT_SEEK_START);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cFatFile.Read((uint8*)szDest, 8 KB - 1, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32RenameSameLength(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	lsize			uiBytesRead;
	char			auiFileData[13];

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cVolume.Rename("\\Document.txt", "\\Manequin.txt");
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Manequin.txt", FAT_ATTR_READ_ONLY);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)12, cFatFile.Size());
	eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(12, uiBytesRead);
	auiFileData[12] = '\0';
	AssertString("Mostly Empty", auiFileData);
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32RenameShortToLong(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	lsize			uiBytesRead;
	char			auiFileData[13];
	CChars			sz;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintRootFatFilenames(&sz, &cVolume);
	AssertString("\\Document.txt\n", sz.Text());
	sz.Kill();

	eResult = cVolume.Rename("\\Document.txt", "\\Dashwood contempt on mr unlocked resolved provided of of - Stanhill wondered it it welcomed oh - Hundred no prudent he however smiling at an offence - If earnestly extremity he he propriety something admitting convinced ye - Pleasant in to although as.if");
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Dashwood contempt on mr unlocked resolved provided of of - Stanhill wondered it it welcomed oh - Hundred no prudent he however smiling at an offence - If earnestly extremity he he propriety something admitting convinced ye - Pleasant in to although as.if", FAT_ATTR_READ_ONLY);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)12, cFatFile.Size());
	eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(12, uiBytesRead);
	auiFileData[12] = '\0';
	AssertString("Mostly Empty", auiFileData);
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32CreateFileRenameFileCreateFile(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	lsize			uiBytesRead;
	char			auiFileData[13];
	CChars			sz;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File1.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintRootFatFilenames(&sz, &cVolume);
	AssertString("\\File1.txt\n\
\\Document.txt\n", sz.Text());
	sz.Kill();

	eResult = cVolume.Rename("\\Document.txt", "\\Dashwood contempt on mr unlocked resolved provided of of - Stanhill wondered it it welcomed oh - Hundred no prudent he however smiling at an offence - If earnestly extremity he he propriety something admitting convinced ye - Pleasant in to although as.if");
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Dashwood contempt on mr unlocked resolved provided of of - Stanhill wondered it it welcomed oh - Hundred no prudent he however smiling at an offence - If earnestly extremity he he propriety something admitting convinced ye - Pleasant in to although as.if", FAT_ATTR_READ_ONLY);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)12, cFatFile.Size());
	eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(12, uiBytesRead);
	auiFileData[12] = '\0';
	AssertString("Mostly Empty", auiFileData);
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File2.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintRootFatFilenames(&sz, &cVolume);
	AssertString("\\File1.txt\n\
\\File2.txt\n\
\\Dashwood contempt on mr unlocked resolved provided of of - Stanhill wondered it it welcomed oh - Hundred no prudent he however smiling at an offence - If earnestly extremity he he propriety something admitting convinced ye - Pleasant in to although as.if\n", sz.Text());
	sz.Kill();

	sz.Init();
	PrintRootDirectoryEntries(&sz, &cVolume, false);
	AssertStringApproximate("\
---------------------------------------\n\
Sequence:     66\n\
Long name[1]:  Info\n\
Long name[2]: rmatio\n\
Long name[3]: n\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     114\n\
---------------------------------------\n\
Sequence:     1\n\
Long name[1]: Syste\n\
Long name[2]: m Volu\n\
Long name[3]: me\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     114\n\
---------------------------------------\n\
Short name: SYSTEM~1\n\
Cluster:    3\n\
Size:       0\n\
Attributes: HIDDEN, SYSTEM, DIRECTORY\n\
---------------------------------------\n\
Sequence:     65\n\
Long name[1]: File1\n\
Long name[2]: .txt\n\
Long name[3]: ÿÿ\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     225\n\
---------------------------------------\n\
Short name: FILE1~0.TXT\n\
Cluster:    0\n\
Size:       0\n\
Attributes: ARCHIVE\n\
---------------------------------------\n\
Sequence:     65\n\
Long name[1]: File2\n\
Long name[2]: .txt\n\
Long name[3]: ÿÿ\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     221\n\
---------------------------------------\n\
Short name: FILE2~0.TXT\n\
Cluster:    0\n\
Size:       0\n\
Attributes: ARCHIVE\n\
---------------------------------------\n\
Sequence:     65\n\
Long name[1]: Pico\n\
Long name[2]: ÿÿÿÿÿÿ\n\
Long name[3]: ÿÿ\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     56\n\
---------------------------------------\n\
Short name: PICO\n\
Cluster:    6\n\
Size:       0\n\
Attributes: DIRECTORY\n\
---------------------------------------\n\
Sequence:     84\n\
Long name[1]: h as.\n\
Long name[2]: if\n\
Long name[3]: ÿÿ\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     19\n\
Long name[1]: in to\n\
Long name[2]:  altho\n\
Long name[3]: ug\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     18\n\
Long name[1]: e - P\n\
Long name[2]: leasan\n\
Long name[3]: t \n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     17\n\
Long name[1]: g con\n\
Long name[2]: vinced\n\
Long name[3]:  y\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     16\n\
Long name[1]: hing \n\
Long name[2]: admitt\n\
Long name[3]: in\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     15\n\
Long name[1]: oprie\n\
Long name[2]: ty som\n\
Long name[3]: et\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     14\n\
Long name[1]: mity \n\
Long name[2]: he he \n\
Long name[3]: pr\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     13\n\
Long name[1]: rnest\n\
Long name[2]: ly ext\n\
Long name[3]: re\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     12\n\
Long name[1]: fence\n\
Long name[2]:  - If \n\
Long name[3]: ea\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     11\n\
Long name[1]: ling \n\
Long name[2]: at an \n\
Long name[3]: of\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     10\n\
Long name[1]: e how\n\
Long name[2]: ever s\n\
Long name[3]: mi\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     9\n\
Long name[1]:  no p\n\
Long name[2]: rudent\n\
Long name[3]:  h\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     8\n\
Long name[1]:  oh -\n\
Long name[2]:  Hundr\n\
Long name[3]: ed\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     7\n\
Long name[1]: t it \n\
Long name[2]: welcom\n\
Long name[3]: ed\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     6\n\
Long name[1]: ll wo\n\
Long name[2]: ndered\n\
Long name[3]:  i\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     5\n\
Long name[1]: f of \n\
Long name[2]: - Stan\n\
Long name[3]: hi\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     4\n\
Long name[1]: ed pr\n\
Long name[2]: ovided\n\
Long name[3]:  o\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     3\n\
Long name[1]: locke\n\
Long name[2]: d reso\n\
Long name[3]: lv\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     2\n\
Long name[1]: empt \n\
Long name[2]: on mr \n\
Long name[3]: un\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Sequence:     1\n\
Long name[1]: Dashw\n\
Long name[2]: ood co\n\
Long name[3]: nt\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     223\n\
---------------------------------------\n\
Short name: DASHWO~0.IF\n\
Cluster:    597\n\
Size:       12\n\
Attributes: ARCHIVE\n", sz.Text());
	sz.Kill();

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32Delete(void)
{
	CFatTestDrive		cMemoryDrive;
	EFatCode			eResult;
	CFatFile			cFatFile;
	CFatVolume			cVolume;
	CChars				sz;
	SFatDirectoryEntry	sEntry;
	uint32				uiCluster;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	eResult = PrintFATClusters(&sz, &cVolume, 0x250, 0x260);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  250 ->  EOC,   251 ->  EOC,   252 ->  EOC,   253 ->  EOC,   254 ->  EOC,   255 ->  EOC,   256 ->    0,   257 ->    0,   258 ->    0,   259 ->    0,   25a ->    0,   25b ->    0,   25c ->    0,   25d ->    0,   25e ->    0,   25f ->    0\n", sz.Text());
	sz.Kill();

	eResult = cVolume.GetFileEntry("\\Document.txt", &sEntry);
	AssertInt(FAT_SUCCESS, eResult);
	uiCluster = cVolume.GetEntryCluster(&sEntry.sRaw);
	AssertInt((uint32)0x255, uiCluster);

	eResult = cVolume.Delete("\\Document.txt");
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Document.txt", FAT_ATTR_READ_ONLY);
	AssertInt(FAT_FILE_NOT_FOUND, eResult);
	eResult = cFatFile.Close();
	AssertInt(FAT_INVALID_HANDLE, eResult);

	sz.Init();
	eResult = PrintFATClusters(&sz, &cVolume, 0x250, 0x260);
	AssertInt(FAT_SUCCESS, eResult);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  250 ->  EOC,   251 ->  EOC,   252 ->  EOC,   253 ->  EOC,   254 ->  EOC,   255 ->    0,   256 ->    0,   257 ->    0,   258 ->    0,   259 ->    0,   25a ->    0,   25b ->    0,   25c ->    0,   25d ->    0,   25e ->    0,   25f ->    0\n", sz.Text());
	sz.Kill();

	sz.Init();
	PrintRootFatFilenames(&sz, &cVolume);
	AssertString("", sz.Text());
	sz.Kill();

	sz.Init();
	PrintRootDirectoryEntries(&sz, &cVolume, false);
	AssertString("\
---------------------------------------\n\
Sequence:     66\n\
Long name[1]:  Info\n\
Long name[2]: rmatio\n\
Long name[3]: n\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     114\n\
---------------------------------------\n\
Sequence:     1\n\
Long name[1]: Syste\n\
Long name[2]: m Volu\n\
Long name[3]: me\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     114\n\
---------------------------------------\n\
Short name: SYSTEM~1\n\
Cluster:    3\n\
Size:       0\n\
Attributes: HIDDEN, SYSTEM, DIRECTORY\n\
---------------------------------------\n\
Sequence:     65\n\
Long name[1]: Pico\n\
Long name[2]: ÿÿÿÿÿÿ\n\
Long name[3]: ÿÿ\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     56\n\
---------------------------------------\n\
Short name: PICO\n\
Cluster:    6\n\
Size:       0\n\
Attributes: DIRECTORY\n", sz.Text());
	sz.Kill();

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32WriteLargerThanOneClusterAppend(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	char* szSource;
	char* szRead;
	lsize			uiBytesRead;
	uint64			uiMaxSectorSize;
	CChars			sz;

	szSource = AllocateStringBuffer(33 KB);

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	cMemoryDrive.Erase();
	uiMaxSectorSize = cMemoryDrive.SetMaxSectorForTesting(((16 MB) / 512) * 1024);

	eResult = FatFormat(FAT_FS_TYPE_FAT32, "Fat32", 64, &cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.SetMaxSectorForTesting(uiMaxSectorSize);

	szRead = (char*)malloc(32769);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File1.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE | FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Write((uint8*)szSource, 32769);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Close();

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File1.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	memset(szRead, 0xcc, 32769);
	eResult = cFatFile.Read((uint8*)szRead, 32769, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(32769, uiBytesRead);
	AssertMemory(szSource, szRead, 32769);
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File2.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE | FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Write((uint8*)szSource, 32768);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Write((uint8*)(&szSource[32768]), 1);
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintRootDirectoryEntries(&sz, &cVolume, false);
	AssertStringApproximate("\
---------------------------------------\n\
Short name: Fat32\n\
Cluster:    0\n\
Size:       0\n\
Attributes: VOLUME_ID\n\
---------------------------------------\n\
Sequence:     65\n\
Long name[1]: File1\n\
Long name[2]: .txt\n\
Long name[3]: ÿÿ\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     225\n\
---------------------------------------\n\
Short name: FILE1~0.TXT\n\
Cluster:    4\n\
Size:       32769\n\
Attributes: ARCHIVE\n\
---------------------------------------\n\
Sequence:     65\n\
Long name[1]: File2\n\
Long name[2]: .txt\n\
Long name[3]: ÿÿ\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     221\n\
---------------------------------------\n\
Short name: FILE2~0.TXT\n\
Cluster:    6\n\
Size:       0\n\
Attributes: ARCHIVE\n", sz.Text());
	sz.Kill();

	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0, 0x10);
	AssertString("\
-------------------------------------------------------------------- Sector (32) --------------------------------------------------------------------\n\
    0 -> FFF8,     1 ->  EOC,     2 ->    0,     3 ->  EOC,     4 ->    5,     5 ->  EOC,     6 ->    7,     7 ->  EOC,     8 ->    0,     9 ->    0,     a ->    0,     b ->    0,     c ->    0,     d ->    0,     e ->    0,     f ->    0\n", sz.Text());
	sz.Kill();

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintRootDirectoryEntries(&sz, &cVolume, false);
	AssertStringApproximate("\
---------------------------------------\n\
Short name: Fat32\n\
Cluster:    0\n\
Size:       0\n\
Attributes: VOLUME_ID\n\
--------------------------------------- \n\
Sequence:     65\n\
Long name[1]: File1\n\
Long name[2]: .txt\n\
Long name[3]: ÿÿ\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     225\n\
--------------------------------------- \n\
Short name: FILE1~0.TXT\n\
Cluster:    4\n\
Size:       32769\n\
Attributes: ARCHIVE\n\
--------------------------------------- \n\
Sequence:     65\n\
Long name[1]: File2\n\
Long name[2]: .txt\n\
Long name[3]: ÿÿ\n\
Cluster:      0\n\
Attributes:   LONG_NAME\n\
Checksum:     221\n\
---------------------------------------\n\
Short name: FILE2~0.TXT\n\
Cluster:    6\n\
Size:       32769\n\
Attributes: ARCHIVE\n", sz.Text());
	sz.Kill();

	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0, 0x10);
	AssertString("\
-------------------------------------------------------------------- Sector (32) --------------------------------------------------------------------\n\
    0 -> FFF8,     1 ->  EOC,     2 ->    0,     3 ->  EOC,     4 ->    5,     5 ->  EOC,     6 ->    7,     7 ->  EOC,     8 ->    0,     9 ->    0,     a ->    0,     b ->    0,     c ->    0,     d ->    0,     e ->    0,     f ->    0\n", sz.Text());
	sz.Kill();

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File2.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	memset(szRead, 0xcc, 32769);
	eResult = cFatFile.Read((uint8*)szRead, 32769, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertMemory(szSource, szRead, 32769);
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();

	free(szRead);
	free(szSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32WriteLargerThanOneClusterOverwrite(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatFile		cFatFile;
	CFatVolume		cVolume;
	char*			szSource;
	char*			szRead;
	lsize			uiBytesRead;
	uint64			uiMaxSectorSize;
	int32			i;
	bool			bAllSuccessful;

	szSource = AllocateStringBuffer(33 KB);

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	cMemoryDrive.Erase();
	uiMaxSectorSize = cMemoryDrive.SetMaxSectorForTesting(((16 MB) / 512) * 1024);

	eResult = FatFormat(FAT_FS_TYPE_FAT32, "Fat32", 64, &cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.SetMaxSectorForTesting(uiMaxSectorSize);

	szRead = (char*)malloc(32769);

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File1.txt", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE | FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);

	eResult = cFatFile.Write((uint8*)szSource, 32768);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(32768, cFatFile.Size());

	eResult = cFatFile.Seek(32767, FAT_SEEK_START);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cFatFile.Write((uint8*)&szSource[32767], 2);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(32769, cFatFile.Size());

	cFatFile.Seek(0, FAT_SEEK_START);
	eResult = cFatFile.Read((uint8*)szRead, 32769, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(32769, uiBytesRead);
	AssertMemory(szSource, szRead, uiBytesRead);

	eResult = cFatFile.Write((uint8*)szSource, 32768 + 64);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(65601, cFatFile.Size());

	bAllSuccessful = true;
	for (i = 32768; i >= 0; i--)
	{
		eResult = cFatFile.Seek(i, FAT_SEEK_START);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccessful = false;
			break;
		}

		eResult = cFatFile.Read((uint8*)szRead, 1, &uiBytesRead);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccessful = false;
			break;
		}

		if (uiBytesRead != 1)
		{
			AssertSize(1, uiBytesRead);
			bAllSuccessful = false;
			break;

		}

		if (szRead[0] != szSource[i])
		{
			AssertChar(szSource[i], szRead[i]);
			bAllSuccessful = false;
			break;
		}
	}

	AssertTrue(bAllSuccessful);

	bAllSuccessful = true;
	for (i = 65600; i > 32768; i--)
	{
		eResult = cFatFile.Seek(i, FAT_SEEK_START);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccessful = false;
			break;
		}

		eResult = cFatFile.Read((uint8*)szRead, 1, &uiBytesRead);
		if (eResult != FAT_SUCCESS)
		{
			AssertInt(FAT_SUCCESS, eResult);
			bAllSuccessful = false;
			break;
		}

		if (uiBytesRead != 1)
		{
			AssertSize(1, uiBytesRead);
			bAllSuccessful = false;
			break;

		}

		if (szRead[0] != szSource[i - 32769])
		{
			AssertChar(szSource[i], szRead[i]);
			bAllSuccessful = false;
			break;
		}
	}

	AssertTrue(bAllSuccessful);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\File1.txt", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	memset(szRead, 0xcc, 32769);
	eResult = cFatFile.Read((uint8*)szRead, 32769, &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(32769, uiBytesRead);
	AssertMemory(szSource, szRead, 32768);
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();

	free(szRead);
	free(szSource);
}


//////////////////////////////////////////////////////////////////////////
//
// A mount that fails leaves the volume unmounted, so unmounting it writes
// nothing: it used to keep the file system of the volume mounted before
// (FAT32) and write that volume's file system information sector to the
// drive it failed to mount.
//
//////////////////////////////////////////////////////////////////////////
void TestFat32FailedMountAfterMount(void)
{
	CFatTestDrive	cFat32Drive;
	CFatTestDrive	cPartitionsDrive;
	CFatVolume		cVolume;

	AssertTrue(cFat32Drive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));
	AssertTrue(cPartitionsDrive.InitImage("Input" _FS_ "Fat32" _FS_ "SDCard4PartitionMBR.img"));

	AssertInt(FAT_SUCCESS, cVolume.Mount(&cFat32Drive));
	AssertInt(FAT_SUCCESS, cVolume.Unmount());

	AssertInt(FAT_INVALID_FAT_VOLUME, cVolume.Mount(&cPartitionsDrive));
	AssertInt(FAT_SUCCESS, cVolume.Unmount());
	AssertInt((uint32)0, cPartitionsDrive.NumWrites());

	cPartitionsDrive.Kill();
	cFat32Drive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32PartionsMount(void)
{
	CFatTestDrive	cMemoryDrive;
	EFatCode		eResult;
	CFatVolume		cVolume;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "SDCard4PartitionMBR.img"));

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_INVALID_FAT_VOLUME, eResult);

	eResult = cVolume.Unmount();
	AssertInt(FAT_SUCCESS, eResult);

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32Close(void)
{
	CFatTestDrive	cMemoryDrive;
	CDiskFile		cFile;
	lsize			uiLength;
	EFatCode		eResult;
	CFatFile		cFatFile;
	bool			bResult;
	CFatVolume		cVolume;
	static char			auiFileData[44 KB];
	static char			auiExpectedData[44 KB];
	lsize			uiBytesRead;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	cFile.Init("Input" _FS_ "Fat32" _FS_ "SDCard.cpp");
	bResult = cFile.Open(EFM_Read);
	AssertTrue(bResult);
	uiLength = (lsize)cFile.Size();
	AssertSize(44000, uiLength);
	cFile.Read(auiExpectedData, uiLength, 1);
	cFile.Close();
	cFile.Kill();

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(44000, cFatFile.Size());
	AssertInt((uint32)0, cFatFile.Tell());
	eResult = cFatFile.Seek(0, FAT_SEEK_START);
	AssertInt(FAT_SUCCESS, eResult);
	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(44000, cFatFile.Size());
	AssertInt((uint32)0, cFatFile.Tell());

	eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(44000, uiBytesRead);
	AssertMemory(auiExpectedData, auiFileData, (int)uiLength);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32TruncateToSmall(void)
{
	CFatTestDrive	cMemoryDrive;
	CDiskFile		cFile;
	lsize			uiLength;
	EFatCode		eResult;
	CFatFile		cFatFile;
	bool			bResult;
	CFatVolume		cVolume;
	static char			auiFileData[44 KB];
	static char			auiExpectedData[44 KB];
	lsize			uiBytesRead;
	CChars			sz;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	cFile.Init("Input" _FS_ "Fat32" _FS_ "SDCard.cpp");
	bResult = cFile.Open(EFM_Read);
	AssertTrue(bResult);
	uiLength = (lsize)cFile.Size();
	AssertSize(44000, uiLength);
	cFile.Read(auiExpectedData, uiLength, 1);
	cFile.Close();
	cFile.Kill();

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(44000, cFatFile.Size());
	AssertInt((uint32)0, cFatFile.Tell());
	eResult = cFatFile.Seek(1, FAT_SEEK_START);
	AssertInt(FAT_SUCCESS, eResult);
	
	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0x240, 0x250);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  240 ->  EOC,   241 ->  EOC,   242 ->  EOC,   243 ->  EOC,   244 ->  EOC,   245 ->  EOC,   246 ->  EOC,   247 ->  EOC,   248 ->  EOC,   249 ->  24a,   24a ->  EOC,   24b ->  EOC,   24c ->  EOC,   24d ->  EOC,   24e ->  EOC,   24f ->  EOC\n", sz.Text());
	sz.Kill();
	eResult = cFatFile.Truncate();
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)1, cFatFile.Size());
	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0x240, 0x250);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  240 ->  EOC,   241 ->  EOC,   242 ->  EOC,   243 ->  EOC,   244 ->  EOC,   245 ->  EOC,   246 ->  EOC,   247 ->  EOC,   248 ->  EOC,   249 ->  EOC,   24a ->    0,   24b ->  EOC,   24c ->  EOC,   24d ->  EOC,   24e ->  EOC,   24f ->  EOC\n", sz.Text());
	sz.Kill();

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)1, cFatFile.Size());

	eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(1, uiBytesRead);
	AssertMemory(auiExpectedData, auiFileData, (int)uiBytesRead);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32OpenOverwrite(void)
{
	CFatTestDrive	cMemoryDrive;
	CDiskFile		cFile;
	lsize			uiLength;
	EFatCode		eResult;
	CFatFile		cFatFile;
	bool			bResult;
	CFatVolume		cVolume;
	static char			auiFileData[44 KB];
	static char			auiExpectedData[44 KB];
	lsize			uiBytesRead;
	CChars			sz;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	cFile.Init("Input" _FS_ "Fat32" _FS_ "SDCard.cpp");
	bResult = cFile.Open(EFM_Read);
	AssertTrue(bResult);
	uiLength = (lsize)cFile.Size();
	AssertSize(44000, uiLength);
	cFile.Read(auiExpectedData, uiLength, 1);
	cFile.Close();
	cFile.Kill();

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(44000, cFatFile.Size());
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0x249, cFatFile.CalculateFirstCluster());
	cFatFile.Close();

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE | FAT_FILE_ACCESS_OVERWRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.CalculateFirstCluster());

	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0x240, 0x250);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  240 ->  EOC,   241 ->  EOC,   242 ->  EOC,   243 ->  EOC,   244 ->  EOC,   245 ->  EOC,   246 ->  EOC,   247 ->  EOC,   248 ->  EOC,   249 ->    0,   24a ->    0,   24b ->  EOC,   24c ->  EOC,   24d ->  EOC,   24e ->  EOC,   24f ->  EOC\n", sz.Text());
	sz.Kill();

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());

	eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(0, uiBytesRead);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32TruncateToZero(void)
{
	CFatTestDrive	cMemoryDrive;
	CDiskFile		cFile;
	lsize			uiLength;
	EFatCode		eResult;
	CFatFile		cFatFile;
	bool			bResult;
	CFatVolume		cVolume;
	static char			auiFileData[44 KB];
	static char			auiExpectedData[44 KB];
	lsize			uiBytesRead;
	CChars			sz;

	AssertTrue(cMemoryDrive.InitImage("Input" _FS_ "Fat32" _FS_ "ComplexDisk.img"));

	cFile.Init("Input" _FS_ "Fat32" _FS_ "SDCard.cpp");
	bResult = cFile.Open(EFM_Read);
	AssertTrue(bResult);
	uiLength = (lsize)cFile.Size();
	AssertSize(44000, uiLength);
	cFile.Read(auiExpectedData, uiLength, 1);
	cFile.Close();
	cFile.Kill();

	eResult = cVolume.Mount(&cMemoryDrive);
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt(44000, cFatFile.Size());
	AssertInt((uint32)0, cFatFile.Tell());
	AssertInt(FAT_SUCCESS, eResult);

	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0x240, 0x250);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  240 ->  EOC,   241 ->  EOC,   242 ->  EOC,   243 ->  EOC,   244 ->  EOC,   245 ->  EOC,   246 ->  EOC,   247 ->  EOC,   248 ->  EOC,   249 ->  24a,   24a ->  EOC,   24b ->  EOC,   24c ->  EOC,   24d ->  EOC,   24e ->  EOC,   24f ->  EOC\n", sz.Text());
	sz.Kill();
	eResult = cFatFile.Truncate();
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());
	sz.Init();
	PrintFATClusters(&sz, &cVolume, 0x240, 0x250);
	AssertString("\
-------------------------------------------------------------------- Sector (602) --------------------------------------------------------------------\n\
  240 ->  EOC,   241 ->  EOC,   242 ->  EOC,   243 ->  EOC,   244 ->  EOC,   245 ->  EOC,   246 ->  EOC,   247 ->  EOC,   248 ->  EOC,   249 ->    0,   24a ->    0,   24b ->  EOC,   24c ->  EOC,   24d ->  EOC,   24e ->  EOC,   24f ->  EOC\n", sz.Text());
	sz.Kill();

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cFatFile.Init(&cVolume);
	eResult = cFatFile.Open("\\Pico\\LCDBusReader\\src\\SDCard.cpp", FAT_FILE_ACCESS_READ | FAT_FILE_ACCESS_WRITE);
	AssertInt(FAT_SUCCESS, eResult);
	AssertInt((uint32)0, cFatFile.Size());

	eResult = cFatFile.Read((uint8*)auiFileData, cFatFile.Size(), &uiBytesRead);
	AssertInt(FAT_SUCCESS, eResult);
	AssertSize(0, uiBytesRead);

	eResult = cFatFile.Close();
	AssertInt(FAT_SUCCESS, eResult);

	cVolume.Unmount();

	cMemoryDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFat32(void)
{
	TypeConverterInit();
	WordsInit();
	BeginTests();

	TestFat32ReadSpecific();
	TestFat32ReadDirectoryTree();
	TestFat32Format();
	TestFat32CreateFileEntries();
	TestFat32OpenWriteMode();
	TestFat32Write();
	TestFat32CreateDirectory();
	TestFat32FormatAndCreateDirectory();
	TestFat32CreateFileEntryAndAllocate();
	TestFat32CreateDirectoryAfterWrite();
	TestFat32GreatWrite();
	TestFat32MultipleSmallReads();
	TestFat32Seek();
	TestFat32WriteAndSeek();
	TestFat32SeekWriteAndRead1();
	TestFat32SeekWriteAndRead2();
	TestFat32SeekWriteAndRead3();
	TestFat32RenameSameLength();
	TestFat32RenameShortToLong();
	TestFat32CreateFileRenameFileCreateFile();
	TestFat32Delete();
	TestFat32WriteLargerThanOneClusterAppend();
	TestFat32WriteLargerThanOneClusterOverwrite();
	TestFat32Close();
	TestFat32OpenOverwrite();
	TestFat32TruncateToSmall();
	TestFat32TruncateToZero();
	TestFat32PartionsMount();
	TestFat32FailedMountAfterMount();

	TestStatistics();
	WordsKill();
	TypeConverterKill();
}

