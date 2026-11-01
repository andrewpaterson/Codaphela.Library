#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/PointerContainer.h"
#include "TestLib/Assert.h"
#include "ObjectTestSetup.h"
#include "EmbeddedObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeEmbeddedFlags(void)
{
	ObjectsInit();
	gcObjects.AddConstructor<CEmbeddedComplex>();
	gcObjects.AddConstructor<CEmbeddedTest>();
	
	Ptr<CEmbeddedComplex>	pComplex;

	AssertLong(0, gcObjects.NumMemoryIndexes());

	pComplex = OMalloc<CEmbeddedComplex>();
	AssertLong(1, gcObjects.NumMemoryIndexes());

	pComplex->Add(OMalloc<CEmbeddedTest>());
	AssertLong(2, gcObjects.NumMemoryIndexes());

	pComplex = NULL;
	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeEmbedded(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestFreeEmbeddedFlags();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

