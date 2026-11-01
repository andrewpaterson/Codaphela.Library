#include "BaseLib/CharsImmutable.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsImmutableReadAndWrite(void)
{
	CCharsImmutable		szImm;
	CMemoryFile*		pcMemoryFile;
	CFileBasic			cFile;
	CChars				szChars;
	size				i;
	CChars				szSub;

	MemoryInit();

	szChars.Init("0123456789ABCDEFGHIJK");

	pcMemoryFile = MemoryFile();
	cFile.Init(pcMemoryFile);
	cFile.Open(EFM_ReadWrite_Create);

	for (i = 0; i < szChars.Length(); i++)
	{
		szSub.Init(szChars, 0, i);
		szImm.Init(szChars.Text(), i);

		AssertString(szSub.Text(), szImm.Text());
		szImm.WriteString(&cFile);

		szImm.Kill();
		szSub.Kill();
	}

	cFile.Close();

	cFile.Open(EFM_Read);

	for (i = 0; i < szChars.Length(); i++)
	{
		szSub.Init(szChars, 0, i);

		szImm.ReadString(&cFile);
		AssertString(szSub.Text(), szImm.Text());

		szImm.Kill();
		szSub.Kill();
	}

	cFile.Close();
	
	cFile.Kill();
	szChars.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsImmutableSizeof(void)
{
	if (sizeof(int) == 4)
	{
#ifdef BUILD_64
		AssertSize(32, sizeof(CCharsImmutable));
#else
		AssertSize(16, sizeof(CCharsImmutable));
#endif // BUILD_64
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsImmutable(void)
{
	BeginTests();

	TestCharsImmutableReadAndWrite();
	TestCharsImmutableSizeof();

	TestStatistics();
}

