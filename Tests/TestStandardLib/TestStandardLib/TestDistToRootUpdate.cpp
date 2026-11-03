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


//The distances to the root are kept up to date as pointers change (SetExpectedDistToRoot when one is added, the dist
//calculator when one goes).  These tests check them against a breadth first search from the root after every change.


#define DIST_TO_ROOT_RANDOM_OBJECTS		40
#define DIST_TO_ROOT_RANDOM_ARRAYS		6
#define DIST_TO_ROOT_FREEING_MAX		80


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
	CArrayBaseObjectPtr		apcQueue;
	CArrayEmbeddedObjectPtr	apcTos;
	SIndexesIterator		sIter;
	CBaseObject*			pcObject;
	CBaseObject*			pcTo;
	CBaseObject*			pcRoot;
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

	apcQueue.Init();
	pcRoot = NULL;
	for (i = 0; i < uiNumObjects; i++)
	{
		if (apcObjects.GetPtr(i)->IsRoot())
		{
			pcRoot = apcObjects.GetPtr(i);
			aiDist.SetValue(i, ROOT_DIST_TO_ROOT);
			apcQueue.Add(pcRoot);
		}
	}

	uiFirst = 0;
	while (uiFirst < apcQueue.NumElements())
	{
		pcObject = apcQueue.GetPtr(uiFirst);
		uiFirst++;
		for (i = 0; apcObjects.GetPtr(i) != pcObject; i++)
		{
		}
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
						apcQueue.Add(pcTo);
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

	apcQueue.Kill();
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
// Random changes where nothing is held on the stack between steps, so
// every object cut off from the root is freed.  The graph runs through
// arrays, a set, an index and a map as well as object pointers.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToRootUpdateRandomFreeing(uint32 uiSeed, size uiSteps)
{
	CRandom							cRandom;
	SIndexesIterator				sIter;
	CBaseObject*					pcObject;
	CArrayTemplatePtr<CTestObject>	apcObjects;
	CArrayTemplatePtr<CArrayObject>	apcArrays;
	CArrayTemplatePtr<CSetObject>	apcSets;
	CArrayTemplatePtr<CIndexObject>	apcIndices;
	CArrayTemplatePtr<CMapObject>	apcMaps;
	CArrayTemplatePtr<CCollection>	apcCollections;
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

			pcA = NULL;
			pcB = NULL;
			if (apcObjects.NumElements() > 0)
			{
				pcA = apcObjects.GetPtr(cRandom.Next(0, (int32)apcObjects.NumElements() - 1));
				pcB = apcObjects.GetPtr(cRandom.Next(0, (int32)apcObjects.NumElements() - 1));
			}
			pcCollection = NULL;
			if (apcCollections.NumElements() > 0)
			{
				pcCollection = apcCollections.GetPtr(cRandom.Next(0, (int32)apcCollections.NumElements() - 1));
			}
			sprintf(szKey, "k%i", cRandom.Next(0, 9));

			iOperation = cRandom.Next(0, 14);
			if ((pcA == NULL) || ((iOperation == 0) && (apcObjects.NumElements() < DIST_TO_ROOT_FREEING_MAX)))
			{
				Ptr<CTestObject>	pNew;

				//A new object somewhere reachable (or not, then it is freed straight away).
				pNew = OMalloc<CTestObject>();
				if (apcArrays.NumElements() > 0)
				{
					apcArrays.GetPtr(cRandom.Next(0, (int32)apcArrays.NumElements() - 1))->Add(pNew);
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
					if (apcArrays.NumElements() > 0)
					{
						apcArrays.GetPtr(cRandom.Next(0, (int32)apcArrays.NumElements() - 1))->Add(pA);
					}
					break;
				case 6:
					if (apcArrays.NumElements() > 0)
					{
						CArrayObject*	pcArray;

						pcArray = apcArrays.GetPtr(cRandom.Next(0, (int32)apcArrays.NumElements() - 1));
						if (pcArray->NumElements() > 0)
						{
							pcArray->Remove((size)cRandom.Next(0, (int32)pcArray->NumElements() - 1));
						}
					}
					break;
				case 7:
					if (apcSets.NumElements() > 0)
					{
						if (apcSets.GetPtr(0)->Contains(pA))
						{
							apcSets.GetPtr(0)->Remove(pA);
						}
						else
						{
							apcSets.GetPtr(0)->Add(pA);
						}
					}
					break;
				case 8:
					if (apcIndices.NumElements() > 0)
					{
						apcIndices.GetPtr(0)->Put(szKey, pA);
					}
					break;
				case 9:
					if (apcIndices.NumElements() > 0)
					{
						if (apcIndices.GetPtr(0)->Get(szKey).IsNotNull())
						{
							apcIndices.GetPtr(0)->Remove(szKey);
						}
					}
					break;
				case 10:
					if (apcMaps.NumElements() > 0)
					{
						apcMaps.GetPtr(0)->Put(pA, pB);
					}
					break;
				case 11:
					if (apcMaps.NumElements() > 0)
					{
						if (apcMaps.GetPtr(0)->Get(pA).IsNotNull())
						{
							apcMaps.GetPtr(0)->Remove(pA);
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

			apcCollections.Kill();
			apcMaps.Kill();
			apcIndices.Kill();
			apcSets.Kill();
			apcArrays.Kill();
			apcObjects.Kill();

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

	uiDepth = getenv("DIST_TO_ROOT_TREE_DEPTH") ? (size)atoi(getenv("DIST_TO_ROOT_TREE_DEPTH")) : 6;
	fMilliseconds = TestDistToRootUpdateFocusInWindow(uiDepth, 3, false);
	if (getenv("DIST_TO_ROOT_TREE_DEPTH"))
	{
		gcLogger.Info2("Focus across a window tree of depth ", IntToString((int)uiDepth), ": ", IntToString((int)fMilliseconds), " ms", NULL);
	}
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

	uiDepth = getenv("DIST_TO_ROOT_TREE_DEPTH") ? (size)atoi(getenv("DIST_TO_ROOT_TREE_DEPTH")) : 6;
	fMilliseconds = TestDistToRootUpdateReattachTree(uiDepth, 3, false);
	if (getenv("DIST_TO_ROOT_TREE_DEPTH"))
	{
		gcLogger.Info2("Reattach a tree of depth ", IntToString((int)uiDepth), ": ", IntToString((int)fMilliseconds), " ms", NULL);
	}
	AssertTrue(fMilliseconds < 10000.0f);
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
	TestDistToRootUpdateRandom(1, 1500);
	TestDistToRootUpdateRandom(2, 1500);
	TestDistToRootUpdateRandom(3, 1500);
	TestDistToRootUpdateAlternatePaths();
	TestDistToRootUpdateBackPointersToHub();
	TestDistToRootUpdateUnattachedPointingAtHub();
	TestDistToRootUpdateRandomFreeing(1, 1500);
	TestDistToRootUpdateRandomFreeing(2, 1500);
	TestDistToRootUpdateRandomFreeing(3, 1500);
	TestDistToRootUpdateFocusInWindow(4, 3, true);
	TestDistToRootUpdateFocusSpeed();
	TestDistToRootUpdateReattachTree(3, 3, true);
	TestDistToRootUpdateReattachTree(5, 2, true);
	TestDistToRootUpdateReattachSpeed();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

