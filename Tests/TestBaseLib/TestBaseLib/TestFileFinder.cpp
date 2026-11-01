#include "BaseLib/FileUtil.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileFinder(void)
{
	BeginTests();

	CFileUtil		cFileUtil;
	CArrayChars		aszFiles;
	
	aszFiles.Init();
	cFileUtil.FindFilesWithExtension("Finder", "txt", &aszFiles);
	AssertSize(5, aszFiles.NumElements());

	AssertFilePath("Finder" _FS_ "Ants.txt", aszFiles.Get(0)->Text());
	AssertFilePath("Finder" _FS_ "File.txt", aszFiles.Get(1)->Text());
	AssertFilePath("Finder" _FS_ "Last.txt", aszFiles.Get(2)->Text());
	AssertFilePath("Finder" _FS_ "Marching.txt", aszFiles.Get(3)->Text());
	AssertFilePath("Finder" _FS_ "Ninja.txt", aszFiles.Get(4)->Text());
	aszFiles.Kill();

	aszFiles.Init();
	cFileUtil.FindFilesWithNameContaining("Finder", "Mar", &aszFiles);
	AssertSize(3, aszFiles.NumElements());
	AssertFilePath("Finder" _FS_ "Glenmark.wav", aszFiles.Get(0)->Text());
	AssertFilePath("Finder" _FS_ "March.rar", aszFiles.Get(1)->Text());
	AssertFilePath("Finder" _FS_ "Marching.txt", aszFiles.Get(2)->Text());
	aszFiles.Kill();

	aszFiles.Init();
	cFileUtil.FindAllDirectories("Finder", &aszFiles);
	aszFiles.QuickSort();
	AssertSize(3, aszFiles.NumElements());
	AssertFilePath("Finder" _FS_ "Lord", aszFiles.Get(0)->Text());
	AssertFilePath("Finder" _FS_ "Rising", aszFiles.Get(1)->Text());
	AssertFilePath("Finder" _FS_ "Storm", aszFiles.Get(2)->Text());
	aszFiles.Kill();

	aszFiles.Init();
	cFileUtil.FindFilesWithExtension("Finder", "rar", &aszFiles, true);
	AssertSize(6, aszFiles.NumElements());
	AssertFilePath("Finder" _FS_ "Lord" _FS_ "1.rar", aszFiles.Get(0)->Text());
	AssertFilePath("Finder" _FS_ "Rising" _FS_ "2.rar", aszFiles.Get(1)->Text());
	AssertFilePath("Finder" _FS_ "Storm" _FS_ "3.rar", aszFiles.Get(2)->Text());
	AssertFilePath("Finder" _FS_ "Ants.rar", aszFiles.Get(3)->Text());
	AssertFilePath("Finder" _FS_ "Echo.rar", aszFiles.Get(4)->Text());
	AssertFilePath("Finder" _FS_ "March.rar", aszFiles.Get(5)->Text());
	aszFiles.Kill();

	aszFiles.Init();
	cFileUtil.FindAllFiles("Finder", &aszFiles, true);
	AssertSize(12, aszFiles.NumElements());
	aszFiles.Kill();

	TestStatistics();
}

