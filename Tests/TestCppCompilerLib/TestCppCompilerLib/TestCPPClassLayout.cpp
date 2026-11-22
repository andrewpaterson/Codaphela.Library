#include "CppCompilerLib/CPP65Type.h"
#include "CppCompilerLib/CPPClass.h"
#include "CppCompilerLib/CPPClassLayout.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A class with no name, whose type is its own.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPClassLayoutClass(CCPPClass* pcClass, CCPP65Types* pcTypes, bool bUnion)
{
	pcClass->Init(NULL, -1);
	pcClass->mpsType = pcTypes->Class(pcClass);
	pcClass->mbUnion = bUnion;
}


//////////////////////////////////////////////////////////////////////////
//
// A field of psType (a bit-field iBitWidth wide, -1 for a field that isn't
// one) with an alignas of iAlign, 0 for none.
//
//////////////////////////////////////////////////////////////////////////
static SCPPField* TestCPPClassLayoutField(CCPPClass* pcClass, SCPP65Type* psType, int iBitWidth = -1, int iAlign = 0)
{
	SCPPField*	psField;

	psField = pcClass->macFields.Add();
	psField->pcSymbol = NULL;
	psField->lliName = -1;
	psField->psType = psType;
	psField->iOffset = 0;
	psField->iAlign = iAlign;
	psField->pcInitialiser = NULL;
	psField->bMutable = false;
	psField->iBitWidth = iBitWidth;
	psField->iBitOffset = 0;
	return psField;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays the class out and makes it complete: its size is returned.
//
//////////////////////////////////////////////////////////////////////////
static int TestCPPClassLayoutPlace(CCPPClass* pcClass, CCPP65Types* pcTypes, int iPack = 0, int iClassAlign = 0)
{
	CCPPClassLayout	cLayout;
	int				iBytes;

	cLayout.Init(pcClass, iPack, 4);
	cLayout.Layout(iClassAlign);
	iBytes = cLayout.GetBytes();
	cLayout.Kill();
	pcTypes->SetClassBytes(pcClass->mpsType, iBytes);
	pcClass->meState = CPPCS_Complete;
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// Fields go in order, each aligned as its type is, and the class is as big
// as a whole number of its strictest member.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPClassLayoutFields(void)
{
	CCPP65Types	cTypes;
	CCPPClass	cClass;
	CCPPClass	cEmpty;
	CCPPClass	cOuter;

	cTypes.Init(2, 4);

	TestCPPClassLayoutClass(&cClass, &cTypes, false);
	TestCPPClassLayoutField(&cClass, &cTypes.msChar);
	TestCPPClassLayoutField(&cClass, &cTypes.msLong);
	TestCPPClassLayoutField(&cClass, &cTypes.msShort);
	TestCPPClassLayoutField(&cClass, cTypes.Pointer(&cTypes.msChar));
	TestCPPClassLayoutField(&cClass, cTypes.Array(&cTypes.msShort, 3));
	TestCPPClassLayoutField(&cClass, &cTypes.msDouble);
	AssertInt(32, TestCPPClassLayoutPlace(&cClass, &cTypes));
	AssertInt(0, cClass.macFields.Get(0)->iOffset);
	AssertInt(4, cClass.macFields.Get(1)->iOffset);
	AssertInt(8, cClass.macFields.Get(2)->iOffset);
	AssertInt(12, cClass.macFields.Get(3)->iOffset);
	AssertInt(16, cClass.macFields.Get(4)->iOffset);
	AssertInt(24, cClass.macFields.Get(5)->iOffset);
	AssertInt(8, cClass.miAlign);
	AssertFalse(cClass.mbEmpty);
	AssertInt(32, cClass.mpsType->iBytes);

	//A class with nothing in it is a byte, and is empty.
	TestCPPClassLayoutClass(&cEmpty, &cTypes, false);
	AssertInt(1, TestCPPClassLayoutPlace(&cEmpty, &cTypes));
	AssertInt(1, cEmpty.miAlign);
	AssertTrue(cEmpty.mbEmpty);

	//A class in a class is aligned as its strictest member.
	TestCPPClassLayoutClass(&cOuter, &cTypes, false);
	TestCPPClassLayoutField(&cOuter, &cTypes.msChar);
	TestCPPClassLayoutField(&cOuter, cClass.mpsType);
	TestCPPClassLayoutField(&cOuter, &cTypes.msChar);
	AssertInt(48, TestCPPClassLayoutPlace(&cOuter, &cTypes));
	AssertInt(8, cOuter.macFields.Get(1)->iOffset);
	AssertInt(40, cOuter.macFields.Get(2)->iOffset);

	cOuter.Kill();
	cEmpty.Kill();
	cClass.Kill();
	cTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// #pragma pack limits each member's alignment; alignas on a field, on the
// class or on a member's or a base's class is kept whatever the pack is.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPClassLayoutPackAndAlignas(void)
{
	CCPP65Types	cTypes;
	CCPPClass	cPacked1;
	CCPPClass	cPacked2;
	CCPPClass	cAligned;
	CCPPClass	cClassAligned;
	CCPPClass	cHolder;
	CCPPClass	cDerived;
	SCPPBase*	psBase;

	cTypes.Init(2, 4);

	TestCPPClassLayoutClass(&cPacked1, &cTypes, false);
	TestCPPClassLayoutField(&cPacked1, &cTypes.msChar);
	TestCPPClassLayoutField(&cPacked1, &cTypes.msLong);
	TestCPPClassLayoutField(&cPacked1, &cTypes.msDouble);
	AssertInt(13, TestCPPClassLayoutPlace(&cPacked1, &cTypes, 1));
	AssertInt(1, cPacked1.macFields.Get(1)->iOffset);
	AssertInt(5, cPacked1.macFields.Get(2)->iOffset);
	AssertInt(1, cPacked1.miAlign);

	TestCPPClassLayoutClass(&cPacked2, &cTypes, false);
	TestCPPClassLayoutField(&cPacked2, &cTypes.msChar);
	TestCPPClassLayoutField(&cPacked2, &cTypes.msLong);
	TestCPPClassLayoutField(&cPacked2, &cTypes.msChar);
	AssertInt(8, TestCPPClassLayoutPlace(&cPacked2, &cTypes, 2));
	AssertInt(2, cPacked2.macFields.Get(1)->iOffset);
	AssertInt(6, cPacked2.macFields.Get(2)->iOffset);
	AssertInt(2, cPacked2.miAlign);

	TestCPPClassLayoutClass(&cAligned, &cTypes, false);
	TestCPPClassLayoutField(&cAligned, &cTypes.msChar);
	TestCPPClassLayoutField(&cAligned, &cTypes.msChar, -1, 8);
	AssertInt(16, TestCPPClassLayoutPlace(&cAligned, &cTypes, 1));
	AssertInt(8, cAligned.macFields.Get(1)->iOffset);
	AssertInt(8, cAligned.miAlign);

	TestCPPClassLayoutClass(&cClassAligned, &cTypes, false);
	TestCPPClassLayoutField(&cClassAligned, &cTypes.msChar);
	AssertInt(16, TestCPPClassLayoutPlace(&cClassAligned, &cTypes, 0, 16));
	AssertInt(16, cClassAligned.miAlign);
	AssertInt(16, cClassAligned.miAlignas);

	//A class's alignas is kept where the class is a member, an element or a base in a packed class.
	TestCPPClassLayoutClass(&cHolder, &cTypes, false);
	TestCPPClassLayoutField(&cHolder, &cTypes.msChar);
	TestCPPClassLayoutField(&cHolder, cClassAligned.mpsType);
	TestCPPClassLayoutField(&cHolder, &cTypes.msLong);
	TestCPPClassLayoutField(&cHolder, cTypes.Array(cClassAligned.mpsType, 2));
	AssertInt(80, TestCPPClassLayoutPlace(&cHolder, &cTypes, 1));
	AssertInt(16, cHolder.macFields.Get(1)->iOffset);
	AssertInt(32, cHolder.macFields.Get(2)->iOffset);
	AssertInt(48, cHolder.macFields.Get(3)->iOffset);
	AssertInt(16, cHolder.miAlign);
	AssertInt(16, cHolder.miAlignas);

	TestCPPClassLayoutClass(&cDerived, &cTypes, false);
	psBase = cDerived.macBases.Add();
	psBase->pcClass = &cPacked2;
	psBase->bVirtual = false;
	psBase = cDerived.macBases.Add();
	psBase->pcClass = &cClassAligned;
	psBase->bVirtual = false;
	cDerived.mpcBase = &cPacked2;
	TestCPPClassLayoutField(&cDerived, &cTypes.msLong);
	AssertInt(48, TestCPPClassLayoutPlace(&cDerived, &cTypes, 1));
	AssertInt(0, cDerived.macBases.Get(0)->iOffset);
	AssertInt(16, cDerived.macBases.Get(1)->iOffset);
	AssertInt(32, cDerived.macFields.Get(0)->iOffset);
	AssertInt(16, cDerived.miAlign);

	cDerived.Kill();
	cHolder.Kill();
	cClassAligned.Kill();
	cAligned.Kill();
	cPacked2.Kill();
	cPacked1.Kill();
	cTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A union's members are all at 0 and it is as big as its largest.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPClassLayoutUnion(void)
{
	CCPP65Types	cTypes;
	CCPPClass	cUnion;
	CCPPClass	cBits;

	cTypes.Init(2, 4);

	TestCPPClassLayoutClass(&cUnion, &cTypes, true);
	TestCPPClassLayoutField(&cUnion, &cTypes.msChar);
	TestCPPClassLayoutField(&cUnion, cTypes.Array(&cTypes.msChar, 5));
	TestCPPClassLayoutField(&cUnion, &cTypes.msLong);
	AssertInt(8, TestCPPClassLayoutPlace(&cUnion, &cTypes));
	AssertInt(0, cUnion.macFields.Get(0)->iOffset);
	AssertInt(0, cUnion.macFields.Get(1)->iOffset);
	AssertInt(0, cUnion.macFields.Get(2)->iOffset);
	AssertInt(4, cUnion.miAlign);

	//Each bit-field of a union is at bit 0.
	TestCPPClassLayoutClass(&cBits, &cTypes, true);
	TestCPPClassLayoutField(&cBits, &cTypes.msUnsignedChar, 3);
	TestCPPClassLayoutField(&cBits, &cTypes.msUnsignedShort, 9);
	AssertInt(2, TestCPPClassLayoutPlace(&cBits, &cTypes));
	AssertInt(0, cBits.macFields.Get(1)->iOffset);
	AssertInt(0, cBits.macFields.Get(1)->iBitOffset);

	cBits.Kill();
	cUnion.Kill();
	cTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Bit-fields fill a unit of their type from its lowest bit; one that
// doesn't fit, one of a type of another size, a field that isn't one and a
// width of 0 each end the unit.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPClassLayoutBitFields(void)
{
	CCPP65Types	cTypes;
	CCPPClass	cClass;

	cTypes.Init(2, 4);

	TestCPPClassLayoutClass(&cClass, &cTypes, false);
	TestCPPClassLayoutField(&cClass, &cTypes.msUnsignedChar, 3);	//0: unit at 0, bits 0 to 2.
	TestCPPClassLayoutField(&cClass, &cTypes.msUnsignedChar, 4);	//1: bits 3 to 6.
	TestCPPClassLayoutField(&cClass, &cTypes.msUnsignedChar, 2);	//2: doesn't fit: unit at 1.
	TestCPPClassLayoutField(&cClass, &cTypes.msUnsignedShort, 5);	//3: another size: unit at 2.
	TestCPPClassLayoutField(&cClass, &cTypes.msUnsignedShort, 0);	//4: closes the unit.
	TestCPPClassLayoutField(&cClass, &cTypes.msUnsignedShort, 5);	//5: unit at 4.
	TestCPPClassLayoutField(&cClass, &cTypes.msChar);				//6: a field at 6.
	TestCPPClassLayoutField(&cClass, &cTypes.msUnsignedShort, 1);	//7: unit at 8.
	TestCPPClassLayoutField(&cClass, &cTypes.msUnsignedShort, 15);	//8: bits 1 to 15.
	AssertInt(10, TestCPPClassLayoutPlace(&cClass, &cTypes));
	AssertInt(0, cClass.macFields.Get(0)->iOffset);
	AssertInt(0, cClass.macFields.Get(0)->iBitOffset);
	AssertInt(0, cClass.macFields.Get(1)->iOffset);
	AssertInt(3, cClass.macFields.Get(1)->iBitOffset);
	AssertInt(1, cClass.macFields.Get(2)->iOffset);
	AssertInt(0, cClass.macFields.Get(2)->iBitOffset);
	AssertInt(2, cClass.macFields.Get(3)->iOffset);
	AssertInt(0, cClass.macFields.Get(3)->iBitOffset);
	AssertInt(4, cClass.macFields.Get(5)->iOffset);
	AssertInt(0, cClass.macFields.Get(5)->iBitOffset);
	AssertInt(6, cClass.macFields.Get(6)->iOffset);
	AssertInt(8, cClass.macFields.Get(7)->iOffset);
	AssertInt(0, cClass.macFields.Get(7)->iBitOffset);
	AssertInt(8, cClass.macFields.Get(8)->iOffset);
	AssertInt(1, cClass.macFields.Get(8)->iBitOffset);
	AssertInt(2, cClass.miAlign);

	cClass.Kill();
	cTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Bases go first, in order; an empty base takes no space.  A class with
// virtual functions gets a vtable pointer after its bases, or shares its
// first base's; a second polymorphic base keeps its own, in a secondary
// vtable.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPClassLayoutBasesAndVtables(void)
{
	CCPP65Types		cTypes;
	CCPPClass		cEmpty;
	CCPPClass		cBase;
	CCPPClass		cDerived;
	CCPPClass		cPolymorphic;
	CCPPClass		cOther;
	CCPPClass		cSharing;
	CCPPClass		cBoth;
	CCPPFunction	cVirtual;
	CCPPFunction	cOtherVirtual;
	SCPPBase*		psBase;

	cTypes.Init(2, 4);

	TestCPPClassLayoutClass(&cEmpty, &cTypes, false);
	TestCPPClassLayoutPlace(&cEmpty, &cTypes);

	TestCPPClassLayoutClass(&cBase, &cTypes, false);
	TestCPPClassLayoutField(&cBase, &cTypes.msLong);
	TestCPPClassLayoutField(&cBase, &cTypes.msChar);
	AssertInt(8, TestCPPClassLayoutPlace(&cBase, &cTypes));

	TestCPPClassLayoutClass(&cDerived, &cTypes, false);
	psBase = cDerived.macBases.Add();
	psBase->bVirtual = false;
	psBase->pcClass = &cEmpty;
	psBase->iOffset = -1;
	psBase = cDerived.macBases.Add();
	psBase->bVirtual = false;
	psBase->pcClass = &cBase;
	psBase->iOffset = -1;
	cDerived.mpcBase = &cEmpty;
	TestCPPClassLayoutField(&cDerived, &cTypes.msChar);
	AssertInt(12, TestCPPClassLayoutPlace(&cDerived, &cTypes));
	AssertInt(0, cDerived.macBases.Get(0)->iOffset);
	AssertInt(0, cDerived.macBases.Get(1)->iOffset);
	AssertInt(8, cDerived.macFields.Get(0)->iOffset);
	AssertFalse(cDerived.IsPolymorphic());

	//A vtable pointer of its own.
	TestCPPClassLayoutClass(&cPolymorphic, &cTypes, false);
	cVirtual.Init(&cPolymorphic, 1, CPPFK_Function, &cTypes.msVoid);
	cVirtual.mbVirtual = true;
	cPolymorphic.mcVtable.mapcFunctions.Add(&cVirtual);
	TestCPPClassLayoutField(&cPolymorphic, &cTypes.msChar);
	AssertInt(8, TestCPPClassLayoutPlace(&cPolymorphic, &cTypes));
	AssertInt(0, cPolymorphic.mcVtable.miPointerOffset);
	AssertInt(4, cPolymorphic.macFields.Get(0)->iOffset);
	AssertTrue(cPolymorphic.IsPolymorphic());
	AssertFalse(cPolymorphic.mbEmpty);

	TestCPPClassLayoutClass(&cOther, &cTypes, false);
	cOtherVirtual.Init(&cOther, 2, CPPFK_Function, &cTypes.msVoid);
	cOtherVirtual.mbVirtual = true;
	cOther.mcVtable.mapcFunctions.Add(&cOtherVirtual);
	AssertInt(4, TestCPPClassLayoutPlace(&cOther, &cTypes));

	//The first base's pointer is shared.
	TestCPPClassLayoutClass(&cSharing, &cTypes, false);
	psBase = cSharing.macBases.Add();
	psBase->bVirtual = false;
	psBase->pcClass = &cPolymorphic;
	psBase->iOffset = -1;
	cSharing.mpcBase = &cPolymorphic;
	cSharing.mcVtable.LayoutOwn(&cSharing);
	TestCPPClassLayoutField(&cSharing, &cTypes.msLong);
	AssertInt(12, TestCPPClassLayoutPlace(&cSharing, &cTypes));
	AssertInt(0, cSharing.mcVtable.miPointerOffset);
	AssertInt(1, (int)cSharing.mcVtable.mapcFunctions.NumElements());
	AssertInt(0, (int)cSharing.mcVtable.macSecondaries.NumElements());
	AssertInt(8, cSharing.macFields.Get(0)->iOffset);

	//The second polymorphic base's vtable is a secondary one, at the base's offset.
	TestCPPClassLayoutClass(&cBoth, &cTypes, false);
	psBase = cBoth.macBases.Add();
	psBase->bVirtual = false;
	psBase->pcClass = &cPolymorphic;
	psBase->iOffset = -1;
	psBase = cBoth.macBases.Add();
	psBase->bVirtual = false;
	psBase->pcClass = &cOther;
	psBase->iOffset = -1;
	cBoth.mpcBase = &cPolymorphic;
	cBoth.mcVtable.LayoutOwn(&cBoth);
	AssertInt(12, TestCPPClassLayoutPlace(&cBoth, &cTypes));
	AssertInt(8, cBoth.macBases.Get(1)->iOffset);
	AssertInt(0, cBoth.mcVtable.miPointerOffset);
	AssertInt(1, (int)cBoth.mcVtable.macSecondaries.NumElements());
	AssertInt(8, cBoth.mcVtable.macSecondaries.Get(0)->iOffset);
	AssertTrue(cBoth.mcVtable.macSecondaries.Get(0)->pcOwner == &cOther);
	AssertTrue(cBoth.mcVtable.macSecondaries.Get(0)->mapcFunctions.GetPtr(0) == &cOtherVirtual);
	AssertFalse(cBoth.IsAbstract());
	cOtherVirtual.mbPure = true;
	AssertTrue(cBoth.IsAbstract());

	cBoth.Kill();
	cSharing.Kill();
	cOtherVirtual.Kill();
	cOther.Kill();
	cVirtual.Kill();
	cPolymorphic.Kill();
	cDerived.Kill();
	cBase.Kill();
	cEmpty.Kill();
	cTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A base of a class written into it, not laid out (BasePaths reads the
// bases as they are written).
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPClassBase(CCPPClass* pcClass, CCPPClass* pcBase, int iOffset, bool bVirtual, ECPPAccess eAccess)
{
	SCPPBase*	psBase;

	psBase = pcClass->macBases.Add();
	psBase->pcClass = pcBase;
	psBase->iOffset = iOffset;
	psBase->bVirtual = bVirtual;
	psBase->eAccess = eAccess;
}


//////////////////////////////////////////////////////////////////////////
//
// A class's base paths: itself first, then each base followed by its own,
// once for each way to it, each with where it is (below the last virtual
// base passed), that base, and whether every step was public.
// struct D : B, private virtual V { };  struct B : A { };  struct V : A { };
//
//////////////////////////////////////////////////////////////////////////
static void TestCPPClassBasePaths(void)
{
	CCPP65Types			cTypes;
	CCPPClass			cA;
	CCPPClass			cB;
	CCPPClass			cV;
	CCPPClass			cD;
	CArrayCPPBasePaths*	pasPaths;

	cTypes.Init(2, 4);
	TestCPPClassLayoutClass(&cA, &cTypes, false);
	TestCPPClassLayoutClass(&cB, &cTypes, false);
	TestCPPClassLayoutClass(&cV, &cTypes, false);
	TestCPPClassLayoutClass(&cD, &cTypes, false);
	TestCPPClassBase(&cB, &cA, 2, false, CPPA_Public);
	TestCPPClassBase(&cV, &cA, 0, false, CPPA_Public);
	TestCPPClassBase(&cD, &cB, 4, false, CPPA_Public);
	TestCPPClassBase(&cD, &cV, 0, true, CPPA_Private);

	pasPaths = cD.BasePaths();
	AssertSize(5, pasPaths->NumElements());
	AssertPointer(&cD, pasPaths->Get(0)->pcClass);
	AssertInt(0, pasPaths->Get(0)->iOffset);
	AssertTrue(pasPaths->Get(0)->bPublic);
	AssertPointer(&cB, pasPaths->Get(1)->pcClass);
	AssertInt(4, pasPaths->Get(1)->iOffset);
	AssertNull(pasPaths->Get(1)->pcVirtual);
	AssertPointer(&cA, pasPaths->Get(2)->pcClass);
	AssertInt(6, pasPaths->Get(2)->iOffset);
	AssertTrue(pasPaths->Get(2)->bPublic);
	AssertPointer(&cV, pasPaths->Get(3)->pcClass);
	AssertPointer(&cV, pasPaths->Get(3)->pcVirtual);
	AssertTrue(pasPaths->Get(3)->bVirtualEdge);
	AssertFalse(pasPaths->Get(3)->bPublic);
	AssertPointer(&cA, pasPaths->Get(4)->pcClass);
	AssertPointer(&cV, pasPaths->Get(4)->pcVirtual);
	AssertFalse(pasPaths->Get(4)->bVirtualEdge);
	AssertFalse(pasPaths->Get(4)->bPublic);

	AssertTrue(cD.HasBase(&cA));
	AssertTrue(cD.HasBase(&cV));
	AssertFalse(cB.HasBase(&cV));
	AssertFalse(cD.HasBase(&cD));

	cD.Kill();
	cV.Kill();
	cB.Kill();
	cA.Kill();
	cTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What an integer type holds, an array's element and a reference's type,
// which the unit asks of the type (they were worked out where they were
// wanted).  int is 2 bytes here, as the W65C816's.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65TypeQuestions(void)
{
	CCPP65Types	cTypes;
	SCPP65Type*	psArray;
	SCPP65Type*	psRows;
	SCPP65Type*	psReference;

	cTypes.Init(2, 4);

	AssertLong(-128, cTypes.msSignedChar.Least());
	AssertLong(127, cTypes.msSignedChar.Most());
	AssertLong(0, cTypes.msUnsignedChar.Least());
	AssertLong(255, cTypes.msUnsignedChar.Most());
	AssertLong(-32768, cTypes.msInt.Least());
	AssertLong(32767, cTypes.msInt.Most());
	AssertLong(0, cTypes.msUnsignedInt.Least());
	AssertLong(65535, cTypes.msUnsignedInt.Most());
	AssertLong(-2147483647LL - 1, cTypes.msLong.Least());
	AssertLong(2147483647LL, cTypes.msLong.Most());
	AssertLong(4294967295LL, cTypes.msUnsignedLong.Most());
	AssertLong(-2147483647LL - 1, CPPIntegerLeast(4, true));
	AssertLong(4294967295LL, CPPIntegerMost(4, false));
	AssertLong(0, CPPIntegerLeast(1, false));
	AssertLong(127, CPPIntegerMost(1, true));

	psArray = cTypes.Array(&cTypes.msShort, 3);
	psRows = cTypes.Array(psArray, 2);
	AssertPointer(&cTypes.msShort, psArray->ElementType());
	AssertPointer(psArray, psRows->ElementType());
	AssertPointer(&cTypes.msShort, cTypes.msShort.ElementType());

	psReference = cTypes.Reference(psArray);
	AssertPointer(psArray, psReference->WithoutReference());
	AssertPointer(psArray, psArray->WithoutReference());
	AssertPointer(psReference, psReference->ElementType());

	cTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPPClassLayout(void)
{
	BeginTests();

	TestCPPClassLayoutFields();
	TestCPPClassLayoutPackAndAlignas();
	TestCPPClassLayoutUnion();
	TestCPPClassLayoutBitFields();
	TestCPPClassLayoutBasesAndVtables();
	TestCPP65TypeQuestions();
	TestCPPClassBasePaths();

	TestStatistics();
}

