#ifndef __TEST_CLASS_H__
#define __TEST_CLASS_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Array.h"
#include "StandardLib/Integer.h"
#include "StandardLib/Float.h"
#include "BaseLib/Float4x4.h"


class CTinyTestClass : public CObject
{
CONSTRUCTABLE(CTinyTestClass)
DESTRUCTABLE(CTinyTestClass)
public:
	Int8	mc;

	void Init(void)
	{
		PreInit();
		mc = 127;
		PostInit();
	}

	void Class(void)
	{
		Primitive(&mc, "mc");
	}

	void Free(void)
	{
	}
};


class CTestClass : public CObject
{
CONSTRUCTABLE(CTestClass)
DESTRUCTABLE(CTestClass)
public:
	CPointer			mpObject;
	Ptr<CTestClass>		mpTest;
	Int32				mInt;
	int					miUnmanagedInt;
	CTinyTestClass		mTiny;
	Float64				mDouble;
	uint8				mauiData[12];

	void Init(void)
	{
		PreInit();
		mTiny.Init();
		mInt = 0;
		miUnmanagedInt = 0;
		mDouble = 0;
		memset(mauiData, 0, 12);
		PostInit();
	}

	void Class(void)
	{
		Pointer(mpObject.This(), "mpObject");
		Pointer(mpTest.This(), "mpTest");
		UnmanagedSInt(&miUnmanagedInt, "miUnmanagedInt");
		Primitive(&mInt, "mInt");
		Embedded(&mTiny, "mTiny");
		Primitive(&mDouble, "mDouble");
		UnmanagedData(mauiData, 12, "mauiData");
	}

	void Free(void)
	{
		mInt = -1;
	}
};


class CTestMatrixClass : public CObject
{
CONSTRUCTABLE(CTestMatrixClass)
DESTRUCTABLE(CTestMatrixClass)
public:
	SFloat4x4	msMatrix;
	int32		miAfter;

	void Init(void)
	{
		PreInit();
		memset(&msMatrix, 0, sizeof(SFloat4x4));
		miAfter = 0;
		PostInit();
	}

	void Class(void)
	{
		Unmanaged4x4Float(&msMatrix, "msMatrix");
		UnmanagedInt32(&miAfter, "miAfter");
	}

	void Free(void)
	{
	}
};


#endif // __TEST_CLASS_H__

