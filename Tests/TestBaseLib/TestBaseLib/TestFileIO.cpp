#include "BaseLib/FileIO.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/EnumeratorTemplate.h"
#include "BaseLib/EnumeratorBlock.h"
#include "BaseLib/LinkedListBlockAligned.h"
#include "BaseLib/LinkedListBlock.h"
#include "BaseLib/TreeTemplate.h"
#include "BaseLib/MapTemplateTemplate.h"
#include "BaseLib/TreeBlock.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"
#include "FileIOTestObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOBegin(CMemoryFile* pcMemory, CFileBasic* pcFile)
{
	pcMemory->Init();
	pcFile->Init(pcMemory);

	AssertTrue(pcFile->Open(EFM_Write_Create));
	AssertTrue(pcFile->WriteData("MagicPadding\0", 14));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOMiddle(CFileBasic* pcFile)
{
	char	szMagic[14];

	AssertTrue(pcFile->WriteData("HappyEnding\0", 13));
	AssertTrue(pcFile->Close());
	AssertTrue(pcFile->Open(EFM_Read));
	AssertTrue(pcFile->ReadData(szMagic, 14));
	AssertString("MagicPadding", szMagic);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOEnd(CMemoryFile* pcMemory, CFileBasic* pcFile)
{
	char	szHappy[13];

	AssertTrue(pcFile->ReadData(szHappy, 13));
	AssertString("HappyEnding", szHappy);
	AssertTrue(pcFile->Close());
	pcFile->Kill();
	pcMemory->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOHelpers(void)
{
	CMemoryFile		cMemory;
	CFileBasic		cFile;
	uint8			ab[7] = {0x5e, 0xe1, 0x9e, 0x03, 0x32, 0x1f, 0x93};
	int32			ai[3] = {0x7deb74f3, 0x41b0f58c, 0x5cbf05d8};
	int32			i;
	int64			l;
	float			f;
	double			d;
	char			c;
	uint8			uc;
	bool			b;
	int16			s;
	uint16			us;
	uint8			abi[7];
	int32			aii[3];

	TestFileIOBegin(&cMemory, &cFile);

	AssertTrue(cFile.WriteInt32(0xf48e33a9));
	AssertTrue(cFile.WriteInt64(0xcbb671c1babc0022LL));
	AssertTrue(cFile.WriteFloat(3.34759834536f));
	AssertTrue(cFile.WriteFloat(8.4936205629457392304853));
	AssertTrue(cFile.WriteChar('g'));
	AssertTrue(cFile.WriteInt8((uint8)255));
	AssertTrue(cFile.WriteBool(true));
	AssertTrue(cFile.WriteInt16((int16)0x5a0a));
	AssertTrue(cFile.WriteInt16((uint16)0x6be4));
	AssertTrue(cFile.WriteBits(ab, 7));
	AssertTrue(cFile.WriteIntArray(ai, 3));

	AssertTrue(cFile.WriteInt32(0x091bfe25));
	AssertTrue(cFile.WriteInt64(0xe98ebf36c079e63aLL));
	AssertTrue(cFile.WriteFloat(6.478389585938f));
	AssertTrue(cFile.WriteFloat(2.74944978325209529023823));
	AssertTrue(cFile.WriteChar('@'));
	AssertTrue(cFile.WriteInt8((uint8)1));
	AssertTrue(cFile.WriteBool(false));
	AssertTrue(cFile.WriteInt16((int16)0x0001));
	AssertTrue(cFile.WriteInt16((uint16)0xffff));

	TestFileIOMiddle(&cFile);
	
	AssertTrue(cFile.ReadInt32(&i));
	AssertInt((int32)0xf48e33a9, i);
	AssertTrue(cFile.ReadInt64(&l));
	AssertLong(0xcbb671c1babc0022LL, l);
	AssertTrue(cFile.ReadFloat(&f));
	AssertFloat(3.34759834536f, f, 13);
	AssertTrue(cFile.ReadFloat(&d));
	AssertDouble(8.4936205629457392304853, d, 19);
	AssertTrue(cFile.ReadChar(&c));
	AssertChar('g', c);
	AssertTrue(cFile.ReadInt8(&uc));
	AssertChar((uint8)255, uc);
	AssertTrue(cFile.ReadBool(&b));
	AssertBool(true, b);
	AssertTrue(cFile.ReadInt16(&s));
	AssertShort((int16)0x5a0a, s);
	AssertTrue(cFile.ReadInt16((uint16*)&us));
	AssertShort((uint16)0x6be4, us);
	AssertTrue(cFile.ReadBits(abi, 7));
	AssertMemory(ab, abi, 7);
	AssertTrue(cFile.ReadIntArray(aii, 3));
	AssertMemory(ai, aii, 3 * sizeof(int32));

	AssertTrue(cFile.ReadInt32(&i));
	AssertInt(0x091bfe25, i);
	AssertTrue(cFile.ReadInt64(&l));
	AssertLong(0xe98ebf36c079e63aLL, l);
	AssertTrue(cFile.ReadFloat(&f));
	AssertFloat(6.478389585938f, f, 13);
	AssertTrue(cFile.ReadFloat(&d));
	AssertDouble(2.74944978325209529023823, d, 19);
	AssertTrue(cFile.ReadChar(&c));
	AssertChar('@', c);
	AssertTrue(cFile.ReadInt8(&uc));
	AssertChar(1, uc);
	AssertTrue(cFile.ReadBool(&b));
	AssertBool(false, b);
	AssertTrue(cFile.ReadInt16(&s));
	AssertShort((int16)0x0001, s);
	AssertTrue(cFile.ReadInt16((uint16*)&us));
	AssertShort((uint16)0xffff, us);

	TestFileIOEnd(&cMemory, &cFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOLittleEndian(void)
{
	CMemoryFile		cMemory;
	CFileBasic		cFile;
	uint8			abExpected[12] = {0x0a, 0x5a,  0xfe, 0xff,  0xa9, 0x33, 0x8e, 0xf4,  0x01, 0x00, 0x00, 0x80};
	uint8			ab[12];
	int16			s;
	uint16			us;
	int32			i;
	uint32			ui;

	cMemory.Init();
	cFile.Init(&cMemory);
	AssertTrue(cFile.Open(EFM_Write_Create));
	AssertTrue(cFile.WriteLittleEndianInt16((uint16)0x5a0a));
	AssertTrue(cFile.WriteLittleEndianInt16((int16)-2));
	AssertTrue(cFile.WriteLittleEndianInt32((uint32)0xf48e33a9));
	AssertTrue(cFile.WriteLittleEndianInt32((int32)0x80000001));
	AssertTrue(cFile.Close());

	//Least significant byte first.
	AssertTrue(cFile.Open(EFM_Read));
	AssertTrue(cFile.ReadData(ab, 12));
	AssertMemory(abExpected, ab, 12);

	AssertTrue(cFile.Seek(0));
	AssertTrue(cFile.ReadLittleEndianInt16(&us));
	AssertShort((uint16)0x5a0a, us);
	AssertTrue(cFile.ReadLittleEndianInt16(&s));
	AssertShort((int16)-2, s);
	AssertTrue(cFile.ReadLittleEndianInt32(&ui));
	AssertInt((uint32)0xf48e33a9, ui);
	AssertTrue(cFile.ReadLittleEndianInt32(&i));
	AssertInt((int32)0x80000001, i);

	//Reading past the end fails.
	AssertFalse(cFile.ReadLittleEndianInt16(&us));
	AssertFalse(cFile.ReadLittleEndianInt32(&ui));

	AssertTrue(cFile.Close());
	cFile.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOBigEndian(void)
{
	CMemoryFile		cMemory;
	CFileBasic		cFile;
	uint8			abExpected[12] = {0x5a, 0x0a,  0xff, 0xfe,  0xf4, 0x8e, 0x33, 0xa9,  0x80, 0x00, 0x00, 0x01};
	uint8			ab[12];
	int16			s;
	uint16			us;
	int32			i;
	uint32			ui;

	cMemory.Init();
	cFile.Init(&cMemory);
	AssertTrue(cFile.Open(EFM_Write_Create));
	AssertTrue(cFile.WriteBigEndianInt16((uint16)0x5a0a));
	AssertTrue(cFile.WriteBigEndianInt16((int16)-2));
	AssertTrue(cFile.WriteBigEndianInt32((uint32)0xf48e33a9));
	AssertTrue(cFile.WriteBigEndianInt32((int32)0x80000001));
	AssertTrue(cFile.Close());

	//Most significant byte first.
	AssertTrue(cFile.Open(EFM_Read));
	AssertTrue(cFile.ReadData(ab, 12));
	AssertMemory(abExpected, ab, 12);

	AssertTrue(cFile.Seek(0));
	AssertTrue(cFile.ReadBigEndianInt16(&us));
	AssertShort((uint16)0x5a0a, us);
	AssertTrue(cFile.ReadBigEndianInt16(&s));
	AssertShort((int16)-2, s);
	AssertTrue(cFile.ReadBigEndianInt32(&ui));
	AssertInt((uint32)0xf48e33a9, ui);
	AssertTrue(cFile.ReadBigEndianInt32(&i));
	AssertInt((int32)0x80000001, i);

	//Reading past the end fails.
	AssertFalse(cFile.ReadBigEndianInt16(&us));
	AssertFalse(cFile.ReadBigEndianInt32(&ui));

	AssertTrue(cFile.Close());
	cFile.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOStringTruncated(void)
{
	CMemoryFile		cMemory;
	CFileBasic		cFile;
	char			sz[8];
	int32			i;

	cMemory.Init();
	cFile.Init(&cMemory);
	AssertTrue(cFile.Open(EFM_Write_Create));
	AssertTrue(cFile.WriteString("A string longer than the buffer"));
	AssertTrue(cFile.WriteInt32(1234567));
	AssertTrue(cFile.WriteString("Short"));
	AssertTrue(cFile.WriteInt32(-89));
	AssertTrue(cFile.Close());

	//A string too long for the buffer is cut short and the rest of it skipped, so the next read follows it.
	AssertTrue(cFile.Open(EFM_Read));
	AssertTrue(cFile.ReadString(sz, 8));
	AssertString("A strin", sz);
	AssertTrue(cFile.ReadInt32(&i));
	AssertInt(1234567, i);
	AssertTrue(cFile.ReadString(sz, 8));
	AssertString("Short", sz);
	AssertTrue(cFile.ReadInt32(&i));
	AssertInt((int32)-89, i);
	AssertTrue(cFile.Close());

	cFile.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOEnumerator(void)
{
	CMemoryFile							cMemory;
	CFileBasic							cFile;
	CEnumeratorTemplate<CFileIOTest>	cEnumerator;
	CFileIOTest							cTest;
	CFileIOTest*						pcTestIn;
	CEnumeratorTemplate<CFileIOTest>	cEnumeratorIn;

	TestFileIOBegin(&cMemory, &cFile);

	cTest.Init(83);
	cEnumerator.Init();
	cEnumerator.Add("I Dunno", &cTest, 2);

	AssertTrue(cEnumerator.WriteEnumeratorTemplate(&cFile));

	cEnumerator.Kill();
	TestFileIOMiddle(&cFile);

	AssertTrue(cEnumeratorIn.ReadEnumeratorTemplate(&cFile));
	AssertInt(2, cEnumeratorIn.Get("I Dunno", &pcTestIn));
	AssertTrue(pcTestIn->IsOkay(83));

	cEnumeratorIn.Kill();
	TestFileIOEnd(&cMemory, &cFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOTree(void)
{
	CMemoryFile					cMemory;
	CFileBasic					cFile;
	CTreeTemplate<CFileIOTest>	cTree;
	CTreeBlock					cBlock;
	CFileIOTest*				pcRoot;
	CFileIOTest*				pcChild;
	CFileIOTest*				pcGrandChild;
	CTreeTemplate<CFileIOTest>	cTreeIn;
	CFileIOTest*				pcBlockRoot;
	CFileIOTest*				pcBlockChild;
	CTreeBlock					cBlockIn;
	CFileIOTest*				pcRootIn;
	CFileIOTest*				pcChildIn;
	CFileIOTest*				pcGrandChildIn;

	TestFileIOBegin(&cMemory, &cFile);

	cTree.Init();
	pcRoot = cTree.InsertRoot();
	pcRoot->Init(672);
	pcChild = cTree.InsertOnRightOfChildren(pcRoot);
	pcChild->Init(720);
	pcGrandChild = cTree.InsertOnRightOfChildren(pcChild);
	pcGrandChild->Init(666);
	pcChild = cTree.InsertOnRightOfChildren(pcRoot);
	pcChild->Init(8);
	pcChild = cTree.InsertOnRightOfChildren(pcRoot);
	pcChild->Init(99);
	AssertSize(5, cTree.NumElements());
	AssertTrue(cTree.WriteTreeTemplate(&cFile));
	pcRoot = cTree.GetRoot();
	AssertTrue(pcRoot->IsOkay(672));
	pcChild = cTree.GetUp(pcRoot);
	AssertTrue(pcChild->IsOkay(720));
	AssertNull(cTree.GetDown(pcRoot));

	cBlock.Init();
	pcBlockRoot = (CFileIOTest*)cBlock.InsertRoot(sizeof(CFileIOTest));
	pcBlockRoot->Init(1337);
	pcBlockChild = (CFileIOTest*)cBlock.InsertOnRightOfChildren(pcBlockRoot, sizeof(CFileIOTest));
	pcBlockChild->Init(54321);
	AssertSize(2, cBlock.NumElements());
	AssertTrue(cBlock.WriteTreeUnknown(&cFile));

	cBlock.Kill();
	cTree.Kill();
	TestFileIOMiddle(&cFile);

	AssertTrue(cTreeIn.ReadTreeTemplate(&cFile));
	AssertSize(5, cTreeIn.NumElements());
	pcRootIn = cTreeIn.GetRoot();
	AssertTrue(pcRootIn->IsOkay(672));
	pcChildIn = cTreeIn.GetUp(pcRootIn);
	AssertTrue(pcChildIn->IsOkay(720));
	pcGrandChildIn = cTreeIn.GetUp(pcChildIn);
	AssertTrue(pcGrandChildIn->IsOkay(666));
	pcChildIn = cTreeIn.GetRight(pcChildIn);
	AssertTrue(pcChildIn->IsOkay(8));
	pcChildIn = cTreeIn.GetRight(pcChildIn);
	AssertTrue(pcChildIn->IsOkay(99));
	pcChildIn = cTreeIn.GetRight(pcChildIn);
	AssertNull(pcChildIn);
	pcChildIn = cTreeIn.GetUp(pcRootIn);
	pcChildIn = cTreeIn.GetRight(pcChildIn);
	pcGrandChildIn = cTreeIn.GetUp(pcChildIn);
	AssertNull(pcGrandChildIn);
	AssertNull(cTreeIn.GetDown(pcRootIn));
	cTreeIn.Kill();

	AssertTrue(cBlockIn.ReadTreeUnknown(&cFile));
	AssertSize(2, cBlockIn.NumElements());
	pcBlockRoot = (CFileIOTest*)cBlockIn.GetRoot();
	AssertTrue(pcBlockRoot->IsOkay(1337));
	pcBlockChild = (CFileIOTest*)cBlockIn.GetUp(pcBlockRoot);
	AssertTrue(pcBlockChild->IsOkay(54321));
	cBlockIn.Kill();

	TestFileIOEnd(&cMemory, &cFile);
	cFile.Kill();
	cMemory.Kill();

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOLinkList(void)
{
	CMemoryFile							cMemory;
	CFileBasic							cFile;
	CLinkedListTemplate<CFileIOTest>	cLinkList;
	CFileIOTest*						pcTest1;
	CFileIOTest*						pcTest2;
	CLinkedListTemplate<CFileIOTest>	cLinkListIn;
	CFileIOTest*						pcTestIn;
	CLinkedListBlock					cBlock;
	char								sz[20];
	void*								pv;
	CLinkedListBlock					cBlockIn;

	TestFileIOBegin(&cMemory, &cFile);

	cLinkList.Init();
	pcTest1 = cLinkList.Add();
	pcTest1->Init(3);
	pcTest2 = cLinkList.Add();
	pcTest2->Init(887);
	AssertSize(cLinkList.NumElements(), 2);
	AssertTrue(cLinkList.Write(&cFile));

	cBlock.Init();
	strcpy(sz, "Lurching Larch");
	pv = cBlock.InsertAfterTail(strlen(sz)+1);
	strcpy((char*)pv, sz);
	strcpy(sz, "Black Larch");
	pv = cBlock.InsertAfterTail(strlen(sz)+1);
	strcpy((char*)pv, sz);
	strcpy(sz, "Wheeling Willow");
	pv = cBlock.InsertAfterTail(strlen(sz)+1);
	strcpy((char*)pv, sz);
	AssertSize(cBlock.NumElements(), 3);
	AssertTrue(cBlock.Write(&cFile));

	cBlock.Kill();
	cLinkList.Kill();
	TestFileIOMiddle(&cFile);

	AssertTrue(cLinkListIn.Read(&cFile));
	AssertSize(cLinkListIn.NumElements(), 2);
	pcTestIn = cLinkListIn.GetHead();
	AssertNotNull(pcTestIn);
	AssertTrue(pcTestIn->IsOkay(3));
	pcTestIn = cLinkListIn.GetNext(pcTestIn);
	AssertTrue(pcTestIn->IsOkay(887));
	pcTestIn = cLinkListIn.GetNext(pcTestIn);
	AssertNull(pcTestIn);

	AssertTrue(cBlockIn.Read(&cFile));
	AssertSize(cBlockIn.NumElements(), 3);
	pv = cBlockIn.GetHead();
	AssertString("Lurching Larch", (char*)pv);
	pv = cBlockIn.GetNext(pv);
	AssertString("Black Larch", (char*)pv);
	pv = cBlockIn.GetNext(pv);
	AssertString("Wheeling Willow", (char*)pv);
	pv = cBlockIn.GetNext(pv);
	AssertNull(pv);

	cBlockIn.Kill();
	cLinkListIn.Kill();
	TestFileIOEnd(&cMemory, &cFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOLinkedListBlockAligned(void)
{
	CMemoryFile					cMemory;
	CFileBasic					cFile;
	CLinkedListBlockAligned		cAligned;
	CLinkedListBlockAligned		cAlignedIn;
	CFileIOAlignedTest*			pcAli1;
	CFileIOAlignedTest*			pcAli2;
	CFileIOAlignedTest*			pcAliIn;

	TestFileIOBegin(&cMemory, &cFile);

	cAligned.Init();
	pcAli1 = (CFileIOAlignedTest*)cAligned.Add(sizeof(CFileIOAlignedTest), 13);
	AssertTrue(pcAli1->IsAligned(13));
	pcAli1->Init(56);
	pcAli2 = (CFileIOAlignedTest*)cAligned.Add(sizeof(CFileIOAlignedTest), 17);
	AssertTrue(pcAli2->IsAligned(17));
	pcAli2->Init(-7);
	AssertSize(cAligned.NumElements(), 2);
	AssertTrue(cAligned.Write(&cFile));

	cAligned.Kill();
	TestFileIOMiddle(&cFile);

	AssertTrue(cAlignedIn.Read(&cFile));
	AssertSize(cAlignedIn.NumElements(), 2);
	pcAliIn = (CFileIOAlignedTest*)cAlignedIn.GetHead();
	AssertNotNull(pcAliIn);
	AssertTrue(pcAliIn->IsAligned(13));
	AssertTrue(pcAliIn->IsOkay(56));
	pcAliIn = (CFileIOAlignedTest*)cAlignedIn.GetNext(pcAliIn);
	AssertNotNull(pcAliIn);
	AssertTrue(pcAliIn->IsAligned(17));
	AssertTrue(pcAliIn->IsOkay(-7));
	pcAliIn = (CFileIOAlignedTest*)cAlignedIn.GetNext(pcAliIn);
	AssertNull(pcAliIn);

	TestFileIOEnd(&cMemory, &cFile);

	cAlignedIn.Kill();
}

typedef CArrayTemplate<CFileIOTest>			CArrayFileIOTest;

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOArray(void)
{
	CMemoryFile					cMemory;
	CFileBasic					cFile;
	CArrayFileIOTest			acTest;
	size						i;
	CFileIOTest*				pcTest;
	CArrayFileIOTest			acTestIn;
	CArrayInt					ai;
	CArrayInt					aii;
	size						ii;
	CArrayBlock					av;
	CArrayBlock					avi;

	TestFileIOBegin(&cMemory, &cFile);

	acTest.Init();
	for (i = 0; i <= 8; i++)
	{
		pcTest = acTest.Add();
		pcTest->Init((int)i);
	}
	ai.Init();
	for (i = 0; i <= 5; i++)
	{
		ai.Add((int)(8-i));
	}
	av.Init(&gcSystemAllocator, 1);
	av.Resize(12);
	memcpy(av.GetData(), "ABCDEFGHIJK\0", 12);

	AssertSize(9, acTest.NumElements());
	AssertSize(16, acTest.AllocatedElements());
	AssertSize(6, ai.NumElements());
	AssertSize(8, ai.AllocatedElements());
	AssertSize(12, av.NumElements());
	AssertSize(16, av.AllocatedElements());
	AssertTrue(acTest.Write(&cFile));
	AssertTrue(ai.Write(&cFile));
	AssertTrue(av.Write(&cFile));

	av.Kill();
	ai.Kill();
	acTest.Kill();
	TestFileIOMiddle(&cFile);

	AssertTrue(acTestIn.Read(&cFile));
	AssertSize(9, acTestIn.NumElements());
	AssertSize(16, acTestIn.AllocatedElements());
	for (i = 0; i <= 8; i++)
	{
		pcTest = acTestIn.Get(i);
		AssertTrue(pcTest->IsOkay((int)i));
	}
	AssertTrue(aii.Read(&cFile));
	AssertSize(6, aii.NumElements());
	AssertSize(8, aii.AllocatedElements());
	for (i = 0; i <= 5; i++)
	{
		ii = aii.GetValue(i);
		AssertSize(8-i, ii);
	}
	AssertTrue(avi.Read(&cFile));
	AssertSize(12, avi.NumElements());
	AssertSize(16, avi.AllocatedElements());
	AssertString("ABCDEFGHIJK", (char*)avi.GetData());

	avi.Kill();
	aii.Kill();
	acTestIn.Kill();

	TestFileIOEnd(&cMemory, &cFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOString(void)
{
	CMemoryFile		cMemory;
	CFileBasic		cFile;
	CChars			sz;
	CChars			szi;
	CChars			szi2;
	char			szc[4];
	size			i;

	TestFileIOBegin(&cMemory, &cFile);

	sz.Init("You, flock of seagulls, you know why we're here?");
	AssertTrue(sz.WriteString(&cFile));
	AssertTrue(cFile.WriteString("That means they only eat vegetables, but for you, I think they'd make an exception."));
	AssertTrue(cFile.WriteString("123"));

	sz.Kill();
	TestFileIOMiddle(&cFile);

	AssertTrue(szi.ReadString(&cFile));
	AssertString("You, flock of seagulls, you know why we're here?", szi.Text());
	AssertTrue(szi2.ReadChars(&cFile));
	AssertString("That means they only eat vegetables, but for you, I think they'd make an exception.", szi2.Text());
	AssertTrue(cFile.ReadStringLength(&i));
	AssertSize(4, i);
	AssertTrue(cFile.ReadStringChars(szc, i));
	AssertString("123", szc);

	szi2.Kill();
	szi.Kill();
	TestFileIOEnd(&cMemory, &cFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIOMap(void)
{
	CMemoryFile						cMemory;
	CFileBasic						cFile;
	CMapTemplateTemplate<size, float128>		cMap;
	size							i;
	float128							ld;
	CMapTemplateTemplate<size, float128>		cMapIn;
	CMapStringInt					mssi;
	CMapStringInt					mssii;

	TestFileIOBegin(&cMemory, &cFile);
	
	cMap.Init(&CompareSize, true);
	i = 9;	ld = 1.4783409838732098273349;
	cMap.Put(&i, &ld);
	i = 22;	ld = 1.7983487629380598763752;
	cMap.Put(&i, &ld);
	i = 4;	ld = 1.9237643187634765139872;
	cMap.Put(&i, &ld);
	AssertSize(3, cMap.NumElements());
	AssertTrue(cMap.Write(&cFile));

	mssi.Init(true);
	mssi.Put("lion", 7);
	mssi.Put("eat", 1);
	mssi.Put("zebra", 12);
	mssi.Put("for real!", -1);
	AssertSize(4, mssi.NumElements());
	AssertTrue(mssi.Write(&cFile));

	mssi.Kill();
	cMap.Kill();
	TestFileIOMiddle(&cFile);

	AssertTrue(cMapIn.Read(&cFile, &CompareSize));
	AssertSize(3, cMapIn.NumElements());
	//CMapTemplateTemplate doesn't actually have full read and write methods.

	AssertTrue(mssii.Read(&cFile));
	AssertSize(4, mssii.NumElements());
	//CMapStringInt doesn't actually have full read and write methods.

	mssii.Kill();
	cMapIn.Kill();
	TestFileIOEnd(&cMemory, &cFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileIO(void)
{
	BeginTests();
	MemoryInit();

	TestFileIOHelpers();
	TestFileIOLittleEndian();
	TestFileIOBigEndian();
	TestFileIOArray();
	TestFileIOString();
	TestFileIOStringTruncated();
	TestFileIOEnumerator();
	TestFileIOTree();
	TestFileIOLinkList();
	TestFileIOLinkedListBlockAligned();
	TestFileIOMap();

	MemoryKill();
	TestStatistics();
}
