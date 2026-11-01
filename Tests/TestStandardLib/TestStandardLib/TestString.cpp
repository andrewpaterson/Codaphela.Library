#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/LogToMemory.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringAddConstructors(void)
{
	gcObjects.AddConstructor<CTestEmbeddedStrings>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringDirty(void)
{
	OIndex			oi1;
	CCodabase*		pcDatabase;
	CSequence*		pcSequence;
	CFileUtil		cFileUtil;
	char			szDirectory[] = "Output" _FS_ "String" _FS_ "Database1";
	char			szHexData[] = "Output\\";

	cFileUtil.RemoveDir(szDirectory);
	MemoryInit();
	TypesInit();
	DataIOInit();


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();

	ObjectsInit(pcDatabase, pcSequence);
	Ptr<CString>	pString1;
	{
		Ptr<CRoot>		pRoot;

		TestStringAddConstructors();

		pRoot = ORoot();
		pString1 = OString("Hello");
		oi1 = pString1->GetIndex();
		AssertTrue(pString1.IsDirty());

		pRoot->Add(pString1);
		pString1->Flush();
		AssertFalse(pString1.IsDirty());

		pString1->AppendBool(true);
		AssertTrue(pString1.IsDirty());
		AssertString("HelloTrue", pString1->Text());

		ObjectsFlush();
		AssertFalse(pString1.IsDirty());

		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
	}
	ObjectsKill(false);
	AssertNull(&pString1);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		pString1 = gcObjects.Get(oi1);
		pString1->AppendHexHiLo(szHexData, 7);
		AssertTrue(pString1.IsDirty());
		AssertString("HelloTrue5c74757074754f", pString1->Text());

		ObjectsFlush();
		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
	}
	pString1 = NULL;
	ObjectsKill();

	DataIOKill();
	TypesKill();
	MemoryKill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringMemoryIterate(void)
{
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "String" _FS_ "Database2";
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;

	cFileUtil.RemoveDir(szDirectory);
	MemoryInit();
	TypesInit();
	DataIOInit();


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		Ptr<CTestEmbeddedStrings>	pContainer;
		Ptr<CRoot>					pRoot;
		SIndexesIterator			sIter;
		OIndex						oi;
		CPointer					pcBaseObject;
		uint64						uiExpected;

		TestStringAddConstructors();

		pRoot = ORoot();
		pContainer = OMalloc<CTestEmbeddedStrings>();
		AssertTrue(pContainer.IsDirty());

		pRoot->Add(pContainer);

		uiExpected = 1LL;
		oi = gcObjects.StartMemoryIteration(&sIter);
		while (oi != INVALID_O_INDEX)
		{
			AssertLong(uiExpected, oi);

			pcBaseObject = &gcObjects.Get(oi);
			AssertTrue(pcBaseObject.IsNotNull());

			oi = gcObjects.IterateMemory(&sIter);
			uiExpected++;
		}
		AssertLong(4LL, uiExpected);

		ObjectsFlush();
		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
	}
	ObjectsKill();


	DataIOKill();
	TypesKill();
	MemoryKill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringEmbeddedDirty(void)
{
	OIndex						oiContainer;
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "String" _FS_ "Database3";

	cFileUtil.RemoveDir(szDirectory);
	MemoryInit();
	TypesInit();
	DataIOInit();


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	Ptr<CString>				pString1;
	{
		Ptr<CString>				pString2;
		Ptr<CString>				pString3;
		Ptr<CTestEmbeddedStrings>	pContainer;
		Ptr<CRoot>					pRoot;

		TestStringAddConstructors();

		pRoot = ORoot();
		pContainer = OMalloc<CTestEmbeddedStrings>();
		oiContainer = pContainer->GetIndex();
		AssertTrue(pContainer.IsDirty());

		pRoot->Add(pContainer);
		ObjectsFlush();
		AssertFalse(pString1.IsDirty());

		pContainer->mString1.Set("This is first");
		pContainer->mString2.Set("Burke is great");
		pContainer->mString3.Set("Wooglers");

		AssertLong(INVALID_O_INDEX, pContainer->mString1.GetIndex());
		AssertLong(INVALID_O_INDEX, pContainer->mString2.GetIndex());
		AssertLong(INVALID_O_INDEX, pContainer->mString3.GetIndex());

		ObjectsFlush();
		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
	}
	AssertNull(&pString1);
	ObjectsKill();


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		Ptr<CString>				pString2;
		Ptr<CString>				pString3;
		Ptr<CTestEmbeddedStrings>	pContainer;

		ORoot()->TouchAll();
		pContainer = gcObjects.Get(oiContainer);

		pString1 = &pContainer->mString1;

		pString2 = &pContainer->mString2;
		pString3 = &pContainer->mString3;
		AssertInt(2, pString1.GetDistToRoot());
		AssertInt(2, pString2.GetDistToRoot());
		AssertInt(2, pString3.GetDistToRoot());
		AssertInt(2, pContainer.GetDistToRoot());

		AssertString("This is first", pString1->Text());
		AssertString("Burke is great", pString2->Text());
		AssertString("Wooglers", pString3->Text());

		pString1->RemoveEnd(7);
		AssertTrue(pString1.IsDirty());
		AssertTrue(pContainer.IsDirty());
		AssertString("This is", pString1->Text());

		ObjectsFlush();
		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
	}
	pString1 = NULL;
	ObjectsKill();


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	Ptr<CString>				pString2;
	Ptr<CString>				pString3;
	{
		Ptr<CTestEmbeddedStrings>	pContainer;

		pContainer = gcObjects.Get(oiContainer);
		ORoot()->TouchAll();
		pString1 = &pContainer->mString1;
		pString2 = &pContainer->mString2;
		pString3 = &pContainer->mString3;
		AssertInt(2, pString1.GetDistToRoot());
		AssertInt(2, pString2.GetDistToRoot());
		AssertInt(2, pString3.GetDistToRoot());
		AssertInt(2, pContainer.GetDistToRoot());

		AssertString("This is", pString1->Text());
		AssertString("Burke is great", pString2->Text());
		AssertString("Wooglers", pString3->Text());

		pString1->RemoveEnd(7);
		AssertString("This is", pString1->Text());
		AssertTrue(pString1.IsDirty());
		AssertTrue(pContainer.IsDirty());

		pString2->Insert(9, "the ");
		AssertTrue(pString2.IsDirty());
		pString2->Append("est");
		AssertString("Burke is the greatest", pString2->Text());

		pString3->Set("Boogaloo");
		AssertTrue(pString3.IsDirty());
		AssertString("Boogaloo", pString3->Text());

		ObjectsFlush();
		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
	}
	ObjectsKill(false);
	AssertNull(&pString1);
	AssertNull(&pString2);
	AssertNull(&pString3);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	{
		Ptr<CString>				pString1;
		Ptr<CString>				pString2;
		Ptr<CString>				pString3;
		Ptr<CTestEmbeddedStrings>	pContainer;
		Ptr<CRoot>					pRoot;

		pContainer = gcObjects.Get(oiContainer);
		pString1 = &pContainer->mString1;
		pString2 = &pContainer->mString2;
		pString3 = &pContainer->mString3;
		ORoot()->TouchAll();
		AssertInt(2, pString1.GetDistToRoot());
		AssertInt(2, pString2.GetDistToRoot());
		AssertInt(2, pString3.GetDistToRoot());
		AssertInt(2, pContainer.GetDistToRoot());

		AssertString("This is", pString1->Text());
		AssertString("Burke is the greatest", pString2->Text());
		AssertString("Boogaloo", pString3->Text());

		ObjectsFlush();
	}
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill();


	DataIOKill();
	TypesKill();
	MemoryKill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringAlteration(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CString>	pString1;
		Ptr<CString>	pString2;
		Ptr<CString>	pDest;
		Ptr<CString>	pYes;
		Ptr<CString>	pNo;
		Ptr<CRoot>		pRoot;

		pString1 = OString("Hello");
		pString2 = OString("World!");
		pDest = OString();
		pYes = OString("Yes");
		pNo = OString("No");

		pDest->Set(pString1)->Append(' ')->AppendSubString(pString2, 5);
		AssertString(pDest->Text(), "Hello World");

		pDest->Clear()->AppendBool(true)->Append(pString1, 3)->AppendBool(false, pYes, pNo);
		AssertString(pDest->Text(), "TrueHelNo");

		pDest->Clear()->AppendFlag(0x03, 0x02, pString1);
		pDest->AppendFlag(0x03, 0x01, pString2, true);
		AssertString("Hello, World!", pDest->Text());

		pDest->Set("Left:")->LeftAlign(pString1, ' ', 20)->Append(":Right");
		AssertString("Left:Hello               :Right", pDest->Text());

		pDest->Set("Left:")->RightAlign(pString2, ' ', 20)->Append(":Right");
		AssertString("Left:              World!:Right", pDest->Text());

		pDest->Set(pNo)->Insert(1, pYes);
		AssertString("NYeso", pDest->Text());

		pString1->Set("Banana");
		pString2->Set("omo");

		pString1->Replace(OString("a"), pString2);
		AssertString("Bomonomonomo", pString1->Text());

		pString1->Overwrite(6, pYes);
		AssertString("BomonoYesomo", pString1->Text());
	}
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringTests(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CString>	pString1;
		Ptr<CString>	pDest;
		Ptr<CString>	pYes;
		Ptr<CString>	pNo;
		Ptr<CRoot>		pRoot;

		pDest = OString();
		pYes = OString("Yes");
		pNo = OString("No");

		pDest->Set("Yes");
		AssertTrue(pYes->Equals(pYes));
		AssertTrue(pDest->Equals(pYes));
		AssertFalse(pDest->Equals(pNo));

		pDest->Set("yeS");
		AssertTrue(pYes->EqualsIgnoreCase(pYes));
		AssertTrue(pDest->EqualsIgnoreCase(pYes));
		AssertFalse(pDest->EqualsIgnoreCase(pNo));

		pString1 = OString("Banana in Pajama");

		AssertTrue(pString1->Contains(OString("nana")));
		AssertTrue(pString1->ContainsIgnoreCase(OString("NANa")));
		AssertFalse(pString1->Contains(OString("Panama")));
		AssertFalse(pString1->ContainsIgnoreCase(OString("Panama")));

		AssertTrue(pString1->EndsWith(OString("Pajama")));
		AssertTrue(pString1->EndsWithIgnoreCase(OString("paJAma")));
		AssertFalse(pString1->EndsWith(OString("Pajam")));
		AssertFalse(pString1->EndsWithIgnoreCase(OString("paJAm")));

		AssertTrue(pString1->StartsWith(OString("Banana")));
		AssertTrue(pString1->StartsWithIgnoreCase(OString("baNaNa")));
		AssertFalse(pString1->StartsWith(OString("anana")));
		AssertFalse(pString1->StartsWithIgnoreCase(OString("AnAnA")));

		AssertTrue(pString1->SubStringEquals(1, OString("anana")));
		AssertTrue(pString1->SubStringEqualsIgnoreCase(1, OString("ANANA")));
		AssertFalse(pString1->SubStringEquals(2, OString("anana")));
		AssertFalse(pString1->SubStringEqualsIgnoreCase(2, OString("ANANA")));

		pString1->Append(" in Manila");

		AssertSize(7, pString1->Find("in"));
		AssertSize(7, pString1->Find(7, "in"));
		AssertSize(17, pString1->Find(8, "in"));
		AssertSize(-1, pString1->Find("Nope"));

		AssertSize(17, pString1->FindFromEnd("in"));
		AssertSize(17, pString1->FindFromEnd(17, "in"));
		AssertSize(7, pString1->FindFromEnd(16, "in"));
		AssertSize(-1, pString1->FindFromEnd("Nope"));
	}
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringSplit(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CString>			pString;
		Ptr<CArray<CString>>	paDest;

		pString = OString("And,the,Aardvark,walked,over,the,Hill,and,far,Away");
		AssertLong(1LL, gcObjects.NumMemoryIndexes());

		paDest = pString->Split(',');
		AssertSize(10, paDest->Length());
		AssertLong(12LL, gcObjects.NumMemoryIndexes());

		AssertString("And", paDest->Get(0)->Text());
		AssertString("the", paDest->Get(1)->Text());
		AssertString("Aardvark", paDest->Get(2)->Text());
		AssertString("walked", paDest->Get(3)->Text());
		AssertString("over", paDest->Get(4)->Text());
		AssertString("the", paDest->Get(5)->Text());
		AssertString("Hill", paDest->Get(6)->Text());
		AssertString("and", paDest->Get(7)->Text());
		AssertString("far", paDest->Get(8)->Text());
		AssertString("Away", paDest->Get(9)->Text());

		AssertString("And,the,Aardvark,walked,over,the,Hill,and,far,Away", pString->Text());
		AssertSize(50, pString->Length());
		pString->Append(",Again");
		AssertString("And,the,Aardvark,walked,over,the,Hill,and,far,Away,Again", pString->Text());

		pString = NULL;
		AssertLong(11LL, gcObjects.NumMemoryIndexes());
		paDest->Clear();
		AssertLong(1LL, gcObjects.NumMemoryIndexes());
		paDest = NULL;
		AssertLong(0LL, gcObjects.NumMemoryIndexes());

		pString = OString("And,the,Aardvark,walked,over,the,Hill,and,far,Away");
		AssertLong(1LL, gcObjects.NumMemoryIndexes());

		paDest = pString->Split(',');
		AssertSize(10, paDest->Length());
		AssertLong(12LL, gcObjects.NumMemoryIndexes());

		paDest = NULL;
		AssertLong(1LL, gcObjects.NumMemoryIndexes());
	}
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestString(void)
{
	BeginTests();

	TestStringDirty();
	TestStringMemoryIterate();
	TestStringEmbeddedDirty();
	TestStringAlteration();
	TestStringTests();
	TestStringSplit();

	TestStatistics();
}

