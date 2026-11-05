#include <stdlib.h>
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/Timer.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ArrayObject.h"
#include "StandardLib/SetObject.h"
#include "StandardLib/IndexObject.h"
#include "StandardLib/MapObject.h"
#include "StandardLib/Array.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"


void ValidateObjectsCapturingErrors(char* szErrors, size uiSize);  //TestEmbedded.cpp


//The distances to the root are kept up to date as pointers change (CDistCalculator::FromAdded when one is added, the dist
//calculator when one goes).  These tests check them against a breadth first search from the root after every change.


#define DIST_TO_ROOT_RANDOM_OBJECTS		40
#define DIST_TO_ROOT_RANDOM_ARRAYS		6
#define DIST_TO_ROOT_FREEING_MAX		80

//The 65C64 and W65C816 run these too, smaller: they get 4G instructions each.  Building a chain held only from the
//stack walks up the chain on every pointer change, so that chain is kept shorter.
#ifndef __LIBC65__
#define DIST_TO_ROOT_RANDOM_STEPS		1500
#define DIST_TO_ROOT_SPEED_DEPTH		6
#define DIST_TO_ROOT_MAX_UNROOTED_CHAIN	16384
#else
#define DIST_TO_ROOT_RANDOM_STEPS		100
#define DIST_TO_ROOT_SPEED_DEPTH		3
#define DIST_TO_ROOT_MAX_UNROOTED_CHAIN	32
#endif // !__LIBC65__

//Arrays counted with a 16 bit size (the W65C816's) can't list 100000 objects.
#if defined(__LIBC65__) || defined(SIZE_16)
#define DIST_TO_ROOT_LONG_CHAIN			1000
#else
#define DIST_TO_ROOT_LONG_CHAIN			100000
#endif // __LIBC65__ || SIZE_16


//////////////////////////////////////////////////////////////////////////
//
// An object embedded in an embedded object, with a collection; the outer
// object also has a pointer.
//
//////////////////////////////////////////////////////////////////////////
class CDistToRootInner : public CObject
{
CONSTRUCTABLE(CDistToRootInner);
DESTRUCTABLE(CDistToRootInner);
public:
	CArray<CTestObject>	maObjects;

	void Init(void) { PreInit(); maObjects.Init(); PostInit(); }
	void Class(void) override { M_Embedded(maObjects); }
	void Free(void) override {}
	bool Save(CObjectWriter* pcFile) override { return false; }
	bool Load(CObjectReader* pcFile) override { return false; }
};


class CDistToRootOuter : public CObject
{
CONSTRUCTABLE(CDistToRootOuter);
DESTRUCTABLE(CDistToRootOuter);
public:
	CDistToRootInner	mcInner;
	Ptr<CTestObject>	mpObject;

	void Init(void) { PreInit(); mcInner.Init(); PostInit(); }
	void Class(void) override { M_Embedded(mcInner); M_Pointer(mpObject); }
	void Free(void) override {}
	bool Save(CObjectWriter* pcFile) override { return false; }
	bool Load(CObjectReader* pcFile) override { return false; }
};


//////////////////////////////////////////////////////////////////////////
//
// Its Free() lets go of a shared object through a stack pointer (as a
// canvas's Free() hands its image to SetImage).
//
//////////////////////////////////////////////////////////////////////////
class CDistToRootFreeingHolder : public CObject
{
CONSTRUCTABLE(CDistToRootFreeingHolder);
DESTRUCTABLE(CDistToRootFreeingHolder);
public:
	Ptr<CTestObject>	mpShared;

	void Init(Ptr<CTestObject> pShared) { PreInit(); mpShared = pShared; PostInit(); }
	void Class(void) override { M_Pointer(mpShared); }
	void Free(void) override
	{
		Ptr<CTestObject>	pShared;

		pShared = mpShared;
		mpShared = NULL;
	}
	bool Save(CObjectWriter* pcFile) override { return false; }
	bool Load(CObjectReader* pcFile) override { return false; }
};


//////////////////////////////////////////////////////////////////////////
//
// Every object in memory's distance to the root, worked out with a breadth
// first search from the root through the objects' pointers (and their
// embedded objects' and collections'), is the distance it holds.  Objects
// the search doesn't reach are unattached.  Returns the number of objects
// whose distance is wrong (and logs them).
//
//////////////////////////////////////////////////////////////////////////
size CountWrongDistToRoot(void)
{
	CArrayBaseObjectPtr		apcObjects;
	CArrayInt				aiDist;
	CArrayInt				aiQueue;
	CArrayEmbeddedObjectPtr	apcTos;
	SIndexesIterator		sIter;
	CBaseObject*			pcObject;
	CBaseObject*			pcTo;
	size					i;
	size					j;
	size					k;
	size					uiNumObjects;
	size					uiFirst;
	size					uiWrong;
	int						iDist;
	CChars					sz;

	apcObjects.Init();
	aiDist.Init();
	pcObject = gcObjects.GetMemory()->StartIteration(&sIter);
	while (pcObject)
	{
		apcObjects.Add(pcObject);
		aiDist.Add(UNATTACHED_DIST_TO_ROOT);
		pcObject = gcObjects.GetMemory()->Iterate(&sIter);
	}
	uiNumObjects = apcObjects.NumElements();

	aiQueue.Init();
	for (i = 0; i < uiNumObjects; i++)
	{
		if (apcObjects.GetPtr(i)->IsRoot())
		{
			aiDist.SetValue(i, ROOT_DIST_TO_ROOT);
			aiQueue.Add((int)i);
		}
	}

	for (uiFirst = 0; uiFirst < aiQueue.NumElements(); uiFirst++)
	{
		i = (size)aiQueue.GetValue(uiFirst);
		pcObject = apcObjects.GetPtr(i);
		iDist = aiDist.GetValue(i);

		apcTos.Init();
		pcObject->GetPointerTos(&apcTos);
		for (j = 0; j < apcTos.NumElements(); j++)
		{
			pcTo = apcTos.GetPtr(j)->GetEmbeddingContainer();
			for (k = 0; k < uiNumObjects; k++)
			{
				if (apcObjects.GetPtr(k) == pcTo)
				{
					if (aiDist.GetValue(k) == UNATTACHED_DIST_TO_ROOT)
					{
						aiDist.SetValue(k, iDist + 1);
						aiQueue.Add((int)k);
					}
					break;
				}
			}
		}
		apcTos.Kill();
	}

	uiWrong = 0;
	for (i = 0; i < uiNumObjects; i++)
	{
		pcObject = apcObjects.GetPtr(i);
		if (pcObject->GetDistToRoot() != aiDist.GetValue(i))
		{
			sz.Init();
			pcObject->PrintIdentifier(&sz);
			sz.Append(" has dist to root ");
			sz.Append(pcObject->GetDistToRoot());
			sz.Append(", expected ");
			sz.Append(aiDist.GetValue(i));
			gcLogger.Error(sz.Text());
			sz.Kill();
			uiWrong++;
		}
	}

	aiQueue.Kill();
	aiDist.Kill();
	apcObjects.Kill();
	return uiWrong;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertDistToRoot(void)
{
	AssertSize(0, CountWrongDistToRoot());
}


//////////////////////////////////////////////////////////////////////////
//
// A chain from the root, then a pointer from near the root to the middle
// of it (its far half gets closer), then the pointer removed again.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateChainShortcut(void)
{
	Ptr<CTestObject>	apChain[30];
	size				i;

	ObjectsInit();
	{
		for (i = 0; i < 30; i++)
		{
			apChain[i] = OMalloc<CTestObject>();
			if (i > 0)
			{
				apChain[i - 1]->mpTest = apChain[i];
			}
		}
		ORoot()->Add(apChain[0]);

		//The root is 0, its set 1, the chain starts at 2.
		for (i = 0; i < 30; i++)
		{
			AssertInt((int)i + 2, apChain[i]->GetDistToRoot());
		}
		AssertDistToRoot();

		apChain[0]->mpObject = apChain[20];
		for (i = 0; i < 30; i++)
		{
			AssertInt((i < 20) ? (int)i + 2 : (int)i - 20 + 3, apChain[i]->GetDistToRoot());
		}
		AssertDistToRoot();

		apChain[0]->mpObject = NULL;
		for (i = 0; i < 30; i++)
		{
			AssertInt((int)i + 2, apChain[i]->GetDistToRoot());
		}
		AssertDistToRoot();

		ORoot()->Remove(apChain[0]);
		for (i = 0; i < 30; i++)
		{
			AssertInt(UNATTACHED_DIST_TO_ROOT, apChain[i]->GetDistToRoot());
		}
		AssertDistToRoot();

		for (i = 0; i < 30; i++)
		{
			apChain[i] = NULL;
		}
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A tree like a window's components: each node holds an array of its
// children and each child points back to its parent.  A rooted focus
// object points at one node after another (as CFocus does at the
// component under the pointer).
//
//////////////////////////////////////////////////////////////////////////
void BuildDistToRootTree(Ptr<CTestObject> pParent, size uiDepth, size uiBranching, CArrayBaseObjectPtr* papcNodes)
{
	Ptr<CArrayObject>	pChildren;
	Ptr<CTestObject>	pChild;
	size				i;

	papcNodes->Add(&pParent);
	if (uiDepth == 0)
	{
		return;
	}
	pChildren = OMalloc<CArrayObject>();
	pParent->mpObject = pChildren;
	for (i = 0; i < uiBranching; i++)
	{
		pChild = OMalloc<CTestObject>();
		pChild->mpTest = pParent;
		pChildren->Add(pChild);
		BuildDistToRootTree(pChild, uiDepth - 1, uiBranching, papcNodes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateFocusInTree(size uiDepth, size uiBranching, bool bValidateEachStep)
{
	Ptr<CTestObject>		pTop;
	Ptr<CTestObject>		pFocus;
	CArrayBaseObjectPtr		apcNodes;
	size					i;

	ObjectsInit();
	apcNodes.Init();
	{
		pTop = OMalloc<CTestObject>();
		ORoot()->Add(pTop);
		BuildDistToRootTree(pTop, uiDepth, uiBranching, &apcNodes);
		pFocus = OMalloc<CTestObject>();
		ORoot()->Add(pFocus);
		AssertDistToRoot();

		for (i = 0; i < apcNodes.NumElements(); i++)
		{
			pFocus->mpTest = (CTestObject*)apcNodes.GetPtr(i);
			if (bValidateEachStep)
			{
				AssertDistToRoot();
			}
		}
		pFocus->mpTest = NULL;
		AssertDistToRoot();

		ORoot()->Remove(pFocus);
		ORoot()->Remove(pTop);
		pFocus = NULL;
		pTop = NULL;
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	apcNodes.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A hub every node points at (as every component points at its window),
// and a focus moving across the nodes.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateHub(void)
{
	Ptr<CTestObject>	pHub;
	Ptr<CTestObject>	pFocus;
	Ptr<CArrayObject>	pNodes;
	Ptr<CTestObject>	pNode;
	Ptr<CTestObject>	pPrevious;
	size				i;

	ObjectsInit();
	{
		pHub = OMalloc<CTestObject>();
		pNodes = OMalloc<CArrayObject>();
		pHub->mpObject = pNodes;
		for (i = 0; i < 100; i++)
		{
			pNode = OMalloc<CTestObject>();
			pNode->mpObject = pHub;
			pNode->mpTest = pPrevious;
			pNodes->Add(pNode);
			pPrevious = pNode;
		}
		pNode = NULL;
		pPrevious = NULL;
		ORoot()->Add(pHub);
		pFocus = OMalloc<CTestObject>();
		ORoot()->Add(pFocus);
		AssertDistToRoot();

		for (i = 0; i < 100; i += 7)
		{
			pFocus->mpTest = (CTestObject*)&pNodes->Get(i);
			AssertDistToRoot();
		}
		ORoot()->Remove(pHub);
		AssertDistToRoot();
		pFocus->mpTest = NULL;
		AssertDistToRoot();

		ORoot()->Remove(pFocus);
		pHub = NULL;
		pNodes = NULL;
		pFocus = NULL;
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Collections embedded in embedded objects pass on their distance to
// what they hold.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateEmbeddedCollections(void)
{
	Ptr<CDistToRootOuter>	pOuter;
	Ptr<CTestObject>		pFocus;
	Ptr<CTestObject>		pObject;
	Ptr<CTestObject>		pDeep;
	size					i;

	ObjectsInit();
	{
		pOuter = OMalloc<CDistToRootOuter>();
		for (i = 0; i < 10; i++)
		{
			pObject = OMalloc<CTestObject>();
			pObject->mpTest = pDeep;
			pOuter->mcInner.maObjects.Add(pObject);
			pDeep = pObject;
		}
		pObject = NULL;
		ORoot()->Add(pOuter);
		AssertDistToRoot();
		AssertInt(3, pDeep->GetDistToRoot());

		pFocus = OMalloc<CTestObject>();
		pFocus->mpTest = pDeep;
		pOuter->mpObject = pFocus;
		AssertDistToRoot();

		pOuter->mcInner.maObjects.RemoveAt(9);
		AssertDistToRoot();
		AssertInt(4, pDeep->GetDistToRoot());

		pOuter->mpObject = NULL;
		AssertDistToRoot();
		AssertInt(UNATTACHED_DIST_TO_ROOT, pDeep->GetDistToRoot());

		ORoot()->Remove(pOuter);
		AssertDistToRoot();
		pOuter = NULL;
		pFocus = NULL;
		pDeep = NULL;
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Random changes to a graph of objects and arrays: pointers set and
// cleared, objects added to and removed from arrays and the root.  The
// stack holds every object so nothing is freed until the end.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateRandom(uint32 uiSeed, size uiSteps)
{
	Ptr<CTestObject>	apObjects[DIST_TO_ROOT_RANDOM_OBJECTS];
	Ptr<CArrayObject>	apArrays[DIST_TO_ROOT_RANDOM_ARRAYS];
	bool				abObjectRooted[DIST_TO_ROOT_RANDOM_OBJECTS];
	bool				abArrayRooted[DIST_TO_ROOT_RANDOM_ARRAYS];
	CRandom				cRandom;
	size				uiStep;
	int					iOperation;
	int					i;
	int					j;
	size				uiWrong;

	ObjectsInit();
	cRandom.Init(uiSeed);
	{
		for (i = 0; i < DIST_TO_ROOT_RANDOM_OBJECTS; i++)
		{
			apObjects[i] = OMalloc<CTestObject>();
			abObjectRooted[i] = false;
		}
		for (i = 0; i < DIST_TO_ROOT_RANDOM_ARRAYS; i++)
		{
			apArrays[i] = OMalloc<CArrayObject>();
			abArrayRooted[i] = false;
		}

		uiWrong = 0;
		for (uiStep = 0; (uiStep < uiSteps) && (uiWrong == 0); uiStep++)
		{
			iOperation = cRandom.Next(0, 9);
			i = cRandom.Next(0, DIST_TO_ROOT_RANDOM_OBJECTS - 1);
			j = cRandom.Next(0, DIST_TO_ROOT_RANDOM_OBJECTS - 1);
			switch (iOperation)
			{
			case 0:
			case 1:
				apObjects[i]->mpTest = apObjects[j];
				break;
			case 2:
				apObjects[i]->mpTest = NULL;
				break;
			case 3:
				apObjects[i]->mpObject = apArrays[j % DIST_TO_ROOT_RANDOM_ARRAYS];
				break;
			case 4:
				apObjects[i]->mpObject = (cRandom.Next(0, 1) == 0) ? CPointer() : CPointer(apObjects[j]);
				break;
			case 5:
				apArrays[j % DIST_TO_ROOT_RANDOM_ARRAYS]->Add(apObjects[i]);
				break;
			case 6:
				if (apArrays[j % DIST_TO_ROOT_RANDOM_ARRAYS]->NumElements() > 0)
				{
					apArrays[j % DIST_TO_ROOT_RANDOM_ARRAYS]->Remove((size)cRandom.Next(0, (int32)apArrays[j % DIST_TO_ROOT_RANDOM_ARRAYS]->NumElements() - 1));
				}
				break;
			case 7:
				if (abObjectRooted[i])
				{
					ORoot()->Remove(apObjects[i]);
				}
				else
				{
					ORoot()->Add(apObjects[i]);
				}
				abObjectRooted[i] = !abObjectRooted[i];
				break;
			case 8:
				i = i % DIST_TO_ROOT_RANDOM_ARRAYS;
				if (abArrayRooted[i])
				{
					ORoot()->Remove(apArrays[i]);
				}
				else
				{
					ORoot()->Add(apArrays[i]);
				}
				abArrayRooted[i] = !abArrayRooted[i];
				break;
			case 9:
				apObjects[i]->mpObject = apObjects[i];
				break;
			}
			uiWrong = CountWrongDistToRoot();
		}
		AssertSize(0, uiWrong);
		gcObjects.ValidateObjectsConsistency();

		ORoot()->RemoveAll();
		AssertDistToRoot();
		for (i = 0; i < DIST_TO_ROOT_RANDOM_OBJECTS; i++)
		{
			apObjects[i] = NULL;
		}
		for (i = 0; i < DIST_TO_ROOT_RANDOM_ARRAYS; i++)
		{
			apArrays[i] = NULL;
		}
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	cRandom.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every object in memory can be reached from the root (nothing holds
// objects on the stack between steps, so anything else should be freed).
//
//////////////////////////////////////////////////////////////////////////
size CountUnattached(void)
{
	SIndexesIterator	sIter;
	CBaseObject*		pcObject;
	size				uiUnattached;

	uiUnattached = 0;
	pcObject = gcObjects.GetMemory()->StartIteration(&sIter);
	while (pcObject)
	{
		if (pcObject->GetDistToRoot() < ROOT_DIST_TO_ROOT)
		{
			uiUnattached++;
		}
		pcObject = gcObjects.GetMemory()->Iterate(&sIter);
	}
	return uiUnattached;
}


//////////////////////////////////////////////////////////////////////////
//
// Two routes from the root to the same object.  Removing either one when
// they are the same length leaves its distance alone, removing the shorter
// one moves it to the longer one, removing both leaves it unattached.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateAlternatePaths(void)
{
	Ptr<CTestObject>	pShort;
	Ptr<CTestObject>	pLong1;
	Ptr<CTestObject>	pLong2;
	Ptr<CTestObject>	pSame;
	Ptr<CTestObject>	pTarget;
	Ptr<CTestObject>	pBelow;

	ObjectsInit();
	{
		pShort = OMalloc<CTestObject>();
		pSame = OMalloc<CTestObject>();
		pLong1 = OMalloc<CTestObject>();
		pLong2 = OMalloc<CTestObject>();
		pTarget = OMalloc<CTestObject>();
		pBelow = OMalloc<CTestObject>();
		ORoot()->Add(pShort);
		ORoot()->Add(pSame);
		ORoot()->Add(pLong1);
		pLong1->mpTest = pLong2;
		pTarget->mpTest = pBelow;
		pBelow->mpTest = pTarget;  //Back up to the target: never closer.

		pShort->mpTest = pTarget;
		pSame->mpTest = pTarget;
		pLong2->mpTest = pTarget;
		AssertInt(3, pTarget->GetDistToRoot());
		AssertInt(4, pBelow->GetDistToRoot());
		AssertDistToRoot();

		//Same length: nothing moves.
		pShort->mpTest = NULL;
		AssertInt(3, pTarget->GetDistToRoot());
		AssertInt(4, pBelow->GetDistToRoot());
		AssertDistToRoot();

		//Only the long route left.
		pSame->mpTest = NULL;
		AssertInt(4, pTarget->GetDistToRoot());
		AssertInt(5, pBelow->GetDistToRoot());
		AssertDistToRoot();

		//Closer again.
		pShort->mpTest = pTarget;
		AssertInt(3, pTarget->GetDistToRoot());
		AssertInt(4, pBelow->GetDistToRoot());
		AssertDistToRoot();

		//Neither route: the target and below only point at each other.
		pShort->mpTest = NULL;
		pLong2->mpTest = NULL;
		AssertInt(UNATTACHED_DIST_TO_ROOT, pTarget->GetDistToRoot());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pBelow->GetDistToRoot());
		AssertDistToRoot();

		//The cycle comes back through the bottom of it.
		pLong2->mpObject = pBelow;
		AssertInt(4, pBelow->GetDistToRoot());
		AssertInt(5, pTarget->GetDistToRoot());
		AssertDistToRoot();

		ORoot()->RemoveAll();
		AssertDistToRoot();
		pShort = NULL;
		pSame = NULL;
		pLong1 = NULL;
		pLong2 = NULL;
		pTarget = NULL;
		pBelow = NULL;
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A chain where every link also points back to its parent and to a hub
// near the root.  Cutting the chain leaves the far part unattached even
// though it points at things that are attached (pointing at the hub never
// makes the hub depend on it).  A pointer from the hub to the bottom then
// brings it back from the other end.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateBackPointersToHub(void)
{
	Ptr<CTestTriPointerObject>	pHub;
	Ptr<CTestTriPointerObject>	apChain[12];
	size						i;

	ObjectsInit();
	{
		pHub = OMalloc<CTestTriPointerObject>();
		ORoot()->Add(pHub);
		for (i = 0; i < 12; i++)
		{
			apChain[i] = OMalloc<CTestTriPointerObject>();
			apChain[i]->mpObject2 = pHub;
			if (i > 0)
			{
				apChain[i - 1]->mpObject3 = apChain[i];
				apChain[i]->mpObject1 = apChain[i - 1];
			}
		}
		pHub->mpObject1 = apChain[0];
		for (i = 0; i < 12; i++)
		{
			AssertInt((int)i + 3, apChain[i]->GetDistToRoot());
		}
		AssertInt(2, pHub->GetDistToRoot());
		AssertDistToRoot();

		apChain[5]->mpObject3 = NULL;
		for (i = 0; i < 12; i++)
		{
			AssertInt((i <= 5) ? (int)i + 3 : UNATTACHED_DIST_TO_ROOT, apChain[i]->GetDistToRoot());
		}
		AssertInt(2, pHub->GetDistToRoot());
		AssertDistToRoot();

		pHub->mpObject3 = apChain[11];
		for (i = 0; i < 12; i++)
		{
			AssertInt((i <= 5) ? (int)i + 3 : 3 + (11 - (int)i), apChain[i]->GetDistToRoot());
		}
		AssertDistToRoot();

		apChain[5]->mpObject3 = apChain[6];
		for (i = 0; i < 12; i++)
		{
			AssertInt(((int)i + 3 < 14 - (int)i) ? (int)i + 3 : 14 - (int)i, apChain[i]->GetDistToRoot());
		}
		AssertDistToRoot();

		pHub->mpObject1 = NULL;
		for (i = 0; i < 12; i++)
		{
			AssertInt(3 + (11 - (int)i), apChain[i]->GetDistToRoot());
		}
		AssertDistToRoot();

		ORoot()->RemoveAll();
		AssertDistToRoot();
		pHub = NULL;
		for (i = 0; i < 12; i++)
		{
			apChain[i] = NULL;
		}
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The objects and collections in memory, gathered at the start of a step
// (raw pointers: they are gone by the end of it).
//
//////////////////////////////////////////////////////////////////////////
struct SDistToRootLiveObjects
{
	CArrayTemplatePtr<CTestObject>	apcObjects;
	CArrayTemplatePtr<CArrayObject>	apcArrays;
	CArrayTemplatePtr<CSetObject>	apcSets;
	CArrayTemplatePtr<CIndexObject>	apcIndices;
	CArrayTemplatePtr<CMapObject>	apcMaps;
	CArrayTemplatePtr<CCollection>	apcCollections;

	void Init(void);
	void Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDistToRootLiveObjects::Init(void)
{
	SIndexesIterator	sIter;
	CBaseObject*		pcObject;

	apcObjects.Init();
	apcArrays.Init();
	apcSets.Init();
	apcIndices.Init();
	apcMaps.Init();
	apcCollections.Init();

	pcObject = gcObjects.GetMemory()->StartIteration(&sIter);
	while (pcObject)
	{
		if (dynamic_cast<CTestObject*>(pcObject))
		{
			apcObjects.Add((CTestObject*)pcObject);
		}
		else if ((pcObject != ORoot()->TestGetSet()) && dynamic_cast<CCollection*>(pcObject))
		{
			apcCollections.Add((CCollection*)pcObject);
			if (dynamic_cast<CArrayObject*>(pcObject))
			{
				apcArrays.Add((CArrayObject*)pcObject);
			}
			else if (dynamic_cast<CSetObject*>(pcObject))
			{
				apcSets.Add((CSetObject*)pcObject);
			}
			else if (dynamic_cast<CIndexObject*>(pcObject))
			{
				apcIndices.Add((CIndexObject*)pcObject);
			}
			else if (dynamic_cast<CMapObject*>(pcObject))
			{
				apcMaps.Add((CMapObject*)pcObject);
			}
		}
		pcObject = gcObjects.GetMemory()->Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDistToRootLiveObjects::Kill(void)
{
	apcCollections.Kill();
	apcMaps.Kill();
	apcIndices.Kill();
	apcSets.Kill();
	apcArrays.Kill();
	apcObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A random element, NULL if there are none.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* RandomElement(CRandom* pcRandom, CArrayTemplatePtr<M>* papc)
{
	if (papc->NumElements() == 0)
	{
		return NULL;
	}
	return papc->GetPtr((size)pcRandom->Next(0, (int32)papc->NumElements() - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// Random changes where nothing is held on the stack between steps, so
// every object cut off from the root is freed.  The graph runs through
// arrays, a set, an index and a map as well as object pointers.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateRandomFreeing(uint32 uiSeed, size uiSteps)
{
	CRandom							cRandom;
	SDistToRootLiveObjects			sLive;
	size							uiStep;
	int								iOperation;
	size							uiWrong;
	size							uiUnattached;
	char							szKey[8];
	CTestObject*					pcA;
	CTestObject*					pcB;
	CCollection*					pcCollection;

	ObjectsInit();
	cRandom.Init(uiSeed);
	{
		{
			Ptr<CArrayObject>	pArray1 = OMalloc<CArrayObject>();
			Ptr<CArrayObject>	pArray2 = OMalloc<CArrayObject>();
			Ptr<CSetObject>		pSet = OMalloc<CSetObject>();
			Ptr<CIndexObject>	pIndex = OMalloc<CIndexObject>();
			Ptr<CMapObject>		pMap = OMalloc<CMapObject>();

			ORoot()->Add(pArray1, pArray2, pSet);
			ORoot()->Add(pIndex, pMap);
		}

		uiWrong = 0;
		uiUnattached = 0;
		for (uiStep = 0; (uiStep < uiSteps) && (uiWrong == 0) && (uiUnattached == 0); uiStep++)
		{
			sLive.Init();

			pcA = RandomElement(&cRandom, &sLive.apcObjects);
			pcB = RandomElement(&cRandom, &sLive.apcObjects);
			pcCollection = RandomElement(&cRandom, &sLive.apcCollections);
			sprintf(szKey, "k%i", cRandom.Next(0, 9));

			iOperation = cRandom.Next(0, 14);
			if ((pcA == NULL) || ((iOperation == 0) && (sLive.apcObjects.NumElements() < DIST_TO_ROOT_FREEING_MAX)))
			{
				Ptr<CTestObject>	pNew;

				//A new object somewhere reachable (or not, then it is freed straight away).
				pNew = OMalloc<CTestObject>();
				if (sLive.apcArrays.NumElements() > 0)
				{
					RandomElement(&cRandom, &sLive.apcArrays)->Add(pNew);
				}
				else if (pcA)
				{
					pcA->mpTest = pNew;
				}
			}
			else
			{
				Ptr<CTestObject>	pA;
				Ptr<CTestObject>	pB;

				pA = pcA;
				pB = pcB;
				switch (iOperation)
				{
				case 0:
				case 1:
				case 2:
					pA->mpTest = pB;
					break;
				case 3:
					pA->mpTest = NULL;
					break;
				case 4:
					if (cRandom.Next(0, 2) == 0)
					{
						pA->mpObject = NULL;
					}
					else if (pcCollection && (cRandom.Next(0, 1) == 0))
					{
						pA->mpObject = pcCollection;
					}
					else
					{
						pA->mpObject = pB;
					}
					break;
				case 5:
					if (sLive.apcArrays.NumElements() > 0)
					{
						RandomElement(&cRandom, &sLive.apcArrays)->Add(pA);
					}
					break;
				case 6:
					if (sLive.apcArrays.NumElements() > 0)
					{
						CArrayObject*	pcArray;

						pcArray = RandomElement(&cRandom, &sLive.apcArrays);
						if (pcArray->NumElements() > 0)
						{
							pcArray->Remove((size)cRandom.Next(0, (int32)pcArray->NumElements() - 1));
						}
					}
					break;
				case 7:
					if (sLive.apcSets.NumElements() > 0)
					{
						if (sLive.apcSets.GetPtr(0)->Contains(pA))
						{
							sLive.apcSets.GetPtr(0)->Remove(pA);
						}
						else
						{
							sLive.apcSets.GetPtr(0)->Add(pA);
						}
					}
					break;
				case 8:
					if (sLive.apcIndices.NumElements() > 0)
					{
						sLive.apcIndices.GetPtr(0)->Put(szKey, pA);
					}
					break;
				case 9:
					if (sLive.apcIndices.NumElements() > 0)
					{
						if (sLive.apcIndices.GetPtr(0)->Get(szKey).IsNotNull())
						{
							sLive.apcIndices.GetPtr(0)->Remove(szKey);
						}
					}
					break;
				case 10:
					if (sLive.apcMaps.NumElements() > 0)
					{
						sLive.apcMaps.GetPtr(0)->Put(pA, pB);
					}
					break;
				case 11:
					if (sLive.apcMaps.NumElements() > 0)
					{
						if (sLive.apcMaps.GetPtr(0)->Get(pA).IsNotNull())
						{
							sLive.apcMaps.GetPtr(0)->Remove(pA);
						}
					}
					break;
				case 12:
					//A collection off the root (freed unless an object points to it), or a new one on it.
					if (pcCollection && (cRandom.Next(0, 1) == 0))
					{
						if (pcCollection->GetDistToRoot() == 2)
						{
							ORoot()->Remove(pcCollection);
						}
					}
					else
					{
						Ptr<CArrayObject>	pArray = OMalloc<CArrayObject>();

						ORoot()->Add(pArray);
					}
					break;
				case 13:
					if (pcCollection)
					{
						CPointer	pCollection = pcCollection;

						ORoot()->Add(pCollection);
					}
					break;
				case 14:
					pA->mpObject = pA;
					break;
				}
			}

			sLive.Kill();

			uiWrong = CountWrongDistToRoot();
			uiUnattached = CountUnattached();
			if ((uiStep % 100) == 0)
			{
				gcObjects.ValidateObjectsConsistency();
			}
		}
		AssertSize(0, uiWrong);
		AssertSize(0, uiUnattached);
		gcObjects.ValidateObjectsConsistency();

		ORoot()->RemoveAll();
		AssertDistToRoot();
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	cRandom.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Objects held only on the stack that point at a hub on the root and at
// each other (a component being built before it is added).  Letting go of
// them frees them all; the hub and what hangs off it keep their distances.
// Then the same again, but one of them is added to the hub on the way.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateUnattachedPointingAtHub(void)
{
	Ptr<CTestTriPointerObject>	pHub;
	Ptr<CTestObject>			pHubChild;
	Ptr<CArrayObject>			pHubArray;
	STestObjectFreedNotifier	asNotifiers[8];
	size						i;

	ObjectsInit();
	{
		pHub = OMalloc<CTestTriPointerObject>();
		pHubChild = OMalloc<CTestObject>();
		pHubArray = OMalloc<CArrayObject>();
		ORoot()->Add(pHub);
		pHub->mpObject1 = pHubChild;
		pHub->mpObject2 = pHubArray;
		pHubChild->mpObject = pHub;
		AssertDistToRoot();

		{
			Ptr<CTestObject>	apTemps[8];

			for (i = 0; i < 8; i++)
			{
				apTemps[i] = OMalloc<CTestObject>(&asNotifiers[i]);
				apTemps[i]->mpObject = pHub;
				if (i > 0)
				{
					apTemps[i]->mpTest = apTemps[i - 1];
					apTemps[i - 1]->mpTest = apTemps[i];
				}
			}
			AssertInt(UNATTACHED_DIST_TO_ROOT, apTemps[0]->GetDistToRoot());
			AssertSize(10, pHub->NumHeapFroms());  //The set on the root, the child and the eight.
			AssertDistToRoot();

			for (i = 0; i < 8; i++)
			{
				apTemps[i] = NULL;
			}
		}
		for (i = 0; i < 8; i++)
		{
			AssertTrue(asNotifiers[i].bFreed);
			asNotifiers[i].bFreed = false;
		}
		AssertSize(2, pHub->NumHeapFroms());
		AssertInt(2, pHub->GetDistToRoot());
		AssertInt(3, pHubChild->GetDistToRoot());
		AssertInt(3, pHubArray->GetDistToRoot());
		AssertDistToRoot();

		{
			Ptr<CTestObject>	apTemps[8];

			for (i = 0; i < 8; i++)
			{
				apTemps[i] = OMalloc<CTestObject>(&asNotifiers[i]);
				apTemps[i]->mpObject = pHub;
				if (i > 0)
				{
					apTemps[i]->mpTest = apTemps[i - 1];
				}
			}
			pHubArray->Add(apTemps[7]);
			AssertInt(4, apTemps[7]->GetDistToRoot());
			AssertInt(11, apTemps[0]->GetDistToRoot());
			AssertDistToRoot();

			for (i = 0; i < 8; i++)
			{
				apTemps[i] = NULL;
			}
		}
		for (i = 0; i < 8; i++)
		{
			AssertFalse(asNotifiers[i].bFreed);
		}
		AssertDistToRoot();

		pHubArray->RemoveAll();
		for (i = 0; i < 8; i++)
		{
			AssertTrue(asNotifiers[i].bFreed);
		}
		AssertSize(2, pHub->NumHeapFroms());
		AssertDistToRoot();

		ORoot()->RemoveAll();
		pHub = NULL;
		pHubChild = NULL;
		pHubArray = NULL;
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A window's components: every node points to its parent, to the window
// (a hub with as many froms as there are nodes) and to an array of its
// children.  The window holds the top node and a focus that points at one
// node after another (CFocus's component under the pointer).
//
//////////////////////////////////////////////////////////////////////////
void BuildDistToRootWindowTree(Ptr<CTestTriPointerObject> pWindow, Ptr<CTestTriPointerObject> pParent, size uiDepth, size uiBranching, CArrayBaseObjectPtr* papcNodes)
{
	Ptr<CArrayObject>				pChildren;
	Ptr<CTestTriPointerObject>		pChild;
	size							i;

	papcNodes->Add(&pParent);
	pParent->mpObject2 = pWindow;
	if (uiDepth == 0)
	{
		return;
	}
	pChildren = OMalloc<CArrayObject>();
	pParent->mpObject3 = pChildren;
	for (i = 0; i < uiBranching; i++)
	{
		pChild = OMalloc<CTestTriPointerObject>();
		pChild->mpObject1 = pParent;
		pChildren->Add(pChild);
		BuildDistToRootWindowTree(pWindow, pChild, uiDepth - 1, uiBranching, papcNodes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the milliseconds the focus took to move across every node.
//
//////////////////////////////////////////////////////////////////////////
float TestDistToRootUpdateFocusInWindow(size uiDepth, size uiBranching, bool bValidateEachStep)
{
	Ptr<CTestTriPointerObject>	pWindow;
	Ptr<CTestTriPointerObject>	pTop;
	Ptr<CTestObject>			pFocus;
	CArrayBaseObjectPtr			apcNodes;
	CTimer						cTimer;
	float						fMilliseconds;
	size						i;

	ObjectsInit();
	apcNodes.Init();
	if (!bValidateEachStep)
	{
		//Debug builds validate every object on every change, which would be all this measured.
		gcObjects.DisableValidation();
	}
	{
		pWindow = OMalloc<CTestTriPointerObject>();
		ORoot()->Add(pWindow);
		pTop = OMalloc<CTestTriPointerObject>();
		pTop->mpObject1 = pWindow;
		pWindow->mpObject1 = pTop;
		BuildDistToRootWindowTree(pWindow, pTop, uiDepth, uiBranching, &apcNodes);
		pFocus = OMalloc<CTestObject>();
		pWindow->mpObject2 = pFocus;
		AssertDistToRoot();

		cTimer.Init();
		for (i = 0; i < apcNodes.NumElements(); i++)
		{
			pFocus->mpObject = apcNodes.GetPtr(i);
			if (bValidateEachStep)
			{
				AssertDistToRoot();
			}
		}
		pFocus->mpObject = NULL;
		cTimer.Update();
		fMilliseconds = (float)cTimer.GetTotalTimeInMillieconds();
		cTimer.Kill();
		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();
		AssertDistToRoot();

		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTop = NULL;
		pFocus = NULL;
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	apcNodes.Kill();
	ObjectsFlush();
	ObjectsKill();
	return fMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
// The speed tests' tree depth: DIST_TO_ROOT_TREE_DEPTH makes them bigger
// when measuring on the host (and logs how long they took).
//
//////////////////////////////////////////////////////////////////////////
size DistToRootTreeDepth(void)
{
#ifndef __LIBC65__
	char*	szDepth;

	szDepth = getenv("DIST_TO_ROOT_TREE_DEPTH");
	if (szDepth)
	{
		return (size)atoi(szDepth);
	}
#endif // !__LIBC65__
	return DIST_TO_ROOT_SPEED_DEPTH;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogDistToRootSpeed(const char* szWhat, size uiDepth, float fMilliseconds)
{
#ifndef __LIBC65__
	if (getenv("DIST_TO_ROOT_TREE_DEPTH"))
	{
		gcLogger.Info2(szWhat, " of depth ", IntToString((int)uiDepth), ": ", IntToString((int)fMilliseconds), " ms", NULL);
	}
#endif // !__LIBC65__
}


//////////////////////////////////////////////////////////////////////////
//
// A focus moving across a big tree (a window's worth of components) must
// stay fast: each move used to recalculate every node's distance through
// CanFindRoot, many times over.  DIST_TO_ROOT_TREE_DEPTH can make it
// bigger when measuring.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateFocusSpeed(void)
{
	size		uiDepth;
	float		fMilliseconds;

	uiDepth = DistToRootTreeDepth();
	fMilliseconds = TestDistToRootUpdateFocusInWindow(uiDepth, 3, false);
	LogDistToRootSpeed("Focus across a window tree", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// A tree whose nodes point back to their parents, held by the root at its
// top and at one leaf.  Taking the top away re-finds every node through
// the leaf (every distance changes at once), then putting it back finds
// them through the top again.  Returns the milliseconds both took.
//
//////////////////////////////////////////////////////////////////////////
float TestDistToRootUpdateReattachTree(size uiDepth, size uiBranching, bool bValidate)
{
	Ptr<CTestTriPointerObject>	pWindow;
	Ptr<CTestTriPointerObject>	pTop;
	Ptr<CTestTriPointerObject>	pLeaf;
	CArrayBaseObjectPtr			apcNodes;
	CTimer						cTimer;
	float						fMilliseconds;

	ObjectsInit();
	apcNodes.Init();
	if (!bValidate)
	{
		gcObjects.DisableValidation();
	}
	{
		pWindow = OMalloc<CTestTriPointerObject>();
		pTop = OMalloc<CTestTriPointerObject>();
		BuildDistToRootWindowTree(pWindow, pTop, uiDepth, uiBranching, &apcNodes);
		pLeaf = (CTestTriPointerObject*)apcNodes.GetPtr(apcNodes.NumElements() - 1);
		ORoot()->Add(pTop);
		ORoot()->Add(pLeaf);
		pWindow = NULL;
		AssertDistToRoot();
		AssertInt(2, pTop->GetDistToRoot());
		AssertInt(2, pLeaf->GetDistToRoot());

		cTimer.Init();
		ORoot()->Remove(pTop);
		if (bValidate)
		{
			AssertDistToRoot();
			AssertInt(2 + (int)uiDepth, pTop->GetDistToRoot());  //Up through the parents.
		}
		ORoot()->Add(pTop);
		cTimer.Update();
		fMilliseconds = (float)cTimer.GetTotalTimeInMillieconds();
		cTimer.Kill();

		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();
		AssertDistToRoot();
		AssertInt(2, pTop->GetDistToRoot());

		ORoot()->RemoveAll();
		pTop = NULL;
		pLeaf = NULL;
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	apcNodes.Kill();
	ObjectsFlush();
	ObjectsKill();
	return fMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateReattachSpeed(void)
{
	size		uiDepth;
	float		fMilliseconds;

	uiDepth = DistToRootTreeDepth();
	fMilliseconds = TestDistToRootUpdateReattachTree(uiDepth, 3, false);
	LogDistToRootSpeed("Reattach a tree", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// Components built before they are added to a window: a chain of objects
// held from the stack, each pointing at a hub on the root that already
// has many froms.  Every pointer change in the chain works out its
// distance to the stack; none of it can change the hub's distance to the
// root.  Returns the milliseconds the chain took to build.
//
//////////////////////////////////////////////////////////////////////////
float TestDistToRootUpdateBuildUnrooted(size uiAttached, size uiBuilt, bool bValidate)
{
	Ptr<CTestObject>	pHub;
	Ptr<CArrayObject>	pAttached;
	Ptr<CTestObject>	pFirst;
	Ptr<CTestObject>	pLast;
	Ptr<CTestObject>	pObject;
	CTimer				cTimer;
	float				fMilliseconds;
	size				i;

	ObjectsInit();
	if (!bValidate)
	{
		gcObjects.DisableValidation();
	}
	{
		pHub = OMalloc<CTestObject>();
		pAttached = OMalloc<CArrayObject>();
		ORoot()->Add(pHub);
		pHub->mpObject = pAttached;
		for (i = 0; i < uiAttached; i++)
		{
			pObject = OMalloc<CTestObject>();
			pObject->mpObject = pHub;
			pAttached->Add(pObject);
		}

		cTimer.Init();
		pFirst = OMalloc<CTestObject>();
		pFirst->mpObject = pHub;
		pLast = pFirst;
		for (i = 1; i < uiBuilt; i++)
		{
			pObject = OMalloc<CTestObject>();
			pObject->mpObject = pHub;
			pLast->mpTest = pObject;
			pLast = pObject;
		}
		pObject = NULL;
		pLast = NULL;
		cTimer.Update();
		fMilliseconds = (float)cTimer.GetTotalTimeInMillieconds();
		cTimer.Kill();

		gcObjects.EnableValidation();
		AssertInt(UNATTACHED_DIST_TO_ROOT, pFirst->GetDistToRoot());
		AssertInt(2, pHub->GetDistToRoot());
		AssertSize((size)(uiAttached + uiBuilt + 1), pHub->NumHeapFroms());
		AssertDistToRoot();

		pFirst = NULL;
		AssertSize(uiAttached + 1, pHub->NumHeapFroms());
		AssertDistToRoot();

		ORoot()->RemoveAll();
		pHub = NULL;
		pAttached = NULL;
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
	return fMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateBuildUnrootedSpeed(void)
{
	size		uiDepth;
	size		uiChain;
	float		fMilliseconds;

	uiDepth = DistToRootTreeDepth();
	uiChain = (size)1 << (uiDepth + 4);
	if (uiChain > DIST_TO_ROOT_MAX_UNROOTED_CHAIN)
	{
		uiChain = DIST_TO_ROOT_MAX_UNROOTED_CHAIN;
	}
	fMilliseconds = TestDistToRootUpdateBuildUnrooted((size)1 << (uiDepth + 5), uiChain, false);
	LogDistToRootSpeed("Build unrooted next to a hub", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// A tree built before it is rooted, each node's children added while a
// stack pointer holds the node.  Moving the stack pointer off a child
// finds the node holding it straight away.  Letting go of a node part way
// down frees nothing (its parent holds it); letting go of the top frees
// the lot.  Returns the milliseconds building took.
//
//////////////////////////////////////////////////////////////////////////
float TestDistToRootUpdateBuildUnrootedTree(size uiDepth, size uiBranching, bool bValidate)
{
	Ptr<CTestTriPointerObject>	pTop;
	CArrayBaseObjectPtr			apcNodes;
	CTimer						cTimer;
	float						fMilliseconds;
	size						uiNodes;

	ObjectsInit();
	apcNodes.Init();
	if (!bValidate)
	{
		gcObjects.DisableValidation();
	}
	{
		Ptr<CTestTriPointerObject>	pWindow;

		pWindow = OMalloc<CTestTriPointerObject>();
		ORoot()->Add(pWindow);

		cTimer.Init();
		pTop = OMalloc<CTestTriPointerObject>();
		BuildDistToRootWindowTree(pWindow, pTop, uiDepth, uiBranching, &apcNodes);
		cTimer.Update();
		fMilliseconds = (float)cTimer.GetTotalTimeInMillieconds();
		cTimer.Kill();

		gcObjects.EnableValidation();
		uiNodes = apcNodes.NumElements();
		AssertLong(3 + (int64)uiNodes + (int64)(uiNodes - 1) / (int64)uiBranching, gcObjects.NumMemoryIndexes());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pTop->GetDistToRoot());
		AssertDistToRoot();
		gcObjects.ValidateObjectsConsistency();

		{
			Ptr<CTestTriPointerObject>	pMiddle;

			pMiddle = (CTestTriPointerObject*)apcNodes.GetPtr(1);
			pMiddle = NULL;
		}
		AssertLong(3 + (int64)uiNodes + (int64)(uiNodes - 1) / (int64)uiBranching, gcObjects.NumMemoryIndexes());

		apcNodes.Kill();
		pTop = NULL;
		AssertLong(3, gcObjects.NumMemoryIndexes());
		ORoot()->RemoveAll();
		pWindow = NULL;
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
	return fMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateBuildUnrootedTreeSpeed(void)
{
	size		uiDepth;
	float		fMilliseconds;

	uiDepth = DistToRootTreeDepth();
	fMilliseconds = TestDistToRootUpdateBuildUnrootedTree(uiDepth, 3, false);
	LogDistToRootSpeed("Build an unrooted tree", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// A long chain taken off the root while its head and tail are held from
// the stack: clearing its distances to the root walks down its whole
// length, and letting go of the tail walks back up it to the head to find
// the tail's distance to the stack (both used to recurse, once per
// object).  Then letting go of the head frees all of it.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateLongChain(size uiLength)
{
	Ptr<CTestObject>			pFirst;
	Ptr<CTestObject>			pLast;
	Ptr<CTestObject>			pObject;
	STestObjectFreedNotifier	sLastNotifier;
	size						i;

	ObjectsInit();
	gcObjects.DisableValidation();
	{
		pFirst = OMalloc<CTestObject>();
		ORoot()->Add(pFirst);
		pLast = pFirst;
		for (i = 1; i < uiLength; i++)
		{
			pObject = OMalloc<CTestObject>((i == uiLength - 1) ? &sLastNotifier : NULL);
			pLast->mpTest = pObject;
			pLast = pObject;
		}
		AssertInt((int)uiLength + 1, pLast->GetDistToRoot());
		pObject = NULL;

		ORoot()->Remove(pFirst);
		AssertInt(UNATTACHED_DIST_TO_ROOT, pFirst->GetDistToRoot());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pLast->GetDistToRoot());

		pLast = NULL;
		AssertFalse(sLastNotifier.bFreed);
		AssertLong(2 + (int64)uiLength, gcObjects.NumMemoryIndexes());

		pFirst = NULL;
		AssertTrue(sLastNotifier.bFreed);
		AssertLong(2, gcObjects.NumMemoryIndexes());
	}
	gcObjects.EnableValidation();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A batch of objects freed together, each of which changes pointers in its
// Free().  Those changes must not each work out again what the whole batch
// detaches (it took over a minute for an IDE window).  Returns the
// milliseconds freeing took.
//
//////////////////////////////////////////////////////////////////////////
float TestDistToRootUpdateFreeBatchChangingPointers(size uiHolders, bool bValidate)
{
	Ptr<CArrayObject>			pHolders;
	Ptr<CTestObject>			pShared;
	STestObjectFreedNotifier	sSharedNotifier;
	CTimer						cTimer;
	float						fMilliseconds;
	size						i;

	ObjectsInit();
	if (!bValidate)
	{
		gcObjects.DisableValidation();
	}
	{
		pHolders = OMalloc<CArrayObject>();
		ORoot()->Add(pHolders);
		pShared = OMalloc<CTestObject>(&sSharedNotifier);
		for (i = 0; i < uiHolders; i++)
		{
			Ptr<CDistToRootFreeingHolder>	pHolder;

			pHolder = OMalloc<CDistToRootFreeingHolder>(pShared);
			pHolders->Add(pHolder);
		}
		pShared = NULL;
		AssertLong(4 + (int64)uiHolders, gcObjects.NumMemoryIndexes());
		AssertFalse(sSharedNotifier.bFreed);

		cTimer.Init();
		ORoot()->Remove(pHolders);
		pHolders = NULL;
		cTimer.Update();
		fMilliseconds = (float)cTimer.GetTotalTimeInMillieconds();
		cTimer.Kill();

		AssertTrue(sSharedNotifier.bFreed);
		AssertLong(2, gcObjects.NumMemoryIndexes());
		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill();
	return fMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateFreeBatchSpeed(void)
{
	size		uiDepth;
	float		fMilliseconds;

	uiDepth = DistToRootTreeDepth();
	fMilliseconds = TestDistToRootUpdateFreeBatchChangingPointers((size)1 << (uiDepth + 6), false);
	LogDistToRootSpeed("Free a batch changing pointers", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// Killing a hub that its children, reachable only through it, point back
// to.  Each back pointer is put back for the child's Free (the children
// die with the hub) and then cleared.  Remembering and putting back the
// pointers used to search all the froms, and then all the killed objects,
// for each one.
//
//////////////////////////////////////////////////////////////////////////
float TestDistToRootUpdateKillHubWithBackPointers(size uiChildren, bool bValidate)
{
	Ptr<CTestObject>	pHub;
	Ptr<CArrayObject>	pChildren;
	Ptr<CTestObject>	pChild;
	CTimer				cTimer;
	float				fMilliseconds;
	size				i;
	size				uiRootOnly;

	ObjectsInit();
	if (!bValidate)
	{
		gcObjects.DisableValidation();
	}
	{
		ORoot();
		uiRootOnly = gcObjects.NumMemoryIndexes();
		pHub = OMalloc<CTestObject>();
		pChildren = OMalloc<CArrayObject>();
		ORoot()->Add(pHub);
		pHub->mpObject = pChildren;
		for (i = 0; i < uiChildren; i++)
		{
			pChild = OMalloc<CTestObject>();
			pChild->mpObject = pHub;
			pChildren->Add(pChild);
		}
		pChild = NULL;
		pChildren = NULL;
		AssertSize(uiChildren + 1, pHub->NumHeapFroms());

		cTimer.Init();
		pHub->Kill();
		cTimer.Update();
		fMilliseconds = (float)cTimer.GetTotalTimeInMillieconds();
		cTimer.Kill();

		gcObjects.EnableValidation();
		AssertSize(uiRootOnly, gcObjects.NumMemoryIndexes());
		AssertDistToRoot();
	}
	ObjectsFlush();
	ObjectsKill();
	return fMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateKillHubWithBackPointersSpeed(void)
{
	size		uiDepth;
	float		fMilliseconds;

	uiDepth = DistToRootTreeDepth();
	fMilliseconds = TestDistToRootUpdateKillHubWithBackPointers((size)1 << (uiDepth + 6), false);
	LogDistToRootSpeed("Kill a hub with back pointers", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// The kinds of collection the collection speed tests use.
//
//////////////////////////////////////////////////////////////////////////
enum ESpeedCollection
{
	SC_Array,
	SC_Set,
	SC_Map,
	SC_Index
};


//////////////////////////////////////////////////////////////////////////
//
// Killing, one at a time, the members of a big collection that lives on.
// The collection is only a from on each member, so it has to find each
// one in itself.  A map's members are values; its keys live on.
//
//////////////////////////////////////////////////////////////////////////
float TestDistToRootUpdateKillCollectionMembers(ESpeedCollection eCollection, size uiMembers, bool bValidate)
{
	CArrayBaseObjectPtr		apcMembers;
	CTimer					cTimer;
	float					fMilliseconds;
	size					i;
	size					uiRootOnly;
	char					szKey[32];

	ObjectsInit();
	if (!bValidate)
	{
		gcObjects.DisableValidation();
	}
	apcMembers.Init();
	{
		Ptr<CArrayObject>		pArray;
		Ptr<CSetObject>			pSet;
		Ptr<CMapObject>			pMap;
		Ptr<CIndexObject>		pIndex;
		Ptr<CTestObject>		pMember;

		ORoot();
		uiRootOnly = gcObjects.NumMemoryIndexes();
		if (eCollection == SC_Array)
		{
			pArray = OMalloc<CArrayObject>();
			ORoot()->Add(pArray);
		}
		else if (eCollection == SC_Set)
		{
			pSet = OMalloc<CSetObject>();
			ORoot()->Add(pSet);
		}
		else if (eCollection == SC_Map)
		{
			pMap = OMalloc<CMapObject>();
			ORoot()->Add(pMap);
		}
		else
		{
			pIndex = OMalloc<CIndexObject>();
			ORoot()->Add(pIndex);
		}

		for (i = 0; i < uiMembers; i++)
		{
			pMember = OMalloc<CTestObject>();
			if (eCollection == SC_Array)
			{
				pArray->Add(pMember);
			}
			else if (eCollection == SC_Set)
			{
				pSet->Add(pMember);
			}
			else if (eCollection == SC_Map)
			{
				pMap->Put(OMalloc<CTestObject>(), pMember);
			}
			else
			{
				sprintf(szKey, "k%i", (int)i);
				pIndex->Put(szKey, pMember);
			}
			apcMembers.Add(pMember.BaseObject());
		}
		pMember = NULL;

		cTimer.Init();
		for (i = 0; i < uiMembers; i++)
		{
			apcMembers.GetPtr(i)->Kill();
		}
		cTimer.Update();
		fMilliseconds = (float)cTimer.GetTotalTimeInMillieconds();
		cTimer.Kill();

		gcObjects.EnableValidation();
		if (eCollection == SC_Map)
		{
			AssertSize(uiRootOnly + 1 + uiMembers, gcObjects.NumMemoryIndexes());
		}
		else
		{
			AssertSize(uiRootOnly + 1, gcObjects.NumMemoryIndexes());
		}
		AssertDistToRoot();
	}
	apcMembers.Kill();
	ObjectsFlush();
	ObjectsKill();
	return fMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
// Objects attached to the root letting go, one at a time, of a hub that
// lives on.  The hub has to find each one in its froms.
//
//////////////////////////////////////////////////////////////////////////
float TestDistToRootUpdateLetGoOfLivingHub(size uiHolders, bool bValidate)
{
	CArrayBaseObjectPtr		apcHolders;
	CTimer					cTimer;
	float					fMilliseconds;
	size					i;

	ObjectsInit();
	if (!bValidate)
	{
		gcObjects.DisableValidation();
	}
	apcHolders.Init();
	{
		Ptr<CTestObject>		pHub;
		Ptr<CArrayObject>		pHolders;
		Ptr<CTestObject>		pHolder;

		pHub = OMalloc<CTestObject>();
		pHolders = OMalloc<CArrayObject>();
		ORoot()->Add(pHub);
		ORoot()->Add(pHolders);
		for (i = 0; i < uiHolders; i++)
		{
			pHolder = OMalloc<CTestObject>();
			pHolder->mpObject = pHub;
			pHolders->Add(pHolder);
			apcHolders.Add(pHolder.BaseObject());
		}
		pHolder = NULL;
		AssertSize(uiHolders + 1, pHub->NumHeapFroms());

		cTimer.Init();
		for (i = 0; i < uiHolders; i++)
		{
			((CTestObject*)apcHolders.GetPtr(i))->mpObject = NULL;
		}
		cTimer.Update();
		fMilliseconds = (float)cTimer.GetTotalTimeInMillieconds();
		cTimer.Kill();

		gcObjects.EnableValidation();
		AssertSize(1, pHub->NumHeapFroms());
		AssertDistToRoot();
	}
	apcHolders.Kill();
	ObjectsFlush();
	ObjectsKill();
	return fMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
// Freeing a big array together with its members, by letting go of it.
//
//////////////////////////////////////////////////////////////////////////
float TestDistToRootUpdateFreeCollectionWithMembers(size uiMembers, bool bValidate)
{
	CTimer				cTimer;
	float				fMilliseconds;
	size				i;
	size				uiRootOnly;

	ObjectsInit();
	if (!bValidate)
	{
		gcObjects.DisableValidation();
	}
	{
		Ptr<CArrayObject>	pArray;
		Ptr<CTestObject>	pMember;

		ORoot();
		uiRootOnly = gcObjects.NumMemoryIndexes();
		pArray = OMalloc<CArrayObject>();
		ORoot()->Add(pArray);
		for (i = 0; i < uiMembers; i++)
		{
			pMember = OMalloc<CTestObject>();
			pArray->Add(pMember);
		}
		pMember = NULL;

		cTimer.Init();
		ORoot()->Remove(pArray);
		pArray = NULL;
		cTimer.Update();
		fMilliseconds = (float)cTimer.GetTotalTimeInMillieconds();
		cTimer.Kill();

		gcObjects.EnableValidation();
		AssertSize(uiRootOnly, gcObjects.NumMemoryIndexes());
		AssertDistToRoot();
	}
	ObjectsFlush();
	ObjectsKill();
	return fMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateCollectionSpeeds(void)
{
	size		uiDepth;
	size		uiCount;
	float		fMilliseconds;

	uiDepth = DistToRootTreeDepth();
	uiCount = (size)1 << (uiDepth + 5);

	fMilliseconds = TestDistToRootUpdateKillCollectionMembers(SC_Array, uiCount, false);
	LogDistToRootSpeed("Kill the members of a living array", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);

	fMilliseconds = TestDistToRootUpdateKillCollectionMembers(SC_Set, uiCount, false);
	LogDistToRootSpeed("Kill the members of a living set", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);

	fMilliseconds = TestDistToRootUpdateKillCollectionMembers(SC_Map, uiCount, false);
	LogDistToRootSpeed("Kill the values of a living map", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);

	fMilliseconds = TestDistToRootUpdateKillCollectionMembers(SC_Index, uiCount, false);
	LogDistToRootSpeed("Kill the members of a living index", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);

	fMilliseconds = TestDistToRootUpdateLetGoOfLivingHub(uiCount, false);
	LogDistToRootSpeed("Let go of a living hub", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);

	fMilliseconds = TestDistToRootUpdateFreeCollectionWithMembers(uiCount, false);
	LogDistToRootSpeed("Free an array with its members", uiDepth, fMilliseconds);
	AssertTrue(fMilliseconds < 10000.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// A hub (an object with many heap froms) keeps where each from is and a
// from closer to root.  Its froms are objects (some pointing to it twice)
// and collections; validating checks every location kept, and the
// distances are checked against a search from the root.  Below half the
// froms needed the hub stops keeping them, and starts again above.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateHubFroms(void)
{
	char					szErrors[1024];
	size					i;
	CArrayBaseObjectPtr		apcHolders;
	CTestTriPointerObject*	pcHolder;

	ObjectsInit();
	gcObjects.AddConstructor<CTestTriPointerObject>();
	apcHolders.Init();
	{
		Ptr<CTestObject>			pHub;
		Ptr<CArrayObject>			pHolders;
		Ptr<CSetObject>				pSet;
		Ptr<CTestTriPointerObject>	pHolder;

		pHub = OMalloc<CTestObject>();
		pHolders = OMalloc<CArrayObject>();
		pSet = OMalloc<CSetObject>();
		ORoot()->Add(pHolders);
		ORoot()->Add(pHub);
		pHolders->Add(pSet);
		pSet->Add(pHub);
		pSet->Add(pHub);
		for (i = 0; i < 80; i++)
		{
			pHolder = OMalloc<CTestTriPointerObject>();
			pHolder->mpObject1 = pHub;
			if (i % 5 == 0)
			{
				pHolder->mpObject3 = pHub;
			}
			pHolders->Add(pHolder);
			apcHolders.Add(pHolder.BaseObject());
		}
		pHolder = NULL;
		AssertSize(99, pHub->NumHeapFroms());
		AssertDistToRoot();
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		//The closest from goes: the hub is held through the holders and the set now.
		ORoot()->Remove(pHub);
		AssertSize(98, pHub->NumHeapFroms());
		AssertDistToRoot();
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		//Holders let go from the middle and from the end, one pointer at a time.
		for (i = 0; i < 70; i++)
		{
			pcHolder = (CTestTriPointerObject*)apcHolders.GetPtr((i % 2 == 0) ? (i / 2) : (79 - i / 2));
			if (pcHolder->mpObject3.IsNotNull())
			{
				pcHolder->mpObject3 = NULL;
			}
			else
			{
				pcHolder->mpObject1 = NULL;
			}
			if (i % 10 == 9)
			{
				AssertDistToRoot();
				ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
				AssertString("", szErrors);
			}
		}
		AssertSize(98 - 70, pHub->NumHeapFroms());

		pSet->Remove(pHub);
		AssertDistToRoot();
		for (i = 0; i < 80; i++)
		{
			pcHolder = (CTestTriPointerObject*)apcHolders.GetPtr(i);
			pcHolder->mpObject2 = pHub;
		}
		AssertSize(98 - 70 - 2 + 80, pHub->NumHeapFroms());
		AssertDistToRoot();
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		pHolders->RemoveAll();
		AssertSize(0, pHub->NumHeapFroms());
		AssertDistToRoot();
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);
	}
	apcHolders.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdate(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestDistToRootUpdateChainShortcut();
	TestDistToRootUpdateFocusInTree(3, 3, true);
	TestDistToRootUpdateFocusInTree(5, 2, true);
	TestDistToRootUpdateHub();
	TestDistToRootUpdateFocusInWindow(3, 3, true);
	TestDistToRootUpdateFocusInWindow(4, 2, true);
	TestDistToRootUpdateEmbeddedCollections();
	TestDistToRootUpdateRandom(1, DIST_TO_ROOT_RANDOM_STEPS);
	TestDistToRootUpdateRandom(2, DIST_TO_ROOT_RANDOM_STEPS);
	TestDistToRootUpdateRandom(3, DIST_TO_ROOT_RANDOM_STEPS);
	TestDistToRootUpdateAlternatePaths();
	TestDistToRootUpdateBackPointersToHub();
	TestDistToRootUpdateUnattachedPointingAtHub();
	TestDistToRootUpdateRandomFreeing(1, DIST_TO_ROOT_RANDOM_STEPS);
	TestDistToRootUpdateRandomFreeing(2, DIST_TO_ROOT_RANDOM_STEPS);
	TestDistToRootUpdateRandomFreeing(3, DIST_TO_ROOT_RANDOM_STEPS);
	TestDistToRootUpdateFocusInWindow(4, 3, true);
	TestDistToRootUpdateFocusSpeed();
	TestDistToRootUpdateReattachTree(3, 3, true);
	TestDistToRootUpdateReattachTree(5, 2, true);
	TestDistToRootUpdateReattachSpeed();
	TestDistToRootUpdateBuildUnrooted(20, 10, true);
	TestDistToRootUpdateBuildUnrootedSpeed();
	TestDistToRootUpdateLongChain(DIST_TO_ROOT_LONG_CHAIN);
	TestDistToRootUpdateBuildUnrootedTree(3, 3, true);
	TestDistToRootUpdateBuildUnrootedTreeSpeed();
	TestDistToRootUpdateFreeBatchChangingPointers(5, true);
	TestDistToRootUpdateFreeBatchSpeed();
	TestDistToRootUpdateKillHubWithBackPointers(5, true);
	TestDistToRootUpdateKillHubWithBackPointersSpeed();
	TestDistToRootUpdateKillCollectionMembers(SC_Array, 5, true);
	TestDistToRootUpdateKillCollectionMembers(SC_Set, 5, true);
	TestDistToRootUpdateKillCollectionMembers(SC_Map, 5, true);
	TestDistToRootUpdateKillCollectionMembers(SC_Index, 5, true);
	TestDistToRootUpdateLetGoOfLivingHub(5, true);
	TestDistToRootUpdateLetGoOfLivingHub(100, true);
	TestDistToRootUpdateHubFroms();
	TestDistToRootUpdateFreeCollectionWithMembers(5, true);
	TestDistToRootUpdateCollectionSpeeds();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

