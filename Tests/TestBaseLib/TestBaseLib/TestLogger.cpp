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
// BASELIB_BREAK's values, and what each does to the logger's breaks: "0"
// turns both off, "1" both on, anything else leaves them as they were.
//
//////////////////////////////////////////////////////////////////////////
void TestLoggerBreakSwitch(void)
{
	SLogConfig	sSaved;
	SLogConfig	sConfig;

	AssertInt(BSW_AsBuilt, ParseBreakSwitch(NULL));
	AssertInt(BSW_AsBuilt, ParseBreakSwitch(""));
	AssertInt(BSW_AsBuilt, ParseBreakSwitch("2"));
	AssertInt(BSW_AsBuilt, ParseBreakSwitch("00"));
	AssertInt(BSW_Off, ParseBreakSwitch("0"));
	AssertInt(BSW_On, ParseBreakSwitch("1"));

	gcLogger.GetConfig(&sSaved);

	gcLogger.SetBreakOnError(true);
	gcLogger.SetBreakOnWarning(false);
	gcLogger.ApplyBreakSwitch(BSW_AsBuilt);
	gcLogger.GetConfig(&sConfig);
	AssertTrue(sConfig.bBreakOnError);
	AssertFalse(sConfig.bBreakOnWarning);

	gcLogger.ApplyBreakSwitch(BSW_Off);
	gcLogger.GetConfig(&sConfig);
	AssertFalse(sConfig.bBreakOnError);
	AssertFalse(sConfig.bBreakOnWarning);

	gcLogger.ApplyBreakSwitch(BSW_On);
	gcLogger.GetConfig(&sConfig);
	AssertTrue(sConfig.bBreakOnError);
	AssertTrue(sConfig.bBreakOnWarning);

	gcLogger.SetConfig(&sSaved);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogger(void)
{
	BeginTests();

	TestLoggerMemoryOutput();
	TestLoggerBreakSwitch();

	TestStatistics();
}

