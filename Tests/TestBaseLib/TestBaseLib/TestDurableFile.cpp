#include "BaseLib/Logger.h"
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/DurableFile.h"
#include "BaseLib/DurableFileController.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* SetDurable(char* szRewrite, bool bDurable)
{
	if (bDurable)
	{
		return szRewrite;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileInit(bool bDurable)
{
	CDurableFile			cDurableFile;
	CFileUtil				cFileUtil;
	CDurableFileController	cController;
	char					szDirectory[] = "Output" _FS_ "Durable1";
	char					szRewrite[] = "Output" _FS_ "_Durable1";

	cFileUtil.MakeDirs(true, szDirectory, szRewrite, NULL);

	cController.Init(szDirectory, SetDurable(szRewrite, bDurable));
	cDurableFile.Init(&cController, "TestFile.txt", "_TestFile.txt");
	cController.Begin();

	cController.End();
	cDurableFile.Kill();
	cController.Kill();

	cFileUtil.RemoveDirs(szDirectory, szRewrite, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileWrite(bool bDurable)
{
	CFileUtil				cFileUtil;
	CDurableFile			cDurableFile;
	filePos					iResult;
	CDurableFileController	cController;
	char					szDirectory[] = "Output" _FS_ "Durable2";
	char					szRewrite[] = "Output" _FS_ "_Durable2";

	char			sz1[] = "In the age of chaos";
	char			sz2[] = " two factions battled";
	char			sz3[] = "undys";
	char			sz4[] = "In the age of undys two factions battled";

	cFileUtil.MakeDirs(true, szDirectory, szRewrite, NULL);

	cController.Init(szDirectory, SetDurable(szRewrite, bDurable));
	cDurableFile.Init(&cController, "WrittenFile.txt", "_WrittenFile.txt");
	cController.Begin();

	AssertTrue(cController.IsBegun());
	AssertBool(bDurable, cController.IsDurable());
	AssertLong(0, cDurableFile.Tell());
	AssertLong(0, cDurableFile.Size());
	AssertSize(0, cDurableFile.GetNumWrites());

	iResult = cDurableFile.Write(sz1, strlen(sz1), 1);
	AssertLong(1, iResult);
	AssertLong(19, cDurableFile.Tell());
	AssertLong(19, cDurableFile.Size());
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(1, cDurableFile.GetNumWrites());

	iResult = cDurableFile.Write(sz2, strlen(sz2), 1);
	AssertLong(1, iResult);
	AssertLong(40, cDurableFile.Tell());
	AssertLong(40, cDurableFile.Size());
	AssertSize(1, cDurableFile.GetNumWrites());

	iResult = cDurableFile.Write(14, sz3, strlen(sz3), 1);
	AssertLong(1, iResult);
	AssertLong(19, cDurableFile.Tell());
	AssertLong(40, cDurableFile.Size());
	AssertSize(1, cDurableFile.GetNumWrites());
	AssertMemory(sz4, (char*)cDurableFile.GetWriteData(0), 40);

	cController.End();
	AssertFalse(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(0, cDurableFile.GetNumWrites());
	AssertFalse(cDurableFile.IsBegun());

	AssertFalse(cDurableFile.IsOpen());

	cDurableFile.Kill();
	cController.Kill();

	AssertFileString("Output" _FS_ "Durable2" _FS_ "WrittenFile.txt", sz4);

	cFileUtil.RemoveDirs(szDirectory, szRewrite, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileCloseOpen(bool bDurable)
{
	CFileUtil				cFileUtil;
	CDurableFile			cDurableFile;
	filePos					iResult;
	CDurableFileController	cController;
	char					szDirectory[] = "Output" _FS_ "Durable8";
	char					szRewrite[] = "Output" _FS_ "_Durable8";

	char			szA[] = "AAAA";
	char			szB[] = "BBBB";
	char			szDest[8] = "";

	cFileUtil.MakeDirs(true, szDirectory, szRewrite, NULL);

	cController.Init(szDirectory, SetDurable(szRewrite, bDurable));
	cDurableFile.Init(&cController, "WrittenFile.txt", "_WrittenFile.txt");
	cController.Begin();

	iResult = cDurableFile.Write(4, szA, 4, 1);
	AssertLong(1, iResult);
	AssertLong(8, cDurableFile.Tell());
	AssertLong(8, cDurableFile.Size());
	iResult = cDurableFile.Write(0, szB, 4, 1);

	cController.End();
	cDurableFile.Kill();

	cDurableFile.Init(&cController, "WrittenFile.txt", "_WrittenFile.txt");
	cController.Begin();

	AssertLong(8, cDurableFile.Size());
	iResult = cDurableFile.Read(szDest, 4, 2);
	AssertLong(2, iResult);
	AssertLong(8, cDurableFile.Tell());

	cController.End();
	cController.Kill();
	cDurableFile.Kill();

	cFileUtil.RemoveDirs(szDirectory, szRewrite, NULL);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileComplex(bool bDurable)
{
	CFileUtil				cFileUtil;
	CDurableFile			cDurableFile;
	filePos					iResult;
	CDurableFileController	cController;
	char					szDirectory[] = "Output" _FS_ "Durable3";
	char					szRewrite[] = "Output" _FS_ "_Durable3";

	char			szA[] = "AAAA";
	char			szB[] = "BBBB";
	char			szC[] = "CCDD";
	char			szE[] = "EEEE";
	char			szF[] = "FFFF";

	char			szDest[5];
	char			szAll[18];

	cFileUtil.MakeDirs(true, szDirectory, szRewrite, NULL);

	cController.Init(szDirectory, SetDurable(szRewrite, bDurable));
	cDurableFile.Init(&cController, "WrittenFile.txt", "_WrittenFile.txt");
	cController.Begin();

	iResult = cDurableFile.Write(4, szB, 2, 2);   // ----BBBB
	AssertLong(2, iResult);
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertLong(8, cDurableFile.Tell());
	AssertLong(8, cDurableFile.Size());
	AssertSize(1, cDurableFile.GetNumWrites());

	iResult = cDurableFile.Write(10, szC, 2, 2);  // ----bbbb--CCCC
	AssertLong(2, iResult);
	AssertLong(14, cDurableFile.Tell());
	AssertLong(14, cDurableFile.Size());
	AssertSize(2, cDurableFile.GetNumWrites());

	memset(szDest, 0, 5);
	iResult = cDurableFile.Read(8, szDest, 2, 2);
	AssertLong(2, iResult);
	AssertString("", szDest);
	AssertString("CC", &szDest[2]);
	AssertLong(12, cDurableFile.Tell());
	AssertLong(12, cDurableFile.Tell());
	AssertLong(14, cDurableFile.Size());
	AssertSize(2, cDurableFile.GetNumWrites());
	
	iResult = cDurableFile.Write(12, szE, 1, 4);  // ----bbbb--ccEEEE
	AssertLong(4, iResult);
	AssertLong(16, cDurableFile.Tell());
	AssertLong(16, cDurableFile.Size());
	AssertSize(2, cDurableFile.GetNumWrites());

	memset(szDest, 0, 5);
	iResult = cDurableFile.Read(14, szDest, 2, 2);
	AssertLong(1, iResult);
	AssertString("EE", szDest);
	AssertLong(16, cDurableFile.Tell());
	AssertLong(16, cDurableFile.Tell());
	AssertLong(16, cDurableFile.Size());
	AssertSize(2, cDurableFile.GetNumWrites());

	AssertSize(1, cController.NumFiles());
	AssertString("Output" _FS_ "Durable3" _FS_ "WrittenFile.txt", cDurableFile.GetFilename());

	cController.End();
	AssertFalse(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(0, cDurableFile.GetNumWrites());

	AssertSize(0, cController.NumFiles());

	cController.Begin();
	AssertString("Output" _FS_ "Durable3" _FS_ "WrittenFile.txt", cDurableFile.GetFilename());
	AssertLong(0, cDurableFile.Tell());
	AssertLong(16, cDurableFile.Size());
	AssertSize(0, cDurableFile.GetNumWrites());
	AssertFalse(cDurableFile.TestGetOpenedSinceBegin());

	AssertLong(0, cDurableFile.Tell());
	AssertLong(16, cDurableFile.Size());
	AssertLong(0, cDurableFile.Tell());
	AssertLong(16, cDurableFile.Size());

	iResult = cDurableFile.Write(0, szA, 2, 2);
	AssertLong(2, iResult);
	if (bDurable) AssertLong(4, cDurableFile.Tell());
	iResult = cDurableFile.Write(8, szC, 2, 1);

	memset(szAll, 0, 18);
	iResult = cDurableFile.Read(0, szAll, 4, 4);
	AssertLong(4, iResult);
	AssertString("AAAABBBBCCCCEEEE", szAll);

	memset(szAll, 0, 18);
	iResult = cDurableFile.Read(11, szAll, 3, 2);
	AssertLong(1, iResult);
	AssertString("CEEEE", szAll);

	memset(szAll, 0, 18);
	iResult = cDurableFile.Write(16, szF, 1, 1);
	iResult = cDurableFile.Read(11, szAll, 3, 2);
	AssertLong(2, iResult);
	AssertString("CEEEEF", szAll);

	cController.End();
	cDurableFile.Kill();
	cController.Kill();

	AssertFileString("Output" _FS_ "Durable3" _FS_ "WrittenFile.txt", "AAAABBBBCCCCEEEEF");

	if (bDurable) AssertFileString("Output" _FS_ "_Durable3" _FS_ "_WrittenFile.txt", "AAAABBBBCCCCEEEEF");

	cFileUtil.RemoveDirs(szDirectory, szRewrite, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileRead(bool bDurable)
{
	CFileUtil				cFileUtil;
	CDurableFile			cDurableFile;
	filePos					iResult;
	CDurableFileController	cController;
	CTextFile				cTextFile;
	char					szDirectory[] = "Output" _FS_ "Durable4";
	char					szRewrite[] = "Output" _FS_ "_Durable4";
	char					szDest[2048];

	cFileUtil.MakeDirs(true, szDirectory, szRewrite, NULL);

	cTextFile.Init("Sea lions, like CLions of the holy see");
	cTextFile.Write("Output" _FS_ "Durable4" _FS_ "ReadFile.txt");
	cTextFile.Write("Output" _FS_ "_Durable4" _FS_ "_ReadFile.txt");
	cTextFile.Kill();

	cController.Init(szDirectory, SetDurable(szRewrite, bDurable));
	cDurableFile.Init(&cController, "ReadFile.txt", "_ReadFile.txt");
	cController.Begin();
	AssertFalse(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(0, cController.NumFiles());

	memset(szDest, 0, 10);
	iResult = cDurableFile.Read(szDest, 9, 1);
	AssertLong(1, iResult);
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(0, cDurableFile.GetNumWrites());
	AssertSize(1, cController.NumFiles());
	AssertString("Sea lions", szDest);
	AssertLong(9, cDurableFile.Tell());
	AssertLong(38, cDurableFile.Size());

	memset(szDest, 0, 2);
	iResult = cDurableFile.Read(szDest, 1, 1);
	AssertLong(1, iResult);
	AssertString(",", szDest);
	AssertLong(10, cDurableFile.Tell());
	AssertLong(38, cDurableFile.Size());

	memset(szDest, 0, 7);
	iResult = cDurableFile.Read(16, szDest, 2, 3);
	AssertLong(3, iResult);
	AssertString("CLions", szDest);
	AssertLong(22, cDurableFile.Tell());
	AssertLong(38, cDurableFile.Size());

	memset(szDest, 0, 4);
	iResult = cDurableFile.Read(35, szDest, 3, 1);
	AssertLong(1, iResult);
	AssertString("see", szDest);
	AssertLong(38, cDurableFile.Tell());
	AssertLong(38, cDurableFile.Size());

	memset(szDest, 0, 5);
	iResult = cDurableFile.Read(35, szDest, 4, 1);
	AssertLong(0, iResult);
	AssertString("see", szDest);
	AssertLong(35, cDurableFile.Tell());
	AssertLong(38, cDurableFile.Size());

	cController.End();
	AssertFalse(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(0, cDurableFile.GetNumWrites());
	AssertSize(0, cController.NumFiles());

	cDurableFile.Kill();
	cController.Kill();

	cFileUtil.RemoveDirs(szDirectory, szRewrite, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileWriteRound2(bool bDurable)
{
	CFileUtil				cFileUtil;
	CDurableFile			cDurableFile;
	filePos					iResult;
	CDurableFileController	cController;
	char					szDirectory[] = "Output" _FS_ "Durable5";
	char					szRewrite[] = "Output" _FS_ "_Durable5";
	char					szFileName[] = "Output" _FS_ "Durable5" _FS_ "WrittenFile.txt";
	char					szRewriteName[] = "Output" _FS_ "_Durable5" _FS_ "_WrittenFile.txt";

	char					sz1[] = "Lord";
	char					sz2[] = " ";
	char					sz3[] = "Commander";
	char					sz4[] = "Mothball";
	int						iLen1 = (int)strlen(sz1);
	int						iLen2 = (int)strlen(sz2);
	int						iLen3 = (int)strlen(sz3);
	int						iLen4 = (int)strlen(sz4);

	cFileUtil.MakeDirs(true, szDirectory, szRewrite, NULL);

	cController.Init(szDirectory, SetDurable(szRewrite, bDurable));
	cDurableFile.Init(&cController, "WrittenFile.txt", "_WrittenFile.txt");
	cController.Begin();

	AssertFalse(cFileUtil.Exists(szFileName));
	AssertFalse(cFileUtil.Exists(szRewriteName));

	iResult = cDurableFile.Write(iLen1 + iLen2, sz3, 1, iLen3);
	AssertLong(9, iResult);
	AssertLong(14, cDurableFile.Tell());
	AssertLong(14, cDurableFile.Size());
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(1, cDurableFile.GetNumWrites());
	AssertFalse(cFileUtil.Exists(szFileName));
	AssertFalse(cFileUtil.Exists(szRewriteName));

	iResult = cDurableFile.Write(iLen1, sz2, 1, iLen2);
	AssertLong(1, iResult);
	AssertLong(5, cDurableFile.Tell());
	AssertLong(14, cDurableFile.Size());
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(1, cDurableFile.GetNumWrites());

	iResult = cDurableFile.Write(0, sz1, 1, iLen1);
	AssertLong(4, iResult);
	AssertLong(4, cDurableFile.Tell());
	AssertLong(14, cDurableFile.Size());
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(1, cDurableFile.GetNumWrites());

	iResult = cDurableFile.Write(iLen1 + iLen2 + iLen3 + iLen2, sz4, 1, iLen4);
	AssertLong(8, iResult);
	AssertLong(23, cDurableFile.Tell());
	AssertLong(23, cDurableFile.Size());
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(2, cDurableFile.GetNumWrites());

	iResult = cDurableFile.Write(iLen1 + iLen2 + iLen3, sz2, 1, iLen2);
	AssertLong(1, iResult);
	AssertLong(15, cDurableFile.Tell());
	AssertLong(23, cDurableFile.Size());
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(1, cDurableFile.GetNumWrites());
	AssertFalse(cFileUtil.Exists(szFileName));
	AssertFalse(cFileUtil.Exists(szRewriteName));

	cController.End();
	AssertTrue(cFileUtil.Exists(szFileName));
	AssertBool(bDurable, cFileUtil.Exists(szRewriteName));
	AssertFileString(szFileName, "Lord Commander Mothball");
	if (bDurable) AssertFileString(szRewriteName, "Lord Commander Mothball");

	cDurableFile.Kill();
	cController.Kill();

	cFileUtil.RemoveDirs(szDirectory, szRewrite, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileReadNonExistant(bool bDurable)
{
	CFileUtil				cFileUtil;
	CDurableFile			cDurableFile;
	filePos					iResult;
	CDurableFileController	cController;
	char					szDirectory[] = "Output" _FS_ "Durable6";
	char					szRewrite[] = "Output" _FS_ "_Durable6";
	char					szFileName[] = "Output" _FS_ "Durable6" _FS_ "ReadFile.txt";
	char					szRewriteName[] = "Output" _FS_ "_Durable6" _FS_ "_ReadFile.txt";
	char					szDest[2048];

	cFileUtil.MakeDirs(true, szDirectory, szRewrite, NULL);

	cController.Init(szDirectory, SetDurable(szRewrite, bDurable));
	cDurableFile.Init(&cController, "ReadFile.txt", "_ReadFile.txt");

	AssertFalse(cFileUtil.Exists(szFileName));
	AssertFalse(cFileUtil.Exists(szRewriteName));
	cController.Begin(&cDurableFile, NULL);
	AssertFalse(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(1, cController.NumFiles());

	memset(szDest, 0, 10);
	
	iResult = cDurableFile.Read(szDest, 9, 1);
	AssertLong(0, iResult);
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(0, cDurableFile.GetNumWrites());
	AssertSize(1, cController.NumFiles());
	AssertLong(0, cDurableFile.Tell());
	AssertLong(0, cDurableFile.Size());

	memset(szDest, 0, 10);

	iResult = cDurableFile.Read(EFSO_SET, 0, szDest, 9, 1);
	AssertLong(0, iResult);
	AssertTrue(cDurableFile.TestGetOpenedSinceBegin());
	AssertSize(0, cDurableFile.GetNumWrites());
	AssertSize(1, cController.NumFiles());
	AssertLong(0, cDurableFile.Tell());
	AssertLong(0, cDurableFile.Size());

	cController.End();

	cDurableFile.Kill();
	cController.Kill();

	cFileUtil.RemoveDirs(szDirectory, szRewrite, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFileNoController(void)
{
	CDurableFile	cDurableFile;
	char			szDirectory[] = "Output" _FS_ "Durable7";
	char			szRewrite[] = "Output" _FS_ "_Durable7";
	CFileUtil		cFileUtil;
	char			szDest[1024];
	SLogConfig		sLogConfig;
	bool			bResult;

	//Should probably actually test something...
	cFileUtil.MakeDirs(true, szDirectory, szRewrite, NULL);

	sLogConfig = gcLogger.SetSilent();
	bResult = cDurableFile.Init(NULL, "ReadFile.txt", "_ReadFile.txt");
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(bResult);

	cDurableFile.Read(szDest, 16, 1);

	cDurableFile.Commit();
	cDurableFile.Recommit();

	cDurableFile.Kill();

	cFileUtil.RemoveDirs(szDirectory, szRewrite, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableFile(void)
{
	TypeConverterInit();
	DataMemoryInit();
	BeginTests();
	
	TestDurableFileInit(true);
	TestDurableFileInit(false);
	TestDurableFileWrite(true);
	TestDurableFileWrite(false);
	TestDurableFileCloseOpen(true);
	TestDurableFileCloseOpen(false);
	TestDurableFileComplex(true);
	TestDurableFileComplex(false);
	TestDurableFileRead(true);
	TestDurableFileRead(false);
	TestDurableFileWriteRound2(true);
	TestDurableFileWriteRound2(false);
	TestDurableFileReadNonExistant(true);
	TestDurableFileReadNonExistant(false);
	TestDurableFileNoController();

	TestStatistics();
	DataMemoryKill();
	TypeConverterKill();
}

