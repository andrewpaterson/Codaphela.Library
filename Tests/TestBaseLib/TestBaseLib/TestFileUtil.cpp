#include <stdio.h>
#ifdef LINUX_GNU
#include <sys/stat.h>
#include <unistd.h>
#endif // LINUX_GNU
#include "BaseLib/WindowsHeaders.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "TestLib/Assert.h"


#define FILE_UTIL_TEST_DIR	"Output" _FS_ "FileUtil"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilMisc(void)
{
	CFileUtil	cFileUtil;
	bool		bResult;
	CChars		szPath;

	szPath.Init("TestFileUtil");
	cFileUtil.RemoveDir(szPath.Text());

	bResult = cFileUtil.MakeDir(szPath.Text());
	AssertTrue(bResult);

	cFileUtil.AppendToPath(&szPath, "FileOfDoom.Indiana");
	bResult = cFileUtil.Touch(szPath.Text());
	AssertTrue(bResult);

	bResult = cFileUtil.Exists(szPath.Text());
	AssertTrue(bResult);

	cFileUtil.RemoveLastFromPath(&szPath);
	cFileUtil.AppendToPath(&szPath, "AnotherDir");
	bResult = cFileUtil.MakeDir(szPath.Text());
	AssertTrue(bResult);

	cFileUtil.RemoveLastFromPath(&szPath);
	cFileUtil.AppendToPath(&szPath, "Master.Chief");
	bResult = cFileUtil.Touch(szPath.Text());
	AssertTrue(bResult);

	bResult = cFileUtil.Delete(szPath.Text());
	AssertTrue(bResult);

	bResult = cFileUtil.Exists(szPath.Text());
	AssertFalse(bResult);

	cFileUtil.RemoveLastFromPath(&szPath);
	cFileUtil.RemoveDir(szPath.Text());
	szPath.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilNaming(void)
{
	CFileUtil	cFileUtil;
	CChars		sz;
	CChars		szCurrent;
	CChars		szExpected;

	//FullPath depends on where the tests are run from so compare against the current directory.
	szCurrent.Init();
	cFileUtil.CurrentDirectory(&szCurrent);
	if (szCurrent.EndsWith(_FS_))
	{
		szCurrent.RemoveFromEnd(1);
	}

	sz.Init();
	cFileUtil.FullPath(&sz);
	AssertStringCase(szCurrent.Text(), sz.Text(), false);
	sz.Kill();

	szExpected.Init(szCurrent);
	szExpected.Append(_FS_ "Input");
	sz.Init("Input");
	cFileUtil.FullPath(&sz);
	AssertStringCase(szExpected.Text(), sz.Text(), false);
	sz.Kill();

	sz.Init("Input" _FS_ "." _FS_ ".." _FS_ "Output" _FS_);
	cFileUtil.FullPath(&sz);
	szExpected.Set(szCurrent);
	szExpected.Append(_FS_ "Output");
	AssertStringCase(szExpected.Text(), sz.Text(), false);
	sz.Kill();

	sz.Init(szExpected);
	cFileUtil.FullPath(&sz);
	AssertStringCase(szExpected.Text(), sz.Text(), false);
	sz.Kill();
	szExpected.Kill();
	szCurrent.Kill();

	sz.Init(_FS_ "Documents and Settings" _FS_ "Joe" _FS_ ".." _FS_ ".." _FS_);
	cFileUtil.CollapsePath(&sz);
	AssertString(_FS_, sz.Text());
	sz.Kill();

	sz.Init(_FS_ "Documents" _FS_ "and" _FS_ "Settings" _FS_ ".." _FS_ ".." _FS_ "Joe");
	cFileUtil.CollapsePath(&sz);
	AssertString(_FS_ "Documents" _FS_ "Joe", sz.Text());
	sz.Kill();

	sz.Init(_FS_ "Documents" _FS_ "and" _FS_ "Settings" _FS_ ".." _FS_ ".." _FS_ "Joe" _FS_);
	cFileUtil.CollapsePath(&sz);
	AssertString(_FS_ "Documents" _FS_ "Joe", sz.Text());
	sz.Kill();

	sz.Init("." _FS_ "." _FS_ "." _FS_ ".");
	cFileUtil.CollapsePath(&sz);
	AssertString("", sz.Text());
	sz.Kill();

	sz.Init("." _FS_ "." _FS_ "." _FS_);
	cFileUtil.CollapsePath(&sz);
	AssertString("", sz.Text());
	sz.Kill();

	sz.Init(_FS_ ".." _FS_ "..");
	cFileUtil.CollapsePath(&sz);
	AssertString("", sz.Text());
	sz.Kill();

#ifdef _WIN32
	sz.Init("c:\\Documents and Settings\\Joe\\..\\..\\");
	cFileUtil.CollapsePath(&sz);
	AssertStringCase("C:\\", sz.Text(), false);
	sz.Kill();

	sz.Init("c:\\..");
	cFileUtil.CollapsePath(&sz);
	AssertString("c:", sz.Text());
	sz.Kill();

	sz.Init("c:\\..\\..");
	cFileUtil.CollapsePath(&sz);
	AssertString("c:", sz.Text());
	sz.Kill();
#endif // _WIN32
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilFindExtension(void)
{
	CFileUtil	cFileUtil;
	size			iIndex;

	iIndex = cFileUtil.FindExtension("/mnt/world/hello.txt");
	AssertSize(16, iIndex);

	iIndex = cFileUtil.FindExtension(".");
	AssertSize(0, iIndex);

	iIndex = cFileUtil.FindExtension("We.Are.The.Champions");
	AssertSize(10, iIndex);

	iIndex = cFileUtil.FindExtension("c:\\In\\The\\Age");
	AssertSize(-1, iIndex);

	iIndex = cFileUtil.FindExtension(NULL);
	AssertSize(-1, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilRemoveExtension(void)
{
	CChars		szFilename;
	CFileUtil	cFileUtil;

	szFilename.Init("/mnt/world/hello.txt");
	cFileUtil.RemoveExtension(&szFilename);
	AssertString("/mnt/world/hello", szFilename.Text());

	cFileUtil.RemoveExtension(&szFilename);
	AssertString("/mnt/world/hello", szFilename.Text());
	szFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilMakeNameFromDirectory(void)
{
	CFileUtil	cFileUtil;
	CChars		szName;
	CChars		szDirectory;
	CChars		szFilename;

	szDirectory.Init("c:" _FS_ "Some" _FS_ "Directory" _FS_ "Or" _FS_ "Other" _FS_);
	szFilename.Init("c:" _FS_ "Some" _FS_ "Directory" _FS_ "Or" _FS_ "Other" _FS_ "More" _FS_ "Dir" _FS_ "Filename.txt");
	cFileUtil.MakeNameFromDirectory(&szName, &szFilename, &szDirectory);

	AssertString("More/Dir/Filename.txt", szName.Text());
	szName.Kill();

	cFileUtil.MakeNameFromDirectory(&szName, &szDirectory, &szDirectory);

	AssertString("", szName.Text());
	szName.Kill();

	szDirectory.Kill();
	szFilename.Kill();

	//Absolute filename with a relative directory.
	szDirectory.Init("Some" _FS_ "Directory" _FS_);
	szFilename.Init();
	cFileUtil.CurrentDirectory(&szFilename);
	cFileUtil.AppendToPath(&szFilename, "Some" _FS_ "Directory" _FS_ "Filename.txt");
	cFileUtil.MakeNameFromDirectory(&szName, &szFilename, &szDirectory);

	AssertString("Filename.txt", szName.Text());
	szName.Kill();

	szDirectory.Kill();
	szFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilMakeDirectory(void)
{
	CFileUtil		cFileUtil;
	CChars			szDirectory;
	CChars			szFilename;
	CArrayChars		aszFiles;
	CChars			szParentDirectory;
	CChars			szCurrentDirectory;
	bool			bResult;

	cFileUtil.RemoveDir("file_util");

	szParentDirectory.Init("file_util" _FS_ "directory" _FS_ "make" _FS_ "test");
	szDirectory.Init(szParentDirectory);
	szDirectory.Append(_FS_);
	szDirectory.Append("GABBA");
	szFilename.Init(szDirectory);
	szFilename.Append(_FS_);
	szFilename.Append("Filename.txt");

	bResult = cFileUtil.Touch(szFilename.Text());
	AssertFalse(bResult);

	bResult = cFileUtil.TouchDir(szDirectory.Text(), false);
	AssertTrue(bResult);

	szCurrentDirectory.Init();
	cFileUtil.CurrentDirectory(&szCurrentDirectory);
	szCurrentDirectory.Append(_FS_);
	szCurrentDirectory.Append(szDirectory);

	aszFiles.Init();
	cFileUtil.FindAllDirectories(szParentDirectory.Text(), &aszFiles, false);
	AssertSize(1, aszFiles.NumElements());
	AssertString(szCurrentDirectory.Text(), aszFiles.GetText(0));
	aszFiles.Kill();

	bResult = cFileUtil.Touch(szFilename.Text());
	AssertTrue(bResult);

	szCurrentDirectory.Kill();
	szParentDirectory.Kill();
	szDirectory.Kill();
	szFilename.Kill();

	cFileUtil.RemoveDir("file_util");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilRemoveLastFromPath(void)
{
	CFileUtil	cFileUtil;
	CChars		szFilename;

	szFilename.Init("home" _FS_ "directory");
	cFileUtil.RemoveLastFromPath(&szFilename);
	AssertString("home", szFilename.Text());
	szFilename.Kill();

	szFilename.Init(_FS_ "directory");
	cFileUtil.RemoveLastFromPath(&szFilename);
	AssertString(_FS_, szFilename.Text());
	szFilename.Kill();

	szFilename.Init(_FS_ "home" _FS_ "directory");
	cFileUtil.RemoveLastFromPath(&szFilename);
	AssertString(_FS_ "home", szFilename.Text());
	szFilename.Kill();

#ifdef _WIN32
	szFilename.Init("C:\\directory");
	cFileUtil.RemoveLastFromPath(&szFilename);
	AssertString("C:\\", szFilename.Text());
	szFilename.Kill();

	szFilename.Init("C:\\home\\directory");
	cFileUtil.RemoveLastFromPath(&szFilename);
	AssertString("C:\\home", szFilename.Text());
	szFilename.Kill();
#endif // _WIN32

	szFilename.Init("directory");
	cFileUtil.RemoveLastFromPath(&szFilename);
	AssertString("", szFilename.Text());
	szFilename.Kill();

	szFilename.Init("");
	cFileUtil.RemoveLastFromPath(&szFilename);
	AssertString("", szFilename.Text());
	szFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilTouchDirectory(void)
{
	CFileUtil		cFileUtil;
	CChars			szDirectory;
	CChars			szFilename;

	cFileUtil.RemoveDir("file_util");

	szDirectory.Init("file_util" _FS_ "directory" _FS_ "make" _FS_ "test");
	cFileUtil.FullPath(&szDirectory);
	szFilename.Init(szDirectory);
	cFileUtil.AppendToPath(&szFilename, "Warthog.PIG");
	AssertTrue(cFileUtil.TouchDir(szFilename.Text(), true));
	AssertTrue(cFileUtil.Touch(szFilename.Text()));

	szDirectory.Kill();
	szFilename.Kill();

	cFileUtil.RemoveDir("file_util");

	AssertFalse(cFileUtil.TouchDir("C:\\Warthog.PIG", true));
	AssertFalse(cFileUtil.TouchDir("\\Warthog.PIG", true));

	szDirectory.Init();
	cFileUtil.CurrentDirectory(&szDirectory);
	szDirectory.Append("War-pig.hog");
	AssertFalse(cFileUtil.TouchDir(szDirectory.Text(), true));
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilTouchDirectoryOnlyFilename(void)
{
	CFileUtil		cFileUtil;
	CChars			szDirectory;

	szDirectory.Init("War-pig.hog");
	AssertFalse(cFileUtil.TouchDir(szDirectory.Text(), true));
	szDirectory.Kill();

	szDirectory.Init("C:\\War-pig.hog");
	AssertFalse(cFileUtil.TouchDir(szDirectory.Text(), true));
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilPrependToPath(void)
{
	CFileUtil		cFileUtil;
	CChars			szPathName;

	szPathName.Init("Games" _FS_ "R" _FS_ "Us" _FS_);
	cFileUtil.PrependToPath(&szPathName, "Extraordinary");
	AssertString("Extraordinary" _FS_ "Games" _FS_ "R" _FS_ "Us" _FS_, szPathName.Text());
	szPathName.Kill();

	szPathName.Init(_FS_ "Games" _FS_ "R" _FS_ "Us" _FS_);
	cFileUtil.PrependToPath(&szPathName, "Extraordinary");
	AssertString(_FS_ "Extraordinary" _FS_ "Games" _FS_ "R" _FS_ "Us" _FS_, szPathName.Text());
	szPathName.Kill();

	szPathName.Init("Games");
	cFileUtil.PrependToPath(&szPathName, "Extraordinary");
	AssertString("Extraordinary" _FS_ "Games", szPathName.Text());
	szPathName.Kill();

	szPathName.Init(_FS_);
	cFileUtil.PrependToPath(&szPathName, "");
	AssertString(_FS_, szPathName.Text());
	szPathName.Kill();

	szPathName.Init("");
	cFileUtil.PrependToPath(&szPathName, "Giana");
	AssertString("Giana" _FS_, szPathName.Text());
	szPathName.Kill();

#ifdef _WIN32
	szPathName.Init("C:\\Games\\R\\Us\\");
	cFileUtil.PrependToPath(&szPathName, "Extraordinary");
	AssertString("C:\\Extraordinary\\Games\\R\\Us\\", szPathName.Text());
	szPathName.Kill();

	szPathName.Init("C:\\");
	cFileUtil.PrependToPath(&szPathName, "Extraordinary");
	AssertString("C:\\Extraordinary\\", szPathName.Text());
	szPathName.Kill();
#endif // _WIN32
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilRootDirectory(void)
{
	CFileUtil		cFileUtil;
	AssertTrue(cFileUtil.IsRootDirectory(_FS_));
	AssertFalse(cFileUtil.IsRootDirectory(_FS_ "A"));
	AssertFalse(cFileUtil.IsRootDirectory(_FS_ "A" _FS_));
	AssertFalse(cFileUtil.IsRootDirectory("A" _FS_));

#ifdef _WIN32
	AssertTrue(cFileUtil.IsRootDirectory("C:\\"));
	AssertFalse(cFileUtil.IsRootDirectory("C:\\A"));
	AssertFalse(cFileUtil.IsRootDirectory("C:\\A\\"));
#endif // _WIN32

	AssertFalse(cFileUtil.IsRootDirectory(""));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilWriteFile(const char* szFilename, const char* szText)
{
	FILE*	pFile;

	pFile = fopen(szFilename, "w");
	fputs(szText, pFile);
	fclose(pFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size TestFileUtilCountEndingWith(CArrayChars* paszFiles, const char* szEnd)
{
	size	i;
	size	uiCount;

	uiCount = 0;
	for (i = 0; i < paszFiles->NumElements(); i++)
	{
		if (paszFiles->Get(i)->EndsWith(szEnd))
		{
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilTouchExisting(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR, false));

	//Touch creates a file, or empties it if it already exists.
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Existing.txt", "Content");
	AssertLong(7, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "Existing.txt"));
	AssertTrue(cFileUtil.Touch(FILE_UTIL_TEST_DIR _FS_ "Existing.txt"));
	AssertLong(0, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "Existing.txt"));

	AssertTrue(cFileUtil.Touch(FILE_UTIL_TEST_DIR _FS_ "New.txt"));
	AssertTrue(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "New.txt"));

	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilExistsAndSize(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR, false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Six.txt", "123456");

	AssertTrue(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Six.txt"));
	AssertTrue(cFileUtil.Exists(FILE_UTIL_TEST_DIR));
	AssertFalse(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Missing.txt"));

	//A directory has no size, like a file that does not exist.
	AssertLong(6, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "Six.txt"));
	AssertLong(-1, cFileUtil.Size(FILE_UTIL_TEST_DIR));
	AssertLong(-1, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "Missing.txt"));

	AssertTrue(cFileUtil.MakeDir(FILE_UTIL_TEST_DIR _FS_ "Made"));
	AssertFalse(cFileUtil.MakeDir(FILE_UTIL_TEST_DIR _FS_ "Made"));

#ifdef LINUX_GNU
	//A file that cannot be read still exists.
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Unreadable.txt", "x");
	chmod(FILE_UTIL_TEST_DIR _FS_ "Unreadable.txt", 0);
	AssertTrue(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Unreadable.txt"));
	chmod(FILE_UTIL_TEST_DIR _FS_ "Unreadable.txt", 0644);
#endif // LINUX_GNU

	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilFindHidden(void)
{
	CFileUtil		cFileUtil;
	CArrayChars		aszFiles;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Visible", false));
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ ".HiddenDir", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Shown.txt", "x");
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ ".Hidden.txt", "x");

	//Linux hides dot files.  Windows uses the hidden attribute.
#ifdef _WIN32
	SetFileAttributes(FILE_UTIL_TEST_DIR _FS_ ".HiddenDir", FILE_ATTRIBUTE_HIDDEN);
	SetFileAttributes(FILE_UTIL_TEST_DIR _FS_ ".Hidden.txt", FILE_ATTRIBUTE_HIDDEN);
#endif // _WIN32

	aszFiles.Init();
	AssertTrue(cFileUtil.FindAllFiles(FILE_UTIL_TEST_DIR, &aszFiles, false, false));
	AssertSize(1, aszFiles.NumElements());
	AssertSize(1, TestFileUtilCountEndingWith(&aszFiles, "Shown.txt"));
	aszFiles.Kill();

	aszFiles.Init();
	AssertTrue(cFileUtil.FindAllFiles(FILE_UTIL_TEST_DIR, &aszFiles, false, true));
	AssertSize(2, aszFiles.NumElements());
	AssertSize(1, TestFileUtilCountEndingWith(&aszFiles, ".Hidden.txt"));
	aszFiles.Kill();

	aszFiles.Init();
	cFileUtil.FindAllDirectories(FILE_UTIL_TEST_DIR, &aszFiles, false);
	AssertSize(1, aszFiles.NumElements());
	AssertSize(1, TestFileUtilCountEndingWith(&aszFiles, "Visible"));
	aszFiles.Kill();

	aszFiles.Init();
	cFileUtil.FindAllDirectories(FILE_UTIL_TEST_DIR, &aszFiles, true);
	AssertSize(2, aszFiles.NumElements());
	AssertSize(1, TestFileUtilCountEndingWith(&aszFiles, ".HiddenDir"));
	aszFiles.Kill();

	//A directory that does not exist cannot be searched.
	aszFiles.Init();
	AssertFalse(cFileUtil.FindAllFiles(FILE_UTIL_TEST_DIR _FS_ "Missing", &aszFiles, false, false));
	AssertSize(0, aszFiles.NumElements());
	aszFiles.Kill();

#ifdef _WIN32
	SetFileAttributes(FILE_UTIL_TEST_DIR _FS_ ".HiddenDir", FILE_ATTRIBUTE_NORMAL);
	SetFileAttributes(FILE_UTIL_TEST_DIR _FS_ ".Hidden.txt", FILE_ATTRIBUTE_NORMAL);
#endif // _WIN32
	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilCopyDir(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "A.txt", "Hello");
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub" _FS_ "B.txt", "World!");

	AssertTrue(cFileUtil.MakeDir(FILE_UTIL_TEST_DIR _FS_ "Dest"));
	AssertTrue(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Source", FILE_UTIL_TEST_DIR _FS_ "Dest"));
	AssertLong(5, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "A.txt"));
	AssertLong(6, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Sub" _FS_ "B.txt"));

	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilIsDirectory(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Dir", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "File.txt", "x");

	AssertTrue(cFileUtil.IsDirectory(FILE_UTIL_TEST_DIR));
	AssertTrue(cFileUtil.IsDirectory(FILE_UTIL_TEST_DIR _FS_ "Dir"));
	AssertFalse(cFileUtil.IsDirectory(FILE_UTIL_TEST_DIR _FS_ "File.txt"));
	AssertFalse(cFileUtil.IsDirectory(FILE_UTIL_TEST_DIR _FS_ "Missing"));

	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilCopyDirCreatesDestination(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub" _FS_ "SubSub", false));
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Empty", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "A.txt", "Hello");
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub" _FS_ "SubSub" _FS_ "C.txt", "Deep down");

	//The destination and its parent do not exist yet.  A trailing separator on the source is allowed.
	AssertFalse(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Dest"));
	AssertTrue(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_, FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Deeper"));

	AssertTrue(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "A.txt", FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Deeper" _FS_ "A.txt"));
	AssertTrue(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub" _FS_ "SubSub" _FS_ "C.txt", FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Deeper" _FS_ "Sub" _FS_ "SubSub" _FS_ "C.txt"));
	AssertTrue(cFileUtil.IsDirectory(FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Deeper" _FS_ "Empty"));

	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilCopyDirOverExisting(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "A.txt", "Hello");
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub" _FS_ "B.txt", "World!");

	//Existing files are replaced, even longer ones.  Files only in the destination are kept.
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Sub", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "A.txt", "An older and much longer file");
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Sub" _FS_ "B.txt", "Old");
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Extra.txt", "Extra");

	AssertTrue(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Source", FILE_UTIL_TEST_DIR _FS_ "Dest"));
	AssertTrue(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "A.txt", FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "A.txt"));
	AssertTrue(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub" _FS_ "B.txt", FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Sub" _FS_ "B.txt"));
	AssertLong(5, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "Dest" _FS_ "Extra.txt"));

	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilCopyDirFailures(void)
{
	CFileUtil	cFileUtil;
	SLogConfig	sLogConfig;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "A.txt", "Hello");
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "NotADirectory.txt", "x");

	sLogConfig = gcLogger.SetSilent();

	//A missing source fails without creating the destination.
	AssertFalse(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Missing", FILE_UTIL_TEST_DIR _FS_ "Dest"));
	AssertFalse(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Dest"));

	//A file is not a directory.
	AssertFalse(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "NotADirectory.txt", FILE_UTIL_TEST_DIR _FS_ "Dest"));
	AssertFalse(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Dest"));

	//The destination cannot be the source or inside it.
	AssertFalse(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Source", FILE_UTIL_TEST_DIR _FS_ "Source"));
	AssertFalse(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Source", FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub" _FS_ "Copy"));
	AssertFalse(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Source" _FS_ "Sub" _FS_ "Copy"));

	//A destination with a similar name is not inside the source.
	AssertTrue(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Source", FILE_UTIL_TEST_DIR _FS_ "SourceCopy"));
	AssertTrue(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "SourceCopy" _FS_ "A.txt"));

	//The destination cannot be a file.
	AssertFalse(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Source", FILE_UTIL_TEST_DIR _FS_ "NotADirectory.txt"));

#ifdef LINUX_GNU
	//A file that cannot be read is reported.
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Locked", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Locked" _FS_ "Secret.txt", "x");
	chmod(FILE_UTIL_TEST_DIR _FS_ "Locked" _FS_ "Secret.txt", 0);
	AssertFalse(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Locked", FILE_UTIL_TEST_DIR _FS_ "LockedCopy"));
	chmod(FILE_UTIL_TEST_DIR _FS_ "Locked" _FS_ "Secret.txt", 0644);

	//A sub directory that cannot be read is reported.
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Hidden" _FS_ "Closed", false));
	chmod(FILE_UTIL_TEST_DIR _FS_ "Hidden" _FS_ "Closed", 0);
	AssertFalse(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Hidden", FILE_UTIL_TEST_DIR _FS_ "HiddenCopy"));
	chmod(FILE_UTIL_TEST_DIR _FS_ "Hidden" _FS_ "Closed", 0755);

	//A destination that cannot be written to is reported.
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "ReadOnly", false));
	chmod(FILE_UTIL_TEST_DIR _FS_ "ReadOnly", 0555);
	AssertFalse(cFileUtil.CopyDir(FILE_UTIL_TEST_DIR _FS_ "Source", FILE_UTIL_TEST_DIR _FS_ "ReadOnly"));
	chmod(FILE_UTIL_TEST_DIR _FS_ "ReadOnly", 0755);
#endif // LINUX_GNU

	gcLogger.SetConfig(&sLogConfig);

	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilWriteLargeFile(const char* szFilename, lsize iSize)
{
	FILE*	pFile;
	lsize	i;

	pFile = fopen(szFilename, "wb");
	for (i = 0; i < iSize; i++)
	{
		fputc((int)((i * 7 + i / 251) & 0xff), pFile);
	}
	fclose(pFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilCopyAndCompare(void)
{
	CFileUtil	cFileUtil;
	lsize		iLarge;
	FILE*		pFile;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR, false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Short.txt", "Short");
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Shirt.txt", "Shirt");
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Long.txt", "A much longer file");

	//A missing source cannot be copied and leaves the destination alone.
	AssertFalse(cFileUtil.Copy(FILE_UTIL_TEST_DIR _FS_ "Missing.txt", FILE_UTIL_TEST_DIR _FS_ "Long.txt"));
	AssertLong(18, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "Long.txt"));

	//Copying over a longer file leaves nothing of it.
	AssertTrue(cFileUtil.Copy(FILE_UTIL_TEST_DIR _FS_ "Short.txt", FILE_UTIL_TEST_DIR _FS_ "Long.txt"));
	AssertLong(5, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "Long.txt"));
	AssertTrue(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Short.txt", FILE_UTIL_TEST_DIR _FS_ "Long.txt"));

	//Two missing files are the same.  One missing file is different.
	AssertTrue(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Missing1.txt", FILE_UTIL_TEST_DIR _FS_ "Missing2.txt"));
	AssertFalse(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Short.txt", FILE_UTIL_TEST_DIR _FS_ "Missing.txt"));
	AssertFalse(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Missing.txt", FILE_UTIL_TEST_DIR _FS_ "Short.txt"));

	//The same size with different contents.
	AssertFalse(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Short.txt", FILE_UTIL_TEST_DIR _FS_ "Shirt.txt"));
	AssertTrue(cFileUtil.CompareSize(FILE_UTIL_TEST_DIR _FS_ "Short.txt", FILE_UTIL_TEST_DIR _FS_ "Shirt.txt"));

	//A file bigger than one copy block, with a partial last block.
	iLarge = FILE_BLOCK_SIZE * 2 + 1000;
	TestFileUtilWriteLargeFile(FILE_UTIL_TEST_DIR _FS_ "Large.bin", iLarge);
	AssertTrue(cFileUtil.Copy(FILE_UTIL_TEST_DIR _FS_ "Large.bin", FILE_UTIL_TEST_DIR _FS_ "LargeCopy.bin"));
	AssertLong(iLarge, cFileUtil.Size(FILE_UTIL_TEST_DIR _FS_ "LargeCopy.bin"));
	AssertTrue(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Large.bin", FILE_UTIL_TEST_DIR _FS_ "LargeCopy.bin"));

	//A difference in the last partial block is found.
	pFile = fopen(FILE_UTIL_TEST_DIR _FS_ "LargeCopy.bin", "r+b");
	fseek(pFile, (long)(iLarge - 1), SEEK_SET);
	fputc(0x5a ^ fgetc(pFile), pFile);
	fclose(pFile);
	AssertFalse(cFileUtil.Compare(FILE_UTIL_TEST_DIR _FS_ "Large.bin", FILE_UTIL_TEST_DIR _FS_ "LargeCopy.bin"));

	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtilRemoveDir(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR);

	//A directory that does not exist is already removed.
	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR _FS_ "Missing"));

	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Tree" _FS_ "Sub", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Tree" _FS_ "Sub" _FS_ "File.txt", "x");
	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR _FS_ "Tree"));
	AssertFalse(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Tree"));

#ifdef LINUX_GNU
	//A symbolic link to a directory is removed, not followed into.
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Target", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Target" _FS_ "Keep.txt", "x");
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Linked", false));
	AssertInt(0, symlink(".." _FS_ "Target", FILE_UTIL_TEST_DIR _FS_ "Linked" _FS_ "Link"));
	AssertInt(0, symlink("Nowhere", FILE_UTIL_TEST_DIR _FS_ "Linked" _FS_ "Broken"));
	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR _FS_ "Linked"));
	AssertFalse(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Linked"));
	AssertTrue(cFileUtil.Exists(FILE_UTIL_TEST_DIR _FS_ "Target" _FS_ "Keep.txt"));

	//A file that cannot be deleted is reported.
	AssertTrue(cFileUtil.TouchDir(FILE_UTIL_TEST_DIR _FS_ "Locked", false));
	TestFileUtilWriteFile(FILE_UTIL_TEST_DIR _FS_ "Locked" _FS_ "File.txt", "x");
	chmod(FILE_UTIL_TEST_DIR _FS_ "Locked", 0555);
	AssertFalse(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR _FS_ "Locked"));
	chmod(FILE_UTIL_TEST_DIR _FS_ "Locked", 0755);
#endif // LINUX_GNU

	AssertTrue(cFileUtil.RemoveDir(FILE_UTIL_TEST_DIR));
	AssertFalse(cFileUtil.Exists(FILE_UTIL_TEST_DIR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileUtil(void)
{
	BeginTests();
	MemoryInit();

	TestFileUtilRemoveExtension();
	TestFileUtilFindExtension();
	TestFileUtilNaming();
	TestFileUtilMisc();
	TestFileUtilMakeNameFromDirectory();
	TestFileUtilMakeDirectory();
	TestFileUtilRemoveLastFromPath();
	TestFileUtilTouchDirectory();
	TestFileUtilTouchDirectoryOnlyFilename();
	TestFileUtilPrependToPath();
	TestFileUtilRootDirectory();
	TestFileUtilTouchExisting();
	TestFileUtilExistsAndSize();
	TestFileUtilFindHidden();
	TestFileUtilIsDirectory();
	TestFileUtilCopyDir();
	TestFileUtilCopyDirCreatesDestination();
	TestFileUtilCopyDirOverExisting();
	TestFileUtilCopyDirFailures();
	TestFileUtilCopyAndCompare();
	TestFileUtilRemoveDir();

	MemoryKill();
	TestStatistics();
}

