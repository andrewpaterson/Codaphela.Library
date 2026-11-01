#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/IndexTreeMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeMemory(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();

	CIndexTreeMemory			cIndexTree;
	uint64						ui1;
	uint64						ui2;
	uint64						ui3;
	uint64						uiKey;
	uint64						uiExpected;
	SIndexTreeMemoryIterator	sIter;
	bool						bResult;
	size						iKeyLength;

	ui1 = 1;
	ui2 = 2;
	ui3 = 3;

	cIndexTree.Init();
	cIndexTree.Put((uint8*)&ui1, sizeof(uint64), &ui1, sizeof(uint64));
	cIndexTree.Put((uint8*)&ui2, sizeof(uint64), &ui2, sizeof(uint64));
	cIndexTree.Put((uint8*)&ui3, sizeof(uint64), &ui3, sizeof(uint64));

	uiExpected = 1;
	bResult = cIndexTree.StartIteration(&sIter, (uint8*)&uiKey, &iKeyLength, sizeof(uint64), NULL, NULL, 0);
	while (bResult)
	{
		AssertLong(uiExpected, uiKey);
		bResult = cIndexTree.Iterate(&sIter, (uint8*)&uiKey, &iKeyLength, sizeof(uint64), NULL, NULL, 0);
		uiExpected++;
	}
	AssertLong(4LL, uiExpected);

	cIndexTree.Kill();

	TypesKill();
	MemoryKill();
	TestStatistics();
}