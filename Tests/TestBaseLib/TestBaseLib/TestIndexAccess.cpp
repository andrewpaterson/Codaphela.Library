#include "BaseLib/MapIndexAccess.h"
#include "BaseLib/MapBlock.h"
#include "BaseLib/IndexTreeMemory.h"
#include "BaseLib/IndexTreeMemoryAccess.h"
#include "TestLib/Assert.h"


struct STestIndexAccessData
{
	int		ma;
	int		mb;
	int		mc;
	int		md;

	void Init(int a, int b, int c, int d)
	{
		ma = a;
		mb = b;
		mc = c;
		md = d;
	}
};



struct STestIndexAccessKey
{
	int32	ma;
	int16	mb;

	void Init(int32 a, int16 b)
	{
		//The whole struct is the key: its padding after mb must be the same every time.
		memset(this, 0, sizeof(STestIndexAccessKey));
		ma = a;
		mb = b;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexAccessMapBlock(void)
{
	CMapBlock					cMap;
	CMapIndexAccess				cAccess;
	STestIndexAccessData		s1;
	STestIndexAccessData		s2;
	STestIndexAccessData		s3;
	STestIndexAccessData		s4;
	STestIndexAccessData		s5;
	STestIndexAccessData		s6;
	STestIndexAccessData		s7;
	STestIndexAccessKey			sk1;
	STestIndexAccessKey			sk2;
	STestIndexAccessKey			sk3;
	STestIndexAccessData*		ps;
	STestIndexAccessData		sDest;
	size						uiSize;
	char						sz[256];

	s1.Init(1, 2, 3, 4);
	s2.Init(5, 6, 7, 8);
	s3.Init(11, 12, 13, 14);
	s4.Init(15, 16, 17, 18);
	s5.Init(21, 22, 23, 24);
	s6.Init(25, 26, 27, 28);
	s7.Init(31, 32, 33, 34);

	sk1.Init(0x10000001, 0x0000);
	sk2.Init(0x10000001, 0x0001);
	sk3.Init(0x10000001, 0x0002);

	cMap.Init();
	cAccess.Init(&cMap);

	AssertTrue(cAccess.PutLongPtr(0x1000000000000001L, &s1));
	AssertTrue(cAccess.PutLongInt(0x1000000000000004L, 102));
	AssertTrue(cAccess.PutLongLong(0x1000000000000006L, 0x1000000000000022L));
	AssertTrue(cAccess.PutLongChar(0x1000000000000003L, 'c'));
	AssertTrue(cAccess.PutLongString(0x1000000000000002L, "Whole Hole"));
	AssertTrue(cAccess.PutLongData(0x1000000000000005L, &s2, sizeof(STestIndexAccessData)));
	AssertTrue(cAccess.PutIntPtr(0x10000003, &s3));
	AssertTrue(cAccess.PutIntInt(0x10000001, 101));
	AssertTrue(cAccess.PutIntLong(0x10000004, 0x1000000000000004L));
	AssertTrue(cAccess.PutIntChar(0x10000002, 'd'));
	AssertTrue(cAccess.PutIntString(0x10000005, "Spot the Dog"));
	AssertTrue(cAccess.PutIntData(0x10000006, &s4, sizeof(STestIndexAccessData)));
	AssertTrue(cAccess.PutStringPtr("01", &s5));
	AssertTrue(cAccess.PutStringData("02", &s6, sizeof(STestIndexAccessData)));
	AssertTrue(cAccess.PutStringChar("1", 'e'));
	AssertTrue(cAccess.PutStringInt("10", 203));
	AssertTrue(cAccess.PutStringLong("11", 0x1000000000000001L));
	AssertTrue(cAccess.PutStringString("12", "Jip the Cat"));
	AssertTrue(cAccess.PutKeyData((uint8*)&sk1, sizeof(STestIndexAccessKey), &s7, sizeof(STestIndexAccessData)));
	AssertTrue(cAccess.PutKeyInt((uint8*)&sk2, sizeof(STestIndexAccessKey), 88));
	AssertTrue(cAccess.PutKeyString((uint8*)&sk3, sizeof(STestIndexAccessKey), "Dangerous"));

	ps = (STestIndexAccessData*)cAccess.GetLongPtr(0x1000000000000001L);
	AssertPointer(&s1, ps);
	AssertInt((int32)102, cAccess.GetLongInt(0x1000000000000004L));
	AssertLong(0x1000000000000022L, cAccess.GetLongLong(0x1000000000000006L));
	AssertChar('c', cAccess.GetLongChar(0x1000000000000003L));
	AssertString("Whole Hole", cAccess.GetLongString(0x1000000000000002L, sz, 256));
	AssertTrue(cAccess.GetLongData(0x1000000000000005L, &sDest, &uiSize, sizeof(STestIndexAccessData)));
	AssertSize(sizeof(STestIndexAccessData), uiSize);
	AssertMemory(&s2, &sDest, sizeof(STestIndexAccessData));
	ps = (STestIndexAccessData*)cAccess.GetIntPtr(0x10000003);
	AssertPointer(&s3, ps);
	AssertInt((int32)101, cAccess.GetIntInt(0x10000001, 101));
	AssertLong(0x1000000000000004L, cAccess.GetIntLong(0x10000004));
	AssertChar('d', cAccess.GetIntChar(0x10000002));
	AssertString("Spot the Dog", cAccess.GetIntString(0x10000005, sz, 256));
	AssertTrue(cAccess.GetIntData(0x10000006, &sDest, &uiSize, sizeof(STestIndexAccessData)));
	AssertSize(sizeof(STestIndexAccessData), uiSize);
	AssertMemory(&s4, &sDest, sizeof(STestIndexAccessData));
	ps = (STestIndexAccessData*)cAccess.GetStringPtr("01");
	AssertPointer(&s5, ps);
	AssertTrue(cAccess.GetStringData("02", &sDest, &uiSize, sizeof(STestIndexAccessData)));
	AssertSize(sizeof(STestIndexAccessData), uiSize);
	AssertMemory(&s6, &sDest, sizeof(STestIndexAccessData));
	AssertChar('e', cAccess.GetStringChar("1"));
	AssertInt((int32)203, cAccess.GetStringInt("10"));
	AssertLong(0x1000000000000001L, cAccess.GetStringLong("11"));
	AssertString("Jip the Cat", cAccess.GetStringString("12", sz, 256));
	AssertTrue(cAccess.GetKeyData((uint8*)&sk1, sizeof(STestIndexAccessKey), &sDest, &uiSize, sizeof(STestIndexAccessData)));
	AssertSize(sizeof(STestIndexAccessData), uiSize);
	AssertMemory(&s7, &sDest, sizeof(STestIndexAccessData));
	AssertInt((int32)88, cAccess.GetKeyInt((uint8*)&sk2, sizeof(STestIndexAccessKey)));
	AssertString("Dangerous", cAccess.GetKeyString((uint8*)&sk3, sizeof(STestIndexAccessKey), sz, 256));
			
	cAccess.Kill();
	cMap.Kill();
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexAccessIndexTreeMemory(void)
{
	CIndexTreeMemory			cMap;
	CIndexTreeMemoryAccess		cAccess;
	STestIndexAccessData		s1;
	STestIndexAccessData		s2;
	STestIndexAccessData		s3;
	STestIndexAccessData		s4;
	STestIndexAccessData		s5;
	STestIndexAccessData		s6;
	STestIndexAccessData		s7;
	STestIndexAccessKey			sk1;
	STestIndexAccessKey			sk2;
	STestIndexAccessKey			sk3;
	STestIndexAccessData*		ps;
	STestIndexAccessData		sDest;
	size						uiSize;
	char						sz[256];

	s1.Init(1, 2, 3, 4);
	s2.Init(5, 6, 7, 8);
	s3.Init(11, 12, 13, 14);
	s4.Init(15, 16, 17, 18);
	s5.Init(21, 22, 23, 24);
	s6.Init(25, 26, 27, 28);
	s7.Init(31, 32, 33, 34);

	sk1.Init(0x10000001, 0x0000);
	sk2.Init(0x10000001, 0x0001);
	sk3.Init(0x10000001, 0x0002);

	cMap.Init();
	cAccess.Init(&cMap);

	AssertTrue(cAccess.PutLongPtr(0x1000000000000001L, &s1));
	AssertTrue(cAccess.PutLongInt(0x1000000000000004L, 102));
	AssertTrue(cAccess.PutLongLong(0x1000000000000006L, 0x1000000000000022L));
	AssertTrue(cAccess.PutLongChar(0x1000000000000003L, 'c'));
	AssertTrue(cAccess.PutLongString(0x1000000000000002L, "Whole Hole"));
	AssertTrue(cAccess.PutLongData(0x1000000000000005L, &s2, sizeof(STestIndexAccessData)));
	AssertTrue(cAccess.PutIntPtr(0x10000003, &s3));
	AssertTrue(cAccess.PutIntInt(0x10000001, 101));
	AssertTrue(cAccess.PutIntLong(0x10000004, 0x1000000000000004L));
	AssertTrue(cAccess.PutIntChar(0x10000002, 'd'));
	AssertTrue(cAccess.PutIntString(0x10000005, "Spot the Dog"));
	AssertTrue(cAccess.PutIntData(0x10000006, &s4, sizeof(STestIndexAccessData)));
	AssertTrue(cAccess.PutStringPtr("01", &s5));
	AssertTrue(cAccess.PutStringData("02", &s6, sizeof(STestIndexAccessData)));
	AssertTrue(cAccess.PutStringChar("1", 'e'));
	AssertTrue(cAccess.PutStringInt("10", 203));
	AssertTrue(cAccess.PutStringLong("11", 0x1000000000000001L));
	AssertTrue(cAccess.PutStringString("12", "Jip the Cat"));
	AssertTrue(cAccess.PutKeyData((uint8*)&sk1, sizeof(STestIndexAccessKey), &s7, sizeof(STestIndexAccessData)));
	AssertTrue(cAccess.PutKeyInt((uint8*)&sk2, sizeof(STestIndexAccessKey), 88));
	AssertTrue(cAccess.PutKeyString((uint8*)&sk3, sizeof(STestIndexAccessKey), "Dangerous"));

	ps = (STestIndexAccessData*)cAccess.GetLongPtr(0x1000000000000001L);
	AssertPointer(&s1, ps);
	AssertInt((int32)102, cAccess.GetLongInt(0x1000000000000004L));
	AssertLong(0x1000000000000022L, cAccess.GetLongLong(0x1000000000000006L));
	AssertChar('c', cAccess.GetLongChar(0x1000000000000003L));
	AssertString("Whole Hole", cAccess.GetLongString(0x1000000000000002L, sz, 256));
	AssertTrue(cAccess.GetLongData(0x1000000000000005L, &sDest, &uiSize, sizeof(STestIndexAccessData)));
	AssertSize(sizeof(STestIndexAccessData), uiSize);
	AssertMemory(&s2, &sDest, sizeof(STestIndexAccessData));
	ps = (STestIndexAccessData*)cAccess.GetIntPtr(0x10000003);
	AssertPointer(&s3, ps);
	AssertInt((int32)101, cAccess.GetIntInt(0x10000001, 101));
	AssertLong(0x1000000000000004L, cAccess.GetIntLong(0x10000004));
	AssertChar('d', cAccess.GetIntChar(0x10000002));
	AssertString("Spot the Dog", cAccess.GetIntString(0x10000005, sz, 256));
	AssertTrue(cAccess.GetIntData(0x10000006, &sDest, &uiSize, sizeof(STestIndexAccessData)));
	AssertSize(sizeof(STestIndexAccessData), uiSize);
	AssertMemory(&s4, &sDest, sizeof(STestIndexAccessData));
	ps = (STestIndexAccessData*)cAccess.GetStringPtr("01");
	AssertPointer(&s5, ps);
	AssertTrue(cAccess.GetStringData("02", &sDest, &uiSize, sizeof(STestIndexAccessData)));
	AssertSize(sizeof(STestIndexAccessData), uiSize);
	AssertMemory(&s6, &sDest, sizeof(STestIndexAccessData));
	AssertChar('e', cAccess.GetStringChar("1"));
	AssertInt((int32)203, cAccess.GetStringInt("10"));
	AssertLong(0x1000000000000001L, cAccess.GetStringLong("11"));
	AssertString("Jip the Cat", cAccess.GetStringString("12", sz, 256));
	AssertTrue(cAccess.GetKeyData((uint8*)&sk1, sizeof(STestIndexAccessKey), &sDest, &uiSize, sizeof(STestIndexAccessData)));
	AssertSize(sizeof(STestIndexAccessData), uiSize);
	AssertMemory(&s7, &sDest, sizeof(STestIndexAccessData));
	AssertInt((int32)88, cAccess.GetKeyInt((uint8*)&sk2, sizeof(STestIndexAccessKey)));
	AssertString("Dangerous", cAccess.GetKeyString((uint8*)&sk3, sizeof(STestIndexAccessKey), sz, 256));

	cAccess.Kill();
	cMap.Kill();
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexAccess(void)
{
	BeginTests();

	TestIndexAccessMapBlock();
	TestIndexAccessIndexTreeMemory();

	TestStatistics();
}


