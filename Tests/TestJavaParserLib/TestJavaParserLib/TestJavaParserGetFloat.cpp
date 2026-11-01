#include "BaseLib/TextParser.h"
#include "BaseLib/Numbers.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void AssertFloating(TRISTATE tResult, long double ldfActualValue, long double ldfExpectedValue, int iBaseActual, int iBaseExpected, int iExponentActual, int iExponentExpected, int iSuffixActual, int iSuffixExpected, int iNumWholeDigitsActual, int iNumWholeDigitsExpected, int iNumDecimalDigitsActual, int iNumDecimalDigitsExpected, int iNumExponentDigitsActual, int iNumExponentDigitsExpected)
{
	AssertTristate(TRITRUE, tResult);
	AssertInt(iBaseExpected, iBaseActual);
	AssertInt(iExponentExpected, iExponentActual);
	AssertInt(iSuffixExpected, iSuffixActual);
	AssertInt(iNumWholeDigitsExpected, iNumWholeDigitsActual);
	AssertInt(iNumDecimalDigitsExpected, iNumDecimalDigitsActual);
	AssertInt(iNumExponentDigitsExpected, iNumExponentDigitsActual);
	AssertLongDouble(ldfExpectedValue, ldfActualValue, 10);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseFloat(char* sz, long double* pldf, uint16* piBase, uint16* piExponent, uint16* piSuffix, uint16* piNumWholeDigits, uint16* piNumDecimalDigits, uint16* piNumExponentDigits)
{
	CTextParser		cParser;
	TRISTATE		tResult;

	cParser.Init(sz);
	tResult = cParser.GetFloatLiteral(pldf, FLOAT_PREFIX_ALL, piBase, FLOAT_SUFFIX_CPP, piSuffix, FLOAT_EXPONENT_ALL, piExponent, NUMBER_SEPARATOR_APOSTROPHE, piNumWholeDigits, piNumDecimalDigits, piNumExponentDigits, false);
	cParser.Kill();

	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetFloatSuffixes(void)
{
	TRISTATE		tResult;
	long double		ldf;
	uint16			iBase;
	uint16			iNumWholeDigits;
	uint16			iNumDecimalDigits;
	uint16			iNumExponentDigits;
	uint16			iSuffix;
	uint16			iExponent;

	tResult = ParseFloat("0.f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 0);

	tResult = ParseFloat("1.f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 1.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 0);

	tResult = ParseFloat("0.f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 0);

	tResult = ParseFloat("0.0f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 0);

	tResult = ParseFloat("1.0f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 1.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 0);

	tResult = ParseFloat("0.0f;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 0);

	tResult = ParseFloat("1.0f;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 1.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 0);

	tResult = ParseFloat("1.", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 1.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 0);

	tResult = ParseFloat("0.", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 0);

	tResult = ParseFloat("1.;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 1.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 0);

	tResult = ParseFloat("0.;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 0);

	tResult = ParseFloat("0.0", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 0);

	tResult = ParseFloat("1.0", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 1.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 0);

	tResult = ParseFloat("0.0;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 0);

	tResult = ParseFloat("1.0;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 1.l, iBase, 10, iExponent, FLOAT_EXPONENT_DEFAULT, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 0);


	tResult = ParseFloat("0.e1f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 1);

	tResult = ParseFloat("1.e1f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 10.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 1);

	tResult = ParseFloat("0.e1f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 1);

	tResult = ParseFloat("0.0e1f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 1);

	tResult = ParseFloat("1.0e1f", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 10.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 1);

	tResult = ParseFloat("0.0e1f;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 1);

	tResult = ParseFloat("1.0e1f;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 10.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_F, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 1);

	tResult = ParseFloat("1.e1", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 10.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 1);

	tResult = ParseFloat("0.e1", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 1);

	tResult = ParseFloat("1.e1;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 10.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 1);

	tResult = ParseFloat("0.e1;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 0, iNumExponentDigits, 1);

	tResult = ParseFloat("0.0e1", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 1);

	tResult = ParseFloat("1.0e1", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 10.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 1);

	tResult = ParseFloat("0.0e1;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 0.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 1);

	tResult = ParseFloat("1.0e1;", &ldf, &iBase, &iExponent, &iSuffix, &iNumWholeDigits, &iNumDecimalDigits, &iNumExponentDigits);
	AssertFloating(tResult, ldf, 10.l, iBase, 10, iExponent, FLOAT_EXPONENT_DECIMAL, iSuffix, FLOAT_SUFFIX_NONE, iNumWholeDigits, 1, iNumDecimalDigits, 1, iNumExponentDigits, 1);


	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetFloat(void)
{
	BeginTests();

	TestJavaParserGetFloatSuffixes();

	TestStatistics();
}

