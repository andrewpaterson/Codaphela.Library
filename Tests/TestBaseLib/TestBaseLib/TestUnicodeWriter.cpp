#include "BaseLib/TextFile.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/UnicodeWriter.h"
#include "BaseLib/UTF8.h"
#include "BaseLib/UTF16.h"
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
//
//////////////////////////////////////////////////////////////////////////
void TestUnicodeWriter(void)
{
	BeginTests();

	TestUnicodeWriterUTF16LEToUTF8();
	TestUnicodeWriterUTF8ToUTF16();

	TestStatistics();
}

