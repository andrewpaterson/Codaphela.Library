#include "BaseLib/MD5HashFile.h"
#include "BaseLib/FileIO.h"
#include "BaseLib/MemoryFile.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMD5FileWrite(void)
{
	CFileBasic		cFile;
	CMD5HashFile	cHashFile;
	char			szFox[] = "The quick brown fox jumps over the lazy dog";
	uint8			ucFoxMD5[] = {0x9e, 0x10, 0x7d, 0x9d, 0x37, 0x2b, 0xb6, 0x82, 0x6b, 0xd8, 0x1d, 0x35, 0x42, 0xa4, 0x19, 0xd6};
	int32			i;
	char			szDest[128];
	int32				iFoxLen;
	filePos			iRead;
	char			szLorem[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
	uint8			ucLoremMD5[] = {0xfa, 0x5c, 0x89, 0xf3, 0xc8, 0x8b, 0x81, 0xbf, 0xd5, 0xe8, 0x21, 0xb0, 0x31, 0x65, 0x69, 0xaf};
	int32			iLoremLen;

	iFoxLen = (int32)strlen(szFox);
	iLoremLen = (int32)strlen(szLorem);

	cHashFile.Init(MemoryFile());
	cFile.Init(&cHashFile);
	cFile.Open(EFM_Write_Create);
	cFile.Write(szFox, 1, iFoxLen);
	cFile.Close();
	AssertMD5(ucFoxMD5, cHashFile.msMD5Context.digest);
	cFile.Kill();
	cHashFile.Kill();

	cHashFile.Init(MemoryFile());
	cFile.Init(&cHashFile);
	cFile.Open(EFM_Write_Create);
	for (i = 0; i < iFoxLen; i++)
	{
		cFile.Write(&szFox[i], 1, 1);
	}
	cFile.Close();
	AssertMD5(ucFoxMD5, cHashFile.msMD5Context.digest);

	cFile.Open(EFM_Read);
	iRead = cFile.Read(szDest, 1, iFoxLen+1);
	AssertLong(iFoxLen, iRead);
	AssertMD5(ucFoxMD5, cHashFile.msMD5Context.digest);

	cFile.Seek(0);

	iRead = cFile.Read(szDest, iFoxLen, 1);
	AssertLong(1, iRead);
	AssertMD5(ucFoxMD5, cHashFile.msMD5Context.digest);
	cFile.Close();
	cFile.Kill();
	cHashFile.Kill();

	cHashFile.Init(MemoryFile());
	cFile.Init(&cHashFile);
	cFile.Open(EFM_Write_Create);
	cFile.Write(szLorem, 1, iLoremLen);
	cFile.Close();
	AssertMD5(ucLoremMD5, cHashFile.msMD5Context.digest);
	cFile.Kill();
	cHashFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMD5File(void)
{
	BeginTests();

	TestMD5FileWrite();

	TestStatistics();
}

