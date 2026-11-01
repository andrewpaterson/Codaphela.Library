#include "BaseLib/FileUtil.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/MemoryAllocator.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/MapStringString.h"
#include "BaseLib/Logger.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/IndexTreeHelper.h"
#include "BaseLib/IndexTreeFile.h"
#include "BaseLib/IndexTreeFileAccess.h"
#include "TestLib/Assert.h"
#include "TestIndexTreeObject.h"
#include "TestIndexTreeFile.h"


char	gszIndexTreeString[257];


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* GetString(CIndexTreeFileAccess* pcAccess, const char* szKey)
{
	char*	szResult;

	szResult = pcAccess->GetStringString(szKey, gszIndexTreeString);
	return szResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertIndexFileEmpty(const char* szFileName)
{
	CNaiveFile	cNaiveFile;
	size		i;
	char*		pcFileMem;

	cNaiveFile.Init();
	AssertTrue(cNaiveFile.Read(szFileName));
	pcFileMem = (char*)cNaiveFile.Get();
	for (i = 0; (filePos)i < cNaiveFile.Size(); i++)
	{
		AssertChar(INDEX_FILE_EMPTY_CHAR, pcFileMem[i]);
	}
	cNaiveFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileSizeOfs(void)
{
	CTestIndexTreeFile	cIndexTree;
	size				tRootNodeSize;
	size				tNodeSize;
	size				tNodePointer;

	cIndexTree.FakeInit(IKR_No);
	tRootNodeSize = cIndexTree.CalculateRootNodeSize();
	tNodeSize = cIndexTree.SizeofNode();
	tNodePointer = cIndexTree.SizeofNodePtr();

#ifdef BUILD_64
	AssertSize(32, tNodeSize);
	AssertSize(16, tNodePointer);
#else
	AssertSize(24, tNodeSize);
	AssertSize(12, tNodePointer);
#endif // BUILD_64
	AssertSize(tNodePointer * 256 + tNodeSize, tRootNodeSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileInit(void)
{
	CIndexTreeFile			cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;

	cHelper.Init("Output" _FS_"QuakeMinusOne", "primary", "backup", true);

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cController.End();

	cIndexTree.Kill();
	cController.Kill();

	cHelper.RemoveWorkingDirectory();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileAdd(EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CTestIndexTreeObject2		a;
	CTestIndexTreeObject2		aa;
	CTestIndexTreeObject2		aaa;
	CTestIndexTreeObject2		ab;
	CTestIndexTreeObject2		aab;
	bool						bResult;
	CIndexTreeNodeFile* pcNode;
	CTestIndexTreeObject2** ppvTest;
	SIndexTreeFileUnsafeIterator		sIter;
	bool						bContinue;
	size						iSize;

	cHelper.Init("Output" _FS_"IndexTree0", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, eKeyReverse);
	cAccess.Init(&cIndexTree);

	a.Init("A");
	bResult = cAccess.PutStringPtr(a.GetName(), &a);
	AssertTrue(bResult);
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	pcNode = cIndexTree.GetNode("A", 1);
	ppvTest = (CTestIndexTreeObject2**)pcNode->GetDataPtr();
	AssertPointer(&a, *ppvTest);

	aa.Init("AA");
	bResult = cAccess.PutStringPtr(aa.GetName(), &aa);
	AssertTrue(bResult);
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	aaa.Init("AAA");
	bResult = cAccess.PutStringPtr(aaa.GetName(), &aaa);
	AssertTrue(bResult);
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	ab.Init("AB");
	bResult = cAccess.PutStringPtr(ab.GetName(), &ab);
	AssertTrue(bResult);
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	aab.Init("AAB");
	bResult = cAccess.PutStringPtr(aab.GetName(), &aab);
	AssertTrue(bResult);
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());


	pcNode = cIndexTree.GetNode("A", 1);
	ppvTest = (CTestIndexTreeObject2**)pcNode->GetDataPtr();
	AssertPointer(&a, *ppvTest);

	pcNode = cIndexTree.GetNode("AA", 2);
	ppvTest = (CTestIndexTreeObject2**)pcNode->GetDataPtr();
	AssertPointer(&aa, *ppvTest);

	bContinue = cIndexTree.StartUnsafeIteration(&sIter, (void**)(&ppvTest), &iSize);
	AssertTrue(bContinue);
	AssertSize(sizeof(CTestIndexTreeObject2*), iSize);
	AssertString("A", (*ppvTest)->mszName);

	bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&ppvTest), &iSize);
	AssertTrue(bContinue);
	AssertSize(sizeof(CTestIndexTreeObject2*), iSize);
	AssertString("AA", (*ppvTest)->mszName);

	bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&ppvTest), &iSize);
	AssertTrue(bContinue);
	AssertSize(sizeof(CTestIndexTreeObject2*), iSize);
	AssertString("AAA", (*ppvTest)->mszName);

	if (eKeyReverse == IKR_No)
	{
		bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&ppvTest), &iSize);
		AssertTrue(bContinue);
		AssertSize(sizeof(CTestIndexTreeObject2*), iSize);
		AssertString("AAB", (*ppvTest)->mszName);

		bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&ppvTest), &iSize);
		AssertTrue(bContinue);
		AssertSize(sizeof(CTestIndexTreeObject2*), iSize);
		AssertString("AB", (*ppvTest)->mszName);
	}
	else if (eKeyReverse == IKR_Yes)
	{
		bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&ppvTest), &iSize);
		AssertTrue(bContinue);
		AssertSize(sizeof(CTestIndexTreeObject2*), iSize);
		AssertString("AB", (*ppvTest)->mszName);

		bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&ppvTest), &iSize);
		AssertTrue(bContinue);
		AssertSize(sizeof(CTestIndexTreeObject2*), iSize);
		AssertString("AAB", (*ppvTest)->mszName);
	}

	bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&ppvTest), &iSize);
	AssertFalse(bContinue);

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileGetNodeKey(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CTestIndexTreeObject2		a;
	bool						bResult;
	CIndexTreeNodeFile*			pcNode;
	size						iSize;
	uint8						szKey[7];

	cHelper.Init("Output" _FS_"IndexTree0a", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	a.Init("Helper");
	bResult = cAccess.PutStringPtr(a.GetName(), &a);
	AssertTrue(bResult);

	pcNode = cIndexTree.GetNode(a.GetName(), a.NameLength());
	AssertNotNull(pcNode);

	iSize = cIndexTree.GetNodeKeySize(pcNode);
	AssertSize(a.NameLength(), iSize);

	cIndexTree.GetNodeKey(pcNode, szKey, 7);
	AssertString(a.GetName(), szKey);

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileAddLongKeys(EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CIndexTreeFile				cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CTestIndexTreeObject2		szAmicable;
	CTestIndexTreeObject2		szAmigo;

	cHelper.Init("Output" _FS_"IndexTree0b", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, eKeyReverse);
	cAccess.Init(&cIndexTree);

	szAmicable.Init("Amicable");
	AssertTrue(cAccess.PutStringPtr(szAmicable.GetName(), NULL));

	szAmigo.Init("Amigo");
	AssertTrue(cAccess.PutStringPtr(szAmigo.GetName(), NULL));
	AssertTrue(cAccess.PutLongPtr(0x4142434430313233ll, NULL));

	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileAddSimple(EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CTestIndexTreeObject2		ab;
	bool						bResult;
	CIndexTreeNodeFile*			pcNode;
	CTestIndexTreeObject2**		ppvTest;

	cHelper.Init("Output" _FS_"IndexTree0c", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, eKeyReverse);
	cAccess.Init(&cIndexTree);

	ab.Init("AB");
	bResult = cAccess.PutStringPtr(ab.GetName(), &ab);
	AssertTrue(bResult);
	pcNode = cIndexTree.GetNode("AB", 2);
	ppvTest = (CTestIndexTreeObject2**)pcNode->GetDataPtr();
	AssertPointer(&ab, *ppvTest);
	Pass();
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());


	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileNoCacheEviction(void)
{
	CIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess	cAccess;
	char					sz[MAX_DATA_SIZE];
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	SIndexTreeFileUnsafeIterator	sIter;
	bool					bContinue;
	char*					szData;
	size					iSize;
	SLogConfig				sLogConfig;

	cHelper.Init("Output" _FS_"IndexTree1", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, 255, MAX_KEY_SIZE);
	cAccess.Init(&cIndexTree);

	AssertTrue(cAccess.PutLongString(0x0000000000000000LL, "Zero"));
	AssertInt((uint32)1, cIndexTree.NumElements());
	AssertTrue(cIndexTree.ValidateIndexTree())

	AssertNotNull(cAccess.GetLongString(0x0000000000000000LL, sz));
	AssertString("Zero", sz);

	bContinue = cIndexTree.StartUnsafeIteration(&sIter, (void**)(&szData), &iSize);
	AssertTrue(bContinue);
	AssertSize(5, iSize);
	AssertString("Zero", szData);
	bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&szData), &iSize);
	AssertFalse(bContinue);

	AssertNull(cAccess.GetLongString(0x0100000000000000LL, sz));

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cAccess.PutLongString(0xEE89DD67CC45BB23LL, "Character Count & Word Count Tool is a free character counter tool that provides instant character count & word count statistics for a given text. The tool reports the number of character with spaces and without spaces, also the number of words and sente."));
	gcLogger.SetConfig(&sLogConfig);
	AssertTrue(cAccess.PutLongString(0xEE89DD67CC45BB23LL, "Character Count & Word Count Tool is a free character counter tool that provides instant character count & word count statistics for a given text. The tool reports the number of character with spaces and without spaces, also the number of words and sent."));
	AssertInt((uint32)2, cIndexTree.NumElements());
	AssertTrue(cIndexTree.ValidateIndexTree())

	AssertNotNull(cAccess.GetLongString(0xEE89DD67CC45BB23LL, sz));
	AssertString("Character Count & Word Count Tool is a free character counter tool that provides instant character count & word count statistics for a given text. The tool reports the number of character with spaces and without spaces, also the number of words and sent.", sz);

	bContinue = cIndexTree.StartUnsafeIteration(&sIter, (void**)(&szData), &iSize);
	AssertTrue(bContinue);
	AssertSize(5, iSize);
	AssertString("Zero", szData);
	bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&szData), &iSize);
	AssertTrue(bContinue);
	AssertSize(255, iSize);
	AssertString("Character Count & Word Count Tool is a free character counter tool that provides instant character count & word count statistics for a given text. The tool reports the number of character with spaces and without spaces, also the number of words and sent.", szData);
	bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&szData), &iSize);
	AssertFalse(bContinue);

	AssertTrue(cAccess.PutLongString(0x0098DD67CC45BB23LL, "MORE node DATA"));
	AssertInt((uint32)3, cIndexTree.NumElements());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertNotNull(cAccess.GetLongString(0x0098DD67CC45BB23LL, sz));
	AssertString("MORE node DATA", sz);

	AssertTrue(cAccess.PutLongString(0x0000DD00CC54BB23LL, "Another DATUM of doom"));
	AssertInt((uint32)4, cIndexTree.NumElements());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertNotNull(cAccess.GetLongString(0x0000DD00CC54BB23LL, sz));
	AssertString("Another DATUM of doom", sz);

	AssertTrue(cAccess.PutLongString(0x0098DD67CC45BB23LL, "Changed your data lengh"));
	AssertInt((uint32)4, cIndexTree.NumElements());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertNotNull(cAccess.GetLongString(0x0098DD67CC45BB23LL, sz));
	AssertString("Changed your data lengh", sz);
	AssertNotNull(cAccess.GetLongString(0x0000000000000000LL, sz));
	AssertString("Zero", sz);
	AssertNotNull(cAccess.GetLongString(0xEE89DD67CC45BB23LL, sz));
	AssertString("Character Count & Word Count Tool is a free character counter tool that provides instant character count & word count statistics for a given text. The tool reports the number of character with spaces and without spaces, also the number of words and sent.", sz);
	AssertNotNull(cAccess.GetLongString(0x0000DD00CC54BB23LL, sz));
	AssertString("Another DATUM of doom", sz);

	AssertTrue(cAccess.PutLongString(0xEE89DD67CC45BB23LL, "Make the long int16."));
	AssertInt((uint32)4, cIndexTree.NumElements());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertNotNull(cAccess.GetLongString(0x0098DD67CC45BB23LL, sz));
	AssertString("Changed your data lengh", sz);
	AssertNotNull(cAccess.GetLongString(0x0000000000000000LL, sz));
	AssertString("Zero", sz);
	AssertNotNull(cAccess.GetLongString(0xEE89DD67CC45BB23LL, sz));
	AssertString("Make the long int16.", sz);
	AssertNotNull(cAccess.GetLongString(0x0000DD00CC54BB23LL, sz));
	AssertString("Another DATUM of doom", sz);

	AssertFalse(cAccess.DeleteLong(0x0100000000000000LL));
	AssertFalse(cAccess.DeleteLong(0x00000000000000FFLL));
	AssertTrue(cAccess.DeleteLong(0x0000000000000000LL));
	AssertInt((uint32)3, cIndexTree.NumElements());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertNotNull(cAccess.GetLongString(0x0098DD67CC45BB23LL, sz));
	AssertString("Changed your data lengh", sz);
	AssertNotNull(cAccess.GetLongString(0xEE89DD67CC45BB23LL, sz));
	AssertString("Make the long int16.", sz);
	AssertNotNull(cAccess.GetLongString(0x0000DD00CC54BB23LL, sz));
	AssertString("Another DATUM of doom", sz);

	AssertTrue(cAccess.DeleteLong(0x0098DD67CC45BB23LL));
	AssertInt((uint32)2, cIndexTree.NumElements());
	AssertTrue(cIndexTree.ValidateIndexTree());
	AssertNull(cAccess.GetLongString(0x0098DD67CC45BB23LL, sz));

	AssertNotNull(cAccess.GetLongString(0xEE89DD67CC45BB23LL, sz));
	AssertString("Make the long int16.", sz);
	AssertNotNull(cAccess.GetLongString(0x0000DD00CC54BB23LL, sz));
	AssertString("Another DATUM of doom", sz);

	bContinue = cIndexTree.StartUnsafeIteration(&sIter, (void**)(&szData), &iSize);
	AssertTrue(bContinue);
	AssertSize(21, iSize);
	AssertString("Make the long int16.", szData);

	bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&szData), &iSize);
	AssertTrue(bContinue);
	AssertSize(22, iSize);
	AssertString("Another DATUM of doom", szData);

	bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&szData), &iSize);
	AssertFalse(bContinue);

	AssertTrue(cAccess.DeleteLong(0x0000DD00CC54BB23LL));
	AssertInt((uint32)1, cIndexTree.NumElements());
	AssertNull(cAccess.GetLongString(0x0000DD00CC54BB23LL, sz));

	bContinue = cIndexTree.StartUnsafeIteration(&sIter, (void**)(&szData), &iSize);
	AssertTrue(bContinue);
	AssertSize(21, iSize);
	AssertString("Make the long int16.", szData);

	bContinue = cIndexTree.UnsafeIterate(&sIter, (void**)(&szData), &iSize);
	AssertFalse(bContinue);

	AssertNotNull(cAccess.GetLongString(0xEE89DD67CC45BB23LL, sz));
	AssertString("Make the long int16.", sz);

	AssertTrue(cAccess.DeleteLong(0xEE89DD67CC45BB23LL));
	AssertInt((uint32)0, cIndexTree.NumElements());
	AssertNull(cAccess.GetLongString(0xEE89DD67CC45BB23LL, sz));

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileResizeData(void)
{
	CIndexTreeNodeFile*		pcNode;
	CIndexTreeNodeFile*		pcOldNode;
	CIndexTreeNodeFile*		pcRoot;
	char					szAAObject[] = "Hello";
	char					szACObject[] = "Goodbye";
	char					szAObject[] = "Centrist Policies";
	size					iNodeMemoryOffset1;
	size					iNodeMemoryOffset2;
	CChars					sz;
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeFileAccess	cAccess;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CMemoryAllocator		cAllocator;
	CGeneralMemory*			pcMemory;

	cHelper.Init("Output" _FS_"IndexTree2", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();

	cController.Begin();
	cIndexTree.Init(&cController, NULL, LifeLocal<CMallocator>(&cAllocator), IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringData("AA", szAAObject, strlen(szAAObject) + 1);
	cAccess.PutStringData("AC", szACObject, strlen(szACObject) + 1);

	pcNode = cIndexTree.GetNode("A", 1);
	pcOldNode = pcNode;
	AssertSize(3, pcNode->NumIndexes());
	AssertSize(2, pcNode->NumValidIndexes());
	AssertSize(0, pcNode->GetDataSize());
	iNodeMemoryOffset1 = (size)((ptr)pcNode->GetNodesMemory() - (ptr)pcNode);
	AssertSize(cIndexTree.SizeofNode(), iNodeMemoryOffset1);

	cAccess.PutStringData("A", szAObject, strlen(szAObject) + 1);

	pcNode = cIndexTree.GetNode("A", 1);
	AssertFalse(pcNode == pcOldNode);
	AssertSize(3, pcNode->NumIndexes());
	AssertSize(2, pcNode->NumValidIndexes());
	AssertSize(18, pcNode->GetDataSize());
	iNodeMemoryOffset2 = (size)((ptr)pcNode->GetNodesMemory() - (ptr)pcNode);
	AssertSize(cIndexTree.SizeofDataNode() + pcNode->GetDataSize(), iNodeMemoryOffset2);
	AssertTrue(iNodeMemoryOffset2 > iNodeMemoryOffset1);

	AssertString(szAAObject, GetString(&cAccess, "AA"));
	AssertString(szACObject, GetString(&cAccess, "AC"));
	AssertString(szAObject, GetString(&cAccess, "A"));

	pcRoot = cIndexTree.GetRoot();
	sz.Init(); pcRoot->Print(&sz, false);
	AssertString("0:255 .................................................................X..............................................................................................................................................................................................", sz.Text()); sz.Kill();
	pcNode = pcRoot->Get('A')->u.mpcMemory;
	sz.Init(); pcNode->Print(&sz, false);
	AssertString("65:67 (18) X.X", sz.Text()); sz.Kill();
	sz.Init(); pcNode->Get('A')->u.mpcMemory->Print(&sz, false);
	AssertString("0:0 (6)", sz.Text()); sz.Kill();
	AssertFalse(pcNode->Get('B')->IsValid());
	sz.Init(); pcNode->Get('C')->u.mpcMemory->Print(&sz, false);
	AssertString("0:0 (8)", sz.Text()); sz.Kill();

	cController.End();

	cIndexTree.Kill();
	cController.Kill();

	AssertLong(0, pcMemory->GetTotalAllocatedMemory());
	cAllocator.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRemoveNearestFirstFlushed(void)
{
	CIndexTreeFile			cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CIndexTreeFileAccess	cAccess;

	cHelper.Init("Output" _FS_"IndexTree3", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_Yes);
	cAccess.Init(&cIndexTree);


	cAccess.PutStringLong("cover", 0x7373737339393939LL);
	cAccess.PutStringLong("discover", 0x5858585890909090LL);
	AssertTrue(cAccess.Flush());


	AssertTrue(cAccess.DeleteString("cover"));
	Pass();
	cAccess.PutStringLong("discover", 0x5858585890909090LL);
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());


	cAccess.PutStringLong("cover", 0x7373737339393939LL);
	cAccess.PutStringLong("discover", 0x5858585890909090LL);
	AssertTrue(cAccess.Flush());
	AssertTrue(cAccess.DeleteString("cover"));
	Pass();
	cAccess.PutStringLong("discover", 0x5858585890909090LL);
	cAccess.PutStringLong("cover", 0x7373737339393939LL);
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());


	cAccess.PutStringLong("cover", 0x7373737339393939LL);
	cAccess.PutStringLong("discover", 0x5858585890909090LL);
	AssertTrue(cAccess.Flush());
	AssertTrue(cAccess.DeleteString("cover"));
	AssertTrue(cAccess.DeleteString("discover"));
	cAccess.PutStringLong("cover", 0x7373737339393939LL);
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());


	cAccess.PutStringLong("cover", 0x7373737339393939LL);
	cAccess.PutStringLong("discover", 0x5858585890909090LL);
	AssertTrue(cAccess.Flush());
	AssertTrue(cAccess.DeleteString("cover"));
	AssertTrue(cAccess.DeleteString("discover"));
	cAccess.PutStringLong("discover", 0x7373737339393939LL);
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());


	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRemoveNearestFirst(EIndexWriteThrough eWriteThrough)
{
	char					szAAA[] = "NEAR";
	char					szA[] = "Florida";
	CIndexTreeFile			cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CIndexTreeFileAccess	cAccess;

	cHelper.Init("Output" _FS_"IndexTree3", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, IKR_No);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringData("AAA", szAAA, strlen(szAAA) + 1);
	cAccess.PutStringData("A", szA, strlen(szA) + 1);

	AssertInt((uint32)2, cIndexTree.NumElements());
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertTrue(cAccess.DeleteString("A"));
	AssertString(NULL, GetString(&cAccess, "A"));
	AssertString(szAAA, GetString(&cAccess, "AAA"));
	AssertInt((uint32)1, cIndexTree.NumElements());
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertTrue(cAccess.DeleteString("AAA"));
	AssertString(NULL, GetString(&cAccess, "A"));
	AssertString(NULL, GetString(&cAccess, "AAA"));
	AssertInt((uint32)0, cIndexTree.NumElements());
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRemoveFurthestFirst(EIndexWriteThrough eWriteThrough)
{
	char					szAA[] = "MEDIUM";
	char					szAAA[] = "NEAR";
	char					szA[] = "Florida";
	CIndexTreeFile			cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CIndexTreeFileAccess	cAccess;

	cHelper.Init("Output" _FS_"IndexTree3a", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, IKR_No);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringData("AA", szAA, strlen(szAA) + 1);
	cAccess.PutStringData("AAA", szAAA, strlen(szAAA) + 1);
	cAccess.PutStringData("A", szA, strlen(szA) + 1);
	AssertInt((uint32)3, cIndexTree.NumElements());
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertTrue(cAccess.DeleteString("AAA"));
	AssertString(szA, GetString(&cAccess, "A"));
	AssertString(szAA, GetString(&cAccess, "AA"));
	AssertString(NULL, GetString(&cAccess, "AAA"));
	AssertInt((uint32)2, cIndexTree.NumElements());
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertTrue(cAccess.DeleteString("AA"));
	AssertString(szA, GetString(&cAccess, "A"));
	AssertString(NULL, GetString(&cAccess, "AA"));
	AssertString(NULL, GetString(&cAccess, "AAA"));
	AssertInt((uint32)1, cIndexTree.NumElements());
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertTrue(cAccess.DeleteString("A"));
	AssertString(NULL, GetString(&cAccess, "A"));
	AssertString(NULL, GetString(&cAccess, "AA"));
	AssertString(NULL, GetString(&cAccess, "AAA"));
	AssertInt((uint32)0, cIndexTree.NumElements());
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRemove(EIndexWriteThrough eWriteThrough)
{
	char					szAAAA[] = "One and Ony";
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CIndexTreeFileAccess	cAccess;
	CArrayIndexedFilePtr	apc;
	CIndexedFile*			pcFile;
	CArrayBit				ab;

	cHelper.Init("Output" _FS_"IndexTree3b", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, IKR_No);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringData("AAAA", szAAAA, strlen(szAAAA) + 1);
	AssertInt((uint32)1, cIndexTree.NumElements());
	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	apc.Init();
	cIndexTree.GetFiles(&apc);
	AssertSize(3, apc.NumElements());

	pcFile = *apc.Get(0);
	AssertLSize(2062, pcFile->GetDataSize());
	ab.Init();
	AssertLong(1, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(1, ab.NumElements());
	ab.Kill();

	pcFile = *apc.Get(1);
	AssertLSize(26, pcFile->GetDataSize());
	ab.Init();
	AssertLong(1, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(1, ab.NumElements());
	ab.Kill();

	pcFile = *apc.Get(2);
	AssertLSize(22, pcFile->GetDataSize());
	ab.Init();
	AssertLong(3, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(3, ab.NumElements());
	ab.Kill();
	apc.Kill();


	AssertTrue(cAccess.DeleteString("AAAA"));
	AssertNull(GetString(&cAccess, "AAAA"));
	AssertInt((uint32)0, cIndexTree.NumElements());
	AssertFalse(cAccess.DeleteString("AAAA"));

	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());

	apc.Init();
	cIndexTree.GetFiles(&apc);
	AssertSize(3, apc.NumElements());

	pcFile = *apc.Get(0);
	AssertLSize(2062, pcFile->GetDataSize());
	ab.Init();
	AssertLong(1, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(1, ab.NumElements());
	ab.Kill();

	pcFile = *apc.Get(1);
	AssertLSize(26, pcFile->GetDataSize());
	ab.Init();
	AssertLong(0, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(1, ab.NumElements());
	ab.Kill();

	pcFile = *apc.Get(2);
	AssertLSize(22, pcFile->GetDataSize());
	ab.Init();
	AssertLong(0, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(3, ab.NumElements());
	ab.Kill();
	apc.Kill();

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitTestIndexTreeFileFlushDirtyAndDeleted(CIndexTreeHelper* pcHelper, CIndexTreeFileAccess* pcAccess, CTestIndexTreeFile* pcIndexTree, CDurableFileController* pcController)
{
	pcHelper->Init("Output" _FS_"IndexTree3c", "primary", "backup", true);
	pcController->Init(pcHelper->GetPrimaryDirectory(), pcHelper->GetBackupDirectory());
	pcController->SetSync(DS_No);

	pcController->Begin();
	pcIndexTree->Init(pcController, NULL, IWT_No, IKR_No);
	pcAccess->Init(pcIndexTree);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void KillTestIndexTreeFileFlushDirtyAndDeleted(CIndexTreeHelper* pcHelper, CIndexTreeFileAccess* pcAccess)
{
	CIndexTreeFile*				pcIndexTree;
	CDurableFileController*		pcController;

	pcIndexTree = pcAccess->GetTree();
	pcController = pcIndexTree->GetController();

	AssertTrue(pcIndexTree->ValidateIndexTree());
	pcController->End();
	pcIndexTree->Kill();
	pcController->Kill();
	pcAccess->Kill();

	pcHelper->Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileFlushDirtyAndDeleted(void)
{
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CIndexTreeFileAccess	cAccess;

	InitTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess, &cIndexTree, &cController);
	cAccess.PutStringString("Hello", "12345");
	cAccess.PutStringString("Hel", "54321");
	cAccess.FlushString("Hello");
	cAccess.FlushString("Hel");
	AssertTrue(cAccess.HasString("Hello"));
	AssertTrue(cAccess.HasString("Hel"));
	KillTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess);

	InitTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess, &cIndexTree, &cController);
	cAccess.PutStringString("Hello", "12345");
	cAccess.PutStringString("Hel", "54321");
	cAccess.FlushString("Hel");
	cAccess.FlushString("Hello");
	AssertTrue(cAccess.HasString("Hello"));
	AssertTrue(cAccess.HasString("Hel"));
	KillTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess);

	InitTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess, &cIndexTree, &cController);
	cAccess.PutStringString("Hello", "12345");
	cAccess.PutStringString("Hel", "54321");
	cAccess.DeleteString("Hel");
	cAccess.FlushString("Hello");
	cAccess.FlushString("Hel");
	AssertTrue(cAccess.HasString("Hello"));
	AssertFalse(cAccess.HasString("Hel"));
	KillTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess);

	InitTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess, &cIndexTree, &cController);
	cAccess.PutStringString("Hello", "12345");
	cAccess.PutStringString("Hel", "54321");
	cAccess.DeleteString("Hel");
	cAccess.FlushString("Hel");
	cAccess.FlushString("Hello");
	AssertTrue(cAccess.HasString("Hello"));
	AssertFalse(cAccess.HasString("Hel"));
	KillTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess);

	InitTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess, &cIndexTree, &cController);
	cAccess.PutStringString("Hello", "12345");
	cAccess.PutStringString("Hel", "54321");
	cAccess.DeleteString("Hello");
	cAccess.FlushString("Hello");
	cAccess.FlushString("Hel");
	AssertFalse(cAccess.HasString("Hello"));
	AssertTrue(cAccess.HasString("Hel"));
	KillTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess);

	InitTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess, &cIndexTree, &cController);
	cAccess.PutStringString("Hello", "12345");
	cAccess.PutStringString("Hel", "54321");
	cAccess.DeleteString("Hello");
	cAccess.FlushString("Hel");
	cAccess.FlushString("Hello");
	AssertFalse(cAccess.HasString("Hello"));
	AssertTrue(cAccess.HasString("Hel"));
	KillTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess);

	InitTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess, &cIndexTree, &cController);
	cAccess.PutStringString("Hello", "12345");
	cAccess.PutStringString("Hel", "54321");
	cAccess.DeleteString("Hello");
	cAccess.DeleteString("Hel");
	cAccess.FlushString("Hello");
	cAccess.FlushString("Hel");
	AssertFalse(cAccess.HasString("Hello"));
	AssertFalse(cAccess.HasString("Hel"));
	KillTestIndexTreeFileFlushDirtyAndDeleted(&cHelper, &cAccess);

	Pass();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRemoveAndEvict(void)
{
	char					szABCD[] = "One and Only";
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CIndexTreeFileAccess	cAccess;
	CArrayIndexedFilePtr	apc;
	CIndexedFile*			pcFile;
	CArrayBit				ab;

	cHelper.Init("Output" _FS_"IndexTree3d", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	AssertTrue(cIndexTree.IsFlushed());
	cAccess.PutStringData("ABCD", szABCD, strlen(szABCD) + 1);
	AssertLSize(1, cIndexTree.NumMemoryElements());
	AssertFalse(cIndexTree.IsFlushed());
	AssertTrue(cAccess.EvictString("ABCD"));
	AssertTrue(cIndexTree.IsFlushed());

	apc.Init();
	cIndexTree.GetFiles(&apc);
	AssertSize(3, apc.NumElements());

	pcFile = *apc.Get(0);
	AssertLSize(2062, pcFile->GetDataSize());
	ab.Init();
	AssertLong(1, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(1, ab.NumElements());
	ab.Kill();

	pcFile = *apc.Get(1);
	AssertLSize(27, pcFile->GetDataSize());
	ab.Init();
	AssertLong(1, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(1, ab.NumElements());
	ab.Kill();

	pcFile = *apc.Get(2);
	AssertLSize(22, pcFile->GetDataSize());
	ab.Init();
	AssertLong(3, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(3, ab.NumElements());
	ab.Kill();
	apc.Kill();


	AssertTrue(cAccess.DeleteString("ABCD"));
	Pass();
	AssertTrue(cAccess.EvictString("ABCD"));
	Pass();

	apc.Init();
	cIndexTree.GetFiles(&apc);
	AssertSize(3, apc.NumElements());

	pcFile = *apc.Get(0);
	AssertLSize(2062, pcFile->GetDataSize());
	ab.Init();
	AssertLong(1, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(1, ab.NumElements());
	ab.Kill();

	pcFile = *apc.Get(1);
	AssertLSize(27, pcFile->GetDataSize());
	ab.Init();
	AssertLong(0, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(1, ab.NumElements());
	ab.Kill();

	pcFile = *apc.Get(2);
	AssertLSize(22, pcFile->GetDataSize());
	ab.Init();
	AssertLong(0, pcFile->GetUsedDataIndices(&ab));
	AssertLSize(3, ab.NumElements());
	ab.Kill();
	apc.Kill();

	cIndexTree.ValidateIndexTree();
	cController.End();
	cIndexTree.Kill();
	cController.Kill();
	cAccess.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRemoveAndFlush(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CMemoryAllocator			cAllocator;
	char						szAABAA[] = "AABAA";
	char						szAABBB[] = "AABBB";
	char						szAACAA[] = "AACAA";
	char						szAACBB[] = "AACBB";
	CIndexTreeFileAccess		cAccess;
	char						szDest[16];

	cAllocator.Init();
	cHelper.Init("Output" _FS_"IndexTree3e", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, LifeLocal<CMallocator>(&cAllocator), IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	AssertTrue(cAccess.PutStringString(szAABAA, szAABAA));
	AssertTrue(cAccess.PutStringString(szAABBB, szAABBB));
	AssertTrue(cAccess.PutStringString(szAACAA, szAACAA));
	AssertTrue(cAccess.PutStringString(szAACBB, szAACBB));

	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNotNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAACBB, szDest));

	cController.End();
	cIndexTree.Kill();

	cController.Begin();
	cIndexTree.Init(&cController, NULL, LifeLocal<CMallocator>(&cAllocator), IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);
	AssertLSize(0, cIndexTree.NumMemoryElements());
	AssertLSize(1, cIndexTree.NumMemoryNodes());
	Pass();
	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNotNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAACBB, szDest));
	AssertInt((uint32)4, cIndexTree.NumElements());
	AssertLSize(13, cIndexTree.NumNodes());
	AssertLSize(4, cIndexTree.NumMemoryElements());
	AssertLSize(13, cIndexTree.NumMemoryNodes());
	AssertTrue(cIndexTree.ValidateIndexTree());

	AssertTrue(cAccess.DeleteString(szAACBB));
	AssertTrue(cAccess.Flush());

	AssertLSize(3, cIndexTree.NumMemoryElements());
	AssertLSize(11, cIndexTree.NumMemoryNodes());
	AssertInt((uint32)3, cIndexTree.NumElements());
	AssertLSize(11, cIndexTree.NumNodes());
	AssertTrue(cIndexTree.ValidateIndexTree());

	cController.End();

	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();

	cHelper.Kill(true);
	cAccess.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileDeleteInMemory(void)
{
	char					szAA[] = "MEDIUM";
	char					szAAA[] = "NEAR";
	char					szA[] = "Florida";
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CIndexTreeFileAccess	cAccess;
	
	cHelper.Init("Output" _FS_"IndexTree4", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringData("AA", szAA, strlen(szAA) + 1);
	cAccess.PutStringData("AAA", szAAA, strlen(szAAA) + 1);
	cAccess.PutStringData("A", szA, strlen(szA) + 1);
	AssertInt((uint32)3, cIndexTree.NumElements());

	cIndexTree.SetWriteThrough(IWT_No);

	cAccess.DeleteString("AA");
	AssertString(szA, GetString(&cAccess, "A"));
	AssertString(NULL, GetString(&cAccess, "AA"));
	AssertString(szAAA, GetString(&cAccess, "AAA"));
	AssertInt((uint32)2, cIndexTree.NumElements());
	AssertLSize(4, cIndexTree.NumNodes());

	cIndexTree.Flush();
	AssertLSize(4, cIndexTree.NumNodes());
	cIndexTree.ValidateIndexTree();

	cAccess.DeleteString("A");
	AssertString(NULL, GetString(&cAccess, "A"));
	AssertString(NULL, GetString(&cAccess, "AA"));
	AssertString(szAAA, GetString(&cAccess, "AAA"));
	AssertInt((uint32)1, cIndexTree.NumElements());
	AssertLSize(4, cIndexTree.NumNodes());

	cAccess.DeleteString("AAA");
	AssertString(NULL, GetString(&cAccess, "A"));
	AssertString(NULL, GetString(&cAccess, "AA"));
	AssertString(NULL, GetString(&cAccess, "AAA"));
	AssertInt((uint32)0, cIndexTree.NumElements());
	AssertLSize(4, cIndexTree.NumNodes());

	cIndexTree.Flush();
	AssertLSize(1, cIndexTree.NumNodes());  //Just the root node left.
	cIndexTree.ValidateIndexTree();

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileFindKey(void)
{
	char					szData[] = "Data";
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeFileAccess	cAccess;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	char					szKeyName[] = "Key Name";
	char					szAmphibious[] = "Amphibious";
	char					szAmorphous[] = "Amorphous";
	char					szTestFly[] = "Test Fly";
	CIndexTreeNodeFile*		pcNode1;
	CIndexTreeNodeFile*		pcNode2;
	CIndexTreeNodeFile*		pcNode3;
	CIndexTreeNodeFile*		pcNode4;
	CArrayChar				acKey;
	uint8					auiKey[MAX_KEY_SIZE];
	size					iKeySize;
	CArrayVoidPtr			apvNodes;
	CListCharsMinimal*		paszKeyNames;

	cHelper.Init("Output" _FS_"IndexTree5", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringData(szKeyName, szData, 5);
	cAccess.PutStringData(szAmphibious, szData, 5);
	cAccess.PutStringData(szAmorphous, szData, 5);
	cAccess.PutStringData(szTestFly, szData, 5);

	pcNode1 = cIndexTree.GetNode("Key Name", strlen(szKeyName));
	pcNode2 = cIndexTree.GetNode("Amphibious", strlen(szAmphibious));
	pcNode3 = cIndexTree.GetNode("Amorphous", strlen(szAmorphous));
	pcNode4 = cIndexTree.GetNode("Test Fly", strlen(szTestFly));

	acKey.Init();
	cIndexTree.GetNodeKey(pcNode1, &acKey);
	AssertSize(8, acKey.NumElements());
	acKey.Add(0);
	AssertString(szKeyName, acKey.GetData());
	acKey.Kill();

	acKey.Init();
	cIndexTree.GetNodeKey(pcNode2, &acKey);
	acKey.Add(0);
	AssertString(szAmphibious, acKey.GetData());
	acKey.Kill();

	acKey.Init();
	cIndexTree.GetNodeKey(pcNode3, &acKey);
	acKey.Add(0);
	AssertString(szAmorphous, acKey.GetData());
	acKey.Kill();

	acKey.Init();
	cIndexTree.GetNodeKey(pcNode4, &acKey);
	acKey.Add(0);
	AssertString(szTestFly, acKey.GetData());
	acKey.Kill();

	acKey.Init();
	cIndexTree.GetNodeKey(cIndexTree.GetRoot(), &acKey);
	AssertSize(0, acKey.NumElements());
	acKey.Kill();

	acKey.Init();
	cIndexTree.GetNodeKey(NULL, &acKey);
	AssertSize(0, acKey.NumElements());
	acKey.Kill();

	memset(auiKey, 0, MAX_KEY_SIZE);
	iKeySize = cIndexTree.GetNodeKey(pcNode1, auiKey, MAX_KEY_SIZE);
	AssertSize(8, iKeySize);
	AssertString(szKeyName, (char*)auiKey);

	iKeySize = cIndexTree.GetNodeKey(pcNode4, auiKey, MAX_KEY_SIZE);
	AssertSize(8, iKeySize);
	AssertString(szTestFly, (char*)auiKey);

	iKeySize = cIndexTree.GetNodeKey(pcNode3, auiKey, MAX_KEY_SIZE);
	AssertSize(9, iKeySize);
	AssertString(szAmorphous, (char*)auiKey);

	iKeySize = cIndexTree.GetNodeKey(pcNode2, auiKey, MAX_KEY_SIZE);
	AssertSize(10, iKeySize);
	AssertString(szAmphibious, (char*)auiKey);

	apvNodes.Init();
	apvNodes.Add(pcNode1);
	apvNodes.Add(pcNode2);
	apvNodes.Add(pcNode3);
	apvNodes.Add(pcNode4);

	paszKeyNames = cIndexTree.GetNodesStringKeys(&apvNodes);
	AssertSize(4, paszKeyNames->NumElements());
	AssertString(szKeyName, paszKeyNames->Get(0));
	AssertString(szAmphibious, paszKeyNames->Get(1));
	AssertString(szAmorphous, paszKeyNames->Get(2));
	AssertString(szTestFly, paszKeyNames->Get(3));

	paszKeyNames->Get(0, (size*)&iKeySize);  AssertSize(strlen(szKeyName) + 1, iKeySize);
	paszKeyNames->Get(1, (size*)&iKeySize);  AssertSize(strlen(szAmphibious) + 1, iKeySize);
	paszKeyNames->Get(2, (size*)&iKeySize);  AssertSize(strlen(szAmorphous) + 1, iKeySize);
	paszKeyNames->Get(3, (size*)&iKeySize);  AssertSize(strlen(szTestFly) + 1, iKeySize);

	free(paszKeyNames);
	apvNodes.Kill();

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileDirty(void)
{
	char					szFoot[] = "Foot";
	char					szFork[] = "Fork";
	char					szGemstone[] = "Gemstone";
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CIndexTreeFileAccess	cAccess;

	cHelper.Init("Output" _FS_"IndexTree6", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringData("Foot", szFoot, strlen(szFoot) + 1);
	cIndexTree.Flush();
	AssertLSize(5, cIndexTree.NumNodes());
	cIndexTree.ValidateIndexTree();
	cController.End();

	cController.Begin();
	cAccess.PutStringData("Fork", szFork, strlen(szFork) + 1);
	cAccess.PutStringData("Gemstone", szGemstone, strlen(szGemstone) + 1);
	cIndexTree.Flush();
	AssertLSize(15, cIndexTree.NumNodes());
	cIndexTree.ValidateIndexTree();

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileReplaceData(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CIndexTreeFile				cIndexTree;
	CIndexTreeFileAccess		cAccess;
	bool						bResult;
	char						acData[5];
	char						acResult[5];
	size						iDataSize;

	cHelper.Init("Output" _FS_"IndexTree7", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	memset(acData, 5, 5);
	bResult = cAccess.PutStringData("Quebc", acData, 5);
	AssertTrue(bResult);

	memset(acResult, 0, 5);
	bResult = cAccess.GetStringData("Quebc", acResult, &iDataSize);
	AssertTrue(bResult);
	AssertInt(0, memcmp(acData, acResult, 5));

	memset(acData, 0xfe, 5);
	bResult = cAccess.PutStringData("Quebc", acData, 5);
	AssertTrue(bResult);
	
	memset(acResult, 0, 5);
	bResult = cAccess.GetStringData("Quebc", acResult, &iDataSize);
	AssertTrue(bResult);
	AssertInt(0, memcmp(acData, acResult, 5));

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileAddUnallocated(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CTestIndexTreeObject2		a;
	CTestIndexTreeObject2		b;
	CTestIndexTreeObject2		c;
	bool						bResult;

	cHelper.Init("Output" _FS_"IndexTree8", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	a.Init("1A");
	c.Init("1C");
	b.Init("1B");

	bResult = cAccess.PutStringPtr(a.GetName(), &a);
	AssertTrue(bResult);
	AssertPointer(&a, *((CTestIndexTreeObject2**)cIndexTree.GetNode(a.GetName(), 2)->GetDataPtr()));

	bResult = cAccess.PutStringPtr(c.GetName(), &c);
	AssertTrue(bResult);
	AssertPointer(&a, *((CTestIndexTreeObject2**)cIndexTree.GetNode(a.GetName(), 2)->GetDataPtr()));
	AssertPointer(&c, *((CTestIndexTreeObject2**)cIndexTree.GetNode(c.GetName(), 2)->GetDataPtr()));

	bResult = cAccess.PutStringPtr(b.GetName(), &b);
	AssertTrue(bResult);
	AssertPointer(&a, *((CTestIndexTreeObject2**)cIndexTree.GetNode(a.GetName(), 2)->GetDataPtr()));
	AssertPointer(&b, *((CTestIndexTreeObject2**)cIndexTree.GetNode(b.GetName(), 2)->GetDataPtr()));
	AssertPointer(&c, *((CTestIndexTreeObject2**)cIndexTree.GetNode(c.GetName(), 2)->GetDataPtr()));

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRemoveThenDirty(void)
{
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CIndexTreeFileAccess	cAccess;
	CTestIndexTreeObject2	a;
	CTestIndexTreeObject2	b;
	CTestIndexTreeObject2	c;
	char					sz[MAX_DATA_SIZE];
	CIndexTreeNodeFile*		pcNode;

	cHelper.Init("Output" _FS_"IndexTree9", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	a.Init("1A");
	c.Init("1C");
	b.Init("1B");

	AssertTrue(cAccess.PutStringString(a.GetName(), a.GetName()));
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString(a.GetName(), sz);

	AssertTrue(cAccess.PutStringString(c.GetName(), c.GetName()));
	AssertNotNull(cAccess.GetStringString(c.GetName(), sz));
	AssertString(c.GetName(), sz);

	AssertTrue(cIndexTree.Flush());

	AssertTrue(cAccess.DeleteString(a.GetName()));
	AssertNull(cAccess.GetStringString(a.GetName(), sz));

	AssertTrue(cAccess.PutStringString(a.GetName(), b.GetName()));
	pcNode = cIndexTree.GetNode(a.GetName(), a.NameLength());
	AssertNotNull(pcNode);
	AssertFalse(pcNode->IsDeleted());
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString(b.GetName(), sz);

	AssertTrue(cIndexTree.Flush());
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString(b.GetName(), sz);

	AssertTrue(cAccess.DeleteString(a.GetName()));
	AssertNull(cAccess.GetStringString(a.GetName(), sz));

	AssertTrue(cAccess.PutStringString(a.GetName(), a.GetName()));
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString(a.GetName(), sz);

	AssertTrue(cAccess.DeleteString(a.GetName()));
	AssertNull(cAccess.GetStringString(a.GetName(), sz));

	AssertTrue(cIndexTree.Flush());
	AssertNull(cAccess.GetStringString(a.GetName(), sz));

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileAddThenAdd(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CIndexTreeFile				cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CTestIndexTreeObject2		a;
	CTestIndexTreeObject2		b;
	char						sz[MAX_DATA_SIZE];

	cHelper.Init("Output" _FS_"IndexTree9a", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	a.Init("AA");
	b.Init("AAAA");

	AssertTrue(cAccess.PutStringString(a.GetName(), a.GetName()));

	cIndexTree.Flush();
	cController.End();
	cIndexTree.Kill();


	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	AssertTrue(cAccess.PutStringString(b.GetName(), b.GetName()));

	cIndexTree.Flush();
	cController.End();
	cIndexTree.Kill();


	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString(a.GetName(), sz);
	AssertNotNull(cAccess.GetStringString(b.GetName(), sz));
	AssertString(b.GetName(), sz);

	cIndexTree.Flush();
	cController.End();
	cIndexTree.Kill();

	cAccess.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertContainsFiles(CArrayChars* pacFiles, const char* szFirstFile, ...)
{
	va_list			vaMarker;
	const char*		szFile;
	CArrayChars		asz;
	size			i;

	szFile = szFirstFile;
	asz.Init();

	va_start(vaMarker, szFirstFile);
	while (szFile != NULL)
	{
		asz.Add(szFile);
		szFile = va_arg(vaMarker, char*);
	}
	va_end(vaMarker);

	AssertSize(pacFiles->NumElements(), asz.NumElements());
	for (i = 0; i < asz.NumElements(); i++)
	{
		szFile = asz.GetText(i);
		if (!pacFiles->ContainsSubString(szFile))
		{
			AssertString(szFile, "");
		}
	}
	
	asz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRemoveBeforeFlush(void)
{
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeFileAccess	cAccess;
	CTestIndexTreeObject2	a;
	char					sz[MAX_DATA_SIZE];
	char*					szDirectory;
	CFileUtil				cFileUtil;
	CArrayChars				acFiles;
	CIndexTreeNodeFile*		pcNode;

	cHelper.Init("Output" _FS_"IndexTree9b", "primary", "backup", true);
	szDirectory = cHelper.GetPrimaryDirectory();
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cAccess.Init(&cIndexTree);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);

	a.Init("A");

	AssertTrue(cAccess.PutStringString(a.GetName(), "Onomatopoeia"));
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString("Onomatopoeia", sz);

	AssertTrue(cAccess.DeleteString(a.GetName()));
	AssertNull(cAccess.GetStringString(a.GetName(), sz));

	cAccess.Flush();

	cController.End();

	acFiles.Init();
	cFileUtil.FindFilesWithExtension(szDirectory, "IDAT", &acFiles);
	AssertContainsFiles(&acFiles, "2062", NULL);
	acFiles.Kill();


	cController.Begin();

	AssertTrue(cAccess.PutStringString(a.GetName(), "Zootopic"));
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString("Zootopic", sz);

	AssertTrue(cAccess.PutStringString(a.GetName(), "Franschhoek"));
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString("Franschhoek", sz);

	AssertTrue(cAccess.DeleteString(a.GetName()));
	AssertNull(cAccess.GetStringString(a.GetName(), sz));

	cAccess.Flush();

	cController.End();

	acFiles.Init();
	cFileUtil.FindFilesWithExtension(szDirectory, "IDAT", &acFiles);
	AssertContainsFiles(&acFiles, "2062", NULL);
	acFiles.Kill();


	cController.Begin();

	AssertTrue(cAccess.PutStringString(a.GetName(), "Blobfish"));
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString("Blobfish", sz);

	cAccess.Flush();
	cController.End();

	acFiles.Init();
	cFileUtil.FindFilesWithExtension(szDirectory, "IDAT", &acFiles);
	AssertContainsFiles(&acFiles, "2062", "23", NULL);
	acFiles.Kill();

	cController.Begin();

	AssertTrue(cAccess.DeleteString(a.GetName()));
	AssertNull(cAccess.GetStringString(a.GetName(), sz));

	cAccess.Flush();
	cController.End();
	
	acFiles.Init();
	cFileUtil.FindFilesWithExtension(szDirectory, "IDAT", &acFiles);
	AssertContainsFiles(&acFiles, "2062", "23", NULL);
	acFiles.Kill();


	cController.Begin();

	AssertTrue(cAccess.PutStringString(a.GetName(), "Blobfish"));
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString("Blobfish", sz);

	cAccess.Flush();
	cController.End();

	acFiles.Init();
	cFileUtil.FindFilesWithExtension(szDirectory, "IDAT", &acFiles);
	AssertContainsFiles(&acFiles, "2062", "23", NULL);
	acFiles.Kill();

	cController.Begin();

	AssertTrue(cAccess.PutStringString(a.GetName(), "Avagadrocardo"));
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString("Avagadrocardo", sz);

	AssertTrue(cAccess.DeleteString(a.GetName()));
	AssertNull(cAccess.GetStringString(a.GetName(), sz));

	cAccess.Flush();
	cController.End();

	acFiles.Init();
	cFileUtil.FindFilesWithExtension(szDirectory, "IDAT", &acFiles);
	AssertContainsFiles(&acFiles, "2062", "23", NULL);
	acFiles.Kill();


	cController.Begin();

	AssertTrue(cAccess.PutStringString(a.GetName(), "Escape"));
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString("Escape", sz);

	cAccess.Flush();
	cController.End();

	acFiles.Init();
	cFileUtil.FindFilesWithExtension(szDirectory, "IDAT", &acFiles);
	AssertContainsFiles(&acFiles, "2062", "23", "21", NULL);
	acFiles.Kill();

	cController.Begin();

	AssertTrue(cAccess.DeleteString(a.GetName()));
	pcNode = cIndexTree.GetNode(a.GetName(), a.NameLength());
	AssertTrue(pcNode->IsDeleted());
	AssertTrue(pcNode->IsPathDeleted());
	AssertFalse(pcNode->IsDirty());
	AssertFalse(pcNode->IsPathDirty());
	AssertNull(cAccess.GetStringString(a.GetName(), sz));

	AssertTrue(cAccess.PutStringString(a.GetName(), "Pentatonix"));
	pcNode = cIndexTree.GetNode(a.GetName(), a.NameLength());
	AssertFalse(pcNode->IsDeleted());
	AssertFalse(pcNode->IsPathDeleted());
	AssertTrue(pcNode->IsDirty());
	AssertTrue(pcNode->IsPathDirty());
	AssertNotNull(cAccess.GetStringString(a.GetName(), sz));
	AssertString("Pentatonix", sz);

	cAccess.Flush();
	cController.End();

	acFiles.Init();
	cFileUtil.FindFilesWithExtension(szDirectory, "IDAT", &acFiles);
	AssertContainsFiles(&acFiles, "2062", "23", "21", "25", NULL);
	acFiles.Kill();

	cIndexTree.Kill();
	cController.Kill();
	cAccess.Kill();

	AssertIndexFileEmpty("Output/IndexTree9b/primary/23_0.IDAT");
	AssertIndexFileEmpty("Output/IndexTree9b/primary/21_0.IDAT");

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRead(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess		cAccess;
	char						sz[MAX_DATA_SIZE];
	CIndexTreeNodeFile*			pcHell;
	uint32						iOldFile;
	uint32						iNewFile;
	CIndexedFile*				pcFile;

	cHelper.Init("Output" _FS_"IndexTreeA", "primary", "backup", true);
	cHelper.RemoveWorkingDirectory();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringString("Hello", "World");

	cAccess.Flush();
	cController.End();
	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	AssertTrue(cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory()));
	cController.SetSync(DS_No);
	AssertTrue(cController.Begin());
	AssertTrue(cIndexTree.Init(&cController, NULL, IWT_No, IKR_No));
	AssertTrue(cAccess.Init(&cIndexTree));

	AssertNotNull(cAccess.GetStringString("Hello", sz));
	AssertString("World", sz)

	AssertTrue(cAccess.Flush());
	pcHell = cIndexTree.GetNode("Hell", 4);
	iOldFile = pcHell->GetFileIndex()->miFile;
	pcFile = cIndexTree.GetFile(iOldFile);
	AssertNotNull(pcFile);
	AssertLSize(22, pcFile->GetDataSize());
	AssertTrue(cController.End());
	cIndexTree.Kill();
	cController.Kill();

	AssertTrue(cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory()));
	cController.SetSync(DS_No);
	AssertTrue(cController.Begin());
	AssertTrue(cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No));
	AssertTrue(cController.End());

	AssertTrue(cController.Begin());
	AssertTrue(cAccess.PutStringString("Hell", "Fuzz"));
	pcHell = cIndexTree.GetNode("Hell", 4);
	iNewFile = pcHell->GetFileIndex()->miFile;
	AssertTrue(iOldFile != iNewFile);
	pcFile = cIndexTree.GetFile(iNewFile);
	AssertNotNull(pcFile);
	AssertLSize(27, pcFile->GetDataSize());
	AssertTrue(cIndexTree.ValidateIndexTree());
	AssertTrue(cController.End());
	iOldFile = iNewFile;

	AssertTrue(cController.Begin());
	AssertNotNull(cAccess.GetStringString("Hell", sz));
	AssertString("Fuzz", sz);
	AssertNotNull(cAccess.GetStringString("Hello", sz));
	AssertString("World", sz);
	AssertTrue(cController.End());

	cIndexTree.Kill();

	AssertTrue(cController.Begin());
	AssertTrue(cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No));
	AssertNotNull(cAccess.GetStringString("Hell", sz));
	AssertString("Fuzz", sz);
	pcHell = cIndexTree.GetNode("Hell", 4);
	iNewFile = pcHell->GetFileIndex()->miFile;
	AssertTrue(iOldFile == iNewFile);
	AssertTrue(cController.End());
	cIndexTree.Kill();

	AssertTrue(cController.Begin());
	AssertTrue(cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No));
	AssertTrue(cController.End());

	AssertTrue(cController.Begin());
	AssertTrue(cAccess.PutStringString("HelloX", "3"));
	AssertTrue(cController.End());

	AssertTrue(cController.Begin());
	AssertNotNull(cAccess.GetStringString("Hell", sz));
	AssertString("Fuzz", sz);
	AssertNotNull(cAccess.GetStringString("HelloX", sz));
	AssertString("3", sz);
	AssertNotNull(cAccess.GetStringString("Hello", sz));
	AssertString("World", sz);
	AssertTrue(cController.End());
	cIndexTree.Kill();

	AssertTrue(cAccess.Kill());
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileDeleteOnDisk(void)
{
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;
	CTestIndexTreeFile		cIndexTree;
	CIndexTreeFileAccess	cAccess;
	char					sz[MAX_DATA_SIZE];

	cHelper.Init("Output" _FS_"IndexTreeB", "primary", "backup", true);
	cHelper.RemoveWorkingDirectory();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);
	cAccess.PutStringString("Hello", "World");
	cController.End();
	cIndexTree.Kill();

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.DeleteString("Hello");
	cController.End();
	cIndexTree.Kill();

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	AssertNull(cAccess.GetStringString("Hello", sz));
	cIndexTree.GetNode("H", 1);
	cController.End();
	cIndexTree.Kill();

	AssertIndexFileEmpty("Output/IndexTreeB/primary/20_0.IDAT");
	AssertIndexFileEmpty("Output/IndexTreeB/primary/22_0.IDAT");

	cAccess.Kill();
	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileComplex(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess		cAccess;
	char						sz[MAX_DATA_SIZE];
	CMapStringString			cMap;
	SMapIterator				sIter;
	bool						bHasNext;
	char*						szKey;
	char*						szValue;

	cMap.Init();

	cMap.Put("bane", "poison");
	cMap.Put("baseborn", "of low birth or social standing");
	cMap.Put("bedlam", "an asylum");
	cMap.Put("behoo", "see or observe");
	cMap.Put("behoof", "benefit or advantage");
	cMap.Put("beldam", "an old woman");
	cMap.Put("bethink oneself of", "remember; recollect");
	cMap.Put("betimes", "in good time; early");
	cMap.Put("bibliopole", "a dealer in books");
	cMap.Put("bijoux", "jewellery; trinkets");
	cMap.Put("billow", "a large sea wave");
	cMap.Put("blackguard", "a scoundrel");
	cMap.Put("blow", "produce flowers or be in flower");
	cMap.Put("bodkin", "a dagger");
	cMap.Put("bootless", "ineffectual; useless");
	cMap.Put("breech", "a person's buttocks");
	cMap.Put("bridewell", "a prison or reform school for petty offenders");
	cMap.Put("brimstone", "sulphur");
	cMap.Put("bruit", "a report or rumour");
	cMap.Put("buck", "a fashionable and daring young man");
	cMap.Put("bumper", "a generous glass of an alcoholic drink");
	cMap.Put("burgess", "a full citizen of a town or borough");
	cMap.Put("buss", "a kiss");

	cMap.Put("dame", "an elderly or mature woman");
	cMap.Put("damsel", "a young unmarried woman");
	cMap.Put("dandiprat", "a young or insignificant person");
	cMap.Put("darbies", "handcuffs");
	cMap.Put("dark", "ignorant");
	cMap.Put("degrade", "reduce to a lower rank, especially as a punishment");
	cMap.Put("degree", "social or official rank");
	cMap.Put("delate", "report(an offence)");
	cMap.Put("demesne", "a region or domain");
	cMap.Put("demit", "resign from(an office or position)");
	cMap.Put("demoralize", "corrupt the morals of");
	cMap.Put("dight", "clothed or equipped");
	cMap.Put("discover", "divulge(a secret)");
	cMap.Put("disport", "frolic");
	cMap.Put("dispraise", "censure or criticize");
	cMap.Put("divers", "of varying types; several");
	cMap.Put("doti", "a very small amount of money");
	cMap.Put("dot", "a dowry from which only the interest or annual income was available to the husband");
	cMap.Put("doxy", "a lover or mistress");
	cMap.Put("drab", "a slovenly woman");
	cMap.Put("drought", "thirst");

	cHelper.Init("Output" _FS_"IndexTreeC", "primary", "backup", true);
	cHelper.RemoveWorkingDirectory();

	//////////////////////////////////////////////////////////////////////////////////////
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	bHasNext = cMap.StartIteration(&sIter, (void**)&szKey, NULL, (void**)&szValue, NULL);
	while (bHasNext)
	{
		AssertTrue(cAccess.PutStringString(szKey, szValue));
		cIndexTree.ValidateKey(szKey, strlen(szKey));
		bHasNext = cMap.Iterate(&sIter, (void**)&szKey, NULL, (void**)&szValue, NULL);
	}
	AssertLSize(cMap.NumElements(), cIndexTree.NumElements());
	AssertTrue(cIndexTree.ValidateIndexTree());

	bHasNext = cMap.StartIteration(&sIter, (void**)&szKey, NULL, (void**)&szValue, NULL);
	while (bHasNext)
	{
		AssertNotNull(cAccess.GetStringString(szKey, sz));
		szValue = cMap.Get(szKey);
		AssertString(szValue, sz);

		bHasNext = cMap.Iterate(&sIter, (void**)&szKey, NULL, (void**)&szValue, NULL);
	}

	cController.End();
	cIndexTree.Kill();
	cAccess.Kill();
	cController.Kill();

	//////////////////////////////////////////////////////////////////////////////////////
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	bHasNext = cMap.StartIteration(&sIter, (void**)&szKey, NULL, (void**)&szValue, NULL);
	while (bHasNext)
	{
		AssertNotNull(cAccess.GetStringString(szKey, sz));
		szValue = cMap.Get(szKey);
		AssertString(szValue, sz);
		
		bHasNext = cMap.Iterate(&sIter, (void**)&szKey, NULL, (void**)&szValue, NULL);
	}

	cController.End();
	cIndexTree.Kill();
	cAccess.Kill();
	cController.Kill();

	cMap.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileAddToRoot(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile				cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CTestIndexTreeObject2		a;
	CTestIndexTreeObject2		b;
	bool						bResult;

	//This is supposed to be checking that the nodes indexed in file are in sync with the nodes pointed to in memory.
	cHelper.Init("Output" _FS_"IndexTreeD", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	a.Init("A1");
	bResult = cAccess.PutStringPtr(a.GetName(), &a);
	AssertTrue(bResult);
	cIndexTree.ValidateKey(a.GetName(), a.NameLength());
	AssertTrue(cIndexTree.ValidateIndexTree());

	b.Init("D1");
	bResult = cAccess.PutStringPtr(b.GetName(), &b);
	cIndexTree.ValidateKey(a.GetName(), a.NameLength());
	cIndexTree.ValidateKey(b.GetName(), b.NameLength());
	AssertTrue(bResult);
	AssertTrue(cIndexTree.ValidateIndexTree());

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileMemorySize(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CTestIndexTreeObject2		aaa;
	CTestIndexTreeObject2		aab;
	bool						bResult;
	size						tNodeSize;
	size						tDataNodeSize;
	size						tNodePtrSize;
	size						tRootNodeSize;
	size						tFourNodesSize;
	size						tFiveNodesSize;
	size						iNumNodes;

	cHelper.Init("Output" _FS_"IndexTreeE", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	tNodeSize = cIndexTree.SizeofNode();
	AssertSize(sizeof(CIndexTreeNodeFile), tNodeSize);
	tDataNodeSize = cIndexTree.SizeofDataNode();
	AssertSize(sizeof(CIndexTreeNodeFile) + sizeof(CIndexTreeDataNode), tDataNodeSize);
	tNodePtrSize = cIndexTree.SizeofNodePtr();
	AssertSize(sizeof(CIndexTreeChildNode), tNodePtrSize);

	AssertLSize(1, cIndexTree.NumMemoryNodes());
	tRootNodeSize = tNodeSize + 256 * tNodePtrSize;
	AssertSize(tRootNodeSize, cIndexTree.ByteSize());
	AssertSize(tRootNodeSize, cIndexTree.GetUserMemorySize());
	AssertSize(tRootNodeSize + sizeof(SCountingMemoryAllocation), cIndexTree.GetSystemMemorySize());

	aaa.Init("AAA");
	bResult = cAccess.PutStringPtr(aaa.GetName(), &aaa);
	AssertTrue(bResult);
	Pass();

	iNumNodes = cIndexTree.NumMemoryNodes();
	tFourNodesSize = tRootNodeSize + 2 * (tNodeSize + tNodePtrSize) + (tDataNodeSize + sizeof(void*));
	AssertSize(4, iNumNodes);
	AssertSize(tRootNodeSize + (tNodeSize + tNodePtrSize) * 2 + (tDataNodeSize + sizeof(void*)), cIndexTree.ByteSize());
	AssertSize(tFourNodesSize, cIndexTree.GetUserMemorySize());
	AssertSize(tFourNodesSize + iNumNodes * sizeof(SCountingMemoryAllocation), cIndexTree.GetSystemMemorySize());

	aab.Init("AAB");
	bResult = cAccess.PutStringPtr(aab.GetName(), &aab);
	AssertTrue(bResult);

	iNumNodes = cIndexTree.NumMemoryNodes();
	tFiveNodesSize = tRootNodeSize + (tNodeSize + tNodePtrSize) + (tNodeSize + tNodePtrSize + tNodePtrSize) + 2 * (tDataNodeSize + sizeof(void*));
	AssertSize(5, iNumNodes);
	AssertSize(tFiveNodesSize, cIndexTree.ByteSize());
	AssertSize(tFiveNodesSize, cIndexTree.GetUserMemorySize());
	AssertSize(tFiveNodesSize + iNumNodes * sizeof(SCountingMemoryAllocation), cIndexTree.GetSystemMemorySize());

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileEvictNew(EIndexWriteThrough eWriteThrough)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess		cAccess;
	char						szAAA[] = "North";
	char						szAAAAA[] = "Volcano";
	char						szA[] = "Tier";
	char						szAAAB[] = "Abland";
	CMemoryAllocator			cAllocator;
	CGeneralMemory*				pcMemory;

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();

	cHelper.Init("Output" _FS_"IndexTreeF", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, LifeLocal<CMallocator>(&cAllocator), eWriteThrough, IKR_No);
	cAccess.Init(&cIndexTree);

	AssertInt((uint32)0, cIndexTree.NumElements());
	AssertLSize(1, cIndexTree.NumNodes());
	AssertLSize(0, cIndexTree.NumMemoryElements());
	AssertLSize(1, cIndexTree.NumMemoryNodes());
#ifdef BUILD_64
	AssertLong(4128, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3096, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64

	cAccess.PutStringData("AAA", szAAA, strlen(szAAA) + 1);
	cAccess.PutStringData("AAAAA", szAAAAA, strlen(szAAAAA) + 1);
	cAccess.PutStringData("A", szA, strlen(szA) + 1);
	cAccess.PutStringData("AAAB", szAAAB, strlen(szAAAB) + 1);
#ifdef BUILD_64
	AssertLong(4522LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3374LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	AssertInt((uint32)4, cIndexTree.NumElements());
	AssertLSize(7, cIndexTree.NumNodes());
	AssertLSize(4, cIndexTree.NumMemoryElements());
	AssertLSize(7, cIndexTree.NumMemoryNodes());
	Pass();

	AssertTrue(cAccess.EvictString("AAAAA"));
#ifdef BUILD_64
	AssertLong(4410LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3294LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	AssertLSize(3, cIndexTree.NumMemoryElements());
	AssertLSize(5, cIndexTree.NumMemoryNodes());
	Pass();

	AssertTrue(cAccess.EvictString("AAAB"));
#ifdef BUILD_64
	AssertLong(4347LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3251LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	AssertLSize(2, cIndexTree.NumMemoryElements());
	AssertLSize(4, cIndexTree.NumMemoryNodes());
	AssertTrue(cAccess.EvictString("AAA"));
#ifdef BUILD_64
	AssertLong(4205LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3149LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	AssertLSize(1, cIndexTree.NumMemoryElements());
	AssertLSize(2, cIndexTree.NumMemoryNodes());
	AssertTrue(cAccess.EvictString("A"));
#ifdef BUILD_64
	AssertLong(4128, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3096, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	AssertLSize(0, cIndexTree.NumMemoryElements());
	AssertLSize(1, cIndexTree.NumMemoryNodes());
	Pass();

	AssertTrue(cAccess.Flush());
	AssertTrue(cIndexTree.ValidateIndexTree());
	AssertTrue(cAccess.HasString("A"));
	AssertTrue(cAccess.HasString("AAA"));
	AssertTrue(cAccess.HasString("AAAB"));
	AssertTrue(cAccess.HasString("AAAAA"));
	AssertLSize(4, cIndexTree.NumMemoryElements());
	AssertLSize(7, cIndexTree.NumMemoryNodes());
	AssertInt((uint32)4, cIndexTree.NumElements());
	AssertLSize(7, cIndexTree.NumNodes());
	Pass();

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileEvictDirty(EIndexWriteThrough eWriteThrough)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CTestIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess		cAccess;
	CMemoryAllocator			cAllocator;
	SLogConfig					sLogConfig;

	cAllocator.Init();

	cHelper.Init("Output" _FS_"IndexTreeFa", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, LifeLocal<CMallocator>(&cAllocator), eWriteThrough, IKR_No);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringString("AAA", "North");
	cAccess.PutStringString("AAAAA", "Volcano");
	cAccess.PutStringString("A", "Tier");
	cAccess.PutStringString("AAAB", "Abland");
	AssertTrue(cAccess.Flush());
	AssertLSize(4, cIndexTree.NumMemoryElements());
	AssertTrue(cIndexTree.ValidateIndexTree(false));

	cAccess.PutStringString("AAA", "South");
	cAccess.PutStringString("AAAAA", "Dome");

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cAccess.EvictString("AAA"));
	gcLogger.SetConfig(&sLogConfig);
	AssertLSize(4, cIndexTree.NumMemoryElements());
	AssertTrue(cAccess.EvictString("AAAAA"));
	AssertLSize(3, cIndexTree.NumMemoryElements());
	AssertTrue(cAccess.EvictString("AAAB"));
	AssertLSize(2, cIndexTree.NumMemoryElements());
	AssertTrue(cAccess.EvictString("AAA"));
	AssertTrue(cIndexTree.ValidateIndexTree(false));
	AssertLSize(1, cIndexTree.NumMemoryElements());
	AssertTrue(cAccess.EvictString("A"));
	AssertLSize(0, cIndexTree.NumMemoryElements());
	AssertTrue(cIndexTree.ValidateIndexTree(false));

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();

	cHelper.Kill(true);
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileEvictComplexSetup(CDurableFileController* pcDurableController, CTestIndexTreeFile* pcIndexTree, CMemoryAllocator* pcAllocator)
{
	CIndexTreeFileAccess		cAccess;
	char						szAAAAA[] = "AAAAA";
	char						szAAABB[] = "AAABB";
	char						szAABAA[] = "AABAA";
	char						szAABBB[] = "AABBB";
	char						szAACAA[] = "AACAA";
	char						szAACBB[] = "AACBB";
	char						szDest[16];
	EIndexWriteThrough			eWriteThrough;
	CGeneralMemory*				pcMemory;

	eWriteThrough = IWT_No;
	pcMemory = pcAllocator->GetMemory();

	pcDurableController->Begin();
	pcIndexTree->Init(pcDurableController, NULL, LifeLocal<CMallocator>(pcAllocator), eWriteThrough, IKR_No);
	cAccess.Init(pcIndexTree);

	AssertTrue(cAccess.PutStringString(szAAAAA, szAAAAA));
	AssertTrue(cAccess.PutStringString(szAAABB, szAAABB));
	AssertTrue(cAccess.PutStringString(szAABAA, szAABAA));
	AssertTrue(cAccess.PutStringString(szAABBB, szAABBB));
	AssertTrue(cAccess.PutStringString(szAACAA, szAACAA));
	AssertTrue(cAccess.PutStringString(szAACBB, szAACBB));
	Pass();

#ifdef BUILD_64
	AssertLong(5108LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3804LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	AssertLSize(6, pcIndexTree->NumMemoryElements());
	AssertLSize(18, pcIndexTree->NumMemoryNodes());
	AssertInt((uint32)6, pcIndexTree->NumElements());
	AssertLSize(18, pcIndexTree->NumNodes());
	Pass();

	AssertTrue(cAccess.Flush());
	AssertTrue(pcIndexTree->ValidateIndexTree());
	Pass();

	pcDurableController->End();
	pcIndexTree->Kill();
	AssertLong(0, pcMemory->GetTotalAllocatedMemory());


	pcDurableController->Begin();
	pcIndexTree->Init(pcDurableController, NULL, LifeLocal<CMallocator>(pcAllocator), eWriteThrough, IKR_No);
	cAccess.Init(pcIndexTree);
#ifdef BUILD_64
	AssertLong(4160LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3120LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertLSize(0, pcIndexTree->NumMemoryElements());
	AssertLSize(1, pcIndexTree->NumMemoryNodes());
	AssertInt((uint32)6, pcIndexTree->NumElements());
	AssertLSize(18, pcIndexTree->NumNodes());
	AssertLSize(6, pcIndexTree->NumMemoryElements());
	AssertLSize(18, pcIndexTree->NumMemoryNodes());
#ifdef BUILD_64
	AssertLong(5236LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3900LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertNotNull(cAccess.GetStringString(szAAAAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAAABB, szDest));
	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNotNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAACBB, szDest));
	Pass();

	AssertTrue(cAccess.PutStringString(szAAAAA, "Update 1"));
	AssertTrue(cAccess.PutStringString(szAAABB, "Update 2"));
	AssertTrue(cAccess.PutStringString(szAABAA, "Update 3"));
	AssertTrue(cAccess.DeleteString(szAABBB));
	AssertTrue(cAccess.DeleteString(szAACAA));
	AssertTrue(cAccess.DeleteString(szAACBB));
	Pass();

	cAccess.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileEvictComplexEvictCloseGet(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CGeneralMemory*				pcMemory;
	CTestIndexTreeFile			cIndexTree;
	CMemoryAllocator			cAllocator;
	char						szAAAAA[] = "AAAAA";
	char						szAAABB[] = "AAABB";
	char						szAABAA[] = "AABAA";
	char						szAABBB[] = "AABBB";
	char						szAACAA[] = "AACAA";
	char						szAACBB[] = "AACBB";
	CIndexTreeFileAccess		cAccess;
	char						szDest[16];

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();
	
	cHelper.Init("Output" _FS_"IndexTreeG", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	TestIndexTreeFileEvictComplexSetup(&cController, &cIndexTree, &cAllocator);
	cAccess.Init(&cIndexTree);
#ifdef BUILD_64
	AssertLong(5197LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3873LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	//Note: TestIndexTreeFileEvictComplexSetup does the following
	// 
	//PutStringString(szAAAAA, "Update 1")
	//PutStringString(szAAABB, "Update 2")
	//PutStringString(szAABAA, "Update 3")
	//DeleteString(szAABBB)
	//DeleteString(szAACAA)
	//DeleteString(szAACBB)

	AssertTrue(cAccess.EvictString(szAABBB));
	AssertTrue(cAccess.EvictString(szAAAAA));
	AssertTrue(cAccess.EvictString(szAAABB));
	AssertTrue(cAccess.EvictString(szAABAA));
	AssertTrue(cAccess.EvictString(szAACAA));
	AssertTrue(cAccess.EvictString(szAACBB));
#ifdef BUILD_64
	AssertLong(4272LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3204LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertLSize(0, cIndexTree.NumMemoryElements());
	AssertLSize(3, cIndexTree.NumMemoryNodes());
	AssertInt((uint32)3, cIndexTree.NumElements());
	AssertLSize(11, cIndexTree.NumNodes());
	Pass();

	//Also test to make sure that the CIndexedFiles in the tree reflect the correct number of allocated and deleted nodes.

	cController.End();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();
	cAccess.Kill();

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexTree.Init(&cController, NULL, LifeLocal<CMallocator>(&cAllocator), IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);
	AssertNotNull(cAccess.GetStringString(szAAAAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAAABB, szDest));
	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	Pass();

	AssertNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNull(cAccess.GetStringString(szAACBB, szDest));
	AssertTrue(cIndexTree.ValidateIndexTree());
	Pass();

	cController.End();
	cIndexTree.Kill();

	cController.Kill();
	cAllocator.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileEvictComplexEvictGetClose(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CGeneralMemory*				pcMemory;
	CTestIndexTreeFile			cIndexTree;
	CMemoryAllocator			cAllocator;
	char						szAAAAA[] = "AAAAA";
	char						szAAABB[] = "AAABB";
	char						szAABAA[] = "AABAA";
	char						szAABBB[] = "AABBB";
	char						szAACAA[] = "AACAA";
	char						szAACBB[] = "AACBB";
	CIndexTreeFileAccess		cAccess;
	char						szDest[16];

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();

	cHelper.Init("Output" _FS_"IndexTreeG", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	TestIndexTreeFileEvictComplexSetup(&cController, &cIndexTree, &cAllocator);
	cAccess.Init(&cIndexTree);
#ifdef BUILD_64
	AssertLong(5197LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3873LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64

	AssertTrue(cAccess.EvictString(szAABBB));
	AssertTrue(cAccess.EvictString(szAAAAA));
	AssertTrue(cAccess.EvictString(szAAABB));
	AssertTrue(cAccess.EvictString(szAABAA));
	AssertTrue(cAccess.EvictString(szAACAA));
	AssertTrue(cAccess.EvictString(szAACBB));
	Pass();

	AssertLSize(0, cIndexTree.NumMemoryElements());
	AssertLSize(3, cIndexTree.NumMemoryNodes());
	AssertInt((uint32)3, cIndexTree.NumElements());
	AssertLSize(11, cIndexTree.NumNodes());
	Pass();

	cAccess.Init(&cIndexTree);
	AssertNotNull(cAccess.GetStringString(szAAAAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAAABB, szDest));
	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	AssertNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNull(cAccess.GetStringString(szAACBB, szDest));
	Pass();

	cController.End();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();
	cAccess.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileEvictComplexEvictOdd(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CGeneralMemory*				pcMemory;
	CTestIndexTreeFile			cIndexTree;
	CMemoryAllocator			cAllocator;
	char						szAAAAA[] = "AAAAA";
	char						szAAABB[] = "AAABB";
	char						szAABAA[] = "AABAA";
	char						szAABBB[] = "AABBB";
	char						szAACAA[] = "AACAA";
	char						szAACBB[] = "AACBB";
	CIndexTreeFileAccess		cAccess;
	char						szDest[16];

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();

	cHelper.Init("Output" _FS_"IndexTreeG", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	TestIndexTreeFileEvictComplexSetup(&cController, &cIndexTree, &cAllocator);
	cAccess.Init(&cIndexTree);
#ifdef BUILD_64
	AssertLong(5197LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3873LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertTrue(cAccess.EvictString(szAABBB));
	AssertTrue(cAccess.EvictString(szAAABB));
	AssertTrue(cAccess.EvictString(szAACAA));
	Pass();


	AssertLSize(2, cIndexTree.NumMemoryElements());
	AssertLSize(12, cIndexTree.NumMemoryNodes());
	Pass();

	cAccess.Init(&cIndexTree);
	AssertNotNull(cAccess.GetStringString(szAAAAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAAABB, szDest));
	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	AssertNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNull(cAccess.GetStringString(szAACBB, szDest));
	Pass();

	cAccess.Flush();
	cController.End();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();
	cAccess.Kill();

	cAllocator.Init();
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexTree.Init(&cController, NULL, LifeLocal<CMallocator>(&cAllocator), IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);
	AssertNotNull(cAccess.GetStringString(szAAAAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAAABB, szDest));
	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	Pass();

	AssertNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNull(cAccess.GetStringString(szAACBB, szDest));
	Pass();

	cController.End();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();
	cAccess.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileEvictComplexEvictEven(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CGeneralMemory*				pcMemory;
	CTestIndexTreeFile			cIndexTree;
	CMemoryAllocator			cAllocator;
	char						szAAAAA[] = "AAAAA";
	char						szAAABB[] = "AAABB";
	char						szAABAA[] = "AABAA";
	char						szAABBB[] = "AABBB";
	char						szAACAA[] = "AACAA";
	char						szAACBB[] = "AACBB";
	CIndexTreeFileAccess		cAccess;
	char						szDest[16];

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();

	cHelper.Init("Output" _FS_"IndexTreeG", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	TestIndexTreeFileEvictComplexSetup(&cController, &cIndexTree, &cAllocator);
	cAccess.Init(&cIndexTree);
#ifdef BUILD_64
	AssertLong(5197LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3873LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertTrue(cAccess.EvictString(szAABAA));
	AssertTrue(cAccess.EvictString(szAAAAA));
	AssertTrue(cAccess.EvictString(szAACBB));
	Pass();

	AssertLSize(1, cIndexTree.NumMemoryElements());
	AssertLSize(12, cIndexTree.NumMemoryNodes());
	Pass();

	cAccess.Init(&cIndexTree);
	AssertNotNull(cAccess.GetStringString(szAAAAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAAABB, szDest));
	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	AssertNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNull(cAccess.GetStringString(szAACBB, szDest));
	Pass();

	cAccess.Flush();
	cController.End();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();
	cAccess.Kill();

	cAllocator.Init();
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexTree.Init(&cController, NULL, LifeLocal<CMallocator>(&cAllocator), IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);
	AssertNotNull(cAccess.GetStringString(szAAAAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAAABB, szDest));
	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	AssertNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNull(cAccess.GetStringString(szAACBB, szDest));
	cController.End();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();
	cAccess.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileFlushNodes(void)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CGeneralMemory*				pcMemory;
	CTestIndexTreeFile			cIndexTree;
	CMemoryAllocator			cAllocator;
	char						szAAAAA[] = "AAAAA";
	char						szAAABB[] = "AAABB";
	char						szAABAA[] = "AABAA";
	char						szAABBB[] = "AABBB";
	char						szAACAA[] = "AACAA";
	char						szAACBB[] = "AACBB";
	CIndexTreeFileAccess		cAccess;
	char						szDest[16];

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();

	cHelper.Init("Output" _FS_"IndexTreeH", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	TestIndexTreeFileEvictComplexSetup(&cController, &cIndexTree, &cAllocator);
	cAccess.Init(&cIndexTree);
#ifdef BUILD_64
	AssertLong(5197LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3873LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64

	cAccess.Init(&cIndexTree);
	AssertTrue(cAccess.FlushString(szAAAAA));
	AssertTrue(cAccess.FlushString(szAAABB));
	AssertTrue(cAccess.FlushString(szAABAA));
	AssertTrue(cAccess.FlushString(szAABBB));
	AssertTrue(cAccess.FlushString(szAACAA));
	AssertTrue(cAccess.FlushString(szAACBB));

	cController.End();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();
	cAccess.Kill();

	cAllocator.Init();
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cController.Begin();
	cIndexTree.Init(&cController, NULL, LifeLocal<CMallocator>(&cAllocator), IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);
	AssertNotNull(cAccess.GetStringString(szAAAAA, szDest));
	AssertNotNull(cAccess.GetStringString(szAAABB, szDest));
	AssertNotNull(cAccess.GetStringString(szAABAA, szDest));
	AssertNull(cAccess.GetStringString(szAABBB, szDest));
	AssertNull(cAccess.GetStringString(szAACAA, szDest));
	AssertNull(cAccess.GetStringString(szAACBB, szDest));
	cController.End();
	cIndexTree.Kill();
	cController.Kill();
	cAllocator.Kill();
	cAccess.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileRemoveComplex(EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	CIndexTreeHelper			cHelper;
	CDurableFileController		cController;
	CIndexTreeFile				cIndexTree;
	char						szData[256];
	size						uiSize;
	char						szSmellsLikeTeenSpirit[] = { "Smells Like Teen Spirit" };
	char						szSeizedPotPlants[] = { "Seized pot plants turn out to be daisies" };
	char						szCallingFromWindows[] = { "I am calling you from Windows" };
	CIndexTreeFileAccess		cAccess;


	cHelper.Init("Output" _FS_"IndexTreeI", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, eKeyReverse);
	cAccess.Init(&cIndexTree);
	cController.End();

	cController.Begin();
	cAccess.PutLongString(0x7634, szSmellsLikeTeenSpirit);
	cAccess.PutLongString(0x3589, szSeizedPotPlants);
	cAccess.PutLongString(0x8743, szCallingFromWindows);
	AssertLong(3, cIndexTree.NumElements());
	AssertTrue(cAccess.HasLong(0x7634));
	AssertTrue(cAccess.HasLong(0x3589));
	AssertTrue(cAccess.HasLong(0x8743));

	AssertTrue(cAccess.Flush());
	cController.End();
	cIndexTree.Kill();
	cAccess.Kill();


	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, eKeyReverse);
	cAccess.Init(&cIndexTree);
	cController.End();

	cController.Begin();
	AssertTrue(cAccess.HasLong(0x7634));
	AssertTrue(cAccess.HasLong(0x3589));
	AssertTrue(cAccess.HasLong(0x8743));
	cController.End();
	cIndexTree.Kill();
	cAccess.Kill();


	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, eKeyReverse);
	cAccess.Init(&cIndexTree);
	cController.End();

	cController.Begin();
	AssertTrue(cAccess.DeleteLong(0x7634));
	AssertLong(2, cIndexTree.NumElements());
	AssertFalse(cAccess.HasLong(0x7634));
	AssertTrue(cAccess.HasLong(0x3589));
	AssertTrue(cAccess.HasLong(0x8743));
	AssertFalse(cAccess.GetLongData(0x7634, szData, &uiSize));

	AssertTrue(cAccess.Flush());
	cController.End();
	cIndexTree.Kill();
	cAccess.Kill();


	cController.Begin();
	cIndexTree.Init(&cController, NULL, eWriteThrough, eKeyReverse);
	cAccess.Init(&cIndexTree);
	cController.End();

	cController.Begin();
	AssertTrue(cAccess.DeleteLong(0x3589));
	AssertTrue(cAccess.DeleteLong(0x8743));
	AssertFalse(cAccess.HasLong(0x7634));
	AssertFalse(cAccess.HasLong(0x3589));
	AssertFalse(cAccess.HasLong(0x8743));
	AssertLong(0, cIndexTree.NumElements());
	AssertFalse(cAccess.DeleteLong(0x7634));
	AssertFalse(cAccess.DeleteLong(0x3589));
	AssertFalse(cAccess.DeleteLong(0x8743));

	AssertTrue(cAccess.Flush());
	AssertFalse(cAccess.DeleteLong(0x7634));
	AssertFalse(cAccess.DeleteLong(0x3589));
	AssertFalse(cAccess.DeleteLong(0x8743));

	cController.End();
	cController.Kill();
	cIndexTree.Kill();
	cAccess.Kill();


	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFile(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestIndexTreeFileSizeOfs();
	TestIndexTreeFileInit();
	TestIndexTreeFileAddSimple(IWT_No, IKR_No);
	TestIndexTreeFileAddSimple(IWT_No, IKR_Yes);
	TestIndexTreeFileAddSimple(IWT_Yes, IKR_Yes);
	TestIndexTreeFileAddSimple(IWT_Yes, IKR_No);
	TestIndexTreeFileAdd(IWT_No, IKR_No);
	TestIndexTreeFileAdd(IWT_No, IKR_Yes);
	TestIndexTreeFileAdd(IWT_Yes, IKR_Yes);
	TestIndexTreeFileAdd(IWT_Yes, IKR_No);
	TestIndexTreeFileAddLongKeys(IWT_No, IKR_Yes);
	TestIndexTreeFileAddLongKeys(IWT_Yes, IKR_Yes);
	TestIndexTreeFileAddLongKeys(IWT_No, IKR_No);
	TestIndexTreeFileAddLongKeys(IWT_Yes, IKR_No);
	TestIndexTreeFileGetNodeKey();
	TestIndexTreeFileMemorySize();
	TestIndexTreeFileAddToRoot();
	TestIndexTreeFileAddUnallocated();
	TestIndexTreeFileReplaceData();
	TestIndexTreeFileFindKey();
	TestIndexTreeFileResizeData();
	TestIndexTreeFileRemove(IWT_Yes);
	TestIndexTreeFileRemove(IWT_No);
	TestIndexTreeFileFlushDirtyAndDeleted();
	TestIndexTreeFileRemoveAndEvict();
	TestIndexTreeFileRemoveNearestFirstFlushed();
	TestIndexTreeFileRemoveNearestFirst(IWT_Yes);
	TestIndexTreeFileRemoveNearestFirst(IWT_No);
	TestIndexTreeFileRemoveFurthestFirst(IWT_Yes);
	TestIndexTreeFileRemoveFurthestFirst(IWT_No);
	TestIndexTreeFileRemoveAndFlush();
	TestIndexTreeFileNoCacheEviction();
	TestIndexTreeFileDeleteInMemory();
	TestIndexTreeFileDirty();
	TestIndexTreeFileRemoveBeforeFlush();
	TestIndexTreeFileRemoveThenDirty();
	TestIndexTreeFileAddThenAdd();
	TestIndexTreeFileRead();
	TestIndexTreeFileDeleteOnDisk();
	TestIndexTreeFileComplex();
	TestIndexTreeFileEvictNew(IWT_Yes);
	TestIndexTreeFileEvictNew(IWT_No);
	TestIndexTreeFileEvictDirty(IWT_Yes);
	TestIndexTreeFileEvictDirty(IWT_No);
	TestIndexTreeFileEvictComplexEvictCloseGet();
	TestIndexTreeFileEvictComplexEvictGetClose();
	TestIndexTreeFileEvictComplexEvictOdd();
	TestIndexTreeFileEvictComplexEvictEven();
	TestIndexTreeFileFlushNodes();
	TestIndexTreeFileRemoveComplex(IWT_No, IKR_No);
	TestIndexTreeFileRemoveComplex(IWT_Yes, IKR_No);

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

