#include "BaseLib/TextFile.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/ArrayUint16.h"
#include "BaseLib/UTF16.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF16Example1(void)
{
	CUTF16			cUTF16;
	CTextFile		cTextFile;
	uint16			c16;
	uint32			c32;
	bool			bRead;
	CArrayUint16	aui;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF16LE" _FS_ "Example1.txt");
	AssertTrue(bRead);
	aui.Init(&gcSystemAllocator, (uint16*)cTextFile.Text(), cTextFile.Length() / sizeof(uint16));
	cTextFile.Kill();

	cUTF16.Init(aui.GetData(), aui.NumElements());

	AssertTrue(cUTF16.GetByteOrderMark());

	c16 = cUTF16.GetCodePointUint16();
	AssertChar('H', (char)c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertChar('e', (char)c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertChar('l', (char)c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertChar('l', (char)c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertChar('o', (char)c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertChar(' ', (char)c16);

	c16 = cUTF16.GetCodePointUint16();
	AssertShort((uint16)0x4E16, c16);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x754C, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x1F43C, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x002D, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x20AC, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x00A9, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x0020, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x1D7D9, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x1D7DA, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x1D7DB, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0, c32);

	cUTF16.Kill();
	aui.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF16Example2(void)
{
	CUTF16			cUTF16;
	CTextFile		cTextFile;
	CArrayUint16	aui;
	uint16			c16;
	uint32			c32;
	bool			bRead;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF16LE" _FS_ "Example2.txt");
	AssertTrue(bRead);
	aui.Init(&gcSystemAllocator, (uint16*)cTextFile.Text(), cTextFile.Length() / sizeof(uint16));
	cTextFile.Kill();

	cUTF16.Init(aui.GetData(), aui.NumElements());

	AssertTrue(cUTF16.GetByteOrderMark());

	c32 = cUTF16.GetCodePointUint32();
	AssertInt((uint32)0x1F993, c32);
	c16 = cUTF16.GetCodePointUint16();
	AssertChar('À', (char)c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertChar('é', (char)c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertShort((uint16)0x0142, c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertChar('ö', (char)c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertChar('ü', (char)c16);
	c16 = cUTF16.GetCodePointUint16();
	AssertShort((uint16)0x0000, c16);


	cUTF16.Kill();
	aui.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF16Example3(void)
{
	CUTF16			cUTF16;
	CTextFile		cTextFile;
	CArrayUint16	aui;
	uint32			c32;
	size			uiUTF16ByteLength;
	size			uiCodePointLength;
	size			uiPos;
	uint8			auiBuffer[0x13];
	uint8*			psz;
	bool			bRead;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF16LE" _FS_ "Example3.txt");
	AssertTrue(bRead);
	aui.Init(&gcSystemAllocator, (uint16*)cTextFile.Text(), cTextFile.Length() / sizeof(uint16));
	cTextFile.Kill();

	cUTF16.Init(aui.GetData(), aui.NumElements());

	AssertTrue(cUTF16.GetByteOrderMark());

	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x0041, c32);
	AssertSize(1, cUTF16.GetUnicodeCodePointLength(c32));
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x1F469, c32);
	AssertSize(3, cUTF16.GetUnicodeCodePointLength(c32));
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x200D, c32);
	AssertSize(2, cUTF16.GetUnicodeCodePointLength(c32));
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x1F3FB, c32);
	AssertSize(3, cUTF16.GetUnicodeCodePointLength(c32));
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x200D, c32);
	AssertSize(2, cUTF16.GetUnicodeCodePointLength(c32));
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x1F9B0, c32);
	AssertSize(3, cUTF16.GetUnicodeCodePointLength(c32));
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0, c32);
	cUTF16.Kill();

	cUTF16.Init(aui.GetData(), aui.NumElements());

	AssertTrue(cUTF16.GetByteOrderMark());

	uiPos = cUTF16.GetPosition();
	AssertSize(1, uiPos);
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(2, uiUTF16ByteLength);
	uiPos = cUTF16.GetPosition();
	AssertSize(1, uiPos);

	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x0041, c32);
	AssertSize(1, cUTF16.GetUnicodeCodePointLength(c32));

	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(16, uiUTF16ByteLength);
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x1F469, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x200D, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x1F3FB, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x200D, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0x1F9B0, c32);

	uiPos = cUTF16.GetPosition();
	AssertSize(10, uiPos);
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(0, uiUTF16ByteLength);

	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0, c32);
	cUTF16.Kill();

	cUTF16.Init(aui.GetData(), aui.NumElements());

	AssertTrue(cUTF16.GetByteOrderMark());

	uiPos = cUTF16.GetPosition();
	AssertSize(1, uiPos);
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(2, uiUTF16ByteLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF16.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(1, uiCodePointLength);
	AssertString("A", (char*)auiBuffer);

	uiPos = cUTF16.GetPosition();
	AssertSize(2, uiPos);
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(16, uiUTF16ByteLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF16.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(11, uiCodePointLength);
	psz = auiBuffer;
	AssertString("iô", (char*)psz);
	psz = (uint8*)NextString((char*)psz);
	AssertString("ûó", (char*)psz);
	psz = (uint8*)NextString((char*)psz);
	AssertString("°ù", (char*)psz);

	uiPos = cUTF16.GetPosition();
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(10, uiPos);
	AssertSize(0, uiUTF16ByteLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF16.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(0, uiCodePointLength);
	AssertString("", (char*)auiBuffer);
	cUTF16.Kill();

	aui.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF16Example4(void)
{
	CUTF16			cUTF16;
	CTextFile		cTextFile;
	CArrayUint16	aui;
	size			uiUTF16ByteLength;
	size			uiCodePointLength;
	size			uiPos;
	uint8			auiBuffer[0x13];
	uint8*			psz;
	bool			bRead;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF16LE" _FS_ "Example4.txt");
	AssertTrue(bRead);
	aui.Init(&gcSystemAllocator, (uint16*)cTextFile.Text(), cTextFile.Length() / sizeof(uint16));
	cTextFile.Kill();

	cUTF16.Init(aui.GetData(), aui.NumElements());
	
	AssertTrue(cUTF16.GetByteOrderMark());

	uiPos = cUTF16.GetPosition();
	AssertSize(1, uiPos);
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(2, uiUTF16ByteLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF16.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(2, uiCodePointLength);
	AssertShort((uint16)322, *((uint16*)auiBuffer));

	uiPos = cUTF16.GetPosition();
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(2, uiPos);
	AssertSize(16, uiUTF16ByteLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF16.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(11, uiCodePointLength);
	psz = auiBuffer;
	AssertString("iô", (char*)psz);
	psz = (uint8*)NextString((char*)psz);
	AssertString("ûó", (char*)psz);
	psz = (uint8*)NextString((char*)psz);
	AssertString("°ù", (char*)psz);

	uiPos = cUTF16.GetPosition();
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(10, uiPos);
	AssertSize(2, uiUTF16ByteLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF16.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(2, uiCodePointLength);
	AssertShort((uint16)322, *((uint16*)auiBuffer));

	uiPos = cUTF16.GetPosition();
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(11, uiPos);
	AssertSize(0, uiUTF16ByteLength);
	memset(auiBuffer, 0, 0x13);
	uiCodePointLength = cUTF16.GetCodePointMulti(auiBuffer, 0x12);
	AssertSize(0, uiCodePointLength);
	AssertString((char*)auiBuffer, "");
	cUTF16.Kill();

	aui.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF16Example5(void)
{
	CUTF16			cUTF16;
	CTextFile		cTextFile;
	uint16			c16;
	uint32			c32;
	bool			bRead;
	size			uiUTF16ByteLength;
	CArrayUint16	aui;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF16LE" _FS_ "Example5.txt");
	AssertTrue(bRead);
	aui.Init(&gcSystemAllocator, (uint16*)cTextFile.Text(), cTextFile.Length() / sizeof(uint16));
	cTextFile.Kill();

	cUTF16.Init(aui.GetData(), aui.NumElements());

	AssertTrue(cUTF16.GetByteOrderMark());

	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(2, uiUTF16ByteLength);
	c16 = cUTF16.GetCodePointUint16();
	AssertShortHex((uint16)0x2665, c16);
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(4, uiUTF16ByteLength);
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex(0x12219, c32);
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(2, uiUTF16ByteLength);
	c16 = cUTF16.GetCodePointUint16();
	AssertShortHex((uint16)0x0040, c16);
	uiUTF16ByteLength = cUTF16.PeekUTFBytes();
	AssertSize(0, uiUTF16ByteLength);

	cUTF16.Kill();
	aui.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 ReverseUTF16Endianness(uint32 s)
{
	SwapUint16((uint16*)&s, ((uint16*)&s) + 1);
	return s;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF16Example6(void)
{
	CUTF16			cUTF16;
	CTextFile		cTextFile;
	uint32			c32;
	bool			bRead;
	CArrayUint16	aui;

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF16LE" _FS_ "Example6.txt");
	AssertTrue(bRead);
	aui.Init(&gcSystemAllocator, (uint16*)cTextFile.Text(), cTextFile.Length() / sizeof(uint16));
	cTextFile.Kill();

	cUTF16.Init(aui.GetData(), aui.NumElements());

	AssertTrue(cUTF16.GetByteOrderMark());

	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex(0x1F469, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex(0x1F3FB, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex(0x1F9B0, c32);
	c32 = cUTF16.GetCodePointUint32();
	AssertIntHex((uint32)0, c32);

	c32 = cUTF16.GetUTF16ElementUint32(0x1F469);
	c32 = ReverseUTF16Endianness(c32);
	AssertIntHex(0xD83DDC69, c32);
	c32 = cUTF16.GetUTF16ElementUint32(0x1F3FB);
	c32 = ReverseUTF16Endianness(c32);
	AssertIntHex(0xD83CDFFB, c32);
	c32 = cUTF16.GetUTF16ElementUint32(0x1F9B0);
	c32 = ReverseUTF16Endianness(c32);
	AssertIntHex(0xD83EDDB0, c32);
	c32 = cUTF16.GetUTF16ElementUint32(0);

	cUTF16.Kill();
	aui.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUTF16(void)
{
	BeginTests();

	TestUTF16Example1();
	TestUTF16Example2();
	TestUTF16Example3();
	TestUTF16Example4();
	TestUTF16Example5();
	TestUTF16Example6();

	TestStatistics();
}

