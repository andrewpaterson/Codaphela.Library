#include "BaseLib/UnionFind.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Elements start in sets of their own; joined either way round, a set's
// root is its lowest element.
//
//////////////////////////////////////////////////////////////////////////
void TestUnionFindJoin(void)
{
	CUnionFind	cSets;
	size		i;

	cSets.Init();
	AssertSize(0, cSets.NumElements());
	AssertSize(0, cSets.AddNum(6));
	AssertSize(6, cSets.Add());
	AssertSize(7, cSets.NumElements());
	for (i = 0; i < 7; i++)
	{
		AssertSize(i, cSets.Find(i));
	}

	AssertSize(2, cSets.Join(5, 2));
	AssertSize(2, cSets.Find(5));
	AssertTrue(cSets.SameSet(2, 5));
	AssertFalse(cSets.SameSet(2, 3));

	AssertSize(1, cSets.Join(1, 6));
	AssertSize(1, cSets.Join(6, 5));
	AssertSize(1, cSets.Find(2));
	AssertSize(1, cSets.Find(5));
	AssertSize(1, cSets.Find(6));
	AssertSize(0, cSets.Find(0));
	AssertSize(3, cSets.Find(3));

	//Joining a set to itself changes nothing.
	AssertSize(1, cSets.Join(2, 6));
	AssertSize(1, cSets.Find(6));

	//JoinToFirst keeps the first's root, higher or not.
	AssertSize(4, cSets.JoinToFirst(4, 1));
	AssertSize(4, cSets.Find(6));
	AssertSize(4, cSets.Find(1));
	AssertSize(4, cSets.JoinToFirst(2, 3));
	AssertSize(4, cSets.Find(3));
	AssertSize(4, cSets.JoinToFirst(5, 3));
	AssertSize(0, cSets.JoinToFirst(0, 5));
	AssertSize(0, cSets.Find(3));
	AssertSize(0, cSets.Find(6));

	cSets.ReInit();
	AssertSize(0, cSets.NumElements());
	AssertSize(0, cSets.Add());
	AssertSize(0, cSets.Find(0));
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A long chain, joined so that each element's parent is the one before it,
// is still found right however it is walked (Find halves its paths).
//
//////////////////////////////////////////////////////////////////////////
void TestUnionFindChain(void)
{
	CUnionFind	cSets;
	size		i;
	bool		bResult;

	cSets.Init();
	cSets.AddNum(2000);
	for (i = 1999; i > 0; i--)
	{
		cSets.Join(i, i - 1);
	}
	bResult = true;
	for (i = 2000; i > 0; i--)
	{
		bResult &= (cSets.Find(i - 1) == 0);
	}
	AssertTrue(bResult);

	//Two chains, then joined: everything has the lower root.
	cSets.ReInit();
	cSets.AddNum(100);
	for (i = 0; i < 49; i++)
	{
		cSets.Join(i + 1, i);
		cSets.Join(50 + i + 1, 50 + i);
	}
	AssertSize(0, cSets.Find(49));
	AssertSize(50, cSets.Find(99));
	AssertSize(0, cSets.Join(99, 49));
	bResult = true;
	for (i = 0; i < 100; i++)
	{
		bResult &= (cSets.Find(i) == 0);
	}
	AssertTrue(bResult);
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnionFindLarge(void)
{
	CUnionFindLarge	cSets;
	lsize			i;

	cSets.Init();
	cSets.AddNum(1000);
	for (i = 0; i < 1000; i += 2)
	{
		cSets.Join(i + 1, i);
	}
	AssertLSize(998, cSets.Find(999));
	AssertLSize(500, cSets.Find(501));
	AssertFalse(cSets.SameSet(1, 2));
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnionFind(void)
{
	BeginTests();

	TestUnionFindJoin();
	TestUnionFindChain();
	TestUnionFindLarge();

	TestStatistics();
}

