#include "BaseLib/MapStringIntAndPointer.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/Number.h"
#include "BaseLib/DataTypes.h"
#include "BaseLib/FreeList.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringIntAndPointerPut(void)
{
	CMapStringIntAndPointer		cMap;
	bool						bBool;
	char						szWord[] = "Words are Cool";
	uint						uiType;
	bool*						pbBool;
	char*						pszWord;
	size						uiSize;

	cMap.Init(true, true);

	bBool = true;
	cMap.Put("Hello", PT_bool, &bBool);
	cMap.Put("Word", PT_char8Pointer, szWord);

	pbBool = (bool*)cMap.Get("Hello", &uiType);
	AssertBool(true, *pbBool);
	AssertInt(PT_bool, uiType);
	
	pszWord = (char*)cMap.Get("Word", &uiType);
	AssertString("Words are Cool", pszWord);
	AssertInt(PT_char8Pointer, uiType);
	
	uiSize = cMap.NumElements();
	AssertSize(2, uiSize);

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool TestNumberMatchesIndex(CMapStringIntAndPointer* pcMap, CArrayInt* paiKeys)
{
	size						ui;
	char						szKey[12];
	uint						uiKey;
	CNumber*					pcNumber;
	uint						uiType;
	int32						iValue;
	bool						bPass;
	size						uiNumElements;

	bPass = true;
	uiNumElements = paiKeys->NumElements();

	if (uiNumElements != pcMap->NumElements())
	{
		return false;
	}

	for (ui = 0; ui < uiNumElements; ui++)
	{
		uiKey = paiKeys->GetValue(ui);
		IntToString(szKey, 5, uiKey, 10);
		pcNumber = (CNumber*)pcMap->Get(szKey, &uiType);
		iValue = pcNumber->IntValue();
		if (iValue != (int32)uiKey)
		{
			bPass = false;
			break;
		}
	}

	return bPass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringIntAndPointerRemove(void)
{
	CMapStringIntAndPointer		cMap;
	size						uiSize;
	size						ui;
	char						szKey[12];
	CArrayInt					aiKeys;
	CRandom						cRandom;
	uint						uiKey;
	CFreeList					cFreeNumbers;
	CNumber*					pcNumber;
	size						uiNumberSize;
	bool						bPass;
	size						uiNumElements;
	bool						bRemoved;

	aiKeys.Init();
	for (ui = 0; ui < 1024 + 128; ui++)
	{
		aiKeys.Add((int)ui);
	}

	cRandom.Init(89743636);
	aiKeys.Shuffle(&cRandom);

	cMap.Init(true, true);

	uiNumberSize = NUMBER_SIZE(DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS);
	cFreeNumbers.Init(uiNumberSize);
	for (ui = 0; ui < 1152; ui++)
	{
		uiKey = aiKeys.GetValue(ui);
		IntToString(szKey, 5, uiKey, 10);
		pcNumber = (CNumber*)cFreeNumbers.Add();
		pcNumber->Init((int)uiKey);
		cMap.Put(szKey, PT_Number, pcNumber);
	}


	uiSize = cMap.NumElements();
	AssertSize(1152, uiSize);

	aiKeys.Shuffle(&cRandom);

	bPass = TestNumberMatchesIndex(&cMap, &aiKeys);
	AssertTrue(bPass);

	uiNumElements = aiKeys.NumElements();
	ui = uiNumElements - 1;
	do
	{
		uiKey = aiKeys.GetValue(ui);
		IntToString(szKey, 5, uiKey, 10);

		bRemoved = cMap.Remove(szKey);
		AssertTrue(bRemoved);
		aiKeys.RemoveAt(ui);

		bPass = TestNumberMatchesIndex(&cMap, &aiKeys);
		AssertTrue(bPass);
		ui--;
	} while (ui != MAX_SIZE);

	uiSize = cMap.NumElements();
	AssertSize(0, uiSize);

	aiKeys.Kill();
	cRandom.Kill();

	cMap.Kill();

	cFreeNumbers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringIntAndPointerIterate(void)
{
	CMapStringIntAndPointer		cMap;
	size						uiSize;
	size						ui;
	char						szKey[12];
	CArrayInt					aiKeys;
	CRandom						cRandom;
	uint						uiKey;
	uint						uiType;
	CFreeList					cFreeNumbers;
	CNumber*					pcNumber;
	size						uiNumberSize;
	bool						bPass;
	SMapIterator				sIter;
	char*						szIterKey;
	bool						bValid;
	CChars						szNumber;

	aiKeys.Init();
	for (ui = 0; ui < 32; ui++)
	{
		aiKeys.Add((int)ui);
	}

	cRandom.Init(89743636);
	aiKeys.Shuffle(&cRandom);

	cMap.Init(true, true);

	uiNumberSize = NUMBER_SIZE(DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS);
	cFreeNumbers.Init(uiNumberSize);
	for (ui = 0; ui < 32; ui++)
	{
		uiKey = aiKeys.GetValue(ui);
		IntToString(szKey, 5, uiKey, 10);
		pcNumber = (CNumber*)cFreeNumbers.Add();
		pcNumber->Init((int)uiKey);
		cMap.Put(szKey, PT_Number, pcNumber);
	}

	uiSize = cMap.NumElements();
	AssertSize(32, uiSize);

	aiKeys.Shuffle(&cRandom);

	bPass = TestNumberMatchesIndex(&cMap, &aiKeys);
	AssertTrue(bPass);

	bValid = cMap.StartIteration(&sIter, (void**)&szIterKey, NULL, (void**)&pcNumber, &uiType);
	while (bValid)
	{
		AssertInt(PT_Number, uiType);

		szNumber.Init();
		pcNumber->Print(&szNumber);
		AssertString(szIterKey, szNumber.Text());
		szNumber.Kill();

		bValid = cMap.Iterate(&sIter, (void**)&szIterKey, NULL, (void**)&pcNumber, &uiType);
	}

	aiKeys.Kill();
	cRandom.Kill();

	cMap.Kill();

	cFreeNumbers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringIntAndPointer(void)
{
	BeginTests();
	NumberInit();

	TestMapStringIntAndPointerPut();
	TestMapStringIntAndPointerRemove();
	TestMapStringIntAndPointerIterate();

	NumberKill();
	TestStatistics();
}

