#include "BaseLib/MapMapAccess.h"
#include "BaseLib/IndexMapAccess.h"
#include "BaseLib/MapBlock.h"
#include "BaseLib/IndexBlock.h"
#include "BaseLib/IndexMapAccess.h"
#include "TestLib/Assert.h"


struct STestMapAccessData
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



struct STestMapAccessKey
{
	int32	ma;
	int16	mb;

	void Init(int32 a, int16 b)
	{
		memset(this, 0, sizeof(STestMapAccessKey));  //The padding after mb is part of the key's bytes.
		ma = a;
		mb = b;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapAccessMapBlock(void)
{
	CMapBlock				cMap;
	CMapMapAccess			cAccess;
	STestMapAccessData		s1;
	STestMapAccessData		s2;
	STestMapAccessData		s3;
	STestMapAccessData		s4;
	STestMapAccessData		s5;
	STestMapAccessData		s6;
	STestMapAccessData		s7;
	STestMapAccessKey		sk1;
	STestMapAccessKey		sk2;
	STestMapAccessKey		sk3;
	void**					ppv1;
	int*					pi2;
	int64*					plli3;
	char*					pc4;
	char*					sz5;
	void*					pv6;
	void**					ppv7;
	int*					pi8;
	int64*					plli9;
	char*					pc10;
	char*					sz11;
	void*					pv12;
	void**					ppv13;
	void*					pv14;
	char*					pc15;
	int*					pi16;
	int64*					plli17;
	char*					sz18;
	void*					pv19;
	int*					pi20;
	char*					sz21;

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

	ppv1 = cAccess.PutLongPtr(0x1000000000000001L, &s1);
	pi2 = cAccess.PutLongInt(0x1000000000000004L, 102);
	plli3 = cAccess.PutLongLong(0x1000000000000006L, 0x1000000000000022L);
	pc4 = cAccess.PutLongChar(0x1000000000000003L, 'c');
	sz5 = cAccess.PutLongString(0x1000000000000002L, "Whole Hole");
	pv6 = cAccess.PutLongData(0x1000000000000005L, &s2, sizeof(STestMapAccessData));
	ppv7 = cAccess.PutIntPtr(0x10000003, &s3);
	pi8 = cAccess.PutIntInt(0x10000001, 101);
	plli9 = cAccess.PutIntLong(0x10000004, 0x1000000000000004L);
	pc10 = cAccess.PutIntChar(0x10000002, 'd');
	sz11 = cAccess.PutIntString(0x10000005, "Spot the Dog");
	pv12 = cAccess.PutIntData(0x10000006, &s4, sizeof(STestMapAccessData));
	ppv13 = cAccess.PutStringPtr("01", &s5);
	pv14 = cAccess.PutStringData("02", &s6, sizeof(STestMapAccessData));
	pc15 = cAccess.PutStringChar("1", 'e');
	pi16 = cAccess.PutStringInt("10", 203);
	plli17 = cAccess.PutStringLong("11", 0x1000000000000001L);
	sz18 = cAccess.PutStringString("12", "Jip the Cat");
	pv19 = cAccess.PutKeyData(&sk1, sizeof(STestMapAccessKey), &s7, sizeof(STestMapAccessData));
	pi20 = cAccess.PutKeyInt(&sk2, sizeof(STestMapAccessKey), 88);
	sz21 = cAccess.PutKeyString(&sk3, sizeof(STestMapAccessKey), "Dangerous");

	AssertPointer(ppv1, cAccess.GetLongPtr(0x1000000000000001L));
	AssertInt(s1.ma, (*((STestMapAccessData**)ppv1))->ma);
	AssertInt(s1.mb, (*((STestMapAccessData**)ppv1))->mb);
	AssertInt(s1.mc, (*((STestMapAccessData**)ppv1))->mc);
	AssertInt(s1.md, (*((STestMapAccessData**)ppv1))->md);
	AssertInt((int32)*pi2, cAccess.GetLongInt(0x1000000000000004L));
	AssertInt(102, *pi2);
	AssertLong(*plli3, cAccess.GetLongLong(0x1000000000000006L));
	AssertLong(0x1000000000000022L, *plli3);
	AssertChar(*pc4, cAccess.GetLongChar(0x1000000000000003L));
	AssertChar('c', *pc4);
	AssertString(sz5, cAccess.GetLongString(0x1000000000000002L));
	AssertString("Whole Hole", sz5);
	AssertPointer(pv6, cAccess.GetLongData(0x1000000000000005L));
	AssertMemory(&s2, pv6, sizeof(STestMapAccessData));
	AssertPointer(ppv7, cAccess.GetIntPtr(0x10000003));
	AssertInt(s3.ma, (*((STestMapAccessData**)ppv7))->ma);
	AssertInt(s3.mb, (*((STestMapAccessData**)ppv7))->mb);
	AssertInt(s3.mc, (*((STestMapAccessData**)ppv7))->mc);
	AssertInt(s3.md, (*((STestMapAccessData**)ppv7))->md);
	AssertInt((int32)*pi8, cAccess.GetIntInt(0x10000001, 101));
	AssertInt(101, *pi8);
	AssertLong(*plli9, cAccess.GetIntLong(0x10000004));
	AssertLong(0x1000000000000004L, *plli9);
	AssertChar(*pc10, cAccess.GetIntChar(0x10000002));
	AssertChar('d', *pc10);
	AssertString(sz11, cAccess.GetIntString(0x10000005));
	AssertString("Spot the Dog", sz11);
	AssertPointer(pv12, cAccess.GetIntData(0x10000006));
	AssertMemory(&s4, pv12, sizeof(STestMapAccessData));
	AssertPointer(ppv13, cAccess.GetStringPtr("01"));
	AssertInt(s5.ma, (*((STestMapAccessData**)ppv13))->ma);
	AssertInt(s5.mb, (*((STestMapAccessData**)ppv13))->mb);
	AssertInt(s5.mc, (*((STestMapAccessData**)ppv13))->mc);
	AssertInt(s5.md, (*((STestMapAccessData**)ppv13))->md);
	AssertPointer(pv14, cAccess.GetStringData("02"));
	AssertMemory(&s6, pv14, sizeof(STestMapAccessData));
	AssertChar(*pc15, cAccess.GetStringChar("1"));
	AssertChar('e', *pc15);
	AssertInt((int32)*pi16, cAccess.GetStringInt("10"));
	AssertInt(203, *pi16);
	AssertLong(*plli17, cAccess.GetStringLong("11"));
	AssertLong(0x1000000000000001L, *plli17);
	AssertString(sz18, cAccess.GetStringString("12"));
	AssertString("Jip the Cat", sz18);
	AssertPointer(pv19, cAccess.GetKeyData(&sk1, sizeof(STestMapAccessKey)));
	AssertMemory(&s7, pv19, sizeof(STestMapAccessData));
	AssertInt((int32)*pi20, cAccess.GetKeyInt(&sk2, sizeof(STestMapAccessKey)));
	AssertInt(88, *pi20);
	AssertString(sz21, cAccess.GetKeyString(&sk3, sizeof(STestMapAccessKey)));
	AssertString("Dangerous", sz21);

	cAccess.Kill();
	cMap.Kill();
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapAccessIndexBlock(void)
{
	CIndexBlock				cIndex;
	CIndexMapAccess			cAccess;
	STestMapAccessData		s1;
	STestMapAccessData		s2;
	STestMapAccessData		s3;
	STestMapAccessData		s4;
	STestMapAccessData		s5;
	STestMapAccessData		s6;
	STestMapAccessData		s7;
	STestMapAccessKey		sk1;
	STestMapAccessKey		sk2;
	STestMapAccessKey		sk3;
	void**					ppv1;
	int*					pi2;
	int64*					plli3;
	char*					pc4;
	char*					sz5;
	void*					pv6;
	void**					ppv7;
	int*					pi8;
	int64*					plli9;
	char*					pc10;
	char*					sz11;
	void*					pv12;
	void**					ppv13;
	void*					pv14;
	char*					pc15;
	int*					pi16;
	int64*					plli17;
	char*					sz18;
	void*					pv19;
	int*					pi20;
	char*					sz21;

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

	cIndex.Init();
	cAccess.Init(&cIndex);

	ppv1 = cAccess.PutLongPtr(0x1000000000000001L, &s1);
	pi2 = cAccess.PutLongInt(0x1000000000000004L, 102);
	plli3 = cAccess.PutLongLong(0x1000000000000006L, 0x1000000000000022L);
	pc4 = cAccess.PutLongChar(0x1000000000000003L, 'c');
	sz5 = cAccess.PutLongString(0x1000000000000002L, "Whole Hole");
	pv6 = cAccess.PutLongData(0x1000000000000005L, &s2, sizeof(STestMapAccessData));
	ppv7 = cAccess.PutIntPtr(0x10000003, &s3);
	pi8 = cAccess.PutIntInt(0x10000001, 101);
	plli9 = cAccess.PutIntLong(0x10000004, 0x1000000000000004L);
	pc10 = cAccess.PutIntChar(0x10000002, 'd');
	sz11 = cAccess.PutIntString(0x10000005, "Spot the Dog");
	pv12 = cAccess.PutIntData(0x10000006, &s4, sizeof(STestMapAccessData));
	ppv13 = cAccess.PutStringPtr("01", &s5);
	pv14 = cAccess.PutStringData("02", &s6, sizeof(STestMapAccessData));
	pc15 = cAccess.PutStringChar("1", 'e');
	pi16 = cAccess.PutStringInt("10", 203);
	plli17 = cAccess.PutStringLong("11", 0x1000000000000001L);
	sz18 = cAccess.PutStringString("12", "Jip the Cat");
	pv19 = cAccess.PutKeyData(&sk1, sizeof(STestMapAccessKey), &s7, sizeof(STestMapAccessData));
	pi20 = cAccess.PutKeyInt(&sk2, sizeof(STestMapAccessKey), 88);
	sz21 = cAccess.PutKeyString(&sk3, sizeof(STestMapAccessKey), "Dangerous");

	AssertPointer(ppv1, cAccess.GetLongPtr(0x1000000000000001L));
	AssertInt(s1.ma, (*((STestMapAccessData**)ppv1))->ma);
	AssertInt(s1.mb, (*((STestMapAccessData**)ppv1))->mb);
	AssertInt(s1.mc, (*((STestMapAccessData**)ppv1))->mc);
	AssertInt(s1.md, (*((STestMapAccessData**)ppv1))->md);
	AssertInt((int32)*pi2, cAccess.GetLongInt(0x1000000000000004L));
	AssertInt(102, *pi2);
	AssertLong(*plli3, cAccess.GetLongLong(0x1000000000000006L));
	AssertLong(0x1000000000000022L, *plli3);
	AssertChar(*pc4, cAccess.GetLongChar(0x1000000000000003L));
	AssertChar('c', *pc4);
	AssertString(sz5, cAccess.GetLongString(0x1000000000000002L));
	AssertString("Whole Hole", sz5);
	AssertPointer(pv6, cAccess.GetLongData(0x1000000000000005L));
	AssertMemory(&s2, pv6, sizeof(STestMapAccessData));
	AssertPointer(ppv7, cAccess.GetIntPtr(0x10000003));
	AssertInt(s3.ma, (*((STestMapAccessData**)ppv7))->ma);
	AssertInt(s3.mb, (*((STestMapAccessData**)ppv7))->mb);
	AssertInt(s3.mc, (*((STestMapAccessData**)ppv7))->mc);
	AssertInt(s3.md, (*((STestMapAccessData**)ppv7))->md);
	AssertInt((int32)*pi8, cAccess.GetIntInt(0x10000001, 101));
	AssertInt(101, *pi8);
	AssertLong(*plli9, cAccess.GetIntLong(0x10000004));
	AssertLong(0x1000000000000004L, *plli9);
	AssertChar(*pc10, cAccess.GetIntChar(0x10000002));
	AssertChar('d', *pc10);
	AssertString(sz11, cAccess.GetIntString(0x10000005));
	AssertString("Spot the Dog", sz11);
	AssertPointer(pv12, cAccess.GetIntData(0x10000006));
	AssertMemory(&s4, pv12, sizeof(STestMapAccessData));
	AssertPointer(ppv13, cAccess.GetStringPtr("01"));
	AssertInt(s5.ma, (*((STestMapAccessData**)ppv13))->ma);
	AssertInt(s5.mb, (*((STestMapAccessData**)ppv13))->mb);
	AssertInt(s5.mc, (*((STestMapAccessData**)ppv13))->mc);
	AssertInt(s5.md, (*((STestMapAccessData**)ppv13))->md);
	AssertPointer(pv14, cAccess.GetStringData("02"));
	AssertMemory(&s6, pv14, sizeof(STestMapAccessData));
	AssertChar(*pc15, cAccess.GetStringChar("1"));
	AssertChar('e', *pc15);
	AssertInt((int32)*pi16, cAccess.GetStringInt("10"));
	AssertInt(203, *pi16);
	AssertLong(*plli17, cAccess.GetStringLong("11"));
	AssertLong(0x1000000000000001L, *plli17);
	AssertString(sz18, cAccess.GetStringString("12"));
	AssertString("Jip the Cat", sz18);
	AssertPointer(pv19, cAccess.GetKeyData(&sk1, sizeof(STestMapAccessKey)));
	AssertMemory(&s7, pv19, sizeof(STestMapAccessData));
	AssertInt((int32)*pi20, cAccess.GetKeyInt(&sk2, sizeof(STestMapAccessKey)));
	AssertInt(88, *pi20);
	AssertString(sz21, cAccess.GetKeyString(&sk3, sizeof(STestMapAccessKey)));
	AssertString("Dangerous", sz21);

	cAccess.Kill();
	cIndex.Kill();
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapAccess(void)
{
	BeginTests();

	TestMapAccessMapBlock();
	TestMapAccessIndexBlock();

	TestStatistics();
}


