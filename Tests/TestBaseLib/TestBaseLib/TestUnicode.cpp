#include "BaseLib/UTF8.h"
#include "BaseLib/UTF16.h"
#include "BaseLib/ArrayUint16.h"
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/FileCompare.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeConvertUTF16ToUTF8(void)
{
	CUTF16				cUTF16;
	CTextFile			cTextFile;
	uint16				cCodePoint16;
	uint32				cCodePoint32;
	uint16				cUTF8Element16;
	uint32				cUTF8Element32;
	bool				bRead;
	CArrayUint16		auiUTF16Souce;
	EUnicodeEncoding	eEncoding;
	size				uiUTF16Length;
	size				uiUTF8Length;
	uint8				auiBuffer[0x13];
	uint8*				puiCodePoint;
	size				uiCodePointsLength;
	size				uiCodePointLength;
	CArrayUint8			auiUTF8Dest;
	bool				bSucceeded;
	CChars				sz;
	bool				bWritten;
	CFileUtil			cFileUtil;

	sz.Init();
	cFileUtil.AppendToPath(&sz, "Output");
	cFileUtil.AppendToPath(&sz, "UTF8");
	cFileUtil.RemoveDir(sz.Text());
	cFileUtil.MakeDir(sz.Text());
	sz.Kill();

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF16LE" _FS_ "Long.txt");
	AssertTrue(bRead);
	auiUTF16Souce.Init(&gcSystemAllocator, (uint16*)cTextFile.Text(), cTextFile.Length() / sizeof(uint16));
	cTextFile.Kill();

	cUTF16.Init(auiUTF16Souce.GetData(), auiUTF16Souce.NumElements());

	auiUTF8Dest.Init();

	bSucceeded = true;
	eEncoding = GetUnicodeEncoding(auiUTF16Souce.GetData(), auiUTF16Souce.ByteSize());
	AssertTrue(UE_UTF16LE == eEncoding);
	AssertTrue(cUTF16.GetByteOrderMark());
	uiUTF16Length = cUTF16.PeekUTFBytes();
	while (uiUTF16Length != 0)
	{
		if (uiUTF16Length == 1)
		{
			cCodePoint16 = cUTF16.GetCodePointUint16();
			uiUTF8Length = cUTF16.GetUTF8Length(cCodePoint16);
			if (uiUTF8Length <= 2)
			{
				cUTF8Element16 = cUTF16.GetUTF8ElementUint16((uint32)cCodePoint16);
				if (cUTF16.IsTooSmallOrError(cUTF8Element16))
				{
					bSucceeded = false;
					AssertTrue(bSucceeded);
					break;
				}

				if (uiUTF8Length == 1)
				{
					auiUTF8Dest.Push((uint8)cUTF8Element16);
				}
				else
				{
					auiUTF8Dest.InsertBlockAfterEnd((uint8*)&cUTF8Element16, uiUTF8Length);
				}
			}
			else if (uiUTF8Length <= 4)
			{
				cUTF8Element32 = cUTF16.GetUTF8ElementUint32((uint32)cCodePoint16);
				if (cUTF16.IsTooSmallOrError(cUTF8Element32))
				{
					bSucceeded = false;
					AssertTrue(bSucceeded);
					break;
				}

				if (uiUTF8Length == 1)
				{
					auiUTF8Dest.Push((uint8)cUTF8Element32);
				}
				else
				{
					auiUTF8Dest.InsertBlockAfterEnd((uint8*)&cUTF8Element32, uiUTF8Length);
				}
			}
		}
		else if (uiUTF16Length == 2)
		{
			cCodePoint32 = cUTF16.GetCodePointUint32();
			uiUTF8Length = cUTF16.GetUTF8Length(cCodePoint32);
			if (uiUTF8Length <= 2)
			{
				cUTF8Element16 = cUTF16.GetUTF8ElementUint16(cCodePoint32);
				if (cUTF16.IsTooSmallOrError(cUTF8Element16))
				{
					bSucceeded = false;
					AssertTrue(bSucceeded);
					break;
				}

				if (uiUTF8Length == 1)
				{
					auiUTF8Dest.Push((uint8)cUTF8Element16);
				}
				else
				{
					auiUTF8Dest.InsertBlockAfterEnd((uint8*)&cUTF8Element16, uiUTF8Length);
				}
			}
			else if (uiUTF8Length <= 4)
			{
				cUTF8Element32 = cUTF16.GetUTF8ElementUint32(cCodePoint32);
				if (cUTF16.IsTooSmallOrError(cUTF8Element32))
				{
					bSucceeded = false;
					AssertTrue(bSucceeded);
					break;
				}

				if (uiUTF8Length == 1)
				{
					auiUTF8Dest.Push((uint8)cUTF8Element32);
				}
				else
				{
					auiUTF8Dest.InsertBlockAfterEnd((uint8*)&cUTF8Element32, uiUTF8Length);
				}
			}
		}
		else
		{
			uiCodePointsLength = cUTF16.GetCodePointMulti(auiBuffer, 0x13);
			puiCodePoint = auiBuffer;
			for (;;)
			{
				uiCodePointLength = StrLen((char*)puiCodePoint);
				cCodePoint32 = cUTF16.MakeCodePointUint32FromBuffer(puiCodePoint, uiCodePointLength);
				uiUTF8Length = cUTF16.GetUTF8Length(cCodePoint32);
				cUTF8Element32 = cUTF16.GetUTF8ElementUint32(cCodePoint32);

				if (uiUTF8Length == 1)
				{
					auiUTF8Dest.Push((uint8)cUTF8Element32);
				}
				else
				{
					auiUTF8Dest.InsertBlockAfterEnd((uint8*)&cUTF8Element32, uiUTF8Length);
				}

				puiCodePoint = (uint8*)NextString((char*)puiCodePoint);

				if (uiCodePointLength + 1 >= uiCodePointsLength)
				{
					break;
				}
				auiUTF8Dest.InsertBlockAfterEnd(cUTF16.GetZWJBytes(UE_UTF8), cUTF16.GetZWJLength(UE_UTF8));
				uiCodePointsLength -= (uiCodePointLength + 1);
			}
		}

		uiUTF16Length = cUTF16.PeekUTFBytes();
	}
	AssertTrue(bSucceeded);

	auiUTF8Dest.Push(0);
	AssertSize(107, auiUTF8Dest.NumElements());

	cTextFile.Init((char*)auiUTF8Dest.GetData());
	bWritten = cTextFile.Write("Output" _FS_ "UTF8" _FS_ "Long.txt");
	AssertTrue(bWritten);
	cTextFile.Kill();

	AssertFile("Input" _FS_ "UTF8" _FS_ "Long.txt", "Output" _FS_ "UTF8" _FS_ "Long.txt")

	auiUTF8Dest.Kill();
	cUTF16.Kill();
	auiUTF16Souce.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeConvertUTF8ToUTF16(void)
{
	CUTF8				cUTF8;
	CTextFile			cTextFile;
	uint16				cCodePoint16;
	uint32				cCodePoint32;
	uint16				cUTF16Element16;
	uint32				cUTF16Element32;
	bool				bRead;
	EUnicodeEncoding	eEncoding;
	size				uiUTF8Length;
	size				uiUTF16Length;
	uint8				auiBuffer[0x13];
	uint8*				puiCodePoint;
	size				uiCodePointsLength;
	size				uiCodePointLength;
	CArrayUint16		auiUTF16Dest;
	bool				bSucceeded;
	CChars				sz;
	bool				bWritten;
	CFileUtil			cFileUtil;
	CNaiveFile			cDataFile;

	sz.Init();
	cFileUtil.AppendToPath(&sz, "Output");
	cFileUtil.AppendToPath(&sz, "UTF16LE");
	cFileUtil.RemoveDir(sz.Text());
	cFileUtil.MakeDir(sz.Text());
	sz.Kill();

	cTextFile.Init();
	bRead = cTextFile.Read("Input" _FS_ "UTF8" _FS_ "Long.txt");
	AssertTrue(bRead);
	sz.Init(cTextFile.Text());
	cTextFile.Kill();

	cUTF8.Init(&sz);
	auiUTF16Dest.Init();

	bSucceeded = true;
	eEncoding = GetUnicodeEncoding(sz.Text(), sz.Length());
	AssertTrue(UE_UTF8 == eEncoding);

	auiUTF16Dest.InsertBlockAfterEnd((uint16*)cUTF8.GetBOMBytes(UE_UTF16LE), cUTF8.GetBOMLength(UE_UTF16LE) / sizeof(uint16));
	
	uiUTF8Length = cUTF8.PeekUTFBytes();
	while (uiUTF8Length != 0)
	{
		if (uiUTF8Length == 1)
		{
			cCodePoint16 = cUTF8.GetCodePointUint16();
			uiUTF16Length = cUTF8.GetUTF16Length(cCodePoint16);
			if (uiUTF16Length == 1)
			{
				cUTF16Element16 = cUTF8.GetUTF16ElementUint16((uint32)cCodePoint16);
				if (cUTF8.IsTooSmallOrError(cUTF16Element16))
				{
					bSucceeded = false;
					AssertTrue(bSucceeded);
					break;
				}

				if (uiUTF16Length == 1)
				{
					auiUTF16Dest.Push(cUTF16Element16);
				}
				else
				{
					auiUTF16Dest.InsertBlockAfterEnd(&cUTF16Element16, uiUTF16Length);
				}
			}
			else if (uiUTF16Length == 2)
			{
				cUTF16Element32 = cUTF8.GetUTF16ElementUint32((uint32)cCodePoint16);
				if (cUTF8.IsTooSmallOrError(cUTF16Element32))
				{
					bSucceeded = false;
					AssertTrue(bSucceeded);
					break;
				}

				if (uiUTF16Length == 1)
				{
					auiUTF16Dest.Push((uint16)cUTF16Element32);
				}
				else
				{
					auiUTF16Dest.InsertBlockAfterEnd((uint16*)&cUTF16Element32, uiUTF16Length);
				}
			}
		}
		else if (uiUTF8Length == 2)
		{
			cCodePoint32 = cUTF8.GetCodePointUint32();
			uiUTF16Length = cUTF8.GetUTF16Length(cCodePoint32);
			if (uiUTF16Length <= 2)
			{
				cUTF16Element16 = cUTF8.GetUTF16ElementUint16(cCodePoint32);
				if (cUTF8.IsTooSmallOrError(cUTF16Element16))
				{
					bSucceeded = false;
					AssertTrue(bSucceeded);
					break;
				}

				if (uiUTF16Length == 1)
				{
					auiUTF16Dest.Push(cUTF16Element16);
				}
				else
				{
					auiUTF16Dest.InsertBlockAfterEnd(&cUTF16Element16, uiUTF16Length);
				}
			}
			else if (uiUTF16Length <= 4)
			{
				cUTF16Element32 = cUTF8.GetUTF16ElementUint32(cCodePoint32);
				if (cUTF8.IsTooSmallOrError(cUTF16Element32))
				{
					bSucceeded = false;
					AssertTrue(bSucceeded);
					break;
				}

				if (uiUTF16Length == 1)
				{
					auiUTF16Dest.Push((uint16)cUTF16Element32);
				}
				else
				{
					auiUTF16Dest.InsertBlockAfterEnd((uint16*)&cUTF16Element32, uiUTF16Length);
				}
			}
		}
		else
		{
			uiCodePointsLength = cUTF8.GetCodePointMulti(auiBuffer, 0x13);
			puiCodePoint = auiBuffer;
			for (;;)
			{
				uiCodePointLength = StrLen((char*)puiCodePoint);
				cCodePoint32 = cUTF8.MakeCodePointUint32FromBuffer(puiCodePoint, uiCodePointLength);
				uiUTF16Length = cUTF8.GetUTF16Length(cCodePoint32);
				cUTF16Element32 = cUTF8.GetUTF16ElementUint32(cCodePoint32);

				if (uiUTF16Length == 1)
				{
					auiUTF16Dest.Push((uint16)cUTF16Element32);
				}
				else
				{
					auiUTF16Dest.InsertBlockAfterEnd((uint16*)&cUTF16Element32, uiUTF16Length);
				}

				puiCodePoint = (uint8*)NextString((char*)puiCodePoint);

				if (uiCodePointLength + 1 >= uiCodePointsLength)
				{
					break;
				}
				auiUTF16Dest.InsertBlockAfterEnd((uint16*)cUTF8.GetZWJBytes(UE_UTF16LE), cUTF8.GetZWJLength(UE_UTF16LE) / sizeof(uint16));
				uiCodePointsLength -= (uiCodePointLength + 1);
			}
		}

		uiUTF8Length = cUTF8.PeekUTFBytes();
	}
	AssertTrue(bSucceeded);

	AssertSize(63, auiUTF16Dest.NumElements());

	cDataFile.Init(auiUTF16Dest.GetData(), auiUTF16Dest.ByteSize());
	bWritten = cDataFile.Write("Output" _FS_ "UTF16LE" _FS_ "Long.txt");
	AssertTrue(bWritten);
	cDataFile.Kill();

	AssertFile("Input" _FS_ "UTF16LE" _FS_ "Long.txt", "Output" _FS_ "UTF16LE" _FS_ "Long.txt")

	auiUTF16Dest.Kill();
	cUTF8.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnicode(void)
{
	BeginTests();

	TestUnicodeConvertUTF16ToUTF8();
	TestUnicodeConvertUTF8ToUTF16();

	TestStatistics();
}

