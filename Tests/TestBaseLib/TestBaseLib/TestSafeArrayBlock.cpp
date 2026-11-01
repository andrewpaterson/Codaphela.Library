#include "BaseLib/GlobalMemory.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/SafeArrayBlock.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSafeArrayBlockAdd(void)
{
	CSafeArrayBlock	cArray;
	int				i;
	int				iValue;

	cArray.Init(sizeof(int));
	AssertTrue(cArray.IsEmpty());

	for (i = 0; i < 6; i++)
	{
		cArray.Add(&i);
	}
	AssertSize(6, cArray.NumElements());
	AssertFalse(cArray.IsEmpty());

	for (i = 0; i < 6; i++)
	{
		iValue = -1;
		AssertTrue(cArray.Get(i, &iValue));
		AssertInt(i, iValue);
	}

	iValue = -1;
	AssertFalse(cArray.Get(6, &iValue));
	AssertInt(-1, iValue);

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSafeArrayBlockRemoveRange(void)
{
	CSafeArrayBlock	cArray;
	size			i;
	char			c;

	cArray.Init(1);
	for (i = 0; i < 6; i++)
	{
		cArray.Add((char*)&i);
	}

	cArray.RemoveRange(1, 3, true);

	AssertSize(4, cArray.NumElements());

	AssertTrue(cArray.Get(0, &c));
	AssertChar(0, c);
	AssertTrue(cArray.Get(1, &c));
	AssertChar(3, c);
	AssertTrue(cArray.Get(2, &c));
	AssertChar(4, c);
	AssertTrue(cArray.Get(3, &c));
	AssertChar(5, c);
	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSafeArrayBlockWrite(void)
{
	CSafeArrayBlock	cArray;
	int				i;
	int				iValue;
	CFileBasic		cFile;
	CDiskFile*		pcDisk;
	CFileUtil		cFileUtil;
	char			szDirectory[] = "Output" _FS_ "SafeArrayBlock";
	char			szFile[] = "Output" _FS_ "SafeArrayBlock" _FS_ "Test.DAT";
	bool			b;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	cArray.Init(sizeof(int));
	for (i = 0; i < 3; i++)
	{
		iValue = (i + 1) * 7;
		cArray.Add(&iValue);
	}
	AssertSize(3, cArray.NumElements());

	pcDisk = DiskFile(szFile);
	AssertTrue(pcDisk->Open(EFM_Write_Create));
	cFile.Init(pcDisk);
	AssertTrue(cFile.WriteBool(true));
	AssertTrue(cArray.Write(&cFile));
	AssertTrue(pcDisk->Close());
	cFile.Kill();
	cArray.Kill();

	pcDisk = DiskFile(szFile);
	AssertTrue(pcDisk->Open(EFM_Read));
	cFile.Init(pcDisk);
	AssertTrue(cFile.ReadBool(&b));
	AssertTrue(cArray.Read(&cFile));
	AssertTrue(pcDisk->Close());
	cFile.Kill();

	AssertSize(3, cArray.NumElements());
	for (i = 0; i < 3; i++)
	{
		AssertTrue(cArray.Get(i, &iValue));
		AssertInt((i + 1) * 7, iValue);
	}
	cArray.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//	The tests of CSafeArrayBlock used by more than one thread are in TestThreadLib.
//
//////////////////////////////////////////////////////////////////////////
void TestSafeArrayBlock(void)
{
	BeginTests();
	MemoryInit();

	TestSafeArrayBlockAdd();
	TestSafeArrayBlockRemoveRange();
	TestSafeArrayBlockWrite();

	MemoryKill();
	TestStatistics();
}

