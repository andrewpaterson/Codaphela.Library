#include "TestLib/Assert.h"
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/Int64Vec2.h"
#include "BaseLib/Float64Vec3.h"
#include <stddef.h>


struct TestPrimitiveLongStruct
{
public:
	int64 lli1;
	int64 lli2;
};


struct TestPrimitivePointerArrayStruct
{
public:
	TestPrimitiveLongStruct*	aps[256];
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPrimitiveTypesPointerArray(void)
{
	AssertSize(16, sizeof(TestPrimitiveLongStruct));
	AssertSize(sizeof(void*), sizeof(TestPrimitiveLongStruct*));
	AssertSize(256 * sizeof(TestPrimitiveLongStruct*), sizeof(TestPrimitivePointerArrayStruct));

	TestPrimitiveLongStruct*			p1;
	TestPrimitiveLongStruct*			p2;
	TestPrimitivePointerArrayStruct		sPointers;
	TestPrimitiveLongStruct				sLongs1;
	TestPrimitiveLongStruct				sLongs2;

	memset(&sPointers, 0, sizeof(TestPrimitivePointerArrayStruct));
	p1 = sPointers.aps[0];
	p2 = sPointers.aps[1];

	AssertNull(p1);
	AssertNull(p2);

	sLongs1.lli1 = 1LL;
	sLongs1.lli2 = 3LL;
	sLongs2.lli1 = 2LL;
	sLongs2.lli2 = 4LL;

	sPointers.aps[0] = &sLongs1;
	sPointers.aps[1] = &sLongs2;
	p1 = sPointers.aps[0];
	p2 = sPointers.aps[1];

	AssertPointer(&sLongs1, p1);
	AssertPointer(&sLongs2, p2);
	AssertLong(1LL, p1->lli1);
	AssertLong(2LL, p2->lli1);
	AssertLong(3LL, sPointers.aps[0]->lli2);
	AssertLong(4LL, sPointers.aps[1]->lli2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPrimitiveTypesSizes(void)
{
	AssertSize(1, sizeof(char));
#ifdef _WIN32
	AssertSize(2, sizeof(wchar_t));
#endif // _WIN32
#ifdef LINUX_GNU
	AssertSize(4, sizeof(wchar_t));
#endif // LINUX_GNU
#ifdef __W65C816__
	AssertSize(2, sizeof(int));
#else
	AssertSize(4, sizeof(int));
#endif // __W65C816__
	AssertSize(4, sizeof(int32));
	AssertSize(8, sizeof(int64));
#ifdef BUILD_64
	AssertSize(8, sizeof(void*));
	AssertSize(8, sizeof(size));
	AssertSize(8, sizeof(lsize));
#elif defined(SIZE_16)
	AssertSize(4, sizeof(void*));
	AssertSize(2, sizeof(size));
	AssertSize(4, sizeof(lsize));
#else
	AssertSize(4, sizeof(void*));
	AssertSize(4, sizeof(size));
	AssertSize(4, sizeof(lsize));
#endif // BUILD_64
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit members are aligned to 8 bytes as MSVC does, including inside
// templates where g++ ignores ALIGN_64 on the template argument.
//
//////////////////////////////////////////////////////////////////////////
struct SAlignmentTestInt64
{
	int32		i;
	int64		l;
};

struct SAlignmentTestInt64Vec2
{
	int32		i;
	SInt64Vec2	v;
};

struct SAlignmentTestFloat64Vec3
{
	int32			i;
	SFloat64Vec3	v;
};

struct SAlignmentTestInt32Vec2
{
	int32		i;
	SInt32Vec2	v;
};

void TestPrimitiveTypesAlignment(void)
{
	AssertSize(8, offsetof(SAlignmentTestInt64, l));
	AssertSize(16, sizeof(SAlignmentTestInt64));

	AssertSize(8, alignof(SInt64Vec2));
	AssertSize(8, offsetof(SAlignmentTestInt64Vec2, v));
	AssertSize(24, sizeof(SAlignmentTestInt64Vec2));

	AssertSize(8, alignof(SFloat64Vec3));
	AssertSize(8, offsetof(SAlignmentTestFloat64Vec3, v));
	AssertSize(32, sizeof(SAlignmentTestFloat64Vec3));

	AssertSize(4, alignof(SInt32Vec2));
	AssertSize(4, offsetof(SAlignmentTestInt32Vec2, v));
	AssertSize(12, sizeof(SAlignmentTestInt32Vec2));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPrimitiveTypes(void)
{
	BeginTests();

	TestPrimitiveTypesSizes();
	TestPrimitiveTypesPointerArray();
	TestPrimitiveTypesAlignment();

	TestStatistics();
}

