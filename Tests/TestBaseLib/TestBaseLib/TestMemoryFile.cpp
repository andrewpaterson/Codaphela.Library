#include "BaseLib/MemoryFile.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryFileSimple(void)
{
	CMemoryFile		cMemoryFile;
	char			szHello[] = "Hello";
	char			szWorld[] = " World";
	bool			bResult;
	size			uiResult;
	char			szResult[100];

	cMemoryFile.Init(szHello, strlen(szHello) + 1);
	bResult = cMemoryFile.Open(EFM_Read);
	AssertTrue(bResult);

	uiResult = (int)cMemoryFile.Read(szResult, strlen(szHello) + 1, 1);
	AssertSize(1, uiResult);
	AssertString(szHello, szResult);
	cMemoryFile.Seek(0, EFSO_SET);
	uiResult = cMemoryFile.Read(szResult, 1, strlen(szHello) + 1);
	AssertSize(strlen(szHello) + 1, uiResult);
	AssertString(szHello, szResult);

	bResult = cMemoryFile.Close();
	AssertTrue(bResult);
	cMemoryFile.Kill();

	cMemoryFile.Init(szHello, strlen(szHello));
	bResult = cMemoryFile.Open(EFM_ReadWrite);
	AssertTrue(bResult);
	cMemoryFile.Seek(0, EFSO_END);
	uiResult = cMemoryFile.Write(szWorld, strlen(szWorld) + 1, 1);
	AssertSize(1, uiResult);
	AssertString("Hello World", (char*)cMemoryFile.GetBufferPointer());

	bResult = cMemoryFile.Close();
	AssertTrue(bResult);
	cMemoryFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryFileWriteSeekRewrite(void)
{
	CMemoryFile		cMemoryFile;
	char			szHello[] = "Hello";
	char			szWorld[] = " World";
	char			szBlurgle[] = " Blurgl";
	bool			bResult;
	size			uiResult;
	char			szResult[100];
	filePos			iFilePos;

	cMemoryFile.Init();
	bResult = cMemoryFile.Open(EFM_ReadWrite);
	AssertTrue(bResult);

	uiResult = cMemoryFile.Write(szHello, 1, strlen(szHello));
	AssertSize(5, uiResult);
	uiResult = cMemoryFile.Write(szWorld, 1, strlen(szWorld) + 1);
	AssertSize(7, uiResult);

	bResult = cMemoryFile.Seek(0, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Read(szResult, 1, 100);
	AssertSize(12, uiResult);
	AssertString(szResult, "Hello World");
	
	bResult = cMemoryFile.Seek(5, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Write("@", 1, 1);
	AssertSize(1, uiResult);
	bResult = cMemoryFile.Seek(0, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Read(szResult, 1, 100);
	AssertSize(12, uiResult);
	AssertString("Hello@World", szResult);

	bResult = cMemoryFile.Seek(5, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Write(" Blurgl", 1, strlen(szBlurgle) + 1);
	AssertSize(8, uiResult);
	bResult = cMemoryFile.Seek(0, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Read(szResult, 1, 100);
	AssertSize(13, uiResult);
	AssertString(szResult, "Hello Blurgl");

	bResult = cMemoryFile.Close();
	AssertTrue(bResult);

	bResult = cMemoryFile.Open(EFM_ReadWrite);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Read(szResult, 1, 100);
	AssertSize(13, uiResult);
	AssertString(szResult, "Hello Blurgl");

	uiResult = cMemoryFile.Write(szWorld, 1, strlen(szWorld) + 1);
	AssertSize(7, uiResult);
	bResult = cMemoryFile.Seek(12, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Write("@", 1, 1);
	AssertSize(1, uiResult);
	uiResult = cMemoryFile.Read(szResult, 1, 100);
	AssertSize(7, uiResult);
	AssertString(szResult, " World");

	bResult = cMemoryFile.Seek(0, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Read(szResult, 1, 100);
	AssertSize(20, uiResult);
	AssertString(szResult, "Hello Blurgl@ World");

	iFilePos = cMemoryFile.Size();
	AssertLong(20LL, iFilePos);

	bResult = cMemoryFile.Seek(19, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Write("!", 1, 1);
	AssertSize(1, uiResult);
	uiResult = cMemoryFile.Write("", 1, 1);
	AssertSize(1, uiResult);
	bResult = cMemoryFile.Seek(0, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Read(szResult, 1, 100);
	AssertSize(21, uiResult);
	AssertString(szResult, "Hello Blurgl@ World!");
	iFilePos = cMemoryFile.Tell();
	AssertLong(21LL, iFilePos);

	iFilePos = cMemoryFile.Size();
	AssertLong(21LL, iFilePos);

	bResult = cMemoryFile.Seek(-12, EFSO_END);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Write("", 1, 1);
	AssertSize(1, uiResult);
	bResult = cMemoryFile.Seek(0, EFSO_SET);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Read(szResult, 1, 100);
	AssertSize(21, uiResult);
	AssertString(szResult, "Hello Blu");

	bResult = cMemoryFile.Truncate(10);
	AssertTrue(bResult);
	iFilePos = cMemoryFile.Size();
	AssertLong(10LL, iFilePos);

	bResult = cMemoryFile.Seek(-12, EFSO_END);
	AssertTrue(bResult);
	uiResult = cMemoryFile.Read(szResult, 1, 100);
	AssertSize(10, uiResult);
	AssertString(szResult, "Hello Blu");

	bResult = cMemoryFile.Close();
	AssertTrue(bResult);
	cMemoryFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryFile(void)
{
	BeginTests();

	TestMemoryFileSimple();
	TestMemoryFileWriteSeekRewrite();

	TestStatistics();
}

