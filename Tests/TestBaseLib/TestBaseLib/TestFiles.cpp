#include "BaseLib/TextFile.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/Files.h"
#include "TestLib/AssertFile.h"


void AssertPakFile(const char* szFileName, const char* szContents, CFiles* pcFiles);
void AssertFileName(CMapStringInt* pcMap, int iCount, int iRank, const char* szFileName);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFilesSimple(void)
{
	CFiles			cFiles;
	CAbstractFile*	pcFile;
	CTextFile		cTextFile;

	//If this test fails you probably need to re-generate the source PAK files.
	//Use TestPackFilesPackerSimple to do this.

	cFiles.Init("Input" _FS_ "Game2", "PAK");
	AssertSize(6, cFiles.GetNumPackFiles());

	pcFile = cFiles.GetFile("Sounds/Santa/Seattle.txt");
	AssertNotNull(pcFile);

	cTextFile.Init();
	cTextFile.Read(pcFile);
	AssertString("All night long\r\n", cTextFile.Text());
	cTextFile.Kill();

	pcFile = cFiles.GetFile("Sounds/General.txt");
	AssertNotNull(pcFile);

	cTextFile.Init();
	cTextFile.Read(pcFile);
	AssertString("General", cTextFile.Text());
	cTextFile.Kill();

	pcFile = cFiles.GetFile("Sounds/Santa/Slay/Spelling.txt");
	AssertNotNull(pcFile);

	cTextFile.Init();
	cTextFile.Read(pcFile);
	AssertString("Spelling", cTextFile.Text());
	cTextFile.Kill();

	cFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileSystemIteration(void)
{
	CFileSystem				cFileSystem;
	CFileSystemIterator		cIter;
	CSystemFileNode*		pcSystemFile;
	CChars					szName;
	int						i;
	CArrayChars				aszExpected;
	CChars*					pszExpected;

	aszExpected.Init();
	aszExpected.AddList("Models/Super/Barbie.txt",
		"Models/Super/Ken.txt",
		"Models/Cars.txt",
		"Sounds/Santa/Seattle.txt",
		"Sounds/Ambient.txt",
		"Sounds/Cheese.PAK",
		"Sounds/Santa.PAK",
		"Videos/Intro.txt",
		"Videos/Outro.txt",
		"Models.PAK",
		"Sounds.PAK",
		"Textures.PAK",
		"Videos.PAK",
		NULL);

	cFileSystem.Init("Game2");

	i = 0;
	pcSystemFile = cFileSystem.StartIteration(&cIter);
	while (pcSystemFile)
	{ 
		szName.Init();
		pcSystemFile->GetFullName(&szName);
		pszExpected = aszExpected.Get(i);
		AssertString(pszExpected->Text(), szName.Text());
		szName.Kill();

		i++;
		pcSystemFile = cFileSystem.Iterate(&cIter);
	}
	cFileSystem.Kill();
	aszExpected.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFilesIteration(void)
{
	CFiles			cFiles;
	CFileIterator	cIter;

	cFiles.Init("Input" _FS_ "Game2", "PAK");
	AssertSize(6, cFiles.GetNumPackFiles());
	AssertFilePath("Input" _FS_ "Game2" _FS_ "Sounds" _FS_ "Cheese.PAK", cFiles.GetPackFiles(0)->GetFilename());
	AssertFilePath("Input" _FS_ "Game2" _FS_ "Sounds" _FS_ "Santa.PAK", cFiles.GetPackFiles(1)->GetFilename());
	AssertFilePath("Input" _FS_ "Game2" _FS_ "Textures.PAK", cFiles.GetPackFiles(2)->GetFilename());
	AssertFilePath("Input" _FS_ "Game2" _FS_ "Models.PAK", cFiles.GetPackFiles(3)->GetFilename());
	AssertFilePath("Input" _FS_ "Game2" _FS_ "Sounds.PAK", cFiles.GetPackFiles(4)->GetFilename());
	AssertFilePath("Input" _FS_ "Game2" _FS_ "Videos.PAK", cFiles.GetPackFiles(5)->GetFilename());

	AssertStringCase("Sounds/Cheese/Moose.txt", cFiles.StartIteration(&cIter)->GetFullName(), false);
	AssertInt((uint32)1, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/Cheese/Scream.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)1, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/Santa/Slay/Spelling.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)1, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/Santa/Seattle.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)1, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Models/Super/Barbie.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)0, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Models/Super/Ken.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)0, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Models/Cars.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)0, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/Cheese/Moose.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)0, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/Cheese/Scream.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)0, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/Santa/Seattle.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)0, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/Ambient.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)0, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/General.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)0, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Videos/Intro.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)0, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_PackFiles, cIter.GetCurrent()->GetType());

	AssertStringCase("Models/Super/Barbie.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)FILE_SYSTEM_RANK, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_FileSystem, cIter.GetCurrent()->GetType());

	AssertStringCase("Models/Super/Ken.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)FILE_SYSTEM_RANK, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_FileSystem, cIter.GetCurrent()->GetType());

	AssertStringCase("Models/Cars.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)FILE_SYSTEM_RANK, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_FileSystem, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/Santa/Seattle.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)FILE_SYSTEM_RANK, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_FileSystem, cIter.GetCurrent()->GetType());

	AssertStringCase("Sounds/Ambient.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)FILE_SYSTEM_RANK, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_FileSystem, cIter.GetCurrent()->GetType());

	AssertStringCase("Videos/Intro.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)FILE_SYSTEM_RANK, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_FileSystem, cIter.GetCurrent()->GetType());

	AssertStringCase("Videos/Outro.txt", cFiles.Iterate(&cIter)->GetFullName(), false); 
	AssertInt((uint32)FILE_SYSTEM_RANK, cIter.GetCurrent()->GetFileRank());
	AssertInt(FIRT_FileSystem, cIter.GetCurrent()->GetType());

	AssertNull(cFiles.Iterate(&cIter));
	AssertNull(cIter.GetCurrent());
	cFiles.StopIteration(&cIter);

	cFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGetFileNames(void)
{
	CFiles			cFiles;
	CMapStringInt	cFilenames;
	size				iNumFiles;

	cFiles.Init("Input" _FS_ "Game2", "PAK");

	cFilenames.Init();
	cFiles.GetFilenames(&cFilenames);

	iNumFiles = cFilenames.NumElements();
	AssertSize(11, iNumFiles);
	
	AssertFileName(&cFilenames, 2, FILE_SYSTEM_RANK, "Models/Cars.txt");
	AssertFileName(&cFilenames, 2, FILE_SYSTEM_RANK, "Models/Super/Barbie.txt");
	AssertFileName(&cFilenames, 2, FILE_SYSTEM_RANK, "Models/Super/Ken.txt");
	AssertFileName(&cFilenames, 2, FILE_SYSTEM_RANK, "Sounds/Ambient.txt");
	AssertFileName(&cFilenames, 2, 1,                "Sounds/Cheese/Moose.txt");
	AssertFileName(&cFilenames, 2, 1,                "Sounds/Cheese/Scream.txt");
	AssertFileName(&cFilenames, 1, 0,                "Sounds/General.txt");
	AssertFileName(&cFilenames, 3, FILE_SYSTEM_RANK, "Sounds/Santa/Seattle.txt");
	AssertFileName(&cFilenames, 1, 1,                "Sounds/Santa/Slay/Spelling.txt");
	AssertFileName(&cFilenames, 2, FILE_SYSTEM_RANK, "Videos/Intro.txt");
	AssertFileName(&cFilenames, 1, FILE_SYSTEM_RANK, "Videos/Outro.txt");

	cFilenames.Kill();
	
	cFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertFileName(CMapStringInt* pcMap, int iCount, int iRank, const char* szFileName)
{
	int32		iValue;

	iValue = pcMap->Get(szFileName);
	AssertInt(iCount, (int)(iValue & 0xFFFF));
	AssertInt(iRank, (int)(iValue >> 16));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFilesWholeDirectory(void)
{
	CFiles			cFiles;

	cFiles.Init("Input" _FS_ "Game2", "PAK");

	AssertPakFile("Models/Super/Barbie.txt",		"Barbie",				&cFiles);
	AssertPakFile("Models/Super/Ken.txt",			"Ken",					&cFiles);
	AssertPakFile("Models/Cars.txt",				"Cars",					&cFiles);
	AssertPakFile("Sounds/Cheese/Moose.txt",		"Moose with a view",	&cFiles);
	AssertPakFile("Sounds/Cheese/Scream.txt",		"Scream in a space",	&cFiles);
	AssertPakFile("Sounds/Santa/Seattle.txt",		"All night long\r\n",	&cFiles);
	AssertPakFile("Sounds/Santa/Slay/Spelling.txt", "Spelling",				&cFiles);
	AssertPakFile("Sounds/Ambient.txt",				"Ambient occlusion",	&cFiles);
	AssertPakFile("Sounds/General.txt",				"General",				&cFiles);
	AssertPakFile("Videos/Intro.txt",				"Intro",				&cFiles);
	AssertPakFile("Videos/Outro.txt",				"Outro.txt",			&cFiles);

	cFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertPakFile(const char* szFileName, const char* szContents, CFiles* pcFiles)
{
	CAbstractFile*	pcFile;
	CTextFile		cTextFile;

	pcFile = pcFiles->GetFile(szFileName);
	AssertNotNull(pcFile);

	cTextFile.Init();
	cTextFile.Read(pcFile);
	AssertString(szContents, cTextFile.Text());
	cTextFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFiles(void)
{
	TypeConverterInit();
	MemoryInit();
	BeginTests();

	TestFilesSimple();
	TestFileSystemIteration();
	TestFilesIteration();
	TestGetFileNames();
	TestFilesWholeDirectory();

	TestStatistics();
	MemoryKill();
	TypeConverterKill();
}

