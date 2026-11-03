#include "BaseLib/Chars.h"
#include "CppParserLib/DefineMap.h"
#include "TestLib/Assert.h"


#define TEST_DEFINE_MAP_NAMES	3000


//////////////////////////////////////////////////////////////////////////
//
// The name of define i.
//
//////////////////////////////////////////////////////////////////////////
static void TestDefineMapName(CChars* psz, size i)
{
	psz->Init("D_");
	psz->Append((int)i);
}


//////////////////////////////////////////////////////////////////////////
//
// Exactly the names abDefined says are in the map, each found by its name.
//
//////////////////////////////////////////////////////////////////////////
static bool TestDefineMapMatches(CDefineMap* pcMap, bool* abDefined)
{
	size		i;
	CChars		sz;
	CDefine*	pcDefine;
	size		uiDefined;
	bool		bMatches;

	bMatches = true;
	uiDefined = 0;
	for (i = 0; i < TEST_DEFINE_MAP_NAMES; i++)
	{
		TestDefineMapName(&sz, i);
		pcDefine = pcMap->Get(sz.Text(), true);
		if (abDefined[i])
		{
			uiDefined++;
			bMatches &= (pcDefine != NULL) && (strcmp(pcDefine->GetName(), sz.Text()) == 0);
		}
		else
		{
			bMatches &= pcDefine == NULL;
		}
		sz.Kill();
	}
	bMatches &= pcMap->NumDefines() == uiDefined;
	return bMatches;
}


//////////////////////////////////////////////////////////////////////////
//
// Defines added and removed in a pseudo random order (the table grows,
// and runs of names wrap round its end) are always all found, and those
// removed never are.  A name defined again has a new ID.
//
//////////////////////////////////////////////////////////////////////////
void TestDefineMapAddRemove(void)
{
	CDefineMap	cMap;
	bool		abDefined[TEST_DEFINE_MAP_NAMES];
	int64		alliIDs[TEST_DEFINE_MAP_NAMES];
	size		i;
	size		uiStep;
	uint32		uiRandom;
	CChars		sz;
	CDefine*	pcDefine;
	bool		bMatches;
	bool		bNewIDs;

	cMap.Init();
	memset(abDefined, 0, sizeof(abDefined));
	for (i = 0; i < TEST_DEFINE_MAP_NAMES; i++)
	{
		alliIDs[i] = -1;
	}

	bMatches = true;
	bNewIDs = true;
	uiRandom = 12345;
	for (uiStep = 0; uiStep < 4 * TEST_DEFINE_MAP_NAMES; uiStep++)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		i = (uiRandom >> 8) % TEST_DEFINE_MAP_NAMES;
		TestDefineMapName(&sz, i);
		if (abDefined[i])
		{
			cMap.Remove(sz.Text());
			abDefined[i] = false;
		}
		else
		{
			pcDefine = cMap.AddDefine(sz.Text());
			bMatches &= pcDefine != NULL;
			if (pcDefine)
			{
				bNewIDs &= pcDefine->GetID() != alliIDs[i];
				alliIDs[i] = pcDefine->GetID();
			}
			abDefined[i] = true;
		}
		sz.Kill();

		//Checking every name each time is slow: every so often, and at the end.
		if ((uiStep % 997) == 0)
		{
			bMatches &= TestDefineMapMatches(&cMap, abDefined);
		}
	}
	AssertTrue(bMatches);
	AssertTrue(bNewIDs);
	AssertTrue(TestDefineMapMatches(&cMap, abDefined));

	//A name defined already isn't added again.
	for (i = 0; !abDefined[i]; i++)
	{
	}
	TestDefineMapName(&sz, i);
	AssertNull(cMap.AddDefine(sz.Text()));
	sz.Kill();

	//Everything removed.
	for (i = 0; i < TEST_DEFINE_MAP_NAMES; i++)
	{
		if (abDefined[i])
		{
			TestDefineMapName(&sz, i);
			cMap.Remove(sz.Text());
			sz.Kill();
			abDefined[i] = false;
		}
	}
	AssertSize(0, cMap.NumDefines());
	AssertTrue(TestDefineMapMatches(&cMap, abDefined));

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Looking up a name that isn't exact finds the longest define it starts
// with.
//
//////////////////////////////////////////////////////////////////////////
void TestDefineMapLongestPrefix(void)
{
	CDefineMap	cMap;
	CDefine*	pcDefine;

	cMap.Init();
	AssertNull(cMap.Get((char*)"AB", true));
	cMap.AddDefine((char*)"AB");
	cMap.AddDefine((char*)"ABCD");

	pcDefine = cMap.Get((char*)"ABCDE", false);
	AssertNotNull(pcDefine);
	AssertString("ABCD", pcDefine->GetName());
	pcDefine = cMap.Get((char*)"ABX", false);
	AssertNotNull(pcDefine);
	AssertString("AB", pcDefine->GetName());
	AssertNull(cMap.Get((char*)"A", false));
	AssertNull(cMap.Get((char*)"ABCDE", true));
	AssertNotNull(cMap.Get((char*)"AB", true));

	cMap.Remove((char*)"ABCD");
	pcDefine = cMap.Get((char*)"ABCDE", false);
	AssertNotNull(pcDefine);
	AssertString("AB", pcDefine->GetName());

	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDefineMap(void)
{
	BeginTests();

	TestDefineMapAddRemove();
	TestDefineMapLongestPrefix();

	TestStatistics();
}
