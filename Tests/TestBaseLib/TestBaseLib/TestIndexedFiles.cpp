#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/Logger.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/IndexedFiles.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertIndexedFile(char* szDirectory, const char* szFileName, void* pvExpectedData, size iExpectedLen)
{
	CChars		szTemp;
	CFileUtil	cFileUtil;

	szTemp.Init(szDirectory);
	cFileUtil.AppendToPath(&szTemp, szFileName);
	AssertTrue(cFileUtil.Exists(szTemp.Text()));
	AssertInt(0, CompareFileToMemory(szTemp.Text(), pvExpectedData, iExpectedLen));
	szTemp.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesWorkingDirectory(void)
{
	CIndexedFiles			cIndexedFiles;
	CFileUtil				cFileUtil;
	CDurableFileController	cController;
	char					szDirectorty[] = "Output" _FS_ "Files1";
	char					szRewriteDirectorty[] = "Output" _FS_ "_Files1";

	cFileUtil.RemoveDirs(szDirectorty, szRewriteDirectorty, NULL);

	cController.Init(szDirectorty, szRewriteDirectorty);
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexedFiles.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX");
	AssertTrue(cIndexedFiles.ReadIndexedFileDescriptors());
	AssertFalse(cFileUtil.Exists(szDirectorty));
	AssertFalse(cFileUtil.Exists(szRewriteDirectorty));

	cController.End();
	AssertTrue(cFileUtil.Exists(szDirectorty));
	AssertTrue(cFileUtil.Exists(szRewriteDirectorty));

	cIndexedFiles.Kill();
	cController.Kill();

	cFileUtil.RemoveDirs(szDirectorty, szRewriteDirectorty, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesInitAndKillWihtoutOpen(void)
{
	CIndexedFiles			cIndexedFiles;
	CFileUtil				cFileUtil;
	CDurableFileController	cController;
	char					szDirectorty[] = "Output" _FS_ "Files2";
	char					szRewriteDirectorty[] = "Output" _FS_ "_Files2";
	SLogConfig				sLogConfig;

	cFileUtil.RemoveDirs(szDirectorty, szRewriteDirectorty, NULL);

	cController.Init(szDirectorty, szRewriteDirectorty);
	cController.SetSync(DS_No);
	cIndexedFiles.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX");
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cIndexedFiles.ReadIndexedFileDescriptors());
	gcLogger.SetConfig(&sLogConfig);

	AssertFalse(cFileUtil.Exists(szDirectorty));
	AssertFalse(cFileUtil.Exists(szRewriteDirectorty));

	cIndexedFiles.Kill();
	cController.Kill();

	cFileUtil.RemoveDirs(szDirectorty, szRewriteDirectorty, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesWrite(void)
{
	CIndexedFiles			cIndexedFiles;
	CFileUtil				cFileUtil;
	CDurableFileController	cController;
	char					szDirectorty[] = "Output" _FS_ "Files3";
	char					szRewriteDirectorty[] = "Output" _FS_ "_Files3";
	char					szData1[] = "For the single case of just one bed of a certain type, you use bed directly with a text.";
	char					szData2[] = "Text's trusty Rights and Export Coordinator, Khadija Caffoor, went to the London Book Fair in March.";
	char					szData3[] = "Unpleasant nor diminution excellence apartments imprudence the met new.  Draw part them.";
	char					szData4[] = "Nor hence hoped he after her other known to defer his. For county sister seasonal better had waited.";
	size						iLen1;
	size						iLen2;
	size						iLen3;
	size						iLen4;
	CChars					szTemp;
	CFileDataIndex			cDataIndex;
	CFileDataIndex			cDataIndex1;
	CFileDataIndex			cDataIndex2;
	CFileDataIndex			cDataIndex3;
	CFileDataIndex			cDataIndex4;
	lsize			uiDataSize;
	char					szData[1024];
	SLogConfig				sLogConfig;

	iLen1 = strlen(szData1);
	iLen2 = strlen(szData2);
	AssertSize(88, iLen1);
	AssertSize(100, iLen2);

	cFileUtil.RemoveDirs(szDirectorty, szRewriteDirectorty, NULL);
	AssertFalse(cFileUtil.Exists(szDirectorty));
	AssertFalse(cFileUtil.Exists(szRewriteDirectorty));

	cController.Init(szDirectorty, szRewriteDirectorty);
	cController.SetSync(DS_No);
	cIndexedFiles.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX");

	AssertTrue(cController.Begin());
	AssertTrue(cIndexedFiles.ReadIndexedFileDescriptors());
	AssertLong(0, cIndexedFiles.NumDatas());

	cDataIndex1 = cIndexedFiles.WriteNew(szData1, iLen1);
	AssertTrue(cDataIndex1.HasFile());
	AssertInt((uint32)0, cDataIndex1.miFile);
	AssertLong(0, cDataIndex1.muiIndex);
	AssertLong(1, cIndexedFiles.NumDatas());

	cDataIndex2 = cIndexedFiles.WriteNew(szData2, iLen2);
	AssertTrue(cDataIndex2.HasFile());
	AssertInt((uint32)1, cDataIndex2.miFile);
	AssertLong(0, cDataIndex2.muiIndex);
	AssertLong(2, cIndexedFiles.NumDatas());
	
	AssertTrue(cController.End());
	AssertLong(2, cIndexedFiles.NumDatas());

	AssertTrue(cFileUtil.Exists(szDirectorty));
	AssertTrue(cFileUtil.Exists(szRewriteDirectorty));

	AssertIndexedFile(szDirectorty, "88_0.DAT", szData1, iLen1);
	AssertIndexedFile(szDirectorty, "100_0.DAT", szData2, iLen2);
	AssertIndexedFile(szRewriteDirectorty, "_88_0.DAT", szData1, iLen1);
	AssertIndexedFile(szRewriteDirectorty, "_100_0.DAT", szData2, iLen2);

	cIndexedFiles.Kill();
	cController.Kill();


	iLen3 = strlen(szData3);
	iLen4 = strlen(szData4);
	AssertSize(88, iLen3);
	AssertSize(100, iLen4);

	cController.Init(szDirectorty, szRewriteDirectorty);
	cController.SetSync(DS_No);
	cIndexedFiles.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX");

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cIndexedFiles.ReadIndexedFileDescriptors());  //Controller not begun.
	gcLogger.SetConfig(&sLogConfig);

	AssertTrue(cController.Begin());
	AssertTrue(cIndexedFiles.ReadIndexedFileDescriptors());
	AssertFalse(cIndexedFiles.ReadIndexedFileDescriptors());  //Indices already read.
	AssertTrue(cController.End());
	AssertLong(2, cIndexedFiles.NumDatas());

	AssertSize(2, cIndexedFiles.NumFiles());
	AssertLong(1, cIndexedFiles.NumDatas(88));
	AssertLong(0, cIndexedFiles.NumDatas(89));
	AssertLong(1, cIndexedFiles.NumDatas(100));
	AssertLong(0, cIndexedFiles.NumDatas(101));

	AssertTrue(cController.Begin());

	cDataIndex3 = cIndexedFiles.WriteNew(szData3, iLen3);
	AssertTrue(cDataIndex3.HasFile());
	AssertInt((uint32)0, cDataIndex3.miFile);
	AssertLong(1, cDataIndex3.muiIndex);
	AssertLong(3, cIndexedFiles.NumDatas());

	cDataIndex4 = cIndexedFiles.WriteNew(szData4, iLen4);
	AssertTrue(cDataIndex4.HasFile());
	AssertInt((uint32)1, cDataIndex4.miFile);
	AssertLong(1, cDataIndex4.muiIndex);
	AssertLong(4, cIndexedFiles.NumDatas());
	
	AssertTrue(cController.End());

	szTemp.InitList(szData1, szData3, NULL);
	AssertIndexedFile(szDirectorty, "88_0.DAT", szTemp.Text(), szTemp.Length());
	AssertIndexedFile(szRewriteDirectorty, "_88_0.DAT", szTemp.Text(), szTemp.Length());
	szTemp.Kill();
	szTemp.InitList(szData2, szData4, NULL);
	AssertIndexedFile(szDirectorty, "100_0.DAT", szTemp.Text(), szTemp.Length());
	AssertIndexedFile(szRewriteDirectorty, "_100_0.DAT", szTemp.Text(), szTemp.Length());
	cIndexedFiles.Kill();
	cController.Kill();
	szTemp.Kill();

	cController.Init(szDirectorty, szRewriteDirectorty);
	cController.SetSync(DS_No);
	cIndexedFiles.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX");
	AssertTrue(cController.Begin());
	cIndexedFiles.ReadIndexedFileDescriptors();
	AssertTrue(cController.End());

	AssertSize(2, cIndexedFiles.NumFiles());
	AssertLong(2, cIndexedFiles.NumDatas(88));
	AssertLong(0, cIndexedFiles.NumDatas(89));
	AssertLong(2, cIndexedFiles.NumDatas(100));
	AssertLong(0, cIndexedFiles.NumDatas(101));

	cIndexedFiles.Kill();
	cController.Kill();


	cController.Init(szDirectorty, szRewriteDirectorty);
	cController.SetSync(DS_No);
	cIndexedFiles.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX");
	AssertTrue(cController.Begin());
	AssertTrue(cIndexedFiles.ReadIndexedFileDescriptors());
	AssertTrue(cController.End());
	AssertLong(4, cIndexedFiles.NumDatas());

	cIndexedFiles.Kill();
	cController.Kill();

	cController.Init(szDirectorty, szRewriteDirectorty);
	cController.SetSync(DS_No);
	cIndexedFiles.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX");
	cController.Begin();
	cIndexedFiles.ReadIndexedFileDescriptors();

	uiDataSize = cIndexedFiles.GetFileDataSize(0);
	AssertLSize(88, uiDataSize);

	cDataIndex.Init(0, 0);
	memset(szData, 0, 1024);
	AssertTrue(cIndexedFiles.Read(&cDataIndex, szData));
	AssertString(szData1, szData);

	cDataIndex.Init(0, 1);
	memset(szData, 0, 1024);
	AssertTrue(cIndexedFiles.Read(&cDataIndex, szData));
	AssertString(szData3, szData);

	uiDataSize = cIndexedFiles.GetFileDataSize(1);
	AssertLSize(100, uiDataSize);

	cDataIndex.Init(1, 0);
	memset(szData, 0, 1024);
	AssertTrue(cIndexedFiles.Read(&cDataIndex, szData));
	AssertString(szData2, szData);

	cDataIndex.Init(1, 1);
	memset(szData, 0, 1024);
	AssertTrue(cIndexedFiles.Read(&cDataIndex, szData));
	AssertString(szData4, szData);

	AssertTrue(cController.End());
	cIndexedFiles.Kill();
	cController.Kill();

	cFileUtil.RemoveDirs(szDirectorty, szRewriteDirectorty, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFiles(void)
{
	TypeConverterInit();
	DataMemoryInit();
	BeginTests();

	TestIndexedFilesInitAndKillWihtoutOpen();
	TestIndexedFilesWorkingDirectory();
	TestIndexedFilesWrite();

	TestStatistics();
	DataMemoryKill();
	TypeConverterKill();
}

