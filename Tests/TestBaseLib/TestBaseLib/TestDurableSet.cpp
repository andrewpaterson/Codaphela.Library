#include "TestLib/Assert.h"
#include "BaseLib/DurableFile.h"
#include "BaseLib/DurableSet.h"
#include "BaseLib/DurableFileController.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/TypeConverter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableSetAdd(void)
{
	CFileUtil				cFileUtil;
	CDurableFileController	cController;
	CDurableFile			cDurableFile1;
	CDurableFile			cDurableFile2;
	CDurableFile			cDurableFile3;
	CDurableFile			cDurableFile4;
	char					szDirectory[] = "Output" _FS_ "ODurable2";
	char					szRewrite[] = "Output" _FS_ "_Durable2";
	CDurableFile*			pcDurableFile;

	cFileUtil.MakeDirs(true, szDirectory, szRewrite, NULL);

	cController.Init(szDirectory, szRewrite);

	cDurableFile1.Init(&cController, "Output" _FS_ "Durable2" _FS_ "1.txt", "Output" _FS_ "_Durable2" _FS_ "_1.txt");
	cDurableFile2.Init(&cController, "Output" _FS_ "Durable2" _FS_ "2.txt", "Output" _FS_ "_Durable2" _FS_ "_2.txt");
	cDurableFile3.Init(&cController, "Output" _FS_ "Durable2" _FS_ "3.txt", "Output" _FS_ "_Durable2" _FS_ "_3.txt");
	cDurableFile4.Init(&cController, "Output" _FS_ "Durable2" _FS_ "4.txt", "Output" _FS_ "_Durable2" _FS_ "_4.txt");
	cController.Begin();

	cController.AddFile(&cDurableFile1);
	AssertSize(1, cController.NumFiles());
	pcDurableFile = cController.GetFile(0);
	AssertPointer(&cDurableFile1, pcDurableFile);

	cController.AddFile(&cDurableFile1);
	AssertSize(1, cController.NumFiles());
	pcDurableFile = cController.GetFile(0);
	AssertPointer(&cDurableFile1, pcDurableFile);

	cController.AddFile(&cDurableFile2);
	AssertSize(2, cController.NumFiles());
	cController.AddFile(&cDurableFile3);
	AssertSize(3, cController.NumFiles());
	cController.AddFile(&cDurableFile4);
	AssertSize(4, cController.NumFiles());

	cController.AddFile(&cDurableFile2);
	AssertSize(4, cController.NumFiles());
	cController.AddFile(&cDurableFile3);
	AssertSize(4, cController.NumFiles());
	cController.AddFile(&cDurableFile4);
	AssertSize(4, cController.NumFiles());

	cController.End();
	cDurableFile1.Kill();
	cDurableFile2.Kill();
	cDurableFile3.Kill();
	cDurableFile4.Kill();
	cController.Kill();

	cFileUtil.RemoveDirs(szDirectory, szRewrite, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDurableSet(void)
{
	TypeConverterInit();
	BeginTests();

	TestDurableSetAdd();

	TestStatistics();
	TypeConverterKill();
}

