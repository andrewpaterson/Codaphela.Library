#include "BaseLib/IndexTreeMemory.h"
#include "BaseLib/IndexTreeMemoryAccess.h"
#include "BaseLib/AccessDataOrderer.h"
#include "BaseLib/ModificationDataOrderer.h"
#include "BaseLib/CreationDataOrderer.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertIndexTreeDataOrdererString(CIndexTreeDataOrderer* pcOrderer, const char* szExpected)
{
	SDataOrderIterator	sIter;
	size				iSize;
	bool				bExists;
	CChars				sz;
	char				pc[256];

	sz.Init();
	bExists = pcOrderer->StartIteration(&sIter, NULL, 0, 0, pc, &iSize, 256);
	while (bExists)
	{
		sz.Append(*pc);
		bExists = pcOrderer->Iterate(&sIter, NULL, 0, 0, pc, &iSize, 256);
	}

	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeDataOrdererAccess(void)
{
	CIndexTreeMemory				cIndexTree;
	CIndexTreeMemoryAccess			cAccess;
	CAccessDataOrderer				cOrderer;
	SIndexTreeMemoryUnsafeIterator	sIter;
	bool							bExists;

	cOrderer.Init();
	cIndexTree.Init(IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);

	AssertIndexTreeDataOrdererString(&cOrderer, "");

	cAccess.PutStringChar("0000", 'a');
	cAccess.PutStringChar("0100", 'b');
	cAccess.PutStringChar("0010", 'c');
	cAccess.PutStringChar("0110", 'd');
	AssertIndexTreeDataOrdererString(&cOrderer, "dcba");

	cAccess.GetStringChar("0000");
	AssertIndexTreeDataOrdererString(&cOrderer, "adcb");

	cAccess.GetStringChar("0010");
	AssertIndexTreeDataOrdererString(&cOrderer, "cadb");

	cAccess.DeleteString("0100");
	cAccess.DeleteString("0110");
	AssertIndexTreeDataOrdererString(&cOrderer, "ca");

	cAccess.DeleteString("0010");
	cAccess.DeleteString("0000");
	AssertIndexTreeDataOrdererString(&cOrderer, "");

	cAccess.PutStringChar("0000", 'a');
	cAccess.PutStringChar("0100", 'b');
	cAccess.PutStringChar("0010", 'c');
	cAccess.PutStringChar("0110", 'd');
	AssertIndexTreeDataOrdererString(&cOrderer, "dcba");

	cAccess.HasString("0100");
	AssertIndexTreeDataOrdererString(&cOrderer, "bdca");

	cAccess.HasString("0100");
	AssertIndexTreeDataOrdererString(&cOrderer, "bdca");

	cAccess.HasString("0110");
	AssertIndexTreeDataOrdererString(&cOrderer, "dbca");

	bExists = cIndexTree.StartIteration(&sIter, NULL, NULL);
	AssertTrue(bExists);
	AssertIndexTreeDataOrdererString(&cOrderer, "adbc");
	bExists = cIndexTree.Iterate(&sIter, NULL, NULL);
	AssertTrue(bExists);
	AssertIndexTreeDataOrdererString(&cOrderer, "cadb");
	bExists = cIndexTree.Iterate(&sIter, NULL, NULL);
	AssertTrue(bExists);
	AssertIndexTreeDataOrdererString(&cOrderer, "bcad");
	bExists = cIndexTree.Iterate(&sIter, NULL, NULL);
	AssertTrue(bExists);
	AssertIndexTreeDataOrdererString(&cOrderer, "dbca");
	bExists = cIndexTree.Iterate(&sIter, NULL, NULL);
	AssertFalse(bExists);

	cIndexTree.Kill();
	cAccess.Kill();
	cOrderer.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeDataOrdererRemapListNodes(void)
{
	CIndexTreeMemory		cIndexTree;
	CIndexTreeMemoryAccess	cAccess;
	CAccessDataOrderer		cOrderer;
	size					i;

	cOrderer.Init();
	cIndexTree.Init(IKR_Yes, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);

	AssertIndexTreeDataOrdererString(&cOrderer, "");

	i = 61;
	do 
	{
		i--;
		cAccess.PutIntLong((int32)i << 8, i);
	}
	while (i != 0);

	i = 61;
	do
	{
		i--;
		cAccess.PutIntChar((int32)i << 8, 'A' + (char)i);
	}
	while (i != 0);

	AssertIndexTreeDataOrdererString(&cOrderer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}");

	for (i = 0; i <= 60; i+=2)
	{
		AssertTrue(cAccess.HasInt((int32)i << 8));
	}
	AssertIndexTreeDataOrdererString(&cOrderer, "}{ywusqomkigeca_][YWUSQOMKIGECABDFHJLNPRTVXZ\\^`bdfhjlnprtvxz|");

	for (i = 0; i <= 60; i++)
	{
		cAccess.PutIntLong((int32)i << 8, i);
	}
	for (i = 0; i <= 60; i++)
	{
		cAccess.PutIntChar((int32)i << 8, 'A' + (char)i);
	}
	AssertIndexTreeDataOrdererString(&cOrderer, "}|{zyxwvutsrqponmlkjihgfedcba`_^]\\[ZYXWVUTSRQPONMLKJIHGFEDCBA");

	i = 62;
	do
	{
		i -= 2;
		AssertTrue(cAccess.HasInt((int32)i << 8));
	}
	while (i != 0);

	AssertIndexTreeDataOrdererString(&cOrderer, "ACEGIKMOQSUWY[]_acegikmoqsuwy{}|zxvtrpnljhfdb`^\\ZXVTRPNLJHFDB");

	cIndexTree.Kill();
	cAccess.Kill();
	cOrderer.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeDataOrdererModification(void)
{
	CIndexTreeMemory			cIndexTree;
	CIndexTreeMemoryAccess		cAccess;
	CModificationDataOrderer	cOrderer;
	size						i;
	size						iKey;
	char						c;

	cOrderer.Init();
	cIndexTree.Init(IKR_Yes, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);

	AssertIndexTreeDataOrdererString(&cOrderer, "");

	i = 26;
	do
	{
		i--;
		iKey = i * 0b1010101010101010101010101010;
		cAccess.PutIntChar((int32)iKey, 'A' + (char)i);
	}
	while (i != 0);

	AssertIndexTreeDataOrdererString(&cOrderer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	for (i = 0; i <= 25; i++)
	{
		iKey = i * 0b1010101010101010101010101010;
		c = cAccess.GetIntChar((int32)iKey);
		AssertSize('A' + i, c)
	}
	AssertIndexTreeDataOrdererString(&cOrderer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	for (i = 0; i <= 13; i++)
	{
		iKey = i * 0b1010101010101010101010101010;
		cAccess.PutIntChar((int32)iKey, 'a' + (char)i);
	}
	AssertIndexTreeDataOrdererString(&cOrderer, "nmlkjihgfedcbaOPQRSTUVWXYZ");

	for (i = 0; i <= 25; i+=2)
	{
		iKey = i * 0b1010101010101010101010101010;
		AssertTrue(cAccess.HasInt((int32)iKey));
	}
	AssertIndexTreeDataOrdererString(&cOrderer, "nmlkjihgfedcbaOPQRSTUVWXYZ");

	for (i = 14; i <= 25; i++)
	{
		iKey = i * 0b1010101010101010101010101010;
		cAccess.PutIntChar((int32)iKey, 'a' + (char)i);
	}
	AssertIndexTreeDataOrdererString(&cOrderer, "zyxwvutsrqponmlkjihgfedcba");

	cIndexTree.Kill();
	cAccess.Kill();
	cOrderer.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeDataOrdererCreation(void)
{
	CIndexTreeMemory		cIndexTree;
	CIndexTreeMemoryAccess	cAccess;
	CCreationDataOrderer	cOrderer;

	cOrderer.Init();
	cIndexTree.Init(IKR_Yes, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);

	AssertIndexTreeDataOrdererString(&cOrderer, "");

	cAccess.PutStringChar("0000", 'a');
	cAccess.PutStringChar("0100", 'b');
	cAccess.PutStringChar("0010", 'c');
	cAccess.PutStringChar("0110", 'd');
	AssertIndexTreeDataOrdererString(&cOrderer, "dcba");

	cAccess.PutStringChar("0100", 'x');
	cAccess.PutStringChar("0010", 'y');
	AssertIndexTreeDataOrdererString(&cOrderer, "dyxa");

	cIndexTree.Kill();
	cAccess.Kill();
	cOrderer.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeDataOrderer(void)
{
	BeginTests();
	MemoryInit();

	TestIndexTreeDataOrdererAccess();
	TestIndexTreeDataOrdererRemapListNodes();
	TestIndexTreeDataOrdererModification();
	TestIndexTreeDataOrdererCreation();

	MemoryKill();
	TestStatistics();
}

