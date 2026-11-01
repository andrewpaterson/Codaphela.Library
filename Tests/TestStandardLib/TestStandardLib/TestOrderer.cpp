#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/CodabaseFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/Root.h"
#include "StandardLib/Orderer.h"
#include "TestLib/Assert.h"
#include "ChunkFileObjectWriterTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertOrdered(Ptr<CBaseObject> pExpected, Ptr<CArray<CBaseObject>> pArray, size uiIndex)
{
	AssertPointer(pExpected.BaseObject(), pArray->Get(uiIndex).BaseObject());
}


//////////////////////////////////////////////////////////////////////////
//
// Orders nothing, as Logicim's Orderer does with an empty map.
//
//////////////////////////////////////////////////////////////////////////
void TestOrdererEmpty(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<COrderer>				pOrderer;
		Ptr<CArray<CBaseObject>>	pOrdered;
		Ptr<CArray<CBaseObject>>	pUnordered;

		pRoot = ORoot();
		pOrderer = OMalloc<COrderer>();
		pOrdered = OMalloc<CArray<CBaseObject>>();
		pUnordered = OMalloc<CArray<CBaseObject>>();
		pRoot->Add(pOrderer, pOrdered, pUnordered);

		AssertSize(0, pOrderer->NumObjects());
		AssertTrue(pOrderer->Order(pOrdered, pUnordered));
		AssertSize(0, pOrdered->NumElements());
		AssertSize(0, pUnordered->NumElements());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each object comes after the objects that fulfil what it requires.  A
// chain added backwards takes a pass per link.  Objects ordered earlier in
// a pass fulfil the requirements of objects later in the same pass.
//
//////////////////////////////////////////////////////////////////////////
void TestOrdererChain(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestInteger>			pA;
		Ptr<CTestInteger>			pB;
		Ptr<CTestInteger>			pC;
		Ptr<COrderer>				pOrderer;
		Ptr<CArray<CBaseObject>>	pOrdered;

		pRoot = ORoot();
		pA = OMalloc<CTestInteger>(1, 0, 0);
		pB = OMalloc<CTestInteger>(2, 0, 0);
		pC = OMalloc<CTestInteger>(3, 0, 0);
		pRoot->Add(pA, pB, pC);

		pOrderer = OMalloc<COrderer>();
		pRoot->Add(pOrderer);
		pOrderer->AddFulfilment(pC, "C");
		pOrderer->AddRequirement(pC, "B");
		pOrderer->AddFulfilment(pB, "B");
		pOrderer->AddRequirement(pB, "A");
		pOrderer->AddFulfilment(pA, "A");
		AssertSize(3, pOrderer->NumObjects());

		pOrdered = OMalloc<CArray<CBaseObject>>();
		pRoot->Add(pOrdered);
		AssertTrue(pOrderer->Order(pOrdered));
		AssertSize(3, pOrdered->NumElements());
		AssertOrdered(pA, pOrdered, 0);
		AssertOrdered(pB, pOrdered, 1);
		AssertOrdered(pC, pOrdered, 2);
		pRoot->Remove(pOrderer);

		//Added forwards the whole chain is ordered in one pass.
		pOrderer = OMalloc<COrderer>();
		pRoot->Add(pOrderer);
		pOrderer->AddFulfilment(pA, "A");
		pOrderer->AddFulfilment(pB, "B");
		pOrderer->AddRequirement(pB, "A");
		pOrderer->AddFulfilment(pC, "C");
		pOrderer->AddRequirement(pC, "B");

		pOrdered->RemoveAll();
		AssertTrue(pOrderer->Order(pOrdered));
		AssertSize(3, pOrdered->NumElements());
		AssertOrdered(pA, pOrdered, 0);
		AssertOrdered(pB, pOrdered, 1);
		AssertOrdered(pC, pOrdered, 2);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Objects whose requirements are met in the same pass keep the order they
// were added in.  An object can fulfil and require several names and
// repeated names are kept once.
//
//////////////////////////////////////////////////////////////////////////
void TestOrdererPasses(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestInteger>			pAdder;
		Ptr<CTestInteger>			pRegisters;
		Ptr<CTestInteger>			pClock;
		Ptr<CTestInteger>			pCPU;
		Ptr<CTestInteger>			pLoose;
		Ptr<COrderer>				pOrderer;
		Ptr<CArray<CBaseObject>>	pOrdered;
		Ptr<COrdererHolder>			pHolder;

		pRoot = ORoot();
		pAdder = OMalloc<CTestInteger>(1, 0, 0);
		pRegisters = OMalloc<CTestInteger>(2, 0, 0);
		pClock = OMalloc<CTestInteger>(3, 0, 0);
		pCPU = OMalloc<CTestInteger>(4, 0, 0);
		pLoose = OMalloc<CTestInteger>(5, 0, 0);
		pRoot->Add(pAdder, pRegisters, pClock);
		pRoot->Add(pCPU, pLoose);

		pOrderer = OMalloc<COrderer>();
		pRoot->Add(pOrderer);
		pOrderer->AddFulfilment(pCPU, "CPU");
		pOrderer->AddRequirement(pCPU, "Adder");
		pOrderer->AddRequirement(pCPU, "Registers");
		pOrderer->AddRequirement(pCPU, "Clock");
		pOrderer->AddRequirement(pCPU, "Adder");
		pOrderer->AddFulfilment(pRegisters, "Registers");
		pOrderer->AddRequirement(pRegisters, "Clock");
		pOrderer->AddFulfilment(pAdder, "Adder");
		pOrderer->AddFulfilment(pAdder, "HalfAdder");
		pOrderer->AddFulfilment(pAdder, "Adder");
		pOrderer->Add(pLoose);
		pOrderer->Add(pLoose);
		pOrderer->AddFulfilment(pClock, "Clock");
		AssertSize(5, pOrderer->NumObjects());

		pHolder = pOrderer->GetHolder(pCPU);
		AssertTrue(pHolder.IsNotNull());
		AssertPointer(pCPU.BaseObject(), pHolder->GetObject().BaseObject());
		AssertSize(3, pHolder->GetRequirements()->NumElements());
		AssertSize(1, pHolder->GetFulfilments()->NumElements());
		pHolder = pOrderer->GetHolder(pAdder);
		AssertSize(2, pHolder->GetFulfilments()->NumElements());
		AssertSize(0, pHolder->GetRequirements()->NumElements());
		AssertTrue(pOrderer->GetHolder(pRoot).IsNull());

		//Pass 1: Adder, Loose, Clock.  Pass 2: Registers.  Pass 3: CPU.
		pOrdered = OMalloc<CArray<CBaseObject>>();
		pRoot->Add(pOrdered);
		AssertTrue(pOrderer->Order(pOrdered));
		AssertSize(5, pOrdered->NumElements());
		AssertOrdered(pAdder, pOrdered, 0);
		AssertOrdered(pLoose, pOrdered, 1);
		AssertOrdered(pClock, pOrdered, 2);
		AssertOrdered(pRegisters, pOrdered, 3);
		AssertOrdered(pCPU, pOrdered, 4);

		//Ordering again gives the same order.
		pOrdered->RemoveAll();
		AssertTrue(pOrderer->Order(pOrdered));
		AssertSize(5, pOrdered->NumElements());
		AssertOrdered(pAdder, pOrdered, 0);
		AssertOrdered(pCPU, pOrdered, 4);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A requirement nothing fulfils, objects that require each other and an
// object that requires its own name can't be ordered.  The objects that
// can be are still ordered and the rest are given back in the order they
// were added.  Names are case sensitive.
//
//////////////////////////////////////////////////////////////////////////
void TestOrdererUnsatisfiable(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestInteger>			pX;
		Ptr<CTestInteger>			pY;
		Ptr<CTestInteger>			pZ;
		Ptr<CTestInteger>			pFree;
		Ptr<CTestInteger>			pAfterX;
		Ptr<COrderer>				pOrderer;
		Ptr<CArray<CBaseObject>>	pOrdered;
		Ptr<CArray<CBaseObject>>	pUnordered;

		pRoot = ORoot();
		pX = OMalloc<CTestInteger>(1, 0, 0);
		pY = OMalloc<CTestInteger>(2, 0, 0);
		pZ = OMalloc<CTestInteger>(3, 0, 0);
		pFree = OMalloc<CTestInteger>(4, 0, 0);
		pAfterX = OMalloc<CTestInteger>(5, 0, 0);
		pRoot->Add(pX, pY, pZ);
		pRoot->Add(pFree, pAfterX);
		pOrdered = OMalloc<CArray<CBaseObject>>();
		pUnordered = OMalloc<CArray<CBaseObject>>();
		pRoot->Add(pOrdered, pUnordered);

		//Each other.
		pOrderer = OMalloc<COrderer>();
		pRoot->Add(pOrderer);
		pOrderer->AddFulfilment(pX, "X");
		pOrderer->AddRequirement(pX, "Y");
		pOrderer->AddFulfilment(pAfterX, "AfterX");
		pOrderer->AddRequirement(pAfterX, "X");
		pOrderer->AddFulfilment(pY, "Y");
		pOrderer->AddRequirement(pY, "X");
		pOrderer->AddFulfilment(pFree, "Free");

		AssertFalse(pOrderer->Order(pOrdered, pUnordered));
		AssertSize(1, pOrdered->NumElements());
		AssertOrdered(pFree, pOrdered, 0);
		AssertSize(3, pUnordered->NumElements());
		AssertOrdered(pX, pUnordered, 0);
		AssertOrdered(pAfterX, pUnordered, 1);
		AssertOrdered(pY, pUnordered, 2);

		//Without an unordered array.
		pOrdered->RemoveAll();
		AssertFalse(pOrderer->Order(pOrdered));
		AssertSize(1, pOrdered->NumElements());
		pRoot->Remove(pOrderer);

		//Nothing fulfils "Missing", "z" is not "Z" and Z requires itself.
		pOrderer = OMalloc<COrderer>();
		pRoot->Add(pOrderer);
		pOrderer->AddFulfilment(pFree, "Z");
		pOrderer->AddFulfilment(pX, "X");
		pOrderer->AddRequirement(pX, "Missing");
		pOrderer->AddRequirement(pY, "z");
		pOrderer->AddFulfilment(pZ, "Self");
		pOrderer->AddRequirement(pZ, "Self");
		pOrderer->AddRequirement(pAfterX, "Z");

		pOrdered->RemoveAll();
		pUnordered->RemoveAll();
		AssertFalse(pOrderer->Order(pOrdered, pUnordered));
		AssertSize(2, pOrdered->NumElements());
		AssertOrdered(pFree, pOrdered, 0);
		AssertOrdered(pAfterX, pOrdered, 1);
		AssertSize(3, pUnordered->NumElements());
		AssertOrdered(pX, pUnordered, 0);
		AssertOrdered(pY, pUnordered, 1);
		AssertOrdered(pZ, pUnordered, 2);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's CircuitInstanceOrderer uses it: each instance fulfils its
// id and requires its parent's id so parents come before their children.
//
//////////////////////////////////////////////////////////////////////////
void TestOrdererParentsFirst(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestInteger>			apInstances[7];
		int							aiParents[7] = { 5, 3, 3, -1, 0, 3, 4 };
		Ptr<COrderer>				pOrderer;
		Ptr<CArray<CBaseObject>>	pOrdered;
		Ptr<CTestInteger>			pInstance;
		CChars						szId;
		size						i;
		size						j;
		size						uiParent;

		pRoot = ORoot();
		pOrderer = OMalloc<COrderer>();
		pRoot->Add(pOrderer);
		for (i = 0; i < 7; i++)
		{
			apInstances[i] = OMalloc<CTestInteger>((int32)i, 0, 0);
			pRoot->Add(apInstances[i]);
		}
		for (i = 0; i < 7; i++)
		{
			szId.Init();
			szId.Append((int)i);
			pOrderer->AddFulfilment(apInstances[i], szId.Text());
			szId.Kill();
			if (aiParents[i] != -1)
			{
				szId.Init();
				szId.Append(aiParents[i]);
				pOrderer->AddRequirement(apInstances[i], szId.Text());
				szId.Kill();
			}
		}

		//Pass 1: 3 and 5 (1 and 2 were added before 3).  Pass 2: 0, 1, 2, then 4 and 6 as 0 and 4 are ordered.
		pOrdered = OMalloc<CArray<CBaseObject>>();
		pRoot->Add(pOrdered);
		AssertTrue(pOrderer->Order(pOrdered));
		AssertSize(7, pOrdered->NumElements());
		AssertOrdered(apInstances[3], pOrdered, 0);
		AssertOrdered(apInstances[5], pOrdered, 1);
		AssertOrdered(apInstances[0], pOrdered, 2);
		AssertOrdered(apInstances[1], pOrdered, 3);
		AssertOrdered(apInstances[2], pOrdered, 4);
		AssertOrdered(apInstances[4], pOrdered, 5);
		AssertOrdered(apInstances[6], pOrdered, 6);

		//Every child is after its parent.
		for (i = 0; i < 7; i++)
		{
			pInstance = pOrdered->Get(i);
			if (aiParents[pInstance->mx] != -1)
			{
				uiParent = ARRAY_ELEMENT_NOT_FOUND;
				for (j = 0; j < 7; j++)
				{
					if (apInstances[aiParents[pInstance->mx]] == pOrdered->Get(j))
					{
						uiParent = j;
					}
				}
				AssertTrue(uiParent < i);
			}
		}
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Subcircuit types for the subcircuit tests: each fulfils its own name and
// requires the names of the subcircuits it has instances of.
//
//////////////////////////////////////////////////////////////////////////
const char*		gaszOrdererSubcircuits[] = { "Main", "ALU", "Adder", "Registers", "FlipFlop" };
const char*		gaszOrdererSubcircuitInstances[][3] = {	{ "ALU", "Registers", NULL },
														{ "Adder", NULL, NULL },
														{ NULL, NULL, NULL },
														{ "FlipFlop", "FlipFlop", NULL },
														{ NULL, NULL, NULL } };
#define NUM_ORDERER_SUBCIRCUITS	5


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<COrderer> NewSubcircuitOrderer(Ptr<CTestInteger>* apSubcircuits)
{
	Ptr<COrderer>	pOrderer;
	size			i;
	size			j;

	pOrderer = OMalloc<COrderer>();
	for (i = 0; i < NUM_ORDERER_SUBCIRCUITS; i++)
	{
		pOrderer->AddFulfilment(apSubcircuits[i], gaszOrdererSubcircuits[i]);
		for (j = 0; gaszOrdererSubcircuitInstances[i][j] != NULL; j++)
		{
			pOrderer->AddRequirement(apSubcircuits[i], gaszOrdererSubcircuitInstances[i][j]);
		}
	}
	return pOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's CircuitEditor saves subcircuits: each subcircuit comes after
// the subcircuits it has instances of, so they can be loaded in order.
//
//////////////////////////////////////////////////////////////////////////
void TestOrdererSubcircuitSave(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestInteger>			apSubcircuits[NUM_ORDERER_SUBCIRCUITS];
		Ptr<COrderer>				pOrderer;
		Ptr<CArray<CBaseObject>>	pOrdered;
		size						i;

		pRoot = ORoot();
		for (i = 0; i < NUM_ORDERER_SUBCIRCUITS; i++)
		{
			apSubcircuits[i] = OMalloc<CTestInteger>((int32)i, 0, 0);
			pRoot->Add(apSubcircuits[i]);
		}
		pOrderer = NewSubcircuitOrderer(apSubcircuits);
		pOrdered = OMalloc<CArray<CBaseObject>>();
		pRoot->Add(pOrderer, pOrdered);

		//Pass 1: Adder, FlipFlop.  Pass 2: ALU, Registers.  Pass 3: Main.
		AssertTrue(pOrderer->Order(pOrdered));
		AssertSize(5, pOrdered->NumElements());
		AssertOrdered(apSubcircuits[2], pOrdered, 0);
		AssertOrdered(apSubcircuits[4], pOrdered, 1);
		AssertOrdered(apSubcircuits[1], pOrdered, 2);
		AssertOrdered(apSubcircuits[3], pOrdered, 3);
		AssertOrdered(apSubcircuits[0], pOrdered, 4);

		//An instance of Main in FlipFlop is recursive and the subcircuits can't be saved.  Only Adder and ALU don't
		//use FlipFlop.
		pOrderer->AddRequirement(apSubcircuits[4], "Main");
		pOrdered->RemoveAll();
		AssertFalse(pOrderer->Order(pOrdered));
		AssertSize(2, pOrdered->NumElements());
		AssertOrdered(apSubcircuits[2], pOrdered, 0);
		AssertOrdered(apSubcircuits[1], pOrdered, 1);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim finds the subcircuits that can be instanced in the one being
// edited: a subcircuit is allowed if requiring it from the edited one can
// still be ordered, so a subcircuit never contains itself however deeply.
// CanRequire answers without changing pOrderer; the answer is checked
// against a new orderer with the requirement added, as Logicim does it.
//
//////////////////////////////////////////////////////////////////////////
void AssertAllowedSubcircuitInstances(const char* szExpected, Ptr<CRoot> pRoot, Ptr<COrderer> pOrderer, Ptr<CTestInteger>* apSubcircuits, size uiCurrent)
{
	Ptr<COrderer>				pRebuilt;
	Ptr<CArray<CBaseObject>>	pOrdered;
	size						i;
	CChars						szAllowed;
	bool						bAllowed;

	szAllowed.Init();
	for (i = 0; i < NUM_ORDERER_SUBCIRCUITS; i++)
	{
		if (i != uiCurrent)
		{
			bAllowed = pOrderer->CanRequire(apSubcircuits[uiCurrent], gaszOrdererSubcircuits[i]);
			if (bAllowed)
			{
				szAllowed.Append(gaszOrdererSubcircuits[i]);
				szAllowed.Append(" ");
			}

			pRebuilt = NewSubcircuitOrderer(apSubcircuits);
			pOrdered = OMalloc<CArray<CBaseObject>>();
			pRoot->Add(pRebuilt, pOrdered);
			pRebuilt->AddRequirement(apSubcircuits[uiCurrent], gaszOrdererSubcircuits[i]);
			AssertBool(pRebuilt->Order(pOrdered), bAllowed);
			pRoot->Remove(pOrdered);
			pRoot->Remove(pRebuilt);
		}
	}
	AssertString(szExpected, szAllowed.Text());
	szAllowed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOrdererSubcircuitInstances(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestInteger>			apSubcircuits[NUM_ORDERER_SUBCIRCUITS];
		Ptr<COrderer>				pOrderer;
		Ptr<CArray<CBaseObject>>	pOrdered;
		size						i;

		pRoot = ORoot();
		for (i = 0; i < NUM_ORDERER_SUBCIRCUITS; i++)
		{
			apSubcircuits[i] = OMalloc<CTestInteger>((int32)i, 0, 0);
			pRoot->Add(apSubcircuits[i]);
		}
		pOrderer = NewSubcircuitOrderer(apSubcircuits);
		pRoot->Add(pOrderer);

		AssertAllowedSubcircuitInstances("ALU Adder Registers FlipFlop ", pRoot, pOrderer, apSubcircuits, 0);
		AssertAllowedSubcircuitInstances("Adder Registers FlipFlop ", pRoot, pOrderer, apSubcircuits, 1);
		AssertAllowedSubcircuitInstances("Registers FlipFlop ", pRoot, pOrderer, apSubcircuits, 2);
		AssertAllowedSubcircuitInstances("ALU Adder FlipFlop ", pRoot, pOrderer, apSubcircuits, 3);
		AssertAllowedSubcircuitInstances("ALU Adder ", pRoot, pOrderer, apSubcircuits, 4);

		//CanRequire didn't add any requirements.
		AssertSize(5, pOrderer->NumObjects());
		AssertSize(1, pOrderer->GetHolder(apSubcircuits[1])->GetRequirements()->NumElements());
		AssertSize(0, pOrderer->GetHolder(apSubcircuits[4])->GetRequirements()->NumElements());
		pOrdered = OMalloc<CArray<CBaseObject>>();
		pRoot->Add(pOrdered);
		AssertTrue(pOrderer->Order(pOrdered));
		AssertSize(5, pOrdered->NumElements());
		AssertOrdered(apSubcircuits[2], pOrdered, 0);
		AssertOrdered(apSubcircuits[0], pOrdered, 4);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A requirement it already has, its own name, a name nothing fulfils and a
// name fulfilled later are checked as Order would.  An object not in the
// orderer can require any fulfilled name.  Nothing can be required once
// the objects can't be ordered.
//
//////////////////////////////////////////////////////////////////////////
void TestOrdererCanRequire(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestInteger>			pA;
		Ptr<CTestInteger>			pB;
		Ptr<CTestInteger>			pC;
		Ptr<CTestInteger>			pOutside;
		Ptr<COrderer>				pOrderer;
		Ptr<CArray<CBaseObject>>	pOrdered;

		pRoot = ORoot();
		pA = OMalloc<CTestInteger>(1, 0, 0);
		pB = OMalloc<CTestInteger>(2, 0, 0);
		pC = OMalloc<CTestInteger>(3, 0, 0);
		pOutside = OMalloc<CTestInteger>(4, 0, 0);
		pRoot->Add(pA, pB, pC);
		pRoot->Add(pOutside);

		pOrderer = OMalloc<COrderer>();
		pRoot->Add(pOrderer);
		AssertFalse(pOrderer->CanRequire(pA, "A"));
		AssertFalse(pOrderer->CanRequire(pOutside, "A"));

		pOrderer->AddFulfilment(pA, "A");
		pOrderer->AddFulfilment(pB, "B");
		pOrderer->AddRequirement(pB, "A");
		pOrderer->AddFulfilment(pC, "C");

		AssertTrue(pOrderer->CanRequire(pB, "A"));
		AssertTrue(pOrderer->CanRequire(pA, "C"));
		AssertTrue(pOrderer->CanRequire(pC, "B"));
		AssertFalse(pOrderer->CanRequire(pA, "B"));
		AssertFalse(pOrderer->CanRequire(pA, "A"));
		AssertFalse(pOrderer->CanRequire(pC, "Missing"));
		AssertFalse(pOrderer->CanRequire(pC, "c"));

		AssertTrue(pOrderer->CanRequire(pOutside, "B"));
		AssertFalse(pOrderer->CanRequire(pOutside, "Missing"));
		AssertTrue(pOrderer->GetHolder(pOutside).IsNull());
		AssertSize(3, pOrderer->NumObjects());

		//C requires B so neither A nor B can require C.
		pOrderer->AddRequirement(pC, "B");
		AssertFalse(pOrderer->CanRequire(pA, "C"));
		AssertFalse(pOrderer->CanRequire(pB, "C"));
		AssertTrue(pOrderer->CanRequire(pC, "A"));

		//Already unorderable.
		pOrderer->AddRequirement(pA, "C");
		pOrdered = OMalloc<CArray<CBaseObject>>();
		pRoot->Add(pOrdered);
		AssertFalse(pOrderer->Order(pOrdered));
		AssertFalse(pOrderer->CanRequire(pC, "A"));
		AssertFalse(pOrderer->CanRequire(pOutside, "A"));
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The orderer is saved with its objects and their names, and still finds
// its objects' holders when loaded.
//
//////////////////////////////////////////////////////////////////////////
void TestOrdererSaveAndLoad(void)
{
	CFileUtil					cFileUtil;
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	char						szDirectory[] = "Output" _FS_ "Orderer";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	gcObjects.AddConstructor<CTestInteger>();
	gcObjects.AddConstructor<COrderer>();
	gcObjects.AddConstructor<COrdererHolder>();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestInteger>			pA;
		Ptr<CTestInteger>			pB;
		Ptr<CTestInteger>			pC;
		Ptr<COrderer>				pOrderer;

		pRoot = ORoot();
		pA = ONMalloc<CTestInteger>("A", 1, 0, 0);
		pB = ONMalloc<CTestInteger>("B", 2, 0, 0);
		pC = ONMalloc<CTestInteger>("C", 3, 0, 0);
		pOrderer = ONMalloc<COrderer>("Orderer");
		pRoot->Add(pOrderer);
		pOrderer->AddFulfilment(pC, "C");
		pOrderer->AddRequirement(pC, "B");
		pOrderer->AddRequirement(pC, "A");
		pOrderer->AddFulfilment(pB, "B");
		pOrderer->AddRequirement(pB, "A");
		pOrderer->AddFulfilment(pA, "A");
	}
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	gcObjects.AddConstructor<CTestInteger>();
	gcObjects.AddConstructor<COrderer>();
	gcObjects.AddConstructor<COrdererHolder>();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestInteger>			pA;
		Ptr<CTestInteger>			pB;
		Ptr<CTestInteger>			pC;
		Ptr<CTestInteger>			pD;
		Ptr<COrderer>				pOrderer;
		Ptr<COrdererHolder>			pHolder;
		Ptr<CArray<CBaseObject>>	pOrdered;

		pRoot = ORoot();
		pOrderer = gcObjects.Get("Orderer");
		AssertTrue(pOrderer.IsNotNull());
		AssertSize(3, pOrderer->NumObjects());

		pA = gcObjects.Get("A");
		pB = gcObjects.Get("B");
		pC = gcObjects.Get("C");
		AssertInt((int32)3, pC->mx);
		pHolder = pOrderer->GetHolder(pC);
		AssertTrue(pHolder.IsNotNull());
		AssertSize(2, pHolder->GetRequirements()->NumElements());
		AssertString("B", pHolder->GetRequirements()->GetText(0));
		AssertString("A", pHolder->GetRequirements()->GetText(1));
		AssertSize(1, pHolder->GetFulfilments()->NumElements());
		AssertString("C", pHolder->GetFulfilments()->GetText(0));

		//An object added after loading goes last when it requires nothing.
		pD = OMalloc<CTestInteger>(4, 0, 0);
		pRoot->Add(pD);
		pOrderer->AddRequirement(pD, "C");
		pOrderer->AddFulfilment(pA, "First");
		AssertSize(4, pOrderer->NumObjects());

		pOrdered = OMalloc<CArray<CBaseObject>>();
		pRoot->Add(pOrdered);
		AssertTrue(pOrderer->Order(pOrdered));
		AssertSize(4, pOrdered->NumElements());
		AssertOrdered(pA, pOrdered, 0);
		AssertOrdered(pB, pOrdered, 1);
		AssertOrdered(pC, pOrdered, 2);
		AssertOrdered(pD, pOrdered, 3);
		AssertSize(2, pOrderer->GetHolder(pA)->GetFulfilments()->NumElements());
	}
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOrderer(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestOrdererEmpty();
	TestOrdererChain();
	TestOrdererPasses();
	TestOrdererUnsatisfiable();
	TestOrdererParentsFirst();
	TestOrdererSubcircuitSave();
	TestOrdererSubcircuitInstances();
	TestOrdererCanRequire();
	TestOrdererSaveAndLoad();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

