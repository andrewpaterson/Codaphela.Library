#include "BaseLib/SetBlock.h"
#include "BaseLib/SetString.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/CountingAllocator.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/LogString.h"
#include "TestLib/Words.h"
#include "TestLib/Assert.h"
#include "WordKey.h"


class CTestSetBlock : public CSetBlock
{
public:
	SSNode* GetNode(void* pvKey, size iKeySize)
	{
		return CSetBlock::GetNode(pvKey, iKeySize);
	}
};


size	giSetBlockCallbackData = 0;


class CTestSetDataFree : public CDataFree
{
protected:
	CSetBlock*	mpcSetBlock;

public:
	void	Init(CSetBlock* pcIndexBlock);
	void	FreeData(void* pvData) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestSetDataFree::Init(CSetBlock* pcSetBlock)
{
	mpcSetBlock = pcSetBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestSetDataFree::FreeData(void* pvData)
{
	
	giSetBlockCallbackData++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetBlockInternals(void)
{
	CSetBlock			cSet;
	size				i;
	size				iKey;
	CArrayBlockSorted*	paBlock;
	SSNode**			psNode1;
	SSNode**			psNode2;
	size*				piKey;

	cSet.Init(&CompareSize, true);
	i = 7;
	cSet.Put(&i, sizeof(size));

	i = 9;
	cSet.Put(&i, sizeof(size));
	AssertSize(2, cSet.NumElements());

	paBlock = cSet.GetArray();
	paBlock->InsertHoldingIntoSorted();
	psNode1 = (SSNode**)paBlock->GetInSorted(0);
	psNode2 = (SSNode**)paBlock->GetInSorted(1);
	AssertSize(sizeof(size), (*psNode1)->iDataSize);
	AssertSize(sizeof(size), (*psNode2)->iDataSize);
	piKey = (size*)RemapSinglePointer(*psNode1, sizeof(SSNode));
	AssertSize(7, *piKey);
	piKey = (size*)RemapSinglePointer(*psNode2, sizeof(SSNode));
	AssertSize(9, *piKey);

	iKey = 7;
	piKey = (size*)cSet.Get(&iKey, sizeof(size));
	AssertSize(7, *piKey);

	iKey = 9;
	piKey = (size*)cSet.Get(&iKey, sizeof(size));
	AssertSize(9, *piKey);

	iKey = 3;
	piKey = (size*)cSet.Get(&iKey, sizeof(size));
	AssertNull(piKey);

	cSet.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool AddToSetBlock(CSetBlock* pcSetBlock, const char* szKey)
{
	size				iStrLen;

	iStrLen = strlen(szKey) + 1;
	return pcSetBlock->Put(szKey, iStrLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* GetFromSetBlock(CSetBlock* pcSetBlock, const char* szKey)
{
	size	iStrLen;
	char*	szData;

	iStrLen = strlen(szKey) + 1;
	szData = (char*)pcSetBlock->Get(szKey, iStrLen);
	return szData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertSetBlock(CSetBlock* pcSetBlock, const char* szKey)
{
	size	iStrLen;
	char*	szData;

	iStrLen = strlen(szKey) + 1;
	szData = (char*)pcSetBlock->Get(szKey, iStrLen);
	AssertString(szKey, szData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetBlockGet(void)
{
	CSetBlock	cSet;

	cSet.Init((DataCompare)&StringCompare, true);
	AddToSetBlock(&cSet, "cocker");
	AddToSetBlock(&cSet, "cock");
	AddToSetBlock(&cSet, "cockerel");

	AssertSize(3, cSet.NumElements());
	AssertString("cocker", GetFromSetBlock(&cSet, "cocker"));

	AssertString("cock", GetFromSetBlock(&cSet, "cock"));

	AssertString("cockerel", GetFromSetBlock(&cSet, "cockerel"));

	AddToSetBlock(&cSet, "vizard");
	AddToSetBlock(&cSet, "visard");
	AddToSetBlock(&cSet, "wizard");
	AssertString("vizard", GetFromSetBlock(&cSet, "vizard"));
	AssertString("visard", GetFromSetBlock(&cSet, "visard"));
	AssertString("wizard", GetFromSetBlock(&cSet, "wizard"));

	AddToSetBlock(&cSet, "repletion");
	AddToSetBlock(&cSet, "ponderous");
	AddToSetBlock(&cSet, "quondam");
	AddToSetBlock(&cSet, "inimical");
	AddToSetBlock(&cSet, "ignominy");
	AssertString("inimical", GetFromSetBlock(&cSet, "inimical"));
	AssertString("cocker", GetFromSetBlock(&cSet, "cocker"));

	AddToSetBlock(&cSet, "voluble");
	AddToSetBlock(&cSet, "intransigent");
	AddToSetBlock(&cSet, "potentate");
	AddToSetBlock(&cSet, "sophist");
	AddToSetBlock(&cSet, "solicitous");
	AssertSize(16, cSet.NumElements());
	AssertString("solicitous", GetFromSetBlock(&cSet, "solicitous"));
	AssertString("cock", GetFromSetBlock(&cSet, "cock"));

	AddToSetBlock(&cSet, "resplendent");
	AssertSize(17, cSet.NumElements());

	AssertSetBlock(&cSet, "cocker");
	AssertSetBlock(&cSet, "cock");
	AssertSetBlock(&cSet, "cockerel");
	AssertSetBlock(&cSet, "vizard");
	AssertSetBlock(&cSet, "visard");
	AssertSetBlock(&cSet, "wizard");
	AssertSetBlock(&cSet, "voluble");
	AssertSetBlock(&cSet, "intransigent");
	AssertSetBlock(&cSet, "potentate");
	AssertSetBlock(&cSet, "sophist");
	AssertSetBlock(&cSet, "solicitous");
	AssertSetBlock(&cSet, "repletion");
	AssertSetBlock(&cSet, "ponderous");
	AssertSetBlock(&cSet, "quondam");
	AssertSetBlock(&cSet, "inimical");
	AssertSetBlock(&cSet, "ignominy");
	AssertSetBlock(&cSet, "resplendent");

	cSet.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetBlockAddDuplicate(void)
{
	CSetBlock	cSet;
	size		ia = 'a';
	size		ib = 'a';
	bool		bResult;


	cSet.Init(&CompareSize, true);
	bResult = cSet.Put(&ia, sizeof(size));
	AssertTrue(bResult);
	AssertSize(1, cSet.NumElements());
	AssertSize('a', *((size*)cSet.Get(&ia, sizeof(size))));

	bResult = cSet.Put(&ib, sizeof(size));
	AssertTrue(bResult);
	AssertSize(1, cSet.NumElements());
	AssertSize('a', *((size*)cSet.Get(&ia, sizeof(size))));

	cSet.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetBlockRemove(void)
{
	CSetBlock	cSet;
	size		ia = 'a';
	size		ib = 'b';
	size		ic = 'c';
	bool		bResult;

	cSet.Init(&CompareSize, true);
	bResult = cSet.Put(&ia, sizeof(size));
	bResult = cSet.Put(&ib, sizeof(size));
	bResult = cSet.Put(&ic, sizeof(size));
	AssertSize(3, cSet.NumElements());

	cSet.Remove(&ib, sizeof(size));
	AssertSize(2, cSet.NumElements());
	bResult = cSet.Get(&ia, sizeof(size));
	AssertTrue(bResult);
	bResult = cSet.Get(&ic, sizeof(size));
	AssertTrue(bResult);
	bResult = cSet.Get(&ib, sizeof(size));
	AssertFalse(bResult);

	cSet.Remove(&ia, sizeof(size));
	AssertSize(1, cSet.NumElements());
	bResult = cSet.Get(&ia, sizeof(size));
	AssertFalse(bResult);
	bResult = cSet.Get(&ic, sizeof(size));
	AssertTrue(bResult);
	bResult = cSet.Get(&ib, sizeof(size));
	AssertFalse(bResult);

	cSet.Remove(&ic, sizeof(size));
	AssertSize(0, cSet.NumElements());
	bResult = cSet.Get(&ia, sizeof(size));
	AssertFalse(bResult);
	bResult = cSet.Get(&ic, sizeof(size));
	AssertFalse(bResult);
	bResult = cSet.Get(&ib, sizeof(size));
	AssertFalse(bResult);

	cSet.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetBlockReadWrite(void)
{
	CSetString	mszsz;
	CFileBasic	cFile;
	CSetString	mszszIn;
	const char*	szResult;

	mszsz.Init();

	szResult = mszsz.Put("ABC");
	AssertString(szResult, "ABC");
	szResult = mszsz.Put("Collision");
	AssertString(szResult, "Collision");
	szResult = mszsz.Put("Retro");
	AssertString(szResult, "Retro");
	szResult = mszsz.Put("Blame");
	AssertString(szResult, "Blame");

	AssertString("ABC", mszsz.Get("ABC"));
	AssertString("Collision", mszsz.Get("Collision"));
	AssertString("Retro", mszsz.Get("Retro"));
	AssertString("Blame", mszsz.Get("Blame"));

	cFile.Init(MemoryFile());
	cFile.Open(EFM_Write_Create);
	AssertTrue(mszsz.Write(&cFile));
	mszsz.Kill();

	cFile.Close();
	cFile.Open(EFM_Read);

	AssertTrue(mszszIn.Read(&cFile));

	AssertString("ABC", mszszIn.Get("ABC"));
	AssertString("Collision", mszszIn.Get("Collision"));
	AssertString("Retro", mszszIn.Get("Retro"));
	AssertString("Blame", mszszIn.Get("Blame"));

	mszszIn.Kill();
	cFile.Close();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetBlockDataMemoryUnchanged(void)
{
	CSetString		mszsz;
	size			i;
	CRandom			cRandom;
	CArrayChars		aszWords;
	CChars			sz;
	bool			bFailed;
	bool			bHas;

	WordsInit();

	mszsz.Init();

	cRandom.Init(2367849);
	aszWords.Init();
	GetCommonWords(&aszWords);

	for (i = 0; i < 2048; i++)
	{
		MakeKey(&cRandom, &aszWords, &sz);
		mszsz.Put(sz.Text());
		sz.Kill();
	}
	cRandom.Kill();

	cRandom.Init(2367849);

	bFailed = false;
	for (i = 0; i < 2048; i++)
	{
		MakeKey(&cRandom, &aszWords, &sz);

		bHas = mszsz.Has(sz.Text());
		if (!bHas)
		{
			bFailed = true;
			break;
		}
		sz.Kill();
	}

	AssertFalse(bFailed);
	
	cRandom.Kill();
	mszsz.Kill();

	aszWords.Kill();
	WordsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetBlockRemoveHalf(void)
{
	CSetString		mszsz;
	size			i;
	CRandom			cRandom;
	CArrayChars		aszWords;
	CChars			sz;
	bool			bResult;
	bool			bFailed;
	char*			szResult;

	WordsInit();

	mszsz.Init();

	cRandom.Init(2367849);
	aszWords.Init();
	GetCommonWords(&aszWords);

	for (i = 0; i < 2048; i++)
	{
		MakeKey(&cRandom, &aszWords, &sz);
		mszsz.Put(sz.Text());
		sz.Kill();
	}
	cRandom.Kill();

	cRandom.Init(2367849);
	bResult = true;
	for (i = 0; i < 2048; i++)
	{
		MakeKey(&cRandom, &aszWords, &sz);
		if (i % 2 == 1)
		{
			bResult &= mszsz.Remove(sz.Text());
		}
		sz.Kill();
	}

	AssertTrue(bResult);
	AssertSize(1024, mszsz.NumElements());
	cRandom.Kill();

	bFailed = false;
	cRandom.Init(2367849);
	for (i = 0; i < 2048; i++)
	{
		MakeKey(&cRandom, &aszWords, &sz);
		szResult = mszsz.Get(sz.Text());
		if (i % 2 == 0)
		{
			if (!sz.Equals(szResult))
			{
				bFailed = true;
			}
		}
		else
		{
			if (szResult != NULL)
			{
				bFailed = true;
			}
		}
		sz.Kill();
	}
	AssertFalse(bFailed);
	cRandom.Kill();

	mszsz.Kill();

	aszWords.Kill();
	WordsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetBlock(void)
{
	BeginTests();
	MemoryInit();

	TestSetBlockInternals();
	TestSetBlockGet();
	TestSetBlockAddDuplicate();
	TestSetBlockRemove();
	TestSetBlockReadWrite();
	TestSetBlockDataMemoryUnchanged();
	TestSetBlockRemoveHalf();

	MemoryKill();
	TestStatistics();
}

