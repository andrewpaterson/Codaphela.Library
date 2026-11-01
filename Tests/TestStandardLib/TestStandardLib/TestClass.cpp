#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "StandardLib/UnmanagedField.h"
#include "StandardLib/Classes.h"
#include "TestLib/Assert.h"
#include "TestBaseObject.h"
#include "TestClass.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClassAddConstructors(void)
{
	gcObjects.AddConstructor<CTestClass>();
	gcObjects.AddConstructor<CTinyTestClass>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClassDefinition(void)
{
	Ptr<CTestClass>		pTestClass;
	CClass*				pcTestClassClass;
	CClass*				pcTinyTestClassClass;
	CClasses*			pcClasses;
	CClass*				pcTemp;
	CField*				pcField;
	EPrimitiveType		eType;

	DataIOInit();
	ObjectsInit();

	pcClasses = gcObjects.GetClasses();

	pTestClass = gcObjects.Malloc<CTestClass>();
	pTestClass->Init();

	pcTestClassClass = pTestClass->GetClass();
	AssertNotNull(pcTestClassClass);
	AssertString("CTestClass", pcTestClassClass->GetName());
	AssertTrue(pcTestClassClass->GetType() > CLASS_TYPES);
	AssertTrue(pcTestClassClass->IsComplete());
	eType = pcTestClassClass->GetType();

	pcTemp = pcClasses->Get(eType);
	AssertPointer(pcTestClassClass, pcTemp);
	pcTemp = pcClasses->Get("CTestClass");
	AssertPointer(pcTestClassClass, pcTemp);

	pcTinyTestClassClass = pTestClass->mTiny.GetClass();
	AssertNotNull(pcTinyTestClassClass);
	AssertString("CTinyTestClass", pcTinyTestClassClass->GetName());
	AssertInt(eType + 1, pcTinyTestClassClass->GetType());
	AssertTrue(pcTinyTestClassClass->IsComplete());

	pcTemp = pcClasses->Get(eType + 1);
	AssertPointer(pcTinyTestClassClass, pcTemp);
	pcTemp = pcClasses->Get("CTinyTestClass");
	AssertPointer(pcTinyTestClassClass, pcTemp);

	pcField = pcTestClassClass->GetField("mpObject");
	AssertTrue(pcField->IsPointer());
	AssertString("mpObject", pcField->GetName());
	AssertLayoutSize(OBJECT_SIZE, pcField->GetOffset());

	pcField = pcTestClassClass->GetField("mpTest");
	AssertTrue(pcField->IsPointer());
	AssertLayoutSize(OBJECT_SIZE + POINTER_SIZE, pcField->GetOffset());

	pcField = pcTestClassClass->GetField("mInt");
	AssertTrue(pcField->IsPrimitive());
	AssertString("mInt", pcField->GetName());
	AssertLayoutSize(OBJECT_SIZE + POINTER_SIZE * 2, pcField->GetOffset());

	pcField = pcTestClassClass->GetField("miUnmanagedInt");
	AssertTrue(pcField->IsUnmanaged());
	AssertFalse(pcField->IsArray());
	AssertString("miUnmanagedInt", pcField->GetName());
	AssertLayoutSize(OBJECT_SIZE + POINTER_SIZE * 2 + 8, pcField->GetOffset());

	pcField = pcTestClassClass->GetField("mTiny");
	AssertTrue(pcField->IsEmbeddedObject());
	AssertString("mTiny", pcField->GetName());
	AssertLayoutSize(OBJECT_SIZE + POINTER_SIZE * 2 + 16, pcField->GetOffset());

	pcField = pcTestClassClass->GetField("mDouble");
	AssertTrue(pcField->IsPrimitive());
#ifdef BUILD_64
	AssertLayoutSize(OBJECT_SIZE + 400, pcField->GetOffset());
#elif defined(__W65C816__)
	AssertLayoutSize(OBJECT_SIZE + 200, pcField->GetOffset());
#else
	AssertLayoutSize(OBJECT_SIZE + 224, pcField->GetOffset());
#endif // BUILD_64

	pcField = pcTestClassClass->GetField("mauiData");
	AssertTrue(pcField->IsUnmanaged());
#ifdef BUILD_64
	AssertLayoutSize(OBJECT_SIZE + 416, pcField->GetOffset());
#elif defined(__W65C816__)
	AssertLayoutSize(OBJECT_SIZE + 216, pcField->GetOffset());
#else
	AssertLayoutSize(OBJECT_SIZE + 240, pcField->GetOffset());
#endif // BUILD_64

	pTestClass = NULL;

	ObjectsKill();
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClassOnStackOutOfFrame(void)
{
	CTestClass			cTestClass;
	CClass*				pcTestClassClass;
	CClass*				pcTinyTestClassClass;
	CClasses*			pcClasses;
	CClass*				pcTemp;
	EPrimitiveType		eType;
	CField*				pcField;
	CPointerField*		pcPointerField;
	CPointer*			pcPointer;
	CTestClass*			pcTestClass;
	Ptr<CTestClass>		pTestClass3;

	pcClasses = gcObjects.GetClasses();

	cTestClass.Init();
	pcTestClassClass = cTestClass.GetClass();
	AssertNotNull(pcTestClassClass);
	AssertString("CTestClass", pcTestClassClass->GetName());
	AssertTrue(pcTestClassClass->GetType() > CLASS_TYPES);
	AssertTrue(pcTestClassClass->IsComplete());
	eType = pcTestClassClass->GetType();

	pcTemp = pcClasses->Get(eType);
	AssertPointer(pcTestClassClass, pcTemp);
	pcTemp = pcClasses->Get("CTestClass");
	AssertPointer(pcTestClassClass, pcTemp);

	pcTinyTestClassClass = cTestClass.mTiny.GetClass();
	AssertNotNull(pcTinyTestClassClass);
	AssertString("CTinyTestClass", pcTinyTestClassClass->GetName());
	AssertInt(eType + 1, pcTinyTestClassClass->GetType());
	AssertTrue(pcTinyTestClassClass->IsComplete());

	pcField = pcTestClassClass->GetField("mpObject");
	AssertTrue(pcField->IsPointer());
	AssertString("mpObject", pcField->GetName());
	AssertLayoutSize(OBJECT_SIZE, pcField->GetOffset());
	pcPointerField = (CPointerField*)pcField;


	pTestClass3 = OMalloc<CTestClass>();
	cTestClass.mpObject = pTestClass3;
	pTestClass3->mDouble = 45648739045.0;

	pcPointer = pcPointerField->GetPointer(&cTestClass);
	pcTestClass = (CTestClass*)pcPointer->BaseObject();
	AssertDouble(45648739045.0, pcTestClass->mDouble, 0);

	pTestClass3->Kill();

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClassOnStack(void)
{
	DataIOInit();
	ObjectsInit();

	TestClassOnStackOutOfFrame();

	ObjectsKill();
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClassName(void)
{
	Ptr<CTestClass>		pTestClass1;
	CTestClass*			pcTestClass;
	CObject*			pcNamedObject;
	CObject*			pcObject;
	CBaseObject*		pcBaseObject;
	CEmbeddedObject*	pcEmbeddedObject;
	CUnknown*			pcUnknown;
	CConstructable*		pcConstructable;

	DataIOInit();
	ObjectsInit();

	pTestClass1 = gcObjects.Malloc<CTestClass>();
	pTestClass1->Init();

	pcTestClass = &pTestClass1;
	AssertString("CTestClass", pcTestClass->ClassName());

	pcNamedObject = &pTestClass1;
	AssertString("CTestClass", pcNamedObject->ClassName());

	pcObject = &pTestClass1;
	AssertString("CTestClass", pcObject->ClassName());

	pcBaseObject = &pTestClass1;
	AssertString("CTestClass", pcBaseObject->ClassName());

	pcEmbeddedObject = &pTestClass1;
	AssertString("CTestClass", pcEmbeddedObject->ClassName());

	pcUnknown = &pTestClass1;
	AssertString("CTestClass", pcUnknown->ClassName());

	pcConstructable = &pTestClass1;
	AssertString("CTestClass", pcConstructable->ClassName());

	pTestClass1 = NULL;

	ObjectsKill();
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClassSerialisation(void)
{
	char		szDirectory[] = "Output" _FS_ "TestClass";
	CFileUtil	cFileUtil;

	DataIOInit();
	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();

	{
		Ptr<CTestClass>					pTestClass;
		CExternalObjectSerialiser		cSerialiser;
		CChunkFileObjectWriter			cWriter;
		char							szData[] = "0123456789A";
		CExternalObjectDeserialiser		cGraphDeserialiser;
		CChunkFileSystemObjectReader 	cReader;

		TestClassAddConstructors();

		pTestClass = gcObjects.Malloc<CTestClass>("Burke");
		pTestClass->Init();
		pTestClass->mpObject = NULL;;
		pTestClass->mpTest = NULL;
		memcpy(pTestClass->mauiData, szData, 12);
		pTestClass->mDouble = 7980345645697084.0;
		pTestClass->mInt = 45978634;
		pTestClass->miUnmanagedInt = 28943;
		pTestClass->mTiny.mc = 'c';

		cWriter.Init(szDirectory, "", "File");
		cSerialiser.Init(&cWriter);
		cSerialiser.Write(&pTestClass);
		cSerialiser.Kill();
		cWriter.Kill();

		pTestClass = NULL;
	}

	ObjectsKill();

	ObjectsInit();

	{
		Ptr<CTestClass>					pTestClass;
		CChunkFileObjectWriter			cWriter;
		char							szData[] = "0123456789A";
		CExternalObjectDeserialiser		cGraphDeserialiser;
		CChunkFileSystemObjectReader 	cReader;
		OIndex							oi;

		cReader.Init(szDirectory, "File");
		cGraphDeserialiser.Init(&cReader, false, &gcObjects);
		pTestClass = cGraphDeserialiser.Read("Burke");
		AssertNotNull(&pTestClass);

		cGraphDeserialiser.Kill();
		cReader.Kill();
		
		oi = pTestClass->GetIndex();
		AssertMemory(szData, pTestClass->mauiData, 12);
		AssertInt(45978634, pTestClass->mInt);
		AssertDouble(7980345645697084.0, pTestClass->mDouble, 0);
		AssertInt(28943, pTestClass->miUnmanagedInt);
		AssertChar('c', pTestClass->mTiny.mc);
		AssertLong(oi, pTestClass->GetIndex());

		pTestClass = NULL;
	}

	ObjectsKill();

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClassUnmanagedVectorClasses(void)
{
	CUnmanagedClasses*	pcUnmanaged;

	DataIOInit();
	ObjectsInit();

	pcUnmanaged = gcObjects.GetClasses()->GetUnmanagedClasses();
	AssertInt(PT_M2float32, pcUnmanaged->GetFloat2()->GetType());
	AssertInt(PT_M3float32, pcUnmanaged->GetFloat3()->GetType());
	AssertInt(PT_M4float32, pcUnmanaged->GetFloat4()->GetType());
	AssertInt(PT_M2float64, pcUnmanaged->GetDouble2()->GetType());
	AssertInt(PT_M3float64, pcUnmanaged->GetDouble3()->GetType());
	AssertInt(PT_M4float64, pcUnmanaged->GetDouble4()->GetType());
	AssertInt(PT_M4x4float32, pcUnmanaged->GetFloat4x4()->GetType());
	AssertSize(sizeof(SFloat4x4), pcUnmanaged->GetFloat4x4()->GetSizeOf());

	ObjectsKill();
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClassUnmanagedMatrixSerialisation(void)
{
	char		szDirectory[] = "Output" _FS_ "TestClassMatrix";
	CFileUtil	cFileUtil;
	float*		pf;
	size		i;

	DataIOInit();
	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();

	{
		Ptr<CTestMatrixClass>			pTestClass;
		CExternalObjectSerialiser		cSerialiser;
		CChunkFileObjectWriter			cWriter;
		CArrayVoidPtr*					papvFields;
		CUnmanagedField*				pcField;

		gcObjects.AddConstructor<CTestMatrixClass>();

		pTestClass = gcObjects.Malloc<CTestMatrixClass>("Matrix");
		pTestClass->Init();
		pf = (float*)&pTestClass->msMatrix;
		for (i = 0; i < 16; i++)
		{
			pf[i] = (float)(i + 1);
		}
		pTestClass->miAfter = 12345;

		papvFields = pTestClass->GetClass()->GetUnmanagedFields();
		AssertSize(2, papvFields->NumElements());
		pcField = (CUnmanagedField*)papvFields->GetPtr(0);
		AssertNotNull(pcField->GetClass());
		AssertString("msMatrix", pcField->GetName());
		AssertSize(sizeof(SFloat4x4), pcField->GetSizeOf());
		AssertSize(1, pcField->GetLength());

		cWriter.Init(szDirectory, "", "File");
		cSerialiser.Init(&cWriter);
		cSerialiser.Write(&pTestClass);
		cSerialiser.Kill();
		cWriter.Kill();

		pTestClass = NULL;
	}

	ObjectsKill();

	ObjectsInit();

	{
		Ptr<CTestMatrixClass>			pTestClass;
		CExternalObjectDeserialiser		cGraphDeserialiser;
		CChunkFileSystemObjectReader 	cReader;

		gcObjects.AddConstructor<CTestMatrixClass>();

		cReader.Init(szDirectory, "File");
		cGraphDeserialiser.Init(&cReader, false, &gcObjects);
		pTestClass = cGraphDeserialiser.Read("Matrix");
		AssertNotNull(&pTestClass);

		cGraphDeserialiser.Kill();
		cReader.Kill();

		pf = (float*)&pTestClass->msMatrix;
		for (i = 0; i < 16; i++)
		{
			AssertFloat((float)(i + 1), pf[i], 3);
		}
		AssertInt((int32)12345, pTestClass->miAfter);

		pTestClass = NULL;
	}

	ObjectsKill();

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClassHeapObjectEmbeddedPointers(void)
{
	Ptr<CTestClass>		pTestClass1;
	Ptr<CTestClass>		pTestClass2;

	DataIOInit();
	ObjectsInit();

	pTestClass1 = gcObjects.Malloc<CTestClass>();
	AssertTrue(pTestClass1->IsAllocatedInObjects());
	AssertTrue(pTestClass1->mpObject.IsEmbeddingAllocatedInObjects());
	AssertTrue(pTestClass1->mTiny.IsEmbedded());
	pTestClass1->Init();

	pTestClass2 = gcObjects.Malloc<CTestClass>();
	AssertTrue(pTestClass2->IsAllocatedInObjects());
	AssertTrue(pTestClass2->mpObject.IsEmbeddingAllocatedInObjects());
	AssertTrue(pTestClass2->mTiny.IsEmbedded());
	pTestClass2->Init();

	pTestClass1 = NULL;
	pTestClass2 = NULL;

	ObjectsKill();
	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClass(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();

	TestClassDefinition();
	TestClassOnStack();
	TestClassName();
	TestClassSerialisation();
	TestClassUnmanagedVectorClasses();
	TestClassUnmanagedMatrixSerialisation();
	TestClassHeapObjectEmbeddedPointers();

	TypesKill();
	MemoryKill();
	TestStatistics();
}

