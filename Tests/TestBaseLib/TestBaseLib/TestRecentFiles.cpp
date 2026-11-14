#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/RecentFiles.h"
#include "TestLib/Assert.h"


#define TEST_RECENT_FILE	"Output" _FS_ "Recent" _FS_ "RecentFiles.xml"


//////////////////////////////////////////////////////////////////////////
//
// The latest first, each once whatever its case, at most the number
// kept; written to a file (its directory made) and read back.
//
//////////////////////////////////////////////////////////////////////////
void TestRecentFilesList(void)
{
	CRecentFiles		cRecent;
	CRecentFiles		cRead;
	CFileUtil			cFileUtil;
	CTextFile			cFile;
	CChars				szFull;

	cFileUtil.RemoveDir("Output" _FS_ "Recent");
	cRecent.Init(3);
	AssertSize(0, cRecent.NumFiles());
	cRecent.Add("A.sln");
	cRecent.Add("B.sln");
	cRecent.Add("C.hcp65workspace");
	AssertSize(3, cRecent.NumFiles());
	AssertTrue(cRecent.Find("C.hcp65workspace") == 0);
	AssertTrue(cRecent.Find("A.sln") == 2);

	//The full path is kept.
	szFull.Init("C.hcp65workspace");
	cFileUtil.FullPath(&szFull);
	AssertString(szFull.Text(), cRecent.GetFile(0));
	szFull.Kill();

	//Opened again it is first again, and not twice.
	cRecent.Add("a.SLN");
	AssertSize(3, cRecent.NumFiles());
	AssertTrue(cRecent.Find("A.sln") == 0);
	AssertTrue(cRecent.Find("B.sln") == 2);

	//One more than are kept: the oldest goes.
	cRecent.Add("D.sln");
	AssertSize(3, cRecent.NumFiles());
	AssertTrue(cRecent.Find("D.sln") == 0);
	AssertTrue(cRecent.Find("B.sln") == MAX_SIZE);

	AssertTrue(cRecent.Remove("A.sln"));
	AssertFalse(cRecent.Remove("A.sln"));
	AssertSize(2, cRecent.NumFiles());

	//Written and read back, in order.
	AssertTrue(cRecent.Write(TEST_RECENT_FILE));
	cRead.Init(3);
	AssertTrue(cRead.Read(TEST_RECENT_FILE));
	AssertSize(2, cRead.NumFiles());
	AssertString(cRecent.GetFile(0), cRead.GetFile(0));
	AssertString(cRecent.GetFile(1), cRead.GetFile(1));

	//The file open when the app was closed is kept with them, and none is none.
	AssertString("", cRead.GetOpen());
	cRecent.SetOpen("Output/Kernal.hcp65workspace");
	AssertTrue(cRecent.Write(TEST_RECENT_FILE));
	AssertTrue(cRead.Read(TEST_RECENT_FILE));
	AssertString("Output/Kernal.hcp65workspace", cRead.GetOpen());
	AssertSize(2, cRead.NumFiles());
	cRecent.SetOpen(NULL);
	AssertTrue(cRecent.Write(TEST_RECENT_FILE));
	AssertTrue(cRead.Read(TEST_RECENT_FILE));
	AssertString("", cRead.GetOpen());

	//Cleared and written, there are none.
	cRecent.Clear();
	AssertSize(0, cRecent.NumFiles());
	AssertTrue(cRecent.Write(TEST_RECENT_FILE));
	AssertTrue(cRead.Read(TEST_RECENT_FILE));
	AssertSize(0, cRead.NumFiles());

	//No file, or not a list: none.
	cRead.Add("E.sln");
	AssertFalse(cRead.Read("Output" _FS_ "Recent" _FS_ "Missing.xml"));
	AssertSize(0, cRead.NumFiles());
	cFile.Init("<Workspace/>\n");
	AssertTrue(cFile.Write("Output" _FS_ "Recent" _FS_ "Other.xml"));
	cFile.Kill();
	AssertFalse(cRead.Read("Output" _FS_ "Recent" _FS_ "Other.xml"));

	cRead.Kill();
	cRecent.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRecentFiles(void)
{
	BeginTests();

	TestRecentFilesList();

	TestStatistics();
}

