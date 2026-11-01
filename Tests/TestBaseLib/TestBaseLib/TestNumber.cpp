#include "BaseLib/NumberControl.h"
#include "BaseLib/TypeConverter.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestAdd(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber cNumber1;
	CNumber cNumber2;
	CNumber cNumber3;

	cNumber1.Init("123.45");
	cNumber2.Init("98.76");
	cNumber3.Init(cNumber1.Add(&cNumber2));
	AssertNumber("222.21", &cNumber3);
	AssertNumber("222.21", &cNumber1);
	AssertNumber("98.76", &cNumber2);

	AssertNumber("12345.6789", cNumber1.Init("12345")->Add(cNumber2.Init("0.6789")));
	AssertNumber("1.0", cNumber1.Init("0.5")->Add(cNumber2.Init("0.5")));
	AssertNumber("100", cNumber1.Init("99")->Add(cNumber2.Init("1")));
	AssertNumber("0", cNumber1.Init("20")->Add(cNumber2.Init("-20")));
	AssertNumber("-10", cNumber1.Init("20")->Add(cNumber2.Init("-30")));
	AssertNumber("10", cNumber1.Init("30")->Add(cNumber2.Init("-20")));
	AssertNumber("-10", cNumber1.Init("-30")->Add(cNumber2.Init("20")));
	AssertNumber("10", cNumber1.Init("-20")->Add(cNumber2.Init("30")));
	AssertNumber("0.5", cNumber1.Init("2.5")->Add(cNumber2.Init("-2")));
	AssertNumber("-0.5", cNumber1.Init("-2.5")->Add(cNumber2.Init("2")));

	//Zero's first digit is the units digit; numbers below one used to compare smaller than it.
	AssertNumber("-0.025", cNumber1.Init("-0.025")->Add(cNumber2.Init("0")));
	AssertNumber("0.025", cNumber1.Init("0.025")->Add(cNumber2.Init("0")));
	AssertNumber("-0.025", cNumber1.Init("0")->Add(cNumber2.Init("-0.025")));
	AssertNumber("-0.5", cNumber1.Init("-0.5")->Add(cNumber2.Init("0")));
	AssertNumber("0", cNumber1.Init("0")->Add(cNumber2.Init("0")));
	AssertNumber("-1.5", cNumber1.Init("0.5")->Add(cNumber2.Init("-2")));

	CNumber* pcNumber1;
	CNumber* pcNumber2;

	pcNumber1 = gcNumberControl.Add(2, 6);
	pcNumber2 = gcNumberControl.Add(2, 0);

	pcNumber1->Init("48", 2, 6);
	pcNumber2->Init("80", 2, 0);

	pcNumber1->Add(pcNumber2);
	AssertBool(true, pcNumber1->IsError());
	AssertBool(true, pcNumber1->IsOverflow());

	gcNumberControl.Remove(2);

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSubtract(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber cNumber1;
	CNumber cNumber2;
	CNumber cNumber3;

	cNumber1.Init("123");
	cNumber2.Init("97");
	cNumber3.Init(cNumber1.Subtract(&cNumber2));
	AssertNumber("26", &cNumber3);
	AssertNumber("26", &cNumber1);
	AssertNumber("97", &cNumber2);
	AssertNumber("1", cNumber1.Init("100")->Subtract(cNumber2.Init("99")));
	AssertNumber("12344.3211", cNumber1.Init("12345")->Subtract(cNumber2.Init("0.6789")));
	AssertNumber("0", cNumber1.Init("0.5")->Subtract(cNumber2.Init("0.5")));
	AssertNumber("-10", cNumber1.Init("20")->Subtract(cNumber2.Init("30")));
	AssertNumber("10", cNumber1.Init("30")->Subtract(cNumber2.Init("20")));
	AssertNumber("-10", cNumber1.Init("-30")->Subtract(cNumber2.Init("-20")));
	AssertNumber("10", cNumber1.Init("-20")->Subtract(cNumber2.Init("-30")));
	AssertNumber("0.5", cNumber1.Init("2.5")->Subtract(cNumber2.Init("2")));
	AssertNumber("-0.5", cNumber1.Init("2")->Subtract(cNumber2.Init("2.5")));

	AssertNumber("0.025", cNumber1.Init("0.025")->Subtract(cNumber2.Init("0")));
	AssertNumber("-0.025", cNumber1.Init("-0.025")->Subtract(cNumber2.Init("0")));
	AssertNumber("0.025", cNumber1.Init("0")->Subtract(cNumber2.Init("-0.025")));
	AssertNumber("-0.025", cNumber1.Init("0")->Subtract(cNumber2.Init("0.025")));

	AssertInt(0, gcNumberControl.NumElements());
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGreaterThan(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber cNumber1;
	CNumber cNumber2;

	AssertBool(true, cNumber1.Init("123")->GreaterThan(cNumber2.Init("122")));
	AssertBool(false, cNumber1.Init("23")->GreaterThan(cNumber2.Init("122")));
	AssertBool(true, cNumber1.Init("123")->GreaterThan(cNumber2.Init("22")));
	AssertBool(true, cNumber1.Init("123.5")->GreaterThan(cNumber2.Init("123")));
	AssertBool(false, cNumber1.Init("123")->GreaterThan(cNumber2.Init("123.5")));
	AssertBool(true, cNumber1.Init("1")->GreaterThan(cNumber2.Init("0")));
	AssertBool(false, cNumber1.Init("0")->GreaterThan(cNumber2.Init("1")));
	AssertBool(true, cNumber1.Init("1")->GreaterThan(cNumber2.Init("-1")));
	AssertBool(false, cNumber1.Init("-1")->GreaterThan(cNumber2.Init("1")));
	AssertBool(true, cNumber1.Init("456")->GreaterThan(cNumber2.Init("-7921")));
	AssertBool(false, cNumber1.Init("-7921")->GreaterThan(cNumber2.Init("456")));
	AssertBool(true, cNumber1.Init("-456")->GreaterThan(cNumber2.Init("-456.34")));

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGreaterThanOrEquals(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber cNumber1;
	CNumber cNumber2;

	AssertBool(true, cNumber1.Init("123")->GreaterThanOrEquals(cNumber2.Init("122")));
	AssertBool(true, cNumber1.Init("123")->GreaterThanOrEquals(cNumber2.Init("123")));
	AssertBool(false, cNumber1.Init("23")->GreaterThanOrEquals(cNumber2.Init("122")));
	AssertBool(true, cNumber1.Init("123")->GreaterThanOrEquals(cNumber2.Init("22")));
	AssertBool(true, cNumber1.Init("123.5")->GreaterThanOrEquals(cNumber2.Init("123")));
	AssertBool(false, cNumber1.Init("123")->GreaterThanOrEquals(cNumber2.Init("123.5")));
	AssertBool(true, cNumber1.Init("0")->GreaterThanOrEquals(cNumber2.Init("0")));
	AssertBool(true, cNumber1.Init("1")->GreaterThanOrEquals(cNumber2.Init("-1")));
	AssertBool(false, cNumber1.Init("-1")->GreaterThanOrEquals(cNumber2.Init("1")));
	AssertBool(true, cNumber1.Init("456")->GreaterThanOrEquals(cNumber2.Init("-7921")));
	AssertBool(false, cNumber1.Init("-7921")->GreaterThanOrEquals(cNumber2.Init("456")));
	AssertBool(true, cNumber1.Init("-456")->GreaterThanOrEquals(cNumber2.Init("-456.34")));
	AssertBool(true, cNumber1.Init("-99999.9999")->GreaterThanOrEquals(cNumber2.Init("-99999.9999")));

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMultiply(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber	cNumber1;
	CNumber	cNumber2;

	AssertNumber("4.6512088272417014", cNumber1.Init("2.2360679774997896")->Multiply(cNumber2.Init("2.0800838230519041")));

	AssertNumber("3", cNumber1.Init("6")->Multiply(cNumber2.Init("0.5")));
	AssertNumber("0", cNumber1.Init("0.000000001")->Multiply(cNumber2.Init("0.00000001")));
	AssertBool(true, cNumber1.IsUnderflow());

	AssertNumber("6", cNumber1.Init("2")->Multiply(cNumber2.Init("3")));
	AssertNumber("1", cNumber1.Init("1000")->Multiply(cNumber2.Init("0.001")));
	AssertNumber("891", cNumber1.Init("99")->Multiply(cNumber2.Init("9")));
	AssertNumber("891", cNumber1.Init("9")->Multiply(cNumber2.Init("99")));
	AssertNumber("99999999999999999999800000.0000000000000001", cNumber1.Init("9999999999999.99999999")->Multiply(cNumber2.Init("9999999999999.99999999")));

	cNumber1.Init("999999999999999.99999999")->Multiply(cNumber2.Init("9999999999999.99999999"));
	AssertBool(true, cNumber1.IsOverflow());
	AssertBool(false, cNumber1.IsUnderflow());

	AssertNumber("49", cNumber1.Init("7")->Multiply(cNumber2.Init("7")));
	AssertNumber("-49", cNumber1.Init("7")->Multiply(cNumber2.Init("-7")));
	AssertNumber("-49", cNumber1.Init("-7")->Multiply(cNumber2.Init("7")));
	AssertNumber("49", cNumber1.Init("-7")->Multiply(cNumber2.Init("-7")));
	AssertNumber("0.0000000000000001", cNumber1.Init("0.00000001")->Multiply(cNumber2.Init("0.00000001")));

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDivide(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber				cNumber1;
	CNumber				cNumber2;
	UNumber(26, 21)		uNumber3;
	UNumber(26, 21)		uNumber4;
	UNumber(26, 21)		uResult;

	AssertBool(true, uResult.c.Init("1.471517764685769286382", 26, 21)->Equals(uNumber3.c.Init("4", 26, 21)->Divide(uNumber4.c.Init("2.71828182845904523536", 26, 21))));
	AssertNumber("1.15672535316593280000", cNumber1.Init("11.5672535316593283")->Divide(cNumber2.Init("10")));
	AssertNumber("0.9999999999999999", cNumber1.Init("99999999999999999999")->Divide(cNumber2.Init("100000000000000000000")));
	AssertNumber("3.3333333333333333", cNumber1.Init("0.001")->Divide(cNumber2.Init("0.0003")));
	AssertNumber("0.005", cNumber1.Init("0.001")->Divide(cNumber2.Init("0.2")));
	AssertNumber("-9", cNumber1.Init("-27")->Divide(cNumber2.Init("3")));
	AssertNumber("-9", cNumber1.Init("27")->Divide(cNumber2.Init("-3")));
	AssertNumber("2", cNumber1.Init("88888888888888888888888888.8888888888888888")->Divide(cNumber2.Init("44444444444444444444444444.4444444444444444")));
	AssertNumber("2.001", cNumber1.Init("6.003")->Divide(cNumber2.Init("3")));
	AssertNumber("100000", cNumber1.Init("100")->Divide(cNumber2.Init("0.001")));
	AssertNumber("1", cNumber1.Init("1")->Divide(cNumber2.Init("1")));
	AssertNumber("1", cNumber1.Init("123.456")->Divide(cNumber2.Init("123.456")));
	AssertNumber("0.00001", cNumber1.Init("0.001")->Divide(cNumber2.Init("100")));
	AssertNumber("2", cNumber1.Init("6")->Divide(cNumber2.Init("3")));
	AssertNumber("25.5555555555555555", cNumber1.Init("230")->Divide(cNumber2.Init("9")));

	AssertNumber("1", cNumber1.Init("99999999999999999999999999.9999999999999999")->Divide(cNumber2.Init("99999999999999999999999999.9999999999999999")));
	AssertBool(false, cNumber1.IsOverflow());
	AssertBool(false, cNumber1.IsUnderflow());

	AssertNumber("3.1428571428571428", cNumber1.Init("22")->Divide(cNumber2.Init("7")));
	AssertBool(false, cNumber1.IsOverflow());
	AssertBool(true, cNumber1.IsUnderflow());

	cNumber1.Init("10000000000000000000")->Divide(cNumber2.Init("0.0000000000000001"));
	AssertBool(true, cNumber1.IsOverflow());

	cNumber1.Init("1")->Divide(cNumber2.Init("0"));
	AssertBool(true, cNumber1.IsDivisionByZero());

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestInit(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber cNumber;

	cNumber.Init(123);
	AssertInt(3, cNumber.GetFirstNonZeroDigit());
	AssertInt(1, cNumber.GetLastNonZeroDigit());
	AssertChar(1, cNumber.GetDigitUnsafe(3));
	AssertChar(2, cNumber.GetDigitUnsafe(2));
	AssertChar(3, cNumber.GetDigitUnsafe(1));

	cNumber.Init(6.5f);
	AssertNumber("6.5", &cNumber);

	cNumber.Init(0.065f);
	AssertNumber("0.064999998", &cNumber);

	cNumber.Init(768.32476f);
	AssertNumber("768.32477", &cNumber);

	cNumber.Init(768.0f);
	AssertNumber("768", &cNumber);

	cNumber.Init("99999999999999999999999999.9999999999999999");
	AssertInt(26, cNumber.GetFirstNonZeroDigit());
	AssertInt(-16, cNumber.GetLastNonZeroDigit());
	AssertBool(false, cNumber.IsNegative());
	AssertBool(false, cNumber.IsOverflow());
	AssertBool(false, cNumber.IsUnderflow());

	cNumber.Init("-99999999999999999999999999.9999999999999999");
	AssertInt(26, cNumber.GetFirstNonZeroDigit());
	AssertInt(-16, cNumber.GetLastNonZeroDigit());
	AssertBool(true, cNumber.IsNegative());

	cNumber.Init("199999999999999999999999999.9999999999999999");
	AssertInt(0, cNumber.GetFirstNonZeroDigit());
	AssertInt(0, cNumber.GetLastNonZeroDigit());
	AssertBool(false, cNumber.IsNegative());
	AssertBool(true, cNumber.IsOverflow());
	AssertBool(false, cNumber.IsUnderflow());

	cNumber.Init("99999999999999999999999999.99999999999999991");
	AssertInt(26, cNumber.GetFirstNonZeroDigit());
	AssertInt(-16, cNumber.GetLastNonZeroDigit());
	AssertBool(false, cNumber.IsNegative());
	AssertBool(false, cNumber.IsOverflow());
	AssertBool(true, cNumber.IsUnderflow());

	cNumber.Init("-001.23", 1, 2);
	AssertNumber("-1.23", &cNumber);
	AssertBool(true, cNumber.IsNegative());
	AssertBool(false, cNumber.IsOverflow());
	AssertBool(false, cNumber.IsUnderflow());

	cNumber.Init("091.23", 1, 2);
	AssertBool(false, cNumber.IsNegative());
	AssertBool(true, cNumber.IsOverflow());
	AssertBool(false, cNumber.IsUnderflow());

	cNumber.Init("-091.23", 1, 2);
	AssertBool(true, cNumber.IsNegative());
	AssertBool(true, cNumber.IsOverflow());
	AssertBool(false, cNumber.IsUnderflow());


	cNumber.Init("-001.2300", 1, 2);
	AssertNumber("-1.23", &cNumber);
	AssertBool(true, cNumber.IsNegative());
	AssertBool(false, cNumber.IsOverflow());
	AssertBool(false, cNumber.IsUnderflow());

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestConstants(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber		cTest;
	CNumber		cTwo;
	CNumber		cHalf;
	CNumber		cMinusTwo;
	CNumber		cMinusHalf;
	CNumber		cPi;
	CNumber		cE;
	CNumber		cMinusPi;
	CNumber		cMinusE;
	CNumber		cZero;

	cTwo.Two();
	AssertInt(1, cTwo.GetFirstNonZeroDigit());
	AssertInt(1, cTwo.GetLastNonZeroDigit());
	AssertChar(2, cTwo.GetDigitUnsafe(1));
	AssertChar(0, cTwo.GetDigitUnsafe(2));
	AssertChar(0, cTwo.GetDigitUnsafe(-1));
	AssertTrue(cTwo.IsPositive());

	cTest.Init("2");
	cTest.Equals(&cTwo);

	cHalf.Half();
	AssertInt(-1, cHalf.GetFirstNonZeroDigit());
	AssertInt(-1, cHalf.GetLastNonZeroDigit());
	AssertChar(5, cHalf.GetDigitUnsafe(-1));
	AssertChar(0, cHalf.GetDigitUnsafe(1));
	AssertChar(0, cHalf.GetDigitUnsafe(-2));
	AssertTrue(cHalf.IsPositive());

	cTest.Init("0.5");
	cTest.Equals(&cHalf);

	cMinusTwo.Two(false);
	AssertInt(1, cMinusTwo.GetFirstNonZeroDigit());
	AssertInt(1, cMinusTwo.GetLastNonZeroDigit());
	AssertChar(2, cMinusTwo.GetDigitUnsafe(1));
	AssertChar(0, cMinusTwo.GetDigitUnsafe(2));
	AssertChar(0, cMinusTwo.GetDigitUnsafe(-1));
	AssertTrue(cMinusTwo.IsNegative());

	cTest.Init("-2");
	cTest.Equals(&cMinusTwo);

	cMinusHalf.Half(false);
	AssertInt(-1, cMinusHalf.GetFirstNonZeroDigit());
	AssertInt(-1, cMinusHalf.GetLastNonZeroDigit());
	AssertChar(5, cMinusHalf.GetDigitUnsafe(-1));
	AssertChar(0, cMinusHalf.GetDigitUnsafe(1));
	AssertChar(0, cMinusHalf.GetDigitUnsafe(-2));
	AssertTrue(cMinusHalf.IsNegative());

	cTest.Init("-0.5");
	cTest.Equals(&cMinusHalf);

	cPi.Pi();
	AssertInt(1, cPi.GetFirstNonZeroDigit());
	AssertInt(-16, cPi.GetLastNonZeroDigit());
	AssertChar(0, cPi.GetDigitUnsafe(2));
	AssertChar(3, cPi.GetDigitUnsafe(1));
	AssertChar(1, cPi.GetDigitUnsafe(-1));
	AssertChar(4, cPi.GetDigitUnsafe(-2));
	AssertTrue(cPi.IsPositive());

	cTest.Init("3.1415926535897932");
	cTest.Equals(&cPi);

	cMinusPi.Pi(false);
	AssertInt(1, cMinusPi.GetFirstNonZeroDigit());
	AssertInt(-16, cMinusPi.GetLastNonZeroDigit());
	AssertChar(0, cMinusPi.GetDigitUnsafe(2));
	AssertChar(3, cMinusPi.GetDigitUnsafe(1));
	AssertChar(1, cMinusPi.GetDigitUnsafe(-1));
	AssertChar(4, cMinusPi.GetDigitUnsafe(-2));
	AssertTrue(cMinusPi.IsNegative());

	cTest.Init("-3.1415926535897932");
	cTest.Equals(&cMinusPi);
	
	cE.E();
	AssertInt(1, cE.GetFirstNonZeroDigit());
	AssertInt(-16, cE.GetLastNonZeroDigit());
	AssertChar(0, cE.GetDigitUnsafe(2));
	AssertChar(2, cE.GetDigitUnsafe(1));
	AssertChar(7, cE.GetDigitUnsafe(-1));
	AssertChar(1, cE.GetDigitUnsafe(-2));
	AssertTrue(cE.IsPositive());

	cTest.Init("2.7182818284590452");
	cTest.Equals(&cE);

	cMinusE.E(false);
	AssertInt(1, cMinusE.GetFirstNonZeroDigit());
	AssertInt(-16, cMinusE.GetLastNonZeroDigit());
	AssertChar(0, cMinusE.GetDigitUnsafe(2));
	AssertChar(2, cMinusE.GetDigitUnsafe(1));
	AssertChar(7, cMinusE.GetDigitUnsafe(-1));
	AssertChar(1, cMinusE.GetDigitUnsafe(-2));
	AssertTrue(cMinusE.IsNegative());

	cTest.Init("-2.7182818284590452");
	cTest.Equals(&cMinusE);

	cZero.Zero();
	AssertInt(1, cZero.GetFirstNonZeroDigit());
	AssertInt(1, cZero.GetLastNonZeroDigit());
	AssertChar(0, cZero.GetDigitUnsafe(2));
	AssertChar(0, cZero.GetDigitUnsafe(1));
	AssertChar(0, cZero.GetDigitUnsafe(-1));
	AssertTrue(cZero.IsPositiveOrZero());
	AssertFalse(cZero.IsPositive());
	AssertTrue(cZero.IsZero());
	AssertFalse(cZero.IsNegative());

	cTest.Init("0");
	cTest.Equals(&cZero);

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNumbersErrors(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber	cNumber1;
	CNumber	cNumber2;

	cNumber1.Init("1")->Divide(cNumber2.Init("0"));
	AssertBool(true, cNumber1.IsDivisionByZero());

	cNumber1.Multiply(cNumber2.Init("1"));
	AssertBool(true, cNumber1.IsDivisionByZero());
	AssertBool(false, cNumber2.IsError());

	cNumber2.Multiply(&cNumber1);
	AssertBool(true, cNumber1.IsDivisionByZero());
	AssertBool(true, cNumber2.IsError());

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerPower(void)
{
	CNumber	cNumber1;
	CNumber	cNumber2;

	AssertInt(0, gcNumberControl.NumElements());

	AssertNumber("  1         ",  cNumber1.Init(2)->Power(cNumber2.Init(0)));
	AssertNumber("  2         ",  cNumber1.Init(2)->Power(cNumber2.Init(1)));
	AssertNumber("  4         ",  cNumber1.Init(2)->Power(cNumber2.Init(2)));
	AssertNumber("  8         ",  cNumber1.Init(2)->Power(cNumber2.Init(3)));
	AssertNumber(" 16         ",  cNumber1.Init(2)->Power(cNumber2.Init(4)));
	AssertNumber(" 32         ",  cNumber1.Init(2)->Power(cNumber2.Init(5)));
	AssertNumber(" 64         ",  cNumber1.Init(2)->Power(cNumber2.Init(6)));
	AssertNumber("128         ",  cNumber1.Init(2)->Power(cNumber2.Init(7)));
	AssertNumber("256         ",  cNumber1.Init(2)->Power(cNumber2.Init(8)));
	AssertNumber("  0.5       ", cNumber1.Init(2)->Power(cNumber2.Init(-1)));
	AssertNumber("  0.25      ", cNumber1.Init(2)->Power(cNumber2.Init(-2)));
	AssertNumber("  0.125     ", cNumber1.Init(2)->Power(cNumber2.Init(-3)));
	AssertNumber("  0.0625    ", cNumber1.Init(2)->Power(cNumber2.Init(-4)));
	AssertNumber("  0.03125   ", cNumber1.Init(2)->Power(cNumber2.Init(-5)));
	AssertNumber("  0.015625  ", cNumber1.Init(2)->Power(cNumber2.Init(-6)));
	AssertNumber("  0.0078125 ", cNumber1.Init(2)->Power(cNumber2.Init(-7)));
	AssertNumber("  0.00390625", cNumber1.Init(2)->Power(cNumber2.Init(-8)));

	AssertNumber("    1                  ", cNumber1.Init(3)->Power(cNumber2.Init(0)));
	AssertNumber("    3                  ", cNumber1.Init(3)->Power(cNumber2.Init(1)));
	AssertNumber("    9                  ", cNumber1.Init(3)->Power(cNumber2.Init(2)));
	AssertNumber("   27                  ", cNumber1.Init(3)->Power(cNumber2.Init(3)));
	AssertNumber("   81                  ", cNumber1.Init(3)->Power(cNumber2.Init(4)));
	AssertNumber("  243                  ", cNumber1.Init(3)->Power(cNumber2.Init(5)));
	AssertNumber("  729                  ", cNumber1.Init(3)->Power(cNumber2.Init(6)));
	AssertNumber(" 2187                  ", cNumber1.Init(3)->Power(cNumber2.Init(7)));
	AssertNumber(" 6561                  ", cNumber1.Init(3)->Power(cNumber2.Init(8)));
	AssertNumber("    0.3333333333333333 ", cNumber1.Init(3)->Power(cNumber2.Init(-1)));
	AssertNumber("    0.1111111111111111 ", cNumber1.Init(3)->Power(cNumber2.Init(-2)));
	AssertNumber("    0.0370370370370370 ", cNumber1.Init(3)->Power(cNumber2.Init(-3)));
	AssertNumber("    0.0123456790123456 ", cNumber1.Init(3)->Power(cNumber2.Init(-4)));
	AssertNumber("    0.0041152263374485 ", cNumber1.Init(3)->Power(cNumber2.Init(-5)));
	AssertNumber("    0.0013717421124828 ", cNumber1.Init(3)->Power(cNumber2.Init(-6)));
	AssertNumber("    0.0004572473708276 ", cNumber1.Init(3)->Power(cNumber2.Init(-7)));
	AssertNumber("    0.0001524157902758 ", cNumber1.Init(3)->Power(cNumber2.Init(-8)));

	CNumber*	pcNumber1;

	AssertInt(0, gcNumberControl.NumElements());

	pcNumber1 = gcNumberControl.Add(1, 0);
	AssertNumber("1", pcNumber1->Init(2, 1, 0)->Power(cNumber2.Init(0)));
	AssertNumber("2", pcNumber1->Init(2, 1, 0)->Power(cNumber2.Init(1)));
	AssertNumber("4", pcNumber1->Init(2, 1, 0)->Power(cNumber2.Init(2)));
	AssertNumber("8", pcNumber1->Init(2, 1, 0)->Power(cNumber2.Init(3)));
	pcNumber1->Init(2, 1, 0)->Power(cNumber2.Init(4));
	AssertBool(true, pcNumber1->IsError());
	AssertBool(true, pcNumber1->IsOverflow());
	gcNumberControl.Remove();

	pcNumber1 = gcNumberControl.Add(2, 0);
	AssertNumber("16", pcNumber1->Init(2, 2, 0)->Power(cNumber2.Init(4)));
	AssertNumber("32", pcNumber1->Init(2, 2, 0)->Power(cNumber2.Init(5)));
	AssertNumber("64", pcNumber1->Init(2, 2, 0)->Power(cNumber2.Init(6)));
	pcNumber1->Init(2, 2, 0)->Power(cNumber2.Init(7));
	AssertBool(true, pcNumber1->IsError());
	AssertBool(true, pcNumber1->IsOverflow());
	gcNumberControl.Remove();

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPower(void)
{
	AssertInt(0, gcNumberControl.NumElements());

	CNumber	cNumber1;
	CNumber	cNumber2;

	AssertNumber(" 25", cNumber1.Init(5)->Power(cNumber2.Init(2)));
	AssertNumber("729", cNumber1.Init(9)->Power(cNumber2.Init(3)));
	AssertNumber("  8",	cNumber1.Init(2)->Power(cNumber2.Init("3")));
	AssertNumber("26482031752276.0277477348278374", cNumber1.Init("456.214")->Power(cNumber2.Init("5.0478")));
	AssertNumber("             2.2360679774997896", cNumber1.Init("5")->Power(cNumber2.Init("0.5")));
	AssertNumber("             2.0800838230519039", cNumber1.Init("9")->Power(cNumber2.Init("3")->Inverse()));
	AssertNumber("          1995.4606863463604380", cNumber1.Init("23")->Power(cNumber2.Init("2.42342345412")));
	AssertNumber("          7619.4156630113937693", cNumber1.Init("17.1933")->Power(cNumber2.Init("3.14234235451299")));
	AssertNumber("             1.1858287765828849", cNumber1.Init("234")->Power(cNumber2.Init("0.03124324234")));

	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRoot(void)
{
	CNumber	cNumber1;
	CNumber	cNumber2;

	AssertNumber("26482031752276.1186823146005396", cNumber1.Init("456.214")->Root(cNumber2.Init("0.1981061056301755")));
	AssertNumber("			  10.6919179362360959", cNumber1.Init("823478.1238912")->Root(cNumber2.Init("5.7486223")));
	AssertNumber("			  15.2451039371814556", cNumber1.Init("823478.1238912")->Root(cNumber2.Init("5.0000001")));
	AssertNumber("			  15.2451047678135269", cNumber1.Init("823478.1238912")->Root(cNumber2.Init("5")));
	AssertNumber("             2.2360679774997896", cNumber1.Init("5")->Root(cNumber2.Init("2")));
	AssertNumber("             2.0800838230519041", cNumber1.Init("9")->Root(cNumber2.Init("3")));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNaturalLogarithm(void)
{
	UNumber(26, 30)		uNumber1;

	AssertNumber(" 0.693147180559945309417232121458", uNumber1.c.Init(2, 26, 30)->NaturalLogarithm());
	AssertNumber("46.051701859880913680349829093687", uNumber1.c.Init("99999999999999999999", 26, 30)->NaturalLogarithm());
	AssertNumber("-0.693147180559945309417232121458", uNumber1.c.Init("0.5", 26, 30)->NaturalLogarithm());
	AssertNumber("59.832099999999959348204946348859", uNumber1.c.Init("96549687107865177125153989.7228030628", 26, 30)->NaturalLogarithm());
	AssertNumber("-0.761426021313239740524443075318", uNumber1.c.Init("0.467", 26, 30)->NaturalLogarithm());
	AssertNumber(" 0.607589292198298721215760207503", uNumber1.c.Init("1.836", 26, 30)->NaturalLogarithm());
	AssertNumber(" 0.210721023218417341672392615930", uNumber1.c.Init("1.2345678912379812", DEFAULT_WHOLE_NUMBERS, 30)->NaturalLogarithm());

	UNumber(26, 60)		uNumber2;

	AssertNumber(" 0.693147180559945309417232121458176568075500134360255254120680", uNumber2.c.Init(2, 26, 60)->NaturalLogarithm());
	AssertNumber("46.051701859880913680349829093687284152021979772575459520666557", uNumber2.c.Init("99999999999999999999", 26, 60)->NaturalLogarithm());
	AssertNumber("-0.693147180559945309417232121458176568075500134360255254120680", uNumber2.c.Init("0.5", 26, 60)->NaturalLogarithm());
	AssertNumber("59.832099999999959348204946348859395584510264049665522611580253", uNumber2.c.Init("96549687107865177125153989.7228030628", 26, 60)->NaturalLogarithm());
	AssertNumber("-0.761426021313239740524443075318823636652462475325747369485896", uNumber2.c.Init("0.467", 26, 60)->NaturalLogarithm());
	AssertNumber(" 0.607589292198298721215760207503964162878370488888184722642697", uNumber2.c.Init("1.836", 26, 60)->NaturalLogarithm());
	AssertNumber(" 0.210721023218417341672392615930853311767258874872887732536673", uNumber2.c.Init("1.2345678912379812", DEFAULT_WHOLE_NUMBERS, 60)->NaturalLogarithm());

	UNumber(26, 102)	uNumber3;

	AssertNumber(" 0.693147180559945309417232121458176568075500134360255254120680009493393621969694715605863326996418687542", uNumber3.c.Init(2, DEFAULT_WHOLE_NUMBERS, 102)->NaturalLogarithm());
	AssertNumber("46.051701859880913680349829093687284152021979772575459520666557686018118860213716268886610768458632633486", uNumber3.c.Init("99999999999999999999", DEFAULT_WHOLE_NUMBERS, 102)->NaturalLogarithm());
	AssertNumber("-0.693147180559945309417232121458176568075500134360255254120680009493393621969694715605863326996418687542", uNumber3.c.Init("0.5", DEFAULT_WHOLE_NUMBERS, 102)->NaturalLogarithm());
	AssertNumber("59.832099999999959348204946348859395584510264049665522611580253566029875921038191976969958986177991457508", uNumber3.c.Init("96549687107865177125153989.7228030628", DEFAULT_WHOLE_NUMBERS, 102)->NaturalLogarithm());
	AssertNumber("-0.761426021313239740524443075318823636652462475325747369485896032915659385841000090585731967273138949168", uNumber3.c.Init("0.467", DEFAULT_WHOLE_NUMBERS, 102)->NaturalLogarithm());
	AssertNumber(" 0.607589292198298721215760207503964162878370488888184722642697054878844870364090172036677171202741693940", uNumber3.c.Init("1.836", DEFAULT_WHOLE_NUMBERS, 102)->NaturalLogarithm());
	AssertNumber(" 0.210721023218417341672392615930853311767258874872887732536673787994647093653812636130792225811993635713", uNumber3.c.Init("1.2345678912379812", DEFAULT_WHOLE_NUMBERS, 102)->NaturalLogarithm());

}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntValue(void)
{
	CNumber	cNumber;

	AssertInt((int32)1, cNumber.Init("1")->IntValue());
	AssertInt((int32)432, cNumber.Init("432")->IntValue());
	AssertInt(-2001002003, cNumber.Init("-2001002003")->IntValue());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExponential(void)
{
	UNumber(26, 31)		uNumber2;

	AssertNumber("            26482031752276.1186823146005396637907899035099", uNumber2.c.Init("30.9074875718589175770492405427543", 26, 31)->Exponential());
	//Input has 31 decimal places to fit UNumber(26, 31).  Expected checked against a 90 digit calculation.
	//Use https://keisan.casio.com/calculator
	//2.7182818284590452353602874713526624 ^ 30.9074875718589175770492405427543
	Pass();
	              
	AssertNumber("96549687107869102043246792.8814870126009699798312397107804", uNumber2.c.Init("59.8321", 26, 31)->Exponential());
	Pass();

	AssertNumber("42012104037905142549565934.3071916176841111197329909678646", uNumber2.c.Init("59", 26, 31)->Exponential());
	AssertNumber("						  12.5535061366682314080320232000754", uNumber2.c.Init("2.53", 26, 31)->Exponential());
	AssertNumber("                        12.1824939607034734380701759511679", uNumber2.c.Init("2.5", 26, 31)->Exponential());
	AssertNumber("						  26.0849364446438480919234611552938", uNumber2.c.Init("3.261358", 26, 31)->Exponential());
	AssertNumber("						   4.4816890703380648226020554601192", uNumber2.c.Init("1.5", 26, 31)->Exponential());
	AssertNumber("						   1.6487212707001281468486507878141", uNumber2.c.Init("0.5", 26, 31)->Exponential());
	AssertNumber("						   7.3890560989306502272304274605750", uNumber2.c.Init("2", 26, 31)->Exponential());
	AssertNumber("						   2.7182818284590452353602874713526", uNumber2.c.Init("1", 26, 31)->Exponential());
	AssertNumber("						   1.0",							 uNumber2.c.Init("0", 26, 31)->Exponential());
	AssertNumber("						   0.3678794411714423215955237701614", uNumber2.c.Init("-1", 26, 31)->Exponential());
	AssertNumber("						   0.2231301601484298289332804707640", uNumber2.c.Init("-1.5", 26, 31)->Exponential());
	AssertNumber("						   0.1353352832366126918939994949724", uNumber2.c.Init("-2", 26, 31)->Exponential());
	AssertNumber("						   0.0820849986238987951695286744671", uNumber2.c.Init("-2.5", 26, 31)->Exponential());
	AssertNumber("						   0.0796590202858980257650549064864", uNumber2.c.Init("-2.53", 26, 31)->Exponential());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCopy(void)
{
	CNumber				cNumber1;
	UNumber(100, 60)	uNumber2;
	CNumber				cNumber3;
	UNumber(1, 10)		uNumber4;

	cNumber1.Init("10.56");
	uNumber2.c.Init("10.56", 100, 60);

	cNumber3.E();
	AssertNumber("2.7182818284590452", &cNumber3);
	AssertInt(1, cNumber3.GetFirstNonZeroDigit());
	AssertInt(-16, cNumber3.GetLastNonZeroDigit());
	AssertInt(26, cNumber3.GetMaxWholeNumbers());
	AssertInt(16, cNumber3.GetMaxDecimals());

	uNumber4.c.Copy(&cNumber3, 1, 10);
	AssertNumber("2.7182818284", &uNumber4.c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnderflow(void)
{
	CNumber				cNumber1;
	UNumber(4, 4)		uNumber2;

	cNumber1.Init("0.0000000000001");
	uNumber2.c.Copy(&cNumber1, 4, 4);
	AssertTrue(uNumber2.c.IsZero());
	AssertTrue(uNumber2.c.IsUnderflow());

	cNumber1.Init("0.00012345");
	uNumber2.c.Copy(&cNumber1, 4, 4);
	AssertNumber("0.0001", &uNumber2.c);
	AssertTrue(uNumber2.c.IsUnderflow());

	cNumber1.Init("0.001");
	cNumber1.Shift(-15);
	AssertTrue(cNumber1.IsZero());
	AssertTrue(cNumber1.IsUnderflow());

	cNumber1.Init("0.001");
	cNumber1.Shift(-13);
	AssertNumber("0.0000000000000001", &cNumber1);
	AssertFalse(cNumber1.IsUnderflow());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArithmeticGeometricMean(void)
{
	CNumber				cNumber1;
	CNumber				cNumber2;

	AssertNumber("   13.4581714817256152", cNumber1.Init(24)->ArithmeticGeometricMean(cNumber2.Init(6)));
	AssertNumber("43622.5572810216407774", cNumber1.Init("23536.0287471352662497")->ArithmeticGeometricMean(cNumber2.Init("69995.9574966967627724")));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPrivateSubtract(void)
{
	CNumber	cNumber1;
	CNumber	cNumber2;

	cNumber1.Init("10.02")->PrivateSubtract(cNumber2.Init("0.3322"));
	AssertNumber("9.6878", &cNumber1);

	cNumber1.Init("10.3322")->PrivateSubtract(cNumber2.Init("0.02"));
	AssertNumber("10.3122", &cNumber1);

	cNumber1.Init("10.02")->PrivateSubtract(cNumber2.Init("10.01"));
	AssertNumber("0.01", &cNumber1);

	cNumber1.Init("10.02")->PrivateSubtract(cNumber2.Init("10.02"));
	AssertNumber("0", &cNumber1);

	cNumber1.Init("100")->PrivateSubtract(cNumber2.Init("0.001"));
	AssertNumber("99.999", &cNumber1);

	cNumber1.Init("1")->PrivateSubtract(cNumber2.Init("0"));
	AssertNumber("1", &cNumber1);

	cNumber1.Init("12345")->PrivateSubtract(cNumber2.Init("0.6789"));
	AssertNumber("12344.3211", &cNumber1);

	//The last digit of this is a decimal and the last digit of the other is a whole number.
	cNumber1.Init("2.5")->PrivateSubtract(cNumber2.Init("2"));
	AssertNumber("0.5", &cNumber1);

	cNumber1.Init("10.5")->PrivateSubtract(cNumber2.Init("3"));
	AssertNumber("7.5", &cNumber1);

	cNumber1.Init("100.25")->PrivateSubtract(cNumber2.Init("100"));
	AssertNumber("0.25", &cNumber1);

	cNumber1.Init("432.001")->PrivateSubtract(cNumber2.Init("30"));
	AssertNumber("402.001", &cNumber1);

	CNumber		cNumber3;
	CNumber		cNumber4;

	cNumber3.Init("54.76", 2, 2)->PrivateSubtract(cNumber4.Init("5.448", 1, 3));
	AssertNumber("49.32", &cNumber3);
	AssertBool(false, cNumber3.IsOverflow());
	AssertBool(true, cNumber3.IsUnderflow());

	cNumber3.Init("540", 3, 0)->PrivateSubtract(cNumber4.Init("0.432", 0, 3));
	AssertNumber("540", &cNumber3);
	AssertBool(false, cNumber3.IsOverflow());
	AssertBool(true, cNumber3.IsUnderflow());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNumberDoubleValue(void)
{
	CNumber	cNumber;

	AssertDouble(11.344, cNumber.Init("11.344")->DoubleValue(), 6);
	AssertDouble(-11.344, cNumber.Init("-11.344")->DoubleValue(), 6);
	AssertDouble(-0.5, cNumber.Init("-0.5")->DoubleValue(), 6);
	AssertDouble(-1.0, cNumber.Init("-1")->DoubleValue(), 6);
	AssertDouble(0.0, cNumber.Init("0")->DoubleValue(), 6);
	AssertDouble(-64.77, cNumber.Init("-64.77")->DoubleValue(), 6);
	AssertFloat(-2.5f, cNumber.Init("-2.5")->FloatValue(), 4);
	AssertFloat(-0.25f, cNumber.Init("-0.25")->FloatValue(), 4);
	AssertFloat(3.75f, cNumber.Init("3.75")->FloatValue(), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// The digits are packed BCD with the whole numbers and decimals each
// rounded up to whole words: sizes that aren't multiples of four (spare
// nibbles at the top and below the smallest decimal), numbers of different
// sizes, carries and borrows through several words, shifts by each nibble
// count, and digits lost below the smallest decimal or carried past the
// biggest whole number.
//
//////////////////////////////////////////////////////////////////////////
void TestNumberPackedDigits(void)
{
	CNumber*	pcSmall;
	CNumber*	pcLarge;
	CNumber*	pcNine;
	CNumber*	pcOther;
	CNumber*	pcShifted;

	pcSmall = gcNumberControl.Add(5, 3);
	pcLarge = gcNumberControl.Add(7, 9);
	pcNine = gcNumberControl.Add(9, 9);
	pcOther = gcNumberControl.Add(9, 9);
	pcShifted = gcNumberControl.Add(9, 9);

	//A carry past the biggest whole number lands in the top word's spare nibbles.
	pcSmall->Init("99999", 5, 3);
	pcOther->Init("1", 5, 3);
	pcSmall->Add(pcOther);
	AssertBool(true, pcSmall->IsOverflow());
	pcSmall->Init("99999.999", 5, 3);
	pcLarge->Init("0.001", 7, 9);
	pcSmall->Add(pcLarge);
	AssertBool(true, pcSmall->IsOverflow());

	//A smaller number's decimals past this one's are lost, not carried.
	pcSmall->Init("12345.678", 5, 3);
	pcLarge->Init("0.0005", 7, 9);
	pcSmall->Add(pcLarge);
	AssertNumber("12345.678", pcSmall);
	AssertBool(true, pcSmall->IsUnderflow());
	pcSmall->Init("12345.678", 5, 3);
	pcLarge->Init("1234.5", 7, 9);
	pcSmall->Add(pcLarge);
	AssertNumber("13580.178", pcSmall);
	AssertBool(false, pcSmall->IsUnderflow());

	//Carries and borrows through every word.
	pcNine->Init("9999.9999", 9, 9);
	pcOther->Init("0.0001", 9, 9);
	pcNine->Add(pcOther);
	AssertNumber("10000", pcNine);
	pcNine->Init("100000000", 9, 9);
	pcOther->Init("0.000000001", 9, 9);
	pcNine->Subtract(pcOther);
	AssertNumber("99999999.999999999", pcNine);
	pcNine->Add(pcOther);
	AssertNumber("100000000", pcNine);

	//Shifts by one to five digits, down (losing the smallest decimals) and up.
	pcNine->Init("123456789.123456789", 9, 9);
	pcShifted->Init(pcNine, 9, 9);
	AssertNumber("12345678.912345678", pcShifted->Shift(-1));
	AssertBool(true, pcShifted->IsUnderflow());
	pcShifted->Init(pcNine, 9, 9);
	AssertNumber("1234567.891234567", pcShifted->Shift(-2));
	pcShifted->Init(pcNine, 9, 9);
	AssertNumber("123456.789123456", pcShifted->Shift(-3));
	pcShifted->Init(pcNine, 9, 9);
	AssertNumber("12345.678912345", pcShifted->Shift(-4));
	pcShifted->Init(pcNine, 9, 9);
	AssertNumber("1234.567891234", pcShifted->Shift(-5));
	AssertNumber("12345.6789", pcShifted->Init("1234.56789", 9, 9)->Shift(1));
	AssertNumber("123456.789", pcShifted->Init("1234.56789", 9, 9)->Shift(2));
	AssertNumber("1234567.89", pcShifted->Init("1234.56789", 9, 9)->Shift(3));
	AssertNumber("12345678.9", pcShifted->Init("1234.56789", 9, 9)->Shift(4));
	AssertNumber("123456789", pcShifted->Init("1234.56789", 9, 9)->Shift(5));
	AssertBool(false, pcShifted->IsUnderflow());
	AssertNumber("0.000000123", pcShifted->Init("1234.56789", 9, 9)->Shift(-10));
	AssertBool(true, pcShifted->IsUnderflow());

	//Products and quotients cut at the smallest decimal.
	pcSmall->Init("1.11", 5, 3);
	AssertNumber("1.232", pcSmall->Multiply(pcOther->Init("1.11", 5, 3)));
	AssertBool(true, pcSmall->IsUnderflow());
	pcSmall->Init("0.111", 5, 3);
	AssertNumber("0.001", pcSmall->Multiply(pcOther->Init("0.011", 5, 3)));
	pcSmall->Init("123.45", 5, 3);
	AssertNumber("83810.205", pcSmall->Multiply(pcOther->Init("678.9", 5, 3)));
	AssertBool(false, pcSmall->IsUnderflow());
	pcSmall->Init("1", 5, 3);
	AssertNumber("0.333", pcSmall->Divide(pcOther->Init("3", 5, 3)));
	AssertBool(true, pcSmall->IsUnderflow());
	pcSmall->Init("10", 5, 3);
	AssertNumber("2.5", pcSmall->Divide(pcOther->Init("4", 5, 3)));
	AssertBool(false, pcSmall->IsUnderflow());

	//Leading zeros past the biggest whole number, and trailing zeros past the smallest decimal, aren't lost digits.
	pcSmall->Init("0000012", 2, 0);
	AssertNumber("12", pcSmall);
	AssertBool(false, pcSmall->IsOverflow());
	pcSmall->Init("1.2000", 2, 1);
	AssertNumber("1.2", pcSmall);
	AssertBool(false, pcSmall->IsUnderflow());
	pcSmall->Init("1.25", 2, 1);
	AssertNumber("1.2", pcSmall);
	AssertBool(true, pcSmall->IsUnderflow());

	gcNumberControl.Remove(5);
	AssertInt(0, gcNumberControl.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNumber(void)
{
	BeginTests();

	TypeConverterInit();
	NumberInit();

	TestPrivateSubtract();
	TestNumberPackedDigits();
	TestNumberDoubleValue();
	TestInit();
	TestConstants();
	TestCopy();
	TestUnderflow();
	TestIntValue();
	TestGreaterThan();
	TestGreaterThanOrEquals();
	TestAdd();
	TestSubtract();
	TestMultiply();
	TestDivide();
	TestNumbersErrors();
	TestIntegerPower();
	TestArithmeticGeometricMean();
	TestNaturalLogarithm();
	TestExponential();
	TestRoot();
	TestPower();

	NumberKill();
	TypeConverterKill();

	TestStatistics();
}

