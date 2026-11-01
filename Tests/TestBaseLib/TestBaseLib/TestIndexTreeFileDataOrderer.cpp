#include "BaseLib/AccessDataOrderer.h"
#include "BaseLib/ModificationDataOrderer.h"
#include "BaseLib/CreationDataOrderer.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/IndexTreeHelper.h"
#include "BaseLib/IndexTreeFile.h"
#include "BaseLib/IndexTreeFileAccess.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintOrdererString(CIndexTreeDataOrderer* pcOrderer, CChars* psz)
{
	SDataOrderIterator		sIter;
	char					szData[MAX_DATA_SIZE];
	uint8					szKey[MAX_KEY_SIZE];
	size					iDataSize;
	size					iKeySize;
	bool					bExists;

	bExists = pcOrderer->StartIteration(&sIter, szKey, &iKeySize, MAX_KEY_SIZE, szData, &iDataSize, MAX_DATA_SIZE);
	while (bExists)
	{
		szData[iDataSize] = '\0';
		psz->Append(szData);
		bExists = pcOrderer->Iterate(&sIter, szKey, &iKeySize, MAX_KEY_SIZE, szData, &iDataSize, MAX_DATA_SIZE);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpDataOrdererString(CIndexTreeDataOrderer* pcOrderer)
{
	CChars	sz;

	if (pcOrderer)
	{
		sz.Init();
		PrintOrdererString(pcOrderer, &sz);
		sz.AppendNewLine();
		sz.Dump();
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertIndexTreeFileDataOrdererString(CIndexTreeDataOrderer* pcOrderer, const char* szExpected)
{
	CChars	sz;

	sz.Init();
	PrintOrdererString(pcOrderer, &sz);
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileDataOrdererAccess(EIndexWriteThrough eWriteThrough)
{
	CIndexTreeFile					cIndexTree;
	CIndexTreeFileAccess			cAccess;
	CAccessDataOrderer				cOrderer;
	SIndexTreeFileUnsafeIterator	sIter;
	bool							bExists;
	CIndexTreeHelper				cHelper;
	CDurableFileController			cController;

	cHelper.Init("Output" _FS_"IndexTreeDataOrderer1", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cOrderer.Init();
	cIndexTree.Init(&cController, "Index", eWriteThrough, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);

	AssertIndexTreeFileDataOrdererString(&cOrderer, "");

	cAccess.PutStringChar("0000", 'a');
	cAccess.PutStringChar("0100", 'b');
	cAccess.PutStringChar("0010", 'c');
	cAccess.PutStringChar("0110", 'd');
	AssertIndexTreeFileDataOrdererString(&cOrderer, "dcba");

	cAccess.GetStringChar("0000");
	AssertIndexTreeFileDataOrdererString(&cOrderer, "adcb");

	cAccess.GetStringChar("0010");
	AssertIndexTreeFileDataOrdererString(&cOrderer, "cadb");

	cAccess.DeleteString("0100");
	cAccess.DeleteString("0110");
	AssertIndexTreeFileDataOrdererString(&cOrderer, "ca");

	cAccess.DeleteString("0010");
	cAccess.DeleteString("0000");
	AssertIndexTreeFileDataOrdererString(&cOrderer, "");

	cAccess.PutStringChar("0000", 'a');
	cAccess.PutStringChar("0100", 'b');
	cAccess.PutStringChar("0010", 'c');
	cAccess.PutStringChar("0110", 'd');
	AssertIndexTreeFileDataOrdererString(&cOrderer, "dcba");

	cAccess.HasString("0100");
	AssertIndexTreeFileDataOrdererString(&cOrderer, "bdca");

	cAccess.HasString("0100");
	AssertIndexTreeFileDataOrdererString(&cOrderer, "bdca");

	cAccess.HasString("0110");
	AssertIndexTreeFileDataOrdererString(&cOrderer, "dbca");

	bExists = cIndexTree.StartUnsafeIteration(&sIter, NULL, NULL);
	AssertTrue(bExists);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "adbc");
	bExists = cIndexTree.UnsafeIterate(&sIter, NULL, NULL);
	AssertTrue(bExists);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "cadb");
	bExists = cIndexTree.UnsafeIterate(&sIter, NULL, NULL);
	AssertTrue(bExists);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "bcad");
	bExists = cIndexTree.UnsafeIterate(&sIter, NULL, NULL);
	AssertTrue(bExists);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "dbca");
	bExists = cIndexTree.UnsafeIterate(&sIter, NULL, NULL);
	AssertFalse(bExists);

	cAccess.Flush();
	cController.End();

	cIndexTree.Kill();
	cAccess.Kill();
	cOrderer.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileDataOrdererRemapListNodes(EIndexWriteThrough eWriteThrough)
{
	CIndexTreeFile				cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CAccessDataOrderer			cOrderer;
	size						i;
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;

	cHelper.Init("Output" _FS_"IndexTreeDataOrderer2", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cOrderer.Init();
	cIndexTree.Init(&cController, "Index", eWriteThrough, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);

	AssertIndexTreeFileDataOrdererString(&cOrderer, "");

	i = 61;
	do
	{
		i--;
		cAccess.PutIntLong((int32)i << 8, i);
	}
	while (i != 0);
	i = 61;
	do
	{
		i--;
		cAccess.PutIntChar((int32)i << 8, 'A' + (char)i);
	}
	while (i != 0);

	AssertIndexTreeFileDataOrdererString(&cOrderer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}");

	for (i = 0; i <= 60; i += 2)
	{
		AssertTrue(cAccess.HasInt((int32)i << 8));
	}
	AssertIndexTreeFileDataOrdererString(&cOrderer, "}{ywusqomkigeca_][YWUSQOMKIGECABDFHJLNPRTVXZ\\^`bdfhjlnprtvxz|");

	for (i = 0; i <= 60; i++)
	{
		cAccess.PutIntLong((int32)i << 8, i);
	}
	for (i = 0; i <= 60; i++)
	{
		cAccess.PutIntChar((int32)i << 8, 'A' + (char)i);
	}
	AssertIndexTreeFileDataOrdererString(&cOrderer, "}|{zyxwvutsrqponmlkjihgfedcba`_^]\\[ZYXWVUTSRQPONMLKJIHGFEDCBA");

	i = 62;
	do
	{
		i -= 2;
		AssertTrue(cAccess.HasInt((int32)i << 8));
	}
	while (i != 0);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "ACEGIKMOQSUWY[]_acegikmoqsuwy{}|zxvtrpnljhfdb`^\\ZXVTRPNLJHFDB");

	cAccess.Flush();
	cController.End();

	cIndexTree.Kill();
	cAccess.Kill();
	cOrderer.Kill();
	cController.Kill();

	cHelper.Kill(true);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileDataOrdererModification(EIndexWriteThrough eWriteThrough)
{
	CIndexTreeFile				cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CModificationDataOrderer	cOrderer;
	size						i;
	size						iKey;
	char						c;
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;

	cHelper.Init("Output" _FS_"IndexTreeDataOrderer3", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cOrderer.Init();
	cIndexTree.Init(&cController, "Index", eWriteThrough, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);


	AssertIndexTreeFileDataOrdererString(&cOrderer, "");

	i = 26;
	do
	{
		i--;
		iKey = i * 0b1010101010101010101010101010;
		cAccess.PutIntChar((int32)iKey, 'A' + (char)i);
	}
	while (i != 0);

	AssertIndexTreeFileDataOrdererString(&cOrderer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	for (i = 0; i <= 25; i++)
	{
		iKey = i * 0b1010101010101010101010101010;
		c = cAccess.GetIntChar((int32)iKey);
		AssertSize('A' + i, c)
	}
	AssertIndexTreeFileDataOrdererString(&cOrderer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	for (i = 0; i <= 13; i++)
	{
		iKey = i * 0b1010101010101010101010101010;
		cAccess.PutIntChar((int32)iKey, 'a' + (char)i);
	}
	AssertIndexTreeFileDataOrdererString(&cOrderer, "nmlkjihgfedcbaOPQRSTUVWXYZ");

	for (i = 0; i <= 25; i += 2)
	{
		iKey = i * 0b1010101010101010101010101010;
		AssertTrue(cAccess.HasInt((int32)iKey));
	}
	AssertIndexTreeFileDataOrdererString(&cOrderer, "nmlkjihgfedcbaOPQRSTUVWXYZ");

	for (i = 14; i <= 25; i++)
	{
		iKey = i * 0b1010101010101010101010101010;
		cAccess.PutIntChar((int32)iKey, 'a' + (char)i);
	}
	AssertIndexTreeFileDataOrdererString(&cOrderer, "zyxwvutsrqponmlkjihgfedcba");

	cAccess.Flush();
	cController.End();

	cIndexTree.Kill();
	cAccess.Kill();
	cOrderer.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileDataOrdererCreation(EIndexWriteThrough eWriteThrough)
{
	CIndexTreeFile				cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CCreationDataOrderer		cOrderer;
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;

	cHelper.Init("Output" _FS_"IndexTreeDataOrderer4", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cOrderer.Init();
	cIndexTree.Init(&cController, "Index", eWriteThrough, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);


	AssertIndexTreeFileDataOrdererString(&cOrderer, "");

	cAccess.PutStringChar("0000", 'a');
	cAccess.PutStringChar("0100", 'b');
	cAccess.PutStringChar("0010", 'c');
	cAccess.PutStringChar("0110", 'd');
	AssertIndexTreeFileDataOrdererString(&cOrderer, "dcba");

	cAccess.PutStringChar("0100", 'x');
	cAccess.PutStringChar("0010", 'y');
	AssertIndexTreeFileDataOrdererString(&cOrderer, "dyxa");

	cAccess.Flush();
	cController.End();

	cIndexTree.Kill();
	cAccess.Kill();
	cOrderer.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileDataOrdererEviction(EIndexWriteThrough eWriteThrough)
{
	CIndexTreeFile				cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CAccessDataOrderer			cOrderer;
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	size						i;
	char						c;

	cHelper.Init("Output" _FS_"IndexTreeDataOrderer4", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cOrderer.Init();
	cIndexTree.Init(&cController, "Index", eWriteThrough, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);


	AssertIndexTreeFileDataOrdererString(&cOrderer, "");

	i = 26;
	do
	{
		i--;
		cAccess.PutIntChar((int32)i, 'a' + (char)i);
	}
	while (i != 0);

	AssertIndexTreeFileDataOrdererString(&cOrderer, "abcdefghijklmnopqrstuvwxyz");

	cAccess.EvictInt(12);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "abcdefghijklnopqrstuvwxyz");

	cAccess.EvictInt(13);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "abcdefghijklopqrstuvwxyz");

	c = cAccess.GetIntChar(12);
	AssertChar('m', c);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "mabcdefghijklopqrstuvwxyz");

	c = cAccess.GetIntChar(25);
	AssertChar('z', c);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "zmabcdefghijklopqrstuvwxy");

	c = cAccess.GetIntChar(13);
	AssertChar('n', c);
	AssertIndexTreeFileDataOrdererString(&cOrderer, "nzmabcdefghijklopqrstuvwxy");

	cAccess.Flush();
	cController.End();

	cIndexTree.Kill();
	cAccess.Kill();
	cOrderer.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileDataOrderer(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestIndexTreeFileDataOrdererAccess(IWT_No);
	TestIndexTreeFileDataOrdererAccess(IWT_Yes);
	TestIndexTreeFileDataOrdererRemapListNodes(IWT_No);
	TestIndexTreeFileDataOrdererRemapListNodes(IWT_Yes);
	TestIndexTreeFileDataOrdererModification(IWT_No);
	TestIndexTreeFileDataOrdererModification(IWT_Yes);
	TestIndexTreeFileDataOrdererCreation(IWT_No);
	TestIndexTreeFileDataOrdererCreation(IWT_Yes);
	TestIndexTreeFileDataOrdererEviction(IWT_No);
	TestIndexTreeFileDataOrdererEviction(IWT_Yes);

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

