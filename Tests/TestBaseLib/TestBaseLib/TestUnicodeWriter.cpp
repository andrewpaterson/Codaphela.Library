#include "BaseLib/TextFile.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/UnicodeWriter.h"
#include "BaseLib/UTF8.h"
#include "BaseLib/UTF16.h"
#include "BaseLib/Latin1.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeWriterUTF16LEToUTF8(void)
{
	CFileUtil			cFileUtil;
	CNaiveFile			cDataFile;
	CArrayUint16		auiUTF16Souce;
	CArrayUint8			auiUTF8Dest;
	CChars				sz;
	bool				bRead;
	CUnicodeWriter		cWriter;
	CUTF16				cUTF16;
	int					iResult;
	bool				bWritten;
	size				uiCount;

	sz.Init();
	cFileUtil.AppendToPath(&sz, "Output");
	cFileUtil.AppendToPath(&sz, "UnicodeWriter");
	cFileUtil.RemoveDir(sz.Text());
	cFileUtil.MakeDir(sz.Text());
	sz.Kill();

	cDataFile.Init();
	bRead = cDataFile.Read("Input" _FS_ "UTF16LE" _FS_ "Long.txt");
	AssertTrue(bRead);
	auiUTF16Souce.Init(&gcSystemAllocator, (uint16*)cDataFile.Get(), (size)cDataFile.Length() / sizeof(uint16));
	cDataFile.Kill();

	auiUTF8Dest.Init();
	cUTF16.Init(auiUTF16Souce.GetData(), auiUTF16Souce.NumElements());
	cWriter.Init(&cUTF16, UE_UTF8, &auiUTF8Dest);

	AssertTrue(cWriter.ReadBOM());
	AssertTrue(cWriter.WriteBOM());
	
	uiCount = 0;
	for (;;)
	{
		iResult = cWriter.WriteCharacters(4);
		AssertFalse(iResult == 0);
		if (iResult == -1)
		{
			break;
		}
		uiCount++;
	}
	AssertTrue(iResult == -1);
	AssertSize(20, uiCount);

	cUTF16.Kill();

	cDataFile.Init(auiUTF8Dest.GetData(), auiUTF8Dest.ByteSize());
	bWritten = cDataFile.Write("Output" _FS_ "UnicodeWriter" _FS_ "Long8.txt");
	AssertTrue(bWritten);
	cDataFile.Kill();

	auiUTF8Dest.Kill();
	auiUTF16Souce.Kill();

	AssertFile("Input" _FS_ "UTF8" _FS_ "Long.txt", "Output" _FS_ "UnicodeWriter" _FS_ "Long8.txt")
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeWriterUTF8ToUTF16(void)
{
	CFileUtil			cFileUtil;
	CNaiveFile			cDataFile;
	CArrayUint8			auiUTF8Souce;
	CArrayUint16		auiUTF16Dest;
	CChars				sz;
	bool				bRead;
	CUnicodeWriter		cWriter;
	CUTF8				cUTF8;
	int					iResult;
	bool				bWritten;
	size				uiCount;

	sz.Init();
	cFileUtil.AppendToPath(&sz, "Output");
	cFileUtil.AppendToPath(&sz, "UnicodeWriter");
	cFileUtil.RemoveDir(sz.Text());
	cFileUtil.MakeDir(sz.Text());
	sz.Kill();

	cDataFile.Init();
	bRead = cDataFile.Read("Input" _FS_ "UTF8" _FS_ "Long.txt");
	AssertTrue(bRead);
	auiUTF8Souce.Init(&gcSystemAllocator, (uint8*)cDataFile.Get(), (size)cDataFile.Length());
	cDataFile.Kill();

	auiUTF16Dest.Init();
	cUTF8.Init(auiUTF8Souce.GetData(), auiUTF8Souce.NumElements());
	cWriter.Init(&cUTF8, UE_UTF16LE, &auiUTF16Dest);

	AssertFalse(cWriter.ReadBOM());
	AssertTrue(cWriter.WriteBOM());

	uiCount = 0;
	for (;;)
	{
		iResult = cWriter.WriteCharacters(4);
		AssertFalse(iResult == 0);
		if (iResult == -1)
		{
			break;
		}
		uiCount++;
	}
	AssertTrue(iResult == -1);
	AssertSize(13, uiCount);

	cUTF8.Kill();

	cDataFile.Init(auiUTF16Dest.GetData(), auiUTF16Dest.ByteSize());
	bWritten = cDataFile.Write("Output" _FS_ "UnicodeWriter" _FS_ "Long16LE.txt");
	AssertTrue(bWritten);
	cDataFile.Kill();

	auiUTF16Dest.Kill();
	auiUTF8Souce.Kill();

	AssertFile("Input" _FS_ "UTF16LE" _FS_ "Long.txt", "Output" _FS_ "UnicodeWriter" _FS_ "Long16LE.txt")
}


//////////////////////////////////////////////////////////////////////////
//
// Invalid text stops the writer with an error; it doesn't carry on past
// it or read beyond the text.
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeWriterInvalid(void)
{
	CArrayUint8			auiUTF8;
	CArrayUint16		auiUTF16;
	CUnicodeWriter		cWriter;
	CUTF8				cUTF8;
	CUTF16				cUTF16;
	uint16				auiLoneHigh[] = { 'a', 0xD800, 'b' };
	uint16				auiLoneLow[] = { 'a', 0xDC00, 'b' };
	uint8				auiBadContinuation[] = { 'a', 0xE2, 0x28, 0xA1, 'b' };  //A three byte character whose second byte isn't a continuation.
	uint8				auiCutShort[] = { 'a', 0xE2, 0x82 };  //A three byte character with only two.

	auiUTF8.Init();
	cUTF16.Init(auiLoneHigh, 3);
	cWriter.Init(&cUTF16, UE_UTF8, &auiUTF8);
	AssertInt(0, cWriter.WriteCharacters());
	cWriter.Kill();
	cUTF16.Kill();
	auiUTF8.Kill();

	auiUTF8.Init();
	cUTF16.Init(auiLoneLow, 3);
	cWriter.Init(&cUTF16, UE_UTF8, &auiUTF8);
	AssertInt(0, cWriter.WriteCharacters());
	cWriter.Kill();
	cUTF16.Kill();
	auiUTF8.Kill();

	auiUTF16.Init();
	cUTF8.Init(auiBadContinuation, 5);
	cWriter.Init(&cUTF8, UE_UTF16LE, &auiUTF16);
	AssertInt(0, cWriter.WriteCharacters());
	cWriter.Kill();
	cUTF8.Kill();
	auiUTF16.Kill();

	auiUTF16.Init();
	cUTF8.Init(auiCutShort, 3);
	cWriter.Init(&cUTF8, UE_UTF16LE, &auiUTF16);
	AssertInt(0, cWriter.WriteCharacters());
	cWriter.Kill();
	cUTF8.Kill();
	auiUTF16.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// DEL (0x7F) is ASCII like any other character below 0x80.
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeWriterDelete(void)
{
	CArrayUint16		auiUTF16;
	CUnicodeWriter		cWriter;
	CUTF8				cUTF8;

	auiUTF16.Init();
	cUTF8.Init("a\x7F" "b");
	cWriter.Init(&cUTF8, UE_UTF16LE, &auiUTF16);
	AssertInt(-1, cWriter.WriteCharacters());
	AssertSize(3, auiUTF16.NumElements());
	AssertInt('a', auiUTF16.GetData()[0]);
	AssertInt(0x7F, auiUTF16.GetData()[1]);
	AssertInt('b', auiUTF16.GetData()[2]);
	cWriter.Kill();
	cUTF8.Kill();
	auiUTF16.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes written, as text, for comparing.
//
//////////////////////////////////////////////////////////////////////////
static void AssertBytes(const char* szExpected, CArrayUint8* pauiActual)
{
	CChars	sz;

	sz.Init();
	if (pauiActual->NumElements() > 0)
	{
		sz.Append((const char*)pauiActual->GetData(), pauiActual->NumElements());
	}
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each Latin-1 byte is the code point of the same value.
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeWriterLatin1ToUTF(void)
{
	CArrayUint8			auiUTF8;
	CArrayUint16		auiUTF16;
	CUnicodeWriter		cWriter;
	CLatin1				cLatin1;
	uint8				auiLatin1[] = { 'a', 0xE9, 0xFF, 0x7F, 0xA0 };

	auiUTF8.Init();
	cLatin1.Init(auiLatin1, 5);
	cWriter.Init(&cLatin1, UE_UTF8, &auiUTF8);
	AssertFalse(cWriter.ReadBOM());
	AssertInt(-1, cWriter.WriteCharacters());
	AssertBytes("a\xC3\xA9\xC3\xBF\x7F\xC2\xA0", &auiUTF8);
	cWriter.Kill();
	cLatin1.Kill();
	auiUTF8.Kill();

	auiUTF16.Init();
	cLatin1.Init(auiLatin1, 5);
	cWriter.Init(&cLatin1, UE_UTF16LE, &auiUTF16);
	AssertInt(-1, cWriter.WriteCharacters());
	AssertSize(5, auiUTF16.NumElements());
	AssertInt('a', auiUTF16.GetData()[0]);
	AssertInt(0xE9, auiUTF16.GetData()[1]);
	AssertInt(0xFF, auiUTF16.GetData()[2]);
	AssertInt(0x7F, auiUTF16.GetData()[3]);
	AssertInt(0xA0, auiUTF16.GetData()[4]);
	cWriter.Kill();
	cLatin1.Kill();
	auiUTF16.Kill();

	auiUTF8.Init();
	cLatin1.Init("");
	cWriter.Init(&cLatin1, UE_UTF8, &auiUTF8);
	AssertInt(-1, cWriter.WriteCharacters());
	AssertSize(0, auiUTF8.NumElements());
	cWriter.Kill();
	cLatin1.Kill();
	auiUTF8.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A character Latin-1 doesn't have is a question mark, one for a
// character joined from several code points.  Invalid text is an error.
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeWriterUTFToLatin1(void)
{
	CArrayUint8			auiLatin1;
	CUnicodeWriter		cWriter;
	CUTF8				cUTF8;
	CUTF16				cUTF16;
	uint16				auiUTF16[] = { 'a', 0xD83D, 0xDE00, 0xFF, 0x20AC };  //a, a grinning face, y with diaeresis and the euro sign.
	uint8				auiBadUTF8[] = { 'a', 0xE2, 0x28, 0xA1 };

	//a, e acute, the euro sign, a man ZWJ a woman, z.
	auiLatin1.Init();
	cUTF8.Init("a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9z");
	cWriter.Init(&cUTF8, UE_Latin1, &auiLatin1);
	AssertInt(-1, cWriter.WriteCharacters());
	AssertBytes("a\xE9??z", &auiLatin1);
	cWriter.Kill();
	cUTF8.Kill();
	auiLatin1.Kill();

	auiLatin1.Init();
	cUTF16.Init(auiUTF16, 5);
	cWriter.Init(&cUTF16, UE_Latin1, &auiLatin1);
	AssertInt(-1, cWriter.WriteCharacters());
	AssertBytes("a?\xFF?", &auiLatin1);
	cWriter.Kill();
	cUTF16.Kill();
	auiLatin1.Kill();

	auiLatin1.Init();
	cUTF8.Init(auiBadUTF8, 4);
	cWriter.Init(&cUTF8, UE_Latin1, &auiLatin1);
	AssertInt(0, cWriter.WriteCharacters());
	cWriter.Kill();
	cUTF8.Kill();
	auiLatin1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeWriter(void)
{
	BeginTests();

	TestUnicodeWriterUTF16LEToUTF8();
	TestUnicodeWriterUTF8ToUTF16();
	TestUnicodeWriterInvalid();
	TestUnicodeWriterDelete();
	TestUnicodeWriterLatin1ToUTF();
	TestUnicodeWriterUTFToLatin1();

	TestStatistics();
}

