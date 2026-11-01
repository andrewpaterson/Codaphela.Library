#include "BaseLib/MemoryFile.h"
#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "TestLib/Assert.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLoggerMemoryOutput(void)
{
	CFileUtil		cFileUtil;
	CMemoryFile		cMemory;
	CFileBasic		cBasicMemory;
	char			sz[1024];

	gcLogger.Kill();

	cFileUtil.TouchDir("Output" _FS_ "Logger");
	gcLogger.Init("Output" _FS_ "Logger" _FS_ "Log.txt");
	cMemory.Init();
	gcLogger.AddOutput(&cMemory);

	gcLogger.Info("Hello");
	gcLogger.Info("World");

	gcLogger.RemoveOutput(&cMemory);

	cMemory.Open(EFM_Read);
	cBasicMemory.Init(&cMemory);
	memset(sz, 0, 1024);
	cBasicMemory.ReadStringChars(sz, (int)cBasicMemory.Size());
	cMemory.Close();
	cMemory.Kill();
	gcLogger.Kill();

	AssertString("INFO: Hello\n"
				 "INFO: World\n", sz);

	cFileUtil.RemoveDir("Output" _FS_ "Logger");

	gcLogger.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogger(void)
{
	BeginTests();

	TestLoggerMemoryOutput();

	TestStatistics();
}

