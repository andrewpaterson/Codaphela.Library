#include "BaseLib/TextParser.h"
#include "BaseLib/Numbers.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void AssertInteger(TRISTATE tResult, uint64 ulliActualValue, uint64 ulliExpectedValue, int iActualBase, int iExpectedBase, int iActualSuffix, int bExpectediSuffix, int iActualNumDigits, int iExpectedNumDigits)
{
	AssertTristate(TRITRUE, tResult);
	AssertInt(iExpectedBase, iActualBase);
	AssertInt(bExpectediSuffix, iActualSuffix);
	AssertInt(iExpectedNumDigits, iActualNumDigits);
	AssertLong(ulliExpectedValue, ulliActualValue);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseInteger(char* sz, uint64 * pulli, uint16* piBase, uint16* piSuffix, uint16* piNumDigits)
{
	CTextParser		cParser;
	TRISTATE		tResult;

	cParser.Init(sz);
	tResult = cParser.GetIntegerLiteral(pulli, INTEGER_PREFIX_ALL, piBase, INTEGER_SUFFIX_CPP, piSuffix, NUMBER_SEPARATOR_APOSTROPHE, piNumDigits, false);
	cParser.Kill();

	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetIntegerSuffixes(void)
{
	TRISTATE	tResult;
	uint64		ulliValue;
	uint16		iBase;
	uint16		iNumDigits;
	uint16		iSuffix;

	tResult = ParseInteger("0", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("1", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("9", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("10", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("0L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("1L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("9L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("10L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	tResult = ParseInteger("0;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("1;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("9;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("10;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("0L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("1L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("9L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("10L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	tResult = ParseInteger("0LL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_LL, iNumDigits, 1);

	tResult = ParseInteger("1LL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_LL, iNumDigits, 1);

	tResult = ParseInteger("9LL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_LL, iNumDigits, 1);

	tResult = ParseInteger("10LL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_LL, iNumDigits, 2);

	tResult = ParseInteger("0LL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_LL, iNumDigits, 1);

	tResult = ParseInteger("1LL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_LL, iNumDigits, 1);

	tResult = ParseInteger("9LL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_LL, iNumDigits, 1);

	tResult = ParseInteger("10LL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_LL, iNumDigits, 2);

	tResult = ParseInteger("0ULL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_ULL, iNumDigits, 1);

	tResult = ParseInteger("1ULL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_ULL, iNumDigits, 1);

	tResult = ParseInteger("9ULL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_ULL, iNumDigits, 1);

	tResult = ParseInteger("10ULL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_ULL, iNumDigits, 2);

	tResult = ParseInteger("0ULL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_ULL, iNumDigits, 1);

	tResult = ParseInteger("1ULL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_ULL, iNumDigits, 1);

	tResult = ParseInteger("9ULL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_ULL, iNumDigits, 1);

	tResult = ParseInteger("10ULL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_ULL, iNumDigits, 2);

	tResult = ParseInteger("0UL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_UL, iNumDigits, 1);

	tResult = ParseInteger("1UL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_UL, iNumDigits, 1);

	tResult = ParseInteger("9UL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_UL, iNumDigits, 1);

	tResult = ParseInteger("10UL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_UL, iNumDigits, 2);

	tResult = ParseInteger("0UL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 10, iSuffix, INTEGER_SUFFIX_UL, iNumDigits, 1);

	tResult = ParseInteger("1UL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 10, iSuffix, INTEGER_SUFFIX_UL, iNumDigits, 1);

	tResult = ParseInteger("9UL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 9, iBase, 10, iSuffix, INTEGER_SUFFIX_UL, iNumDigits, 1);

	tResult = ParseInteger("10UL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 10, iBase, 10, iSuffix, INTEGER_SUFFIX_UL, iNumDigits, 2);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetIntegerHexadecimalPrefixes(void)
{
	TRISTATE	tResult;
	uint64		ulliValue;
	uint16		iBase;
	uint16		iNumDigits;
	uint16		iSuffix;

	tResult = ParseInteger("0x0", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0x1", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0x01", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("0xA", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0xA, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0xAB", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0xAB, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("0x0;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0x1;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0x01;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("0xA;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0xA, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0xAB;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0xAB, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("0x0L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0x1L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0x01L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	tResult = ParseInteger("0xAL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0xA, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0xABL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0xAB, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	tResult = ParseInteger("0x0L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0x1L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0x01L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	tResult = ParseInteger("0xAL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0xA, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0xABL;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0xAB, iBase, 16, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetIntegerBinaryPrefixes(void)
{
	TRISTATE	tResult;
	uint64		ulliValue;
	uint16		iBase;
	uint16		iNumDigits;
	uint16		iSuffix;

	tResult = ParseInteger("0b0", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 2, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0b1", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 2, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0b01", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 2, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("0b0;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 2, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0b1;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 2, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("0b01;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 2, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);
	
	tResult = ParseInteger("0b0L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 2, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0b1L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 2, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0b01L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 2, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	tResult = ParseInteger("0b0L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 2, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0b1L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 2, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("0b01L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 2, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetIntegerOctallPrefixes(void)
{
	TRISTATE	tResult;
	uint64		ulliValue;
	uint16		iBase;
	uint16		iNumDigits;
	uint16		iSuffix;

	tResult = ParseInteger("00", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("01", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("001", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("07", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 07, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("067", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 067, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("00;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("01;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("001;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("07;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 07, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 1);

	tResult = ParseInteger("067;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 067, iBase, 8, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 2);

	tResult = ParseInteger("00L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("01L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("001L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	tResult = ParseInteger("07L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 07, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("067L", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 067, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	tResult = ParseInteger("00L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("01L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("001L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	tResult = ParseInteger("07L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 07, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 1);

	tResult = ParseInteger("067L;", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 067, iBase, 8, iSuffix, INTEGER_SUFFIX_L, iNumDigits, 2);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetIntegerSeparators(void)
{
	TRISTATE	tResult;
	uint64		ulliValue;
	uint16		iBase;
	uint16		iNumDigits;
	uint16		iSuffix;

	tResult = ParseInteger("1'000", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1000, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 4);

	tResult = ParseInteger("10'00", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1000, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 4);

	tResult = ParseInteger("100'0", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 1000, iBase, 10, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 4);

	tResult = ParseInteger("0xA'B'C'D", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, 0xABCD, iBase, 16, iSuffix, INTEGER_SUFFIX_NONE, iNumDigits, 4);

	long long int x = 0x3'0B'C'2DULL;  //3193901
	tResult = ParseInteger("0x3'0B'C'2DULL", &ulliValue, &iBase, &iSuffix, &iNumDigits);
	AssertInteger(tResult, ulliValue, x, iBase, 16, iSuffix, INTEGER_SUFFIX_ULL, iNumDigits, 6);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetInteger(void)
{
	BeginTests();

	TestJavaParserGetIntegerSuffixes();
	TestJavaParserGetIntegerHexadecimalPrefixes();
	TestJavaParserGetIntegerBinaryPrefixes();
	TestJavaParserGetIntegerOctallPrefixes();
	TestJavaParserGetIntegerSeparators();

	TestStatistics();
}

