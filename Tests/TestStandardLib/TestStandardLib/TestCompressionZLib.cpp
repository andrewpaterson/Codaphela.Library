#include <string.h>
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/ArrayUint8.h"
#include "StandardLib/Adler32.h"
#include "StandardLib/CRC32.h"
#include "StandardLib/BitReader.h"
#include "StandardLib/BitWriter.h"
#include "StandardLib/Inflater.h"
#include "StandardLib/Deflater.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Inflates puiData into pacOutput (which must be initialised).
//
//////////////////////////////////////////////////////////////////////////
bool InflateMemory(uint8* puiData, lsize uiDataSize, bool bZLib, CArrayLargeUint8* pacOutput, lsize uiMaxOutput = INFLATER_UNLIMITED)
{
	CFileBasic	cFile;
	CInflater	cInflater;
	bool		bResult;

	cFile.Init(MemoryFile(puiData, uiDataSize));
	cFile.Open(EFM_Read);
	cInflater.Init(&cFile, bZLib, uiMaxOutput);
	bResult = cInflater.Inflate(pacOutput);
	cInflater.Kill();
	cFile.Close();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Deflates puiData into pcMemory (which must be initialised).
//
//////////////////////////////////////////////////////////////////////////
bool DeflateMemory(uint8* puiData, lsize uiDataSize, bool bZLib, CMemoryFile* pcMemory)
{
	CFileBasic	cFile;
	CDeflater	cDeflater;
	bool		bResult;

	cFile.Init(pcMemory);
	cFile.Open(EFM_Write_Create);
	bResult = cDeflater.Init(&cFile, bZLib);
	if (bResult)
	{
		bResult = cDeflater.Deflate(puiData, uiDataSize);
		cDeflater.Kill();
	}
	cFile.Close();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Deflates then inflates puiData and checks it comes back unchanged.
// Returns the compressed size.
//
//////////////////////////////////////////////////////////////////////////
lsize AssertDeflateRoundTrip(uint8* puiData, lsize uiDataSize, bool bZLib)
{
	CMemoryFile		cMemory;
	CArrayLargeUint8		acOutput;
	lsize			uiCompressedSize;

	cMemory.Init();
	AssertTrue(DeflateMemory(puiData, uiDataSize, bZLib, &cMemory));
	uiCompressedSize = cMemory.GetBufferSize();

	acOutput.Init();
	AssertTrue(InflateMemory((uint8*)cMemory.GetBufferPointer(), uiCompressedSize, bZLib, &acOutput));
	AssertSize(uiDataSize, acOutput.NumElements());
	if (uiDataSize > 0)
	{
		AssertMemory(puiData, acOutput.GetData(), uiDataSize);
	}
	acOutput.Kill();
	cMemory.Kill();
	return uiCompressedSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionChecksums(void)
{
	CAdler32	cAdler;
	CCRC32		cCRC;
	uint8		auiData[25600];
	int			i;

	for (i = 0; i < 25600; i++)
	{
		auiData[i] = (uint8)i;
	}

	cAdler.Init();
	AssertInt((uint32)1, cAdler.Get());
	cAdler.Update((uint8*)"Wikipedia", 9);
	AssertInt((uint32)0x11E60398, cAdler.Get());

	//Longer than the run that can be summed without reducing the sums.
	cAdler.Init();
	cAdler.Update(auiData, 25600);
	AssertInt((uint32)0x747FD0E0, cAdler.Get());

	cCRC.Init();
	AssertInt((uint32)0, cCRC.Get());
	cCRC.Update((uint8*)"123456789", 9);
	AssertInt((uint32)0xCBF43926, cCRC.Get());

	//Updating in parts is the same as all at once.
	cCRC.Init();
	cCRC.Update(auiData, 1000);
	cCRC.Update(&auiData[1000], 24600);
	AssertInt((uint32)0x130F4D3B, cCRC.Get());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionBits(void)
{
	CMemoryFile		cMemory;
	CFileBasic		cFile;
	CBitWriter		cWriter;
	CBitReader		cReader;
	uint8			auiExpected[] = { 0x8D, 0x1F, 0x01, 0xAB };
	uint32			uiValue;

	cMemory.Init();
	cFile.Init(&cMemory);
	AssertTrue(cFile.Open(EFM_Write_Create));
	cWriter.Init(&cFile);
	cWriter.WriteBits(1, 1);
	cWriter.WriteBits(2, 2);
	cWriter.WriteBits(0x3F1, 10);
	cWriter.AlignToByte();
	cWriter.WriteBits(1, 8);
	cWriter.WriteBytes(&auiExpected[3], 1);
	AssertTrue(cWriter.Flush());
	cWriter.Kill();
	AssertTrue(cFile.Close());

	//Least significant bit first, the last byte padded with zeros.
	AssertSize(4, cMemory.GetBufferSize());
	AssertMemory(auiExpected, cMemory.GetBufferPointer(), 4);

	AssertTrue(cFile.Open(EFM_Read));
	cReader.Init(&cFile);
	AssertTrue(cReader.ReadBit(&uiValue));
	AssertInt((uint32)1, uiValue);
	AssertTrue(cReader.ReadBits(2, &uiValue));
	AssertInt((uint32)2, uiValue);
	AssertTrue(cReader.ReadBits(10, &uiValue));
	AssertInt((uint32)0x3F1, uiValue);
	cReader.AlignToByte();
	AssertTrue(cReader.ReadBits(8, &uiValue));
	AssertInt((uint32)1, uiValue);
	AssertTrue(cReader.ReadBits(8, &uiValue));
	AssertInt((uint32)0xAB, uiValue);
	AssertFalse(cReader.ReadBit(&uiValue));
	cReader.Kill();
	AssertTrue(cFile.Close());
	cFile.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Streams written by zlib with a stored, a fixed and a dynamic block.
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionInflateZLib(void)
{
	CArrayLargeUint8		acOutput;
	const char*		szHello = "Hello Hello Hello Hello Codaphela";
	const char*		szLetters = "abbaadbabbabadcaabaababcbaabcaabacdbababcaacbaacac";

	uint8 auiStored[] = { 0x78, 0x01, 0x01, 0x21, 0x00, 0xde, 0xff, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x43, 0x6f, 0x64, 0x61, 0x70, 0x68, 0x65, 0x6c, 0x61, 0xc5, 0x3a, 0x0b, 0xd2 };
	uint8 auiFixed[] = { 0x78, 0x01, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0xf0, 0xc0, 0x20, 0x9d, 0xf3, 0x53, 0x12, 0x0b, 0x32, 0x52, 0x73, 0x12, 0x01, 0xc5, 0x3a, 0x0b, 0xd2 };
	uint8 auiDynamic[] = { 0x78, 0xda, 0x1d, 0x89, 0x89, 0x0d, 0x00, 0x00, 0x0c, 0x01, 0x67, 0x3d, 0xba, 0xff, 0x0c, 0x45, 0x22, 0xce, 0x83, 0x04, 0x27, 0x02, 0x71, 0x26, 0x1e, 0xc9, 0xc5, 0x9a, 0x7b, 0x2e, 0x77, 0x33, 0x7e, 0xe7, 0x4b, 0x13, 0x1c };

	acOutput.Init();
	AssertTrue(InflateMemory(auiStored, sizeof(auiStored), true, &acOutput));
	AssertSize(strlen(szHello), acOutput.NumElements());
	AssertMemory(szHello, acOutput.GetData(), strlen(szHello));
	acOutput.Kill();

	acOutput.Init();
	AssertTrue(InflateMemory(auiFixed, sizeof(auiFixed), true, &acOutput));
	AssertSize(strlen(szHello), acOutput.NumElements());
	AssertMemory(szHello, acOutput.GetData(), strlen(szHello));
	acOutput.Kill();

	acOutput.Init();
	AssertTrue(InflateMemory(auiDynamic, sizeof(auiDynamic), true, &acOutput));
	AssertSize(strlen(szLetters), acOutput.NumElements());
	AssertMemory(szLetters, acOutput.GetData(), strlen(szLetters));
	acOutput.Kill();

	//The output is appended to what is already in the array.
	acOutput.Init();
	acOutput.Add('>');
	AssertTrue(InflateMemory(auiFixed, sizeof(auiFixed), true, &acOutput));
	AssertSize(strlen(szHello) + 1, acOutput.NumElements());
	AssertInt('>', acOutput.GetValue(0));
	AssertMemory(szHello, &acOutput.GetData()[1], strlen(szHello));
	acOutput.Kill();

	//Exactly enough output is allowed, one byte less isn't.
	acOutput.Init();
	AssertTrue(InflateMemory(auiDynamic, sizeof(auiDynamic), true, &acOutput, strlen(szLetters)));
	acOutput.Kill();
	acOutput.Init();
	AssertFalse(InflateMemory(auiDynamic, sizeof(auiDynamic), true, &acOutput, strlen(szLetters) - 1));
	acOutput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Raw deflate data with no zlib header or checksum.
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionInflateRaw(void)
{
	CArrayLargeUint8		acOutput;

	//A literal 'A' then a match of length 3 at distance 1.
	uint8 auiOverlap[] = { 0x73, 0x04, 0x02, 0x00 };

	acOutput.Init();
	AssertTrue(InflateMemory(auiOverlap, sizeof(auiOverlap), false, &acOutput));
	AssertSize(4, acOutput.NumElements());
	AssertMemory("AAAA", acOutput.GetData(), 4);
	acOutput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Corrupt input must fail.
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionInflateBad(void)
{
	CArrayLargeUint8		acOutput;

	uint8 auiBadCheck[] = { 0x78, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01 };
	uint8 auiBadMethod[] = { 0x77, 0x09, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01 };
	uint8 auiBadWindow[] = { 0x88, 0x1c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01 };
	uint8 auiDictionary[] = { 0x78, 0x20, 0x00, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01 };
	uint8 auiBadAdler[] = { 0x78, 0x01, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0xf0, 0xc0, 0x20, 0x9d, 0xf3, 0x53, 0x12, 0x0b, 0x32, 0x52, 0x73, 0x12, 0x01, 0xc5, 0x3a, 0x0b, 0xd3 };
	uint8 auiTruncated[] = { 0x78, 0x01, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0xf0, 0xc0 };
	uint8 auiBlockType3[] = { 0x07, 0x00 };
	uint8 auiStoredLength[] = { 0x01, 0x05, 0x00, 0x00, 0x00, 0x41, 0x41, 0x41, 0x41, 0x41 };
	uint8 auiTooFar[] = { 0x03, 0x02, 0x00 };
	uint8 auiLengthSymbol286[] = { 0x1b, 0x03 };
	uint8 auiDistanceSymbol30[] = { 0x73, 0x04, 0x3e };
	uint8 auiEmpty[] = { 0 };

	acOutput.Init();
	AssertFalse(InflateMemory(auiBadCheck, sizeof(auiBadCheck), true, &acOutput));
	AssertFalse(InflateMemory(auiBadMethod, sizeof(auiBadMethod), true, &acOutput));
	AssertFalse(InflateMemory(auiBadWindow, sizeof(auiBadWindow), true, &acOutput));
	AssertFalse(InflateMemory(auiDictionary, sizeof(auiDictionary), true, &acOutput));
	AssertFalse(InflateMemory(auiBadAdler, sizeof(auiBadAdler), true, &acOutput));
	AssertFalse(InflateMemory(auiTruncated, sizeof(auiTruncated), true, &acOutput));
	AssertFalse(InflateMemory(auiEmpty, 0, true, &acOutput));
	AssertFalse(InflateMemory(auiBlockType3, sizeof(auiBlockType3), false, &acOutput));
	AssertFalse(InflateMemory(auiStoredLength, sizeof(auiStoredLength), false, &acOutput));
	AssertFalse(InflateMemory(auiTooFar, sizeof(auiTooFar), false, &acOutput));
	AssertFalse(InflateMemory(auiLengthSymbol286, sizeof(auiLengthSymbol286), false, &acOutput));
	AssertFalse(InflateMemory(auiDistanceSymbol30, sizeof(auiDistanceSymbol30), false, &acOutput));
	acOutput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionDeflate(void)
{
	uint8*			puiData;
	lsize			uiSize;
	lsize			i;
	uint32			uiRandom;
	CMemoryFile		cFirst;
	CMemoryFile		cSecond;
	uint8			auiEmptyZLib[] = { 0x78, 0x9C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01 };
	const char*		szText = "Deflate replaces repeated strings with references to an earlier copy of the same string.  Deflate replaces repeated strings.";

	puiData = (uint8*)malloc(200000);

	//Empty input is a single fixed block with only an end of block code.
	cFirst.Init();
	AssertTrue(DeflateMemory(puiData, 0, true, &cFirst));
	AssertSize(sizeof(auiEmptyZLib), cFirst.GetBufferSize());
	AssertMemory(auiEmptyZLib, cFirst.GetBufferPointer(), sizeof(auiEmptyZLib));
	cFirst.Kill();
	AssertDeflateRoundTrip(puiData, 0, true);
	AssertDeflateRoundTrip(puiData, 0, false);

	puiData[0] = 'X';
	AssertDeflateRoundTrip(puiData, 1, true);
	AssertDeflateRoundTrip(puiData, 1, false);

	uiSize = strlen(szText);
	memcpy(puiData, szText, uiSize);
	AssertTrue(AssertDeflateRoundTrip(puiData, uiSize, true) < uiSize);

	//Long runs use matches that overlap themselves.
	memset(puiData, 'z', 100000);
	AssertTrue(AssertDeflateRoundTrip(puiData, 100000, true) < 200);

	//Incompressible data is stored, which needs more than one stored block past 65535 bytes.
	uiRandom = 12345;
	for (i = 0; i < 200000; i++)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		puiData[i] = (uint8)(uiRandom >> 16);
	}
	uiSize = AssertDeflateRoundTrip(puiData, 200000, true);
	AssertTrue(uiSize > 200000);
	AssertTrue(uiSize < 200100);

	//Few distinct bytes with some repetition, spread over many blocks and the whole window.
	for (i = 0; i < 200000; i++)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		puiData[i] = (uint8)((uiRandom >> 16) & 3) + (uint8)((i / 5000) & 1);
	}
	AssertTrue(AssertDeflateRoundTrip(puiData, 200000, true) < 70000);
	AssertDeflateRoundTrip(puiData, 200000, false);

	//The same input always gives the same output.
	cFirst.Init();
	cSecond.Init();
	AssertTrue(DeflateMemory(puiData, 200000, true, &cFirst));
	AssertTrue(DeflateMemory(puiData, 200000, true, &cSecond));
	AssertSize(cFirst.GetBufferSize(), cSecond.GetBufferSize());
	AssertMemory(cFirst.GetBufferPointer(), cSecond.GetBufferPointer(), cFirst.GetBufferSize());
	cSecond.Kill();
	cFirst.Kill();

	free(puiData);
}


//////////////////////////////////////////////////////////////////////////
//
// The CRC-32 of puiData deflated, with its compressed size added to
// puiTotalSize.
//
//////////////////////////////////////////////////////////////////////////
uint32 DeflatedCRC(uint8* puiData, lsize uiDataSize, bool bZLib, lsize* puiTotalSize)
{
	CMemoryFile		cMemory;
	CCRC32			cCRC;

	cMemory.Init();
	AssertTrue(DeflateMemory(puiData, uiDataSize, bZLib, &cMemory));
	*puiTotalSize += cMemory.GetBufferSize();
	cCRC.Init();
	cCRC.Update((uint8*)cMemory.GetBufferPointer(), cMemory.GetBufferSize());
	cMemory.Kill();
	AssertDeflateRoundTrip(puiData, uiDataSize, bZLib);
	return cCRC.Get();
}


//////////////////////////////////////////////////////////////////////////
//
// Changes to how matches are found must not change which matches are
// found.  The expected output was written by the byte at a time match
// finder.
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionDeflateMatches(void)
{
	uint8*			puiData;
	lsize			i;
	lsize			uiLength;
	lsize			uiTotalSize;
	uint32			uiRandom;
	uint32			uiCRC;
	CCRC32			cCRCs;
	const char*		aszWords[] = { "the ", "a ", "tile ", "map ", "image ", "pixel ", "channel ", "opacity ", "blit ", "star ", "control ", "deflate ", "png ", "of ", "and ", "with " };

	puiData = (uint8*)malloc(200000);
	uiTotalSize = 0;
	cCRCs.Init();

	//Text made of repeated words, which gives many matches of many lengths.
	uiRandom = 1;
	i = 0;
	while (i < 60000)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		uiLength = strlen(aszWords[(uiRandom >> 16) & 15]);
		memcpy(&puiData[i], aszWords[(uiRandom >> 16) & 15], uiLength);
		i += uiLength;
	}
	uiCRC = DeflatedCRC(puiData, 60000, true, &uiTotalSize);
	cCRCs.Update((uint8*)&uiCRC, sizeof(uint32));

	//A pattern repeated for longer than the longest match, then broken every so often.
	for (i = 0; i < 100000; i++)
	{
		puiData[i] = (uint8)("abcdefg"[i % 7]);
		if ((i % 1009) == 0)
		{
			puiData[i] = 'X';
		}
	}
	uiCRC = DeflatedCRC(puiData, 100000, true, &uiTotalSize);
	cCRCs.Update((uint8*)&uiCRC, sizeof(uint32));

	//An image like gradient with noise in the low bits.
	for (i = 0; i < 100000; i++)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		puiData[i] = (uint8)(((i % 256) + (i / 1024)) & 0xFC) | (uint8)((uiRandom >> 16) & 3);
	}
	uiCRC = DeflatedCRC(puiData, 100000, false, &uiTotalSize);
	cCRCs.Update((uint8*)&uiCRC, sizeof(uint32));

	//Short inputs whose matches are cut short by the end of the data, either side of 8 bytes.
	for (uiLength = 1; uiLength <= 40; uiLength++)
	{
		for (i = 0; i < uiLength; i++)
		{
			puiData[i] = (uint8)("abcabcabcabdabcab"[i % 17]);
		}
		uiCRC = DeflatedCRC(puiData, uiLength, false, &uiTotalSize);
		cCRCs.Update((uint8*)&uiCRC, sizeof(uint32));
	}

	AssertLSize(79768, uiTotalSize);
	AssertInt((uint32)0xB8EF328D, cCRCs.Get());
	free(puiData);
}


//////////////////////////////////////////////////////////////////////////
//
// Codes longer than the decoder's lookup table and bytes after the end
// of the stream.
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionInflateCodes(void)
{
	uint8*			puiData;
	lsize			i;
	lsize			j;
	uint32			uiRandom;
	CMemoryFile		cMemory;
	CMemoryFile		cFollowed;
	CFileBasic		cFile;
	CInflater		cInflater;
	CArrayLargeUint8		acOutput;
	uint8			uiByte;
	uint8			auiFollowing[2] = { 0xA5, 0x5A };
	lsize			uiCompressedSize;
	bool			bResult;

	puiData = (uint8*)malloc(100000);

	//Symbols with halving frequencies need codes up to the longest allowed.  The data is random so there are few matches.
	uiRandom = 7;
	for (i = 0; i < 100000; i++)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		j = 0;
		while ((j < 20) && ((uiRandom >> (j + 8)) & 1))
		{
			j++;
		}
		puiData[i] = (uint8)(j * 7);
	}
	AssertDeflateRoundTrip(puiData, 100000, true);
	AssertDeflateRoundTrip(puiData, 100000, false);

	//Inflate reads no further than the end of the stream.
	for (j = 0; j < 2; j++)
	{
		cMemory.Init();
		AssertTrue(DeflateMemory(puiData, 3000, j == 0, &cMemory));
		uiCompressedSize = cMemory.GetBufferSize();

		cFollowed.Init();
		cFile.Init(&cFollowed);
		AssertTrue(cFile.Open(EFM_Write_Create));
		AssertTrue(cFile.WriteData(cMemory.GetBufferPointer(), uiCompressedSize));
		AssertTrue(cFile.WriteData(auiFollowing, 2));
		AssertTrue(cFile.Close());

		AssertTrue(cFile.Open(EFM_Read));
		acOutput.Init();
		cInflater.Init(&cFile, j == 0);
		AssertTrue(cInflater.Inflate(&acOutput));
		cInflater.Kill();
		AssertSize(3000, acOutput.NumElements());
		AssertMemory(puiData, acOutput.GetData(), 3000);
		AssertTrue(cFile.ReadInt8(&uiByte));
		AssertInt(0xA5, uiByte);
		acOutput.Kill();
		AssertTrue(cFile.Close());
		cFile.Kill();
		cFollowed.Kill();

		//Every truncation of the stream fails.
		bResult = false;
		for (i = 0; i < uiCompressedSize; i++)
		{
			acOutput.Init();
			bResult |= InflateMemory((uint8*)cMemory.GetBufferPointer(), i, j == 0, &acOutput);
			acOutput.Kill();
		}
		AssertFalse(bResult);
		cMemory.Kill();
	}

	free(puiData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionZLib(void)
{
	BeginTests();
	MemoryInit();

	TestCompressionChecksums();
	TestCompressionBits();
	TestCompressionInflateZLib();
	TestCompressionInflateRaw();
	TestCompressionInflateBad();
	TestCompressionDeflate();
	TestCompressionDeflateMatches();
	TestCompressionInflateCodes();

	MemoryKill();
	TestStatistics();
}
