#include "BaseLib/LeakDetector.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "MicroprocessorLib/MetaTrace.h"
#include "TestLib/Assert.h"


void TestW65C816Reset(void);
void TestW65C816Add(void);
void TestW65C816Subtract(void);
void TestW65C816Arithmetic(void);
void TestW65C816Load(void);
void TestW65C816SingleStep(void);
void TestW65C02SingleStep(void);
void TestW65C02(void);
void TestW65C816Logic(void);
void TestW65C816Mode(void);
void TestW65C816Stack(void);
void TestW65C816Branch(void);
void TestW65C816Interrupt(void);
void TestW65C816BlockMove(void);
void TestW65C816DirectPage(void);
void TestW65C816Decimal(void);
void TestW65C816Timing(void);
void Test65C64(void);
void Test65C64Interrupt(void);
void Test65C64MultiplyDivide(void);
void Test65C64Subcopy(void);
void Test65C64Bits(void);
void Test65C64Operand(void);
void Test65C64Block(void);
void Test65C64Vector(void);
void TestFast65C64(void);
void TestFast65C816(void);
void TestTimeline(void);
void TestVoltageConfiguration(void);
void TestLogicPort(void);
void TestPassive(void);
void TestStandardIntegratedCircuits(void);
void TestGroupIntegratedCircuits(void);
void TestProcessorChips(void);


int __cdecl main(void)
{
	int iResult;

	BreakOnAllocation(0);
	InitTotalStatistics();

	TypesInit();
	TypeConverterInit();
	MemoryInit();
	DataIOInit();
	ObjectsInit();
	gcMetaTraceUndriven.Init();

	TestW65C816Reset();
	TestW65C816Add();
	TestW65C816Subtract();
	TestW65C816Load();
	TestW65C816Logic();
	TestW65C816Mode();
	TestW65C816Stack();
	TestW65C816Branch();
	TestW65C816Interrupt();
	TestW65C816BlockMove();
	TestW65C816DirectPage();
	TestW65C816Decimal();
	TestW65C816Timing();
	TestW65C816SingleStep();
	TestW65C02SingleStep();
	TestW65C02();
	TestW65C816Arithmetic();
	Test65C64();
	Test65C64Interrupt();
	Test65C64MultiplyDivide();
	Test65C64Subcopy();
	Test65C64Bits();
	Test65C64Operand();
	Test65C64Block();
	Test65C64Vector();
	TestFast65C64();
	TestFast65C816();
	TestTimeline();
	TestVoltageConfiguration();
	TestLogicPort();
	TestPassive();
	TestStandardIntegratedCircuits();
	TestGroupIntegratedCircuits();
	TestProcessorChips();

	ObjectsKill();
	DataIOKill();
	MemoryKill();
	TypeConverterKill();
	TypesKill();

	iResult = TestTotalStatistics();
	LeakDetection();
	return iResult;
}

