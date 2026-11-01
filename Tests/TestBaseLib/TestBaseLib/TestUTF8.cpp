#include "BaseLib/TextFile.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/UTF8.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF8Example1(void)
{
	CUTF8		cUTF8;
	CTextFile	cTextFile;
	CChars		sz;
	uint16		c16;
	uint32		c32;
	bool		bRead;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF8" _FS_ "Example1.txt");
	AssertTrue(bRead);
	sz.Init(cTextFile.Text());
	cTextFile.Kill();

	cUTF8.Init(&sz);

	c16 = cUTF8.GetCodePointUint16();
	AssertChar('H', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('e', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('l', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('l', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('o', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar(' ', (char)c16);

	c16 = cUTF8.GetCodePointUint16();
	AssertShort((uint16)0xFFFD, c16);

	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x4E16, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x754C, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x1F43C, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x002D, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x20AC, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x00A9, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x1D7D9, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x1D7DA, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x1D7DB, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0, c32);

	cUTF8.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF8Example2(void)
{
	CUTF8		cUTF8;
	CTextFile	cTextFile;
	CChars		sz;
	uint16		c16;
	uint32		c32;
	bool		bRead;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF8" _FS_ "Example2.txt");
	AssertTrue(bRead);
	sz.Init(cTextFile.Text());
	cTextFile.Kill();

	cUTF8.Init(&sz);

	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x1F993, c32);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('À', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('é', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertShort((uint16)0x0142, c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('ö', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('ü', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertShort((uint16)0x0000, c16);


	cUTF8.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF8Example3(void)
{
	CUTF8		cUTF8;
	CTextFile	cTextFile;
	CChars		sz;
	uint32		c32;
	size		uiUTF8ElementLength;
	size		uiCodePointLength;
	size		uiPos;
	uint8		auiBuffer[0x13];
	uint8*		psz;
	bool		bRead;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF8" _FS_ "Example3.txt");
	AssertTrue(bRead);
	sz.Init(cTextFile.Text());
	cTextFile.Kill();

	cUTF8.Init(&sz);
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0x0041, c32);
	AssertSize(1, cUTF8.GetUnicodeCodePointLength(c32));
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0x1F469, c32);
	AssertSize(3, cUTF8.GetUnicodeCodePointLength(c32));
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0x200D, c32);
	AssertSize(2, cUTF8.GetUnicodeCodePointLength(c32));
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0x1F3FB, c32);
	AssertSize(3, cUTF8.GetUnicodeCodePointLength(c32));
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0x200D, c32);
	AssertSize(2, cUTF8.GetUnicodeCodePointLength(c32));
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0x1F9B0, c32);
	AssertSize(3, cUTF8.GetUnicodeCodePointLength(c32));
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0, c32);
	cUTF8.Kill();

	cUTF8.Init(&sz);
	uiPos = cUTF8.GetPosition();
	AssertSize(0, uiPos);
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	uiPos = cUTF8.GetPosition();
	AssertSize(0, uiPos);
	AssertSize(1, uiUTF8ElementLength);
	uiPos = cUTF8.GetPosition();
	AssertSize(0, uiPos);

	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0x0041, c32);
	AssertSize(1, cUTF8.GetUnicodeCodePointLength(c32));

	uiPos = cUTF8.GetPosition();
	AssertSize(1, uiPos);
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	uiPos = cUTF8.GetPosition();
	AssertSize(1, uiPos);
	AssertSize(18, uiUTF8ElementLength);
	c32 = cUTF8.GetCodePointUint32();
	c32 = cUTF8.GetCodePointUint32();
	c32 = cUTF8.GetCodePointUint32();
	c32 = cUTF8.GetCodePointUint32();
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0x1F9B0, c32);

	uiPos = cUTF8.GetPosition();
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(19, uiPos);
	AssertSize(0, uiUTF8ElementLength);

	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0, c32);
	cUTF8.Kill();

	cUTF8.Init(&sz);
	uiPos = cUTF8.GetPosition();
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(0, uiPos);
	AssertSize(1, uiUTF8ElementLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF8.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(1, uiCodePointLength);
	AssertString("A", (char*)auiBuffer);

	uiPos = cUTF8.GetPosition();
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(1, uiPos);
	AssertSize(18, uiUTF8ElementLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF8.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(11, uiCodePointLength);
	psz = auiBuffer;
	AssertString("iô", (char*)psz);
	psz = (uint8*)NextString((char*)psz);
	AssertString("ûó", (char*)psz);
	psz = (uint8*)NextString((char*)psz);
	AssertString("°ù", (char*)psz);

	uiPos = cUTF8.GetPosition();
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(0x13, uiPos);
	AssertSize(0, uiUTF8ElementLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF8.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(0, uiCodePointLength);
	AssertString("", (char*)auiBuffer);
	cUTF8.Kill();

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF8Example4(void)
{
	CUTF8		cUTF8;
	CTextFile	cTextFile;
	CChars		sz;
	size		uiUTF8ElementLength;
	size		uiCodePointLength;
	size		uiPos;
	uint8		auiBuffer[0x13];
	uint8*		psz;
	bool		bRead;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF8" _FS_ "Example4.txt");
	AssertTrue(bRead);
	sz.Init(cTextFile.Text());
	cTextFile.Kill();

	cUTF8.Init(&sz);
	uiPos = cUTF8.GetPosition();
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(0, uiPos);
	AssertSize(2, uiUTF8ElementLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF8.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(2, uiCodePointLength);
	AssertShort((uint16)322, *((uint16*)auiBuffer));

	uiPos = cUTF8.GetPosition();
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(2, uiPos);
	AssertSize(0x12, uiUTF8ElementLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF8.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(11, uiCodePointLength);
	psz = auiBuffer;
	AssertString("iô", (char*)psz);
	psz = (uint8*)NextString((char*)psz);
	AssertString("ûó", (char*)psz);
	psz = (uint8*)NextString((char*)psz);
	AssertString("°ù", (char*)psz);

	uiPos = cUTF8.GetPosition();
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(0x14, uiPos);
	AssertSize(2, uiUTF8ElementLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF8.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(2, uiCodePointLength);
	AssertShort((uint16)322, *((uint16*)auiBuffer));

	uiPos = cUTF8.GetPosition();
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(0x16, uiPos);
	AssertSize(0, uiUTF8ElementLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF8.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(0, uiCodePointLength);
	AssertString((char*)auiBuffer, "");
	cUTF8.Kill();

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF8Example5(void)
{
	CUTF8		cUTF8;
	CTextFile	cTextFile;
	CChars		sz;
	uint16		c16;
	uint32		c32;
	bool		bRead;
	size		uiUTF8ElementLength;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF8" _FS_ "Example5.txt");
	AssertTrue(bRead);
	sz.Init(cTextFile.Text());
	cTextFile.Kill();

	cUTF8.Init(&sz);

	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(3, uiUTF8ElementLength);
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0x2665, c32);
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(4, uiUTF8ElementLength);
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex(0x12219, c32);
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(1, uiUTF8ElementLength);
	c16 = cUTF8.GetCodePointUint16();
	AssertShortHex((uint16)0x0040, c16);
	uiUTF8ElementLength = cUTF8.PeekUTFBytes();
	AssertSize(0, uiUTF8ElementLength);

	cUTF8.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF8Example6(void)
{
	CUTF8		cUTF8;
	CTextFile	cTextFile;
	CChars		sz;
	uint32		c32;
	bool		bRead;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF8" _FS_ "Example6.txt");
	AssertTrue(bRead);
	sz.Init(cTextFile.Text());
	cTextFile.Kill();

	cUTF8.Init(&sz);

	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex(0x1F469, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex(0x1F3FB, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex(0x1F9B0, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertIntHex((uint32)0, c32);

	c32 = cUTF8.GetUTF8ElementUint32(0x1F469);
	c32 = ReverseIntEndianness(c32);
	AssertIntHex(0xF09F91A9, c32);
	c32 = cUTF8.GetUTF8ElementUint32(0x1F3FB);
	c32 = ReverseIntEndianness(c32);
	AssertIntHex(0xF09F8FBB, c32);
	c32 = cUTF8.GetUTF8ElementUint32(0x1F9B0);
	c32 = ReverseIntEndianness(c32);
	AssertIntHex(0xF09FA6B0, c32);
	c32 = cUTF8.GetUTF8ElementUint32(0);

	cUTF8.Kill();
	sz.Kill();
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF8BOMExample1(void)
{
	CUTF8		cUTF8;
	CTextFile	cTextFile;
	CChars		sz;
	uint16		c16;
	uint32		c32;
	bool		bRead;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF8BOM" _FS_ "Example1.txt");
	AssertTrue(bRead);
	sz.Init(cTextFile.Text());
	cTextFile.Kill();

	cUTF8.Init(&sz);

	AssertTrue(cUTF8.GetByteOrderMark());

	c16 = cUTF8.GetCodePointUint16();
	AssertChar('H', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('e', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('l', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('l', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar('o', (char)c16);
	c16 = cUTF8.GetCodePointUint16();
	AssertChar(' ', (char)c16);

	c16 = cUTF8.GetCodePointUint16();
	AssertShort((uint16)0xFFFD, c16);

	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x4E16, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x754C, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x1F43C, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x002D, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x20AC, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x00A9, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x1D7D9, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x1D7DA, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0x1D7DB, c32);
	c32 = cUTF8.GetCodePointUint32();
	AssertInt((uint32)0, c32);

	cUTF8.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF8(void)
{
	BeginTests();

	TestUTF8Example1();
	TestUTF8Example2();
	TestUTF8Example3();
	TestUTF8Example4();
	TestUTF8Example5();
	TestUTF8Example6();

	TestUTF8BOMExample1();

	TestStatistics();
}

