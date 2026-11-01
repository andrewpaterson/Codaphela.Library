#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "StandardLib/PointerContainer.h"
#include "StandardLib/HollowObject.h"
#include "StandardLib/ObjectHeader.h"
#include "TestLib/Assert.h"


class CTestObjectWithTwoEmbedded : public CObject
{
CONSTRUCTABLE(CTestObjectWithTwoEmbedded)
DESTRUCTABLE(CTestObjectWithTwoEmbedded)
public:
	CString		mString1;
	CString		mString2;

	void Class(void)
	{
		Embedded(&mString1, "mString1");
		Embedded(&mString2, "mString2");
	}

	void Init(void) 
	{
		mString1.Init("String1");
		mString2.Init("String2");
	}

	void Free(void)
	{
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectRemapTosActual(void)
{

	ObjectsInit();
	{
		Ptr<CRoot>	pRoot = ORoot();

		Ptr<CHollowObject>	pHollow = gcObjects.AllocateInternalHollowWithNameAndIndex("Name", gcObjects.GetIndexGenerator()->GetNext(), 3, OBJECT_IDENTIFIER_SIZE_NOT_SET);
		AssertSize(3, pHollow.Object()->NumEmbedded());

		Ptr<CEmbeddedObject> pHollowEmbedded1 = pHollow.Object()->GetEmbeddedObject(1);
		Ptr<CEmbeddedObject> pHollowEmbedded2 = pHollow.Object()->GetEmbeddedObject(2);
		AssertPointer(pHollow.Object(), pHollow.Object()->GetEmbeddedObject(0));
		AssertSize(3, pHollow.Object()->NumEmbedded());

		Ptr<CPointerContainer>	pPointerHollow = OMalloc<CPointerContainer>(pHollow);
		Ptr<CPointerContainer>	pPointerHollowEmbedded1 = OMalloc<CPointerContainer>(pHollowEmbedded1);
		Ptr<CPointerContainer>	pPointerHollowEmbedded2 = OMalloc<CPointerContainer>(pHollowEmbedded2);

		AssertSize(3, pHollow.Object()->NumHeapFroms());
		AssertSize(1, pHollow.Object()->CEmbeddedObject::NumHeapFroms());
		AssertSize(1, pHollowEmbedded1.Object()->CEmbeddedObject::NumHeapFroms());
		AssertSize(1, pHollowEmbedded2.Object()->CEmbeddedObject::NumHeapFroms());

		AssertSize(3, pHollow.Object()->NumStackFroms());
		AssertSize(1, pHollow.Object()->CEmbeddedObject::NumStackFroms());
		AssertSize(1, pHollowEmbedded1.Object()->CEmbeddedObject::NumStackFroms());
		AssertSize(1, pHollowEmbedded2.Object()->CEmbeddedObject::NumStackFroms());

		pRoot->Add(pHollow);

		Ptr<CTestObjectWithTwoEmbedded> pObject = gcObjects.Malloc<CTestObjectWithTwoEmbedded>("Name2");
		AssertSize(3, pObject.Object()->NumEmbedded());

		AssertString("CHollowObject", pHollow.ClassName());
		AssertString("CHollowEmbeddedObject", pHollowEmbedded1.ClassName());
		AssertString("CHollowEmbeddedObject", pHollowEmbedded2.ClassName());

		AssertString("CHollowObject", pPointerHollow->mp.ClassName());
		AssertString("CHollowEmbeddedObject", pPointerHollowEmbedded1->mp.ClassName());
		AssertString("CHollowEmbeddedObject", pPointerHollowEmbedded2->mp.ClassName());

		Ptr<CPointerContainer>	pPointerObject = OMalloc<CPointerContainer>(pObject);

		pHollow.MorphInto(&pObject);

		AssertString("CTestObjectWithTwoEmbedded", pHollow.ClassName());
		AssertString("CString", pHollowEmbedded1.ClassName());
		AssertString("CString", pHollowEmbedded2.ClassName());

		AssertString("CTestObjectWithTwoEmbedded", pPointerHollow->mp.ClassName());
		AssertString("CString", pPointerHollowEmbedded1->mp.ClassName());
		AssertString("CString", pPointerHollowEmbedded2->mp.ClassName());

		AssertPointer(pObject.Object(), pHollow.Object());
		AssertPointer(pObject.Object(), pPointerHollow->mp.Object());
		AssertPointer(pObject.Object(), pPointerObject->mp.Object());
		AssertPointer(pHollowEmbedded1.Object(), pPointerHollowEmbedded1->mp.Object());
		AssertPointer(pHollowEmbedded2.Object(), pPointerHollowEmbedded2->mp.Object());

		AssertSize(5, pHollow.Object()->NumHeapFroms());  //pPointerObject and pPointerHollow now both point to the same object.  pRoot.Set is the fifth.
		AssertSize(3, pHollow.Object()->CEmbeddedObject::NumHeapFroms());
		AssertSize(1, pHollowEmbedded1.Object()->CEmbeddedObject::NumHeapFroms());
		AssertSize(1, pHollowEmbedded2.Object()->CEmbeddedObject::NumHeapFroms());

		AssertSize(4, pHollow.Object()->NumStackFroms());  //pObject and pHollow now both point to the same object.
		AssertSize(2, pHollow.Object()->CEmbeddedObject::NumStackFroms());
		AssertSize(1, pHollowEmbedded1.Object()->CEmbeddedObject::NumStackFroms());
		AssertSize(1, pHollowEmbedded2.Object()->CEmbeddedObject::NumStackFroms());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectRemapTos(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestEmbeddedObjectRemapTosActual();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

