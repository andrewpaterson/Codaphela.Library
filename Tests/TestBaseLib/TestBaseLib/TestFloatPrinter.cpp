#include "BaseLib/FloatPrinter.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatPrinterFullRangeNineDigits(void)
{
	char	sz[1024];

	FloatToString(sz, 1024, 734.645348e10f, -1, true);
	AssertString("7.34645348e+12f", sz);

	FloatToString(sz, 1024, 1e12f, -1, true);
	AssertString("9.99999996e+11f", sz);

	FloatToString(sz, 1024, 1.01e12f, -1, true);
	AssertString("1.01e+12f", sz);

	FloatToString(sz, 1024, 1.25e+12f, -1, true);
	AssertString("1.24999998e+12f", sz);

	FloatToString(sz, 1024, 5873464534810.f, -1, true);
	AssertString("5.87346477e+12f", sz);

	FloatToString(sz, 1024, 587346453481.f, -1, true);
	AssertString("5.87346477e+11f", sz);

	FloatToString(sz, 1024, 58734645348.f, -1, true);
	AssertString("5.87346452e+10f", sz);

	FloatToString(sz, 1024, 5873464534.f, -1, true);
	AssertString("5.87346432e+9f", sz);

	FloatToString(sz, 1024, 587346453.f, -1, true);
	AssertString("587346432.f", sz);

	FloatToString(sz, 1024, 8346453.7f, -1, true);
	AssertString("8346453.5f", sz);

	FloatToString(sz, 1024, 2346453.7f, -1, true);
	AssertString("2346453.75f", sz);

	FloatToString(sz, 1024, 234645.37f, -1, true);
	AssertString("234645.375f", sz);

	FloatToString(sz, 1024, 234645.37f, -1, true);
	AssertString("234645.375f", sz);

	FloatToString(sz, 1024, 23464.536f, -1, true);
	AssertString("23464.5352f", sz);

	FloatToString(sz, 1024, 2346.45361f, -1, true);
	AssertString("2346.45361f", sz);

	FloatToString(sz, 1024, 234.645361f, -1, true);
	AssertString("234.645355f", sz);

	FloatToString(sz, 1024, 23.464536f, -1, true);
	AssertString("23.4645367f", sz);

	FloatToString(sz, 1024, 2.34645343f, -1, true);
	AssertString("2.34645343f", sz);

	FloatToString(sz, 1024, 0.234645343f, -1, true);
	AssertString("0.234645337f", sz);

	FloatToString(sz, 1024, 0.023464534f, -1, true);
	AssertString("0.0234645344f", sz);

	FloatToString(sz, 1024, 0.0023464534f, -1, true);
	AssertString("0.00234645349f", sz);

	FloatToString(sz, 1024, 0.00023464534f, -1, true);
	AssertString("0.00023464534f", sz);

	FloatToString(sz, 1024, 0.000023464534f, -1, true);
	AssertString("2.3464534e-5f", sz);

	FloatToString(sz, 1024, 7.11645356e37f, -1, true);
	AssertString("7.11645356e+37f", sz);

	FloatToString(sz, 1024, 1.70141183e+38f, -1, true);
	AssertString("1.70141183e+38f", sz);

	FloatToString(sz, 1024, 1.17549435e-38f, -1, true);
	AssertString("1.17549435e-38f", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatPrinterRounding(void)
{
	char	sz[1024];

	FloatToString(sz, 1024, 734.645348f, 2, false);
	AssertString("734.65", sz);

	FloatToString(sz, 1024, 734645376, 2, false);
	AssertString("734645376.00", sz);

	FloatToString(sz, 1024, 734645376, 0, false);
	AssertString("734645376", sz);

	FloatToString(sz, 1024, 734645376, 1, false);
	AssertString("734645376.0", sz);

	FloatToString(sz, 1024, 73464534.8f, 2, false);
	AssertString("73464536.00", sz);

	FloatToString(sz, 1024, 734.645325f, -1, false);
	AssertString("734.645325", sz);

	FloatToString(sz, 1024, 3734645376.f, 0, false);
	AssertString("3.73464525e+9", sz);

	FloatToString(sz, 1024, 3734645376.f, -1, false);
	AssertString("3.73464525e+9", sz);

	FloatToString(sz, 1024, 3734645376.f, 1, false);
	AssertString("3.73464525e+9", sz);

	FloatToString(sz, 1024, 0.373464537f, 3, false);
	AssertString("0.373", sz);

	FloatToString(sz, 1024, 0.373464537f, 1, false);
	AssertString("0.4", sz);

	FloatToString(sz, 1024, 0.373464537f, 8, false);
	AssertString("0.37346452", sz);

	FloatToString(sz, 1024, 0.373464537f, 9, false);
	AssertString("0.373464525", sz);

	FloatToString(sz, 1024, 0.373464537f, 10, false);
	AssertString("0.373464525", sz);

	FloatToString(sz, 1024, 0.373464537f, 0, false);
	AssertString("0", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatPrinterNegativeRounding(void)
{
	char	sz[1024];

	FloatToString(sz, 1024, -734.645348f, 2, false);
	AssertString("-734.65", sz);

	FloatToString(sz, 1024, -734645376.f, 2, false);
	AssertString("-734645376.00", sz);

	FloatToString(sz, 1024, -734645376.f, 0, false);
	AssertString("-734645376", sz);

	FloatToString(sz, 1024, -734645376.f, 1, false);
	AssertString("-734645376.0", sz);

	FloatToString(sz, 1024, -73464534.8f, 2, false);
	AssertString("-73464536.00", sz);

	FloatToString(sz, 1024, -734.645325f, -1, false);
	AssertString("-734.645325", sz);

	FloatToString(sz, 1024, -3734645376.f, 0, false);
	AssertString("-3.73464525e+9", sz);

	FloatToString(sz, 1024, -3734645376.f, -1, false);
	AssertString("-3.73464525e+9", sz);

	FloatToString(sz, 1024, -3734645376.f, 1, false);
	AssertString("-3.73464525e+9", sz);

	FloatToString(sz, 1024, -0.373464537f, 3, false);
	AssertString("-0.373", sz);

	FloatToString(sz, 1024, -0.373464537f, 1, false);
	AssertString("-0.4", sz);

	FloatToString(sz, 1024, -0.373464537f, 8, false);
	AssertString("-0.37346452", sz);

	FloatToString(sz, 1024, -0.373464537f, 9, false);
	AssertString("-0.373464525", sz);

	FloatToString(sz, 1024, -0.373464537f, 10, false);
	AssertString("-0.373464525", sz);

	FloatToString(sz, 1024, -0.373464537f, 0, false);
	AssertString("0", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDoublePrinterFullRangeSixteenDigits(void)
{
	char	sz[1024];

	DoubleToString(sz, 1024, 0x1.p1023, -1, true);
	AssertString("8.9884656743115795e+307", sz);

	DoubleToString(sz, 1024, 0x1.p972, -1, true);
	AssertString("3.9916806190694396e+292", sz);

	DoubleToString(sz, 1024, 0xc000000000000.p972, -1, true);
	AssertString("1.3482698511467369e+308", sz);

	DoubleToString(sz, 1024, 0x8000000000001.p972, -1, true);
	AssertString("8.9884656743115835e+307", sz);

	DoubleToString(sz, 1024, 734.6453482389479832432343e303, -1, true);
	AssertString("7.3464534823894801e+305", sz);

	DoubleToString(sz, 1024, -0x1.p1023, -1, true);
	AssertString("-8.9884656743115795e+307", sz);

	DoubleToString(sz, 1024, 1., -1, true);
	AssertString("1.", sz);

	DoubleToString(sz, 1024, 10., -1, true);
	AssertString("10.", sz);

	DoubleToString(sz, 1024, 100., -1, true);
	AssertString("100.", sz);

	DoubleToString(sz, 1024, 1000., -1, true);
	AssertString("1000.", sz);

	DoubleToString(sz, 1024, 0.1, -1, true);
	AssertString("0.10000000000000001", sz);

	DoubleToString(sz, 1024, 0.01, -1, true);
	AssertString("0.01", sz);

	DoubleToString(sz, 1024, 0.001, -1, true);
	AssertString("0.001", sz);

	DoubleToString(sz, 1024, 0.0001, -1, true);
	AssertString("0.0001", sz);

	DoubleToString(sz, 1024, 0.00001, -1, true);
	AssertString("1.0000000000000001e-5", sz);

	DoubleToString(sz, 1024, -652093.82389479832432343e300, -1, true);
	AssertString("-6.520938238947983e+305", sz);

	DoubleToString(sz, 1024, -652093.82389479832432343e-300, -1, true);
	AssertString("-6.5209382389479834e-295", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatPrinterSubnormal(void)
{
	char	sz[1024];

	FloatToString(sz, 1024, 0x0.000002p-126f, -1, true);
	AssertString("1.40129846e-45f", sz);

	FloatToString(sz, 1024, 1.40129846e-45f, -1, true);
	AssertString("1.40129846e-45f", sz);

	FloatToString(sz, 1024, 0x0.000004p-126f, -1, true);
	AssertString("2.80259693e-45f", sz);

	FloatToString(sz, 1024, 3.33333333e-33f, -1, true);
	AssertString("3.33333317e-33f", sz);

	FloatToString(sz, 1024, 3.33333333e-34f, -1, true);
	AssertString("3.33333326e-34f", sz);

	FloatToString(sz, 1024, 3.33333333e-35f, -1, true);
	AssertString("3.3333332e-35f", sz);

	FloatToString(sz, 1024, 3.33333333e-36f, -1, true);
	AssertString("3.33333327e-36f", sz);

	FloatToString(sz, 1024, 3.33333333e-37f, -1, true);
	AssertString("3.33333323e-37f", sz);

	FloatToString(sz, 1024, 3.33333333e-38f, -1, true);
	AssertString("3.3333334e-38f", sz);

	FloatToString(sz, 1024, 3.33333333e-39f, -1, true);
	AssertString("3.33333312e-39f", sz);

	FloatToString(sz, 1024, 3.33333333e-40f, -1, true);
	AssertString("3.33333872e-40f", sz);

	FloatToString(sz, 1024, 3.33333333e-41f, -1, true);
	AssertString("3.33326866e-41f", sz);

	FloatToString(sz, 1024, 3.33333333e-42f, -1, true);
	AssertString("3.33368905e-42f", sz);

	FloatToString(sz, 1024, 3.33333333e-43f, -1, true);
	AssertString("3.33509035e-43f", sz);

	FloatToString(sz, 1024, 3.33333333e-44f, -1, true);
	AssertString("3.36311631e-44f", sz);

	FloatToString(sz, 1024, 3.33333333e-45f, -1, true);
	AssertString("2.80259693e-45f", sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertNumberString(const char* szExpected, CNumber* pcNumber)
{
	CChars		sz;

	sz.Init();
	pcNumber->Print(&sz);
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatPrinterScratchPad(void)
{
	CNumber*	pcNumber1;
	CNumber*	pcNumber2;
	CNumber*	pcNumber3;

	pcNumber1 = gcNumberControl.Add(308, 0);
	pcNumber2 = gcNumberControl.Add(293, 0);
	pcNumber3 = gcNumberControl.Add(309, 0);

	pcNumber1->Init("89884656743115795386465259539451236680898848947115328636715040578866337902750481566354238661203768010560056939935696678829394884407208311246423715319737062188883946712432742638151109800623047059726541476042502884419075341171231440736956555270413618581675255342293149119973622969239858152417678164812112068608", 308, 0);
	AssertNumberString("89884656743115795386465259539451236680898848947115328636715040578866337902750481566354238661203768010560056939935696678829394884407208311246423715319737062188883946712432742638151109800623047059726541476042502884419075341171231440736956555270413618581675255342293149119973622969239858152417678164812112068608", pcNumber1);

	pcNumber2->Init("79833612381388792466254908521473542642698050417418301660101889697488475674758562631398318619705428043573697873767699809758897519535743746429566871931393256812800226918043427060701508857774519487306134269781756959733232418204834815555554210737921766300284836275030241665694339809213760397115392", 293, 0);
	AssertNumberString("79833612381388792466254908521473542642698050417418301660101889697488475674758562631398318619705428043573697873767699809758897519535743746429566871931393256812800226918043427060701508857774519487306134269781756959733232418204834815555554210737921766300284836275030241665694339809213760397115392", pcNumber2);

	pcNumber3->Init(pcNumber1, 309, 0);
	AssertNumberString("79833612381388792466254908521473542642698050417418301660101889697488475674758562631398318619705428043573697873767699809758897519535743746429566871931393256812800226918043427060701508857774519487306134269781756959733232418204834815555554210737921766300284836275030241665694339809213760397115392", pcNumber2);
	AssertNumberString("89884656743115795386465259539451236680898848947115328636715040578866337902750481566354238661203768010560056939935696678829394884407208311246423715319737062188883946712432742638151109800623047059726541476042502884419075341171231440736956555270413618581675255342293149119973622969239858152417678164812112068608", pcNumber1);
	AssertNumberString("89884656743115795386465259539451236680898848947115328636715040578866337902750481566354238661203768010560056939935696678829394884407208311246423715319737062188883946712432742638151109800623047059726541476042502884419075341171231440736956555270413618581675255342293149119973622969239858152417678164812112068608", pcNumber3);

	pcNumber3->Add(pcNumber2);

	AssertNumberString("89884656743115795386465259539451236680898848947115328636715040578866337902750481566354238661203768010560056939935696678829394884407208311246423715319737062188883946712432742638151109800623047059726541476042502884419075341171231440736956555270413618581675255342293149119973622969239858152417678164812112068608", pcNumber1);
	AssertNumberString("79833612381388792466254908521473542642698050417418301660101889697488475674758562631398318619705428043573697873767699809758897519535743746429566871931393256812800226918043427060701508857774519487306134269781756959733232418204834815555554210737921766300284836275030241665694339809213760397115392", pcNumber2);
	AssertNumberString("89884656743115875220077640928243702935807370420657971334765457997167998004640179054829913419766399408878676645363740252527268652107018070143943251063483491755755878105689555438378027844050107761235399250561990190553345122928191173969374760105229174135885993264059449404809897999481523846757487378572509184000", pcNumber3);

	gcNumberControl.Remove(3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatPrinter(void)
{
	BeginTests();

	TestFloatPrinterScratchPad();
	TestFloatPrinterFullRangeNineDigits();
	TestFloatPrinterRounding();
	TestFloatPrinterNegativeRounding();
	TestDoublePrinterFullRangeSixteenDigits();
	TestFloatPrinterSubnormal();

	TestStatistics();
}

