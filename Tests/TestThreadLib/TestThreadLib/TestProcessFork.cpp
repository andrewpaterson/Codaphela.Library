#include "BaseLib/NativeThread.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "ThreadLib/ProcessFork.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestProcessForkFile(char* szFileName)
{
	CFileUtil		cFileUtil;
	int				i;
	bool			bFileExists;
	CTextFile		cTextFile;

	cFileUtil.MakeDir("Output");
	cFileUtil.Delete(szFileName);
	AssertFalse(cFileUtil.Exists(szFileName));

	ForkProcess("--test-client-process", szFileName, false);

	bFileExists = false;
	for (i = 0; i < 50; i++)
	{
		ThreadSleep(16);
		bFileExists = cFileUtil.Exists(szFileName);
		if (bFileExists)
		{
			break;
		}
	}
	Pass();

	AssertTrue(bFileExists);

	AssertTrue(cTextFile.Init()->Read(szFileName));
	AssertString("Hello World\n", cTextFile.Text());
	cTextFile.Kill();
	Pass();

	cFileUtil.Delete(szFileName);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestProcessFork(void)
{
	BeginTests();

	TestProcessForkFile("Output" _FS_ "test-client-process.txt");

	//Each ForkProcess parameter is one argument even with spaces in it.
	TestProcessForkFile("Output" _FS_ "test client process.txt");

	TestStatistics();
}

