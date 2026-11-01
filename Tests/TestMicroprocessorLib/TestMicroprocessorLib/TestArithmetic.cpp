#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/LogToMemory.h"
#include "MicroprocessorLib/W65C816.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "MicroprocessorLib/W65C816Meta.h"
#include "TestLib/Assert.h"
#include "TestW65C816Context.h"




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Arithmetic(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;
	uint8					uiError;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init("TestArithmetic.bin");
	cTestContext.mbEnablePrint = false;

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	uiError = cTestContext.GetByte(0x0288);
	AssertByte((uint8)0, uiError);
	AssertLong(4'916'753LL, uiInstructions);

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

