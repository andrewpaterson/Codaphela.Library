#include "BaseLib/FileSystem.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileSystemInit(void)
{
	CFileSystem		cSystem;
	char*			szFullName;
	CFileUtil		cFileUtil;
	CChars			szWorkingDirectory;
	CChars			szTemp;
	CTextFile		cTextFile;

	szWorkingDirectory.Init("Finder");
	cFileUtil.FullPath(&szWorkingDirectory);

	cSystem.Init("Finder");

	szFullName = cSystem.GetFilename("Lord/1.rar");
	szTemp.Init(szFullName);
	szTemp.RemoveFromStart(szWorkingDirectory.Length()+1);
	szTemp.Replace(FILE_SEPARATOR[0], '/');

	AssertString("Lord/1.rar", szTemp.Text());
	szTemp.Kill();

	szFullName = cSystem.GetFilename("File.txt");
	cTextFile.Init();
	cTextFile.Read(szFullName);

	AssertString("Hello World.", cTextFile.Text());

	szWorkingDirectory.Kill();
	cTextFile.Kill();
	cSystem.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileSystemNearest(void)
{
	CFileSystem				cSystem;
	CFileUtil				cFileUtil;
	CChars					szWorkingDirectory;
	CFileNodeSystemFile*	pcNode;
	CChars					szRemaining;

	szWorkingDirectory.Init("Finder");
	cFileUtil.FullPath(&szWorkingDirectory);

	cSystem.Init("Finder");

	szRemaining.Init();
	pcNode = cSystem.GetFileNode("Lord/1.rar/Happy/Quan", &szRemaining);
	AssertNotNull(pcNode);
	AssertString("Happy/Quan", szRemaining.Text());
	szRemaining.Kill();

	szRemaining.Init();
	pcNode = cSystem.GetFileNode("File.txt/Sad/Quan", &szRemaining);
	AssertNotNull(pcNode);
	AssertString("Sad/Quan", szRemaining.Text());

	szRemaining.Kill();
	szWorkingDirectory.Kill();
	cSystem.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileSystemFindExtension(void)
{
	CFileSystem				cSystem;
	CArraySystemFilePtr	aFileNodePtrs;

	cSystem.Init("Finder");

	aFileNodePtrs.Init();
	cSystem.GetFiles(&aFileNodePtrs, "rar");

	AssertSize(6, aFileNodePtrs.NumElements());
	
	AssertFilePath("Finder" _FS_ "Lord" _FS_ "1.rar", (*aFileNodePtrs.Get(0))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Rising" _FS_ "2.rar", (*aFileNodePtrs.Get(1))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Storm" _FS_ "3.rar", (*aFileNodePtrs.Get(2))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Ants.rar", (*aFileNodePtrs.Get(3))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Echo.rar", (*aFileNodePtrs.Get(4))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "March.rar", (*aFileNodePtrs.Get(5))->File()->GetFullName());

	aFileNodePtrs.Kill();
	cSystem.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileSystemGetFiles(void)
{
	CFileSystem							cSystem;
	CArraySystemFilePtr				cArrayFiles;

	cSystem.Init("Finder");
	cArrayFiles.Init();

	cSystem.GetFiles(&cArrayFiles);

	AssertSize(12, cArrayFiles.NumElements());

	AssertFilePath("Finder" _FS_ "Lord" _FS_ "1.rar", (*cArrayFiles.Get(0))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Rising" _FS_ "2.rar", (*cArrayFiles.Get(1))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Storm" _FS_ "3.rar", (*cArrayFiles.Get(2))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Ants.rar", (*cArrayFiles.Get(3))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Ants.txt", (*cArrayFiles.Get(4))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Echo.rar", (*cArrayFiles.Get(5))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "File.txt", (*cArrayFiles.Get(6))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Glenmark.wav", (*cArrayFiles.Get(7))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Last.txt", (*cArrayFiles.Get(8))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "March.rar", (*cArrayFiles.Get(9))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Marching.txt", (*cArrayFiles.Get(10))->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Ninja.txt", (*cArrayFiles.Get(11))->File()->GetFullName());

	cArrayFiles.Kill();
	cSystem.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileSystemIterator(void)
{
	CFileSystem			cSystem;
	CFileSystemIterator	cIter;

	cSystem.Init("Finder");

	AssertFilePath("Finder" _FS_ "Lord" _FS_ "1.rar", cSystem.StartIteration(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Rising" _FS_ "2.rar", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Storm" _FS_ "3.rar", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Ants.rar", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Ants.txt", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Echo.rar", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "File.txt", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Glenmark.wav", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Last.txt", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "March.rar", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Marching.txt", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertFilePath("Finder" _FS_ "Ninja.txt", cSystem.Iterate(&cIter)->File()->GetFullName());
	AssertNull(cSystem.Iterate(&cIter));
	cSystem.StopIteration(&cIter);

	cSystem.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileSystem(void)
{
	BeginTests();

	TestFileSystemInit();
	TestFileSystemNearest();
	TestFileSystemFindExtension();
	TestFileSystemGetFiles();
	TestFileSystemIterator();

	TestStatistics();
}

