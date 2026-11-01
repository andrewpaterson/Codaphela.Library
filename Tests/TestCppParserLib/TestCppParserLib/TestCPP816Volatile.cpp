#include <stdlib.h>
#include <string.h>
#include "BaseLib/PrimitiveTypes.h"
#include "MicroprocessorLib/Fast65C816.h"
#include "CppParserLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


#define TEST_VOLATILE_MEMORY		0x100000		//The program's flat memory: banks 0 to F.
#define TEST_VOLATILE_DEVICE		0x0E0000		//A page of device registers in a hole in it.
#define TEST_VOLATILE_DEVICE_END	0x0E0100
#define TEST_VOLATILE_STEPS			2000000


//A memory mapped device, as a program polling hardware sees it.  Register 0 reads 0 until its uiReadyOn'th read,
//which reads $80 (ready); register 1 reads 1, 2, 3 ... (a counter that moves on each read).  Every access to the
//page is counted, and writes are kept in order.
struct STestVolatileDevice
{
	uint8*	puiMemory;
	uint32	uiReads[4];
	uint32	uiWrites[4];
	uint8	auiWritten[16];
	uint32	uiNumWritten;
	uint32	uiReadyOn;
	uint8	uiCounter;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint8 TestVolatileRead(void* pvContext, uint32 uiAddress)
{
	STestVolatileDevice*	psDevice;
	uint32					uiRegister;

	psDevice = (STestVolatileDevice*)pvContext;
	if ((uiAddress >= TEST_VOLATILE_DEVICE) && (uiAddress < TEST_VOLATILE_DEVICE_END))
	{
		uiRegister = (uiAddress - TEST_VOLATILE_DEVICE) & 3;
		psDevice->uiReads[uiRegister]++;
		if (uiRegister == 0)
		{
			return (psDevice->uiReads[0] >= psDevice->uiReadyOn) ? 0x80 : 0x00;
		}
		if (uiRegister == 1)
		{
			psDevice->uiCounter++;
			return psDevice->uiCounter;
		}
		return 0;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestVolatileWrite(void* pvContext, uint32 uiAddress, uint8 uiValue)
{
	STestVolatileDevice*	psDevice;
	uint32					uiRegister;

	psDevice = (STestVolatileDevice*)pvContext;
	if ((uiAddress >= TEST_VOLATILE_DEVICE) && (uiAddress < TEST_VOLATILE_DEVICE_END))
	{
		uiRegister = (uiAddress - TEST_VOLATILE_DEVICE) & 3;
		psDevice->uiWrites[uiRegister]++;
		if (psDevice->uiNumWritten < sizeof(psDevice->auiWritten))
		{
			psDevice->auiWritten[psDevice->uiNumWritten++] = uiValue;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles szSource for the W65C816 (with the usual start up), runs it with
// the device at $0E0000 and returns main's int, or -1000000 if it didn't
// build or stop.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestVolatileRun(const char* szSource, STestVolatileDevice* psDevice, uint32 uiReadyOn)
{
	CCPP65Builder		cBuilder;
	CFast65C816			cFast;
	CAsm65Output*		pcOutput;
	SAsm65Segment*		psSegment;
	size				i;
	int32				iResult;
	bool				bResult;

	memset(psDevice, 0, sizeof(STestVolatileDevice));
	psDevice->uiReadyOn = uiReadyOn;
	iResult = -1000000;

	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	TestCPP65AddIncludeDirectories(&cBuilder);
	bResult = cBuilder.AddStartUp();
	bResult &= cBuilder.AddSource("Volatile.cpp", szSource);
	bResult &= cBuilder.Link();
	AssertString("", cBuilder.GetErrors());
	AssertTrue(bResult);
	if (bResult)
	{
		psDevice->puiMemory = (uint8*)malloc(TEST_VOLATILE_MEMORY);
		memset(psDevice->puiMemory, 0, TEST_VOLATILE_MEMORY);
		pcOutput = cBuilder.GetOutput();
		for (i = 0; i < pcOutput->NumSegments(); i++)
		{
			psSegment = pcOutput->GetSegment(i);
			memcpy(psDevice->puiMemory + psSegment->uiAddress, psSegment->cBytes.GetData(), psSegment->cBytes.NumElements());
		}

		cFast.Init(TestVolatileRead, TestVolatileWrite, psDevice, psDevice->puiMemory, TEST_VOLATILE_MEMORY);
		cFast.SetHole(TEST_VOLATILE_DEVICE, TEST_VOLATILE_DEVICE_END);
		cFast.Reset();
		cFast.Run(TEST_VOLATILE_STEPS);
		AssertTrue(cFast.IsStopped());
		if (cFast.IsStopped())
		{
			iResult = (int16)cFast.GetC();
		}
		cFast.Kill();
		free(psDevice->puiMemory);
		psDevice->puiMemory = NULL;
	}
	cBuilder.Kill();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Polling a device register until it says ready.  Each time round the loop
// must read the register again: the read can't be done once before the
// loop (as an invariant), or the loop spins to its limit.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816VolatilePoll(void)
{
	STestVolatileDevice	sDevice;

	static const char* szLocal =
		"int main(void)\n"
		"{\n"
		"	volatile unsigned char* p = (volatile unsigned char*)0x0E0000;\n"
		"	unsigned int i;\n"
		"	for (i = 0; i < 60000; i++)\n"
		"	{\n"
		"		if (p[0] & 0x80) break;\n"
		"	}\n"
		"	return (int)i;\n"
		"}\n";

	AssertInt(9, TestVolatileRun(szLocal, &sDevice, 10));
	AssertInt(10, (int)sDevice.uiReads[0]);

	//The same through a pointer in a struct (as a driver keeps its registers' address).
	static const char* szMember =
		"struct SDriver { volatile unsigned char* puiRegisters; };\n"
		"SDriver gsDriver;\n"
		"int Poll(SDriver* ps)\n"
		"{\n"
		"	unsigned int i;\n"
		"	for (i = 0; i < 60000; i++)\n"
		"	{\n"
		"		if (ps->puiRegisters[0] & 0x80) break;\n"
		"	}\n"
		"	return (int)i;\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"	gsDriver.puiRegisters = (volatile unsigned char*)0x0E0000;\n"
		"	return Poll(&gsDriver);\n"
		"}\n";

	AssertInt(4, TestVolatileRun(szMember, &sDevice, 5));
	AssertInt(5, (int)sDevice.uiReads[0]);

	//A volatile global flag, set by something the compiler can't see (an interrupt, another processor).
	static const char* szGlobal =
		"volatile unsigned char* volatile gpuStatus = (volatile unsigned char*)0x0E0000;\n"
		"int main(void)\n"
		"{\n"
		"	int i = 0;\n"
		"	while (!(*gpuStatus & 0x80)) i++;\n"
		"	return i;\n"
		"}\n";

	AssertInt(6, TestVolatileRun(szGlobal, &sDevice, 7));
	AssertInt(7, (int)sDevice.uiReads[0]);
}


//////////////////////////////////////////////////////////////////////////
//
// Each volatile read and write is done, as many times as written and in
// order: two reads of the same register aren't merged, a read whose value
// is unused isn't removed, and two writes to the same register aren't
// merged into the last.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816VolatileAccesses(void)
{
	STestVolatileDevice	sDevice;

	static const char* szReads =
		"int main(void)\n"
		"{\n"
		"	volatile unsigned char* p = (volatile unsigned char*)0x0E0000;\n"
		"	int a = p[1];\n"
		"	int b = p[1];\n"
		"	return a * 16 + b;\n"
		"}\n";

	AssertInt(0x12, TestVolatileRun(szReads, &sDevice, 1));
	AssertInt(2, (int)sDevice.uiReads[1]);

	static const char* szUnused =
		"int main(void)\n"
		"{\n"
		"	volatile unsigned char* p = (volatile unsigned char*)0x0E0000;\n"
		"	(void)p[1];\n"
		"	p[1];\n"
		"	return 0;\n"
		"}\n";

	AssertInt(0, TestVolatileRun(szUnused, &sDevice, 1));
	AssertInt(2, (int)sDevice.uiReads[1]);

	static const char* szWrites =
		"int main(void)\n"
		"{\n"
		"	volatile unsigned char* p = (volatile unsigned char*)0x0E0000;\n"
		"	p[2] = 0x31;\n"
		"	p[2] = 0x33;\n"
		"	p[2] = 0x31;\n"
		"	return 0;\n"
		"}\n";

	AssertInt(0, TestVolatileRun(szWrites, &sDevice, 1));
	AssertInt(3, (int)sDevice.uiWrites[2]);
	AssertInt(3, (int)sDevice.uiNumWritten);
	AssertIntHex(0x31, sDevice.auiWritten[0]);
	AssertIntHex(0x33, sDevice.auiWritten[1]);
	AssertIntHex(0x31, sDevice.auiWritten[2]);

	//Bit banging: the same register written in a loop, high then low, must see every write.
	static const char* szToggle =
		"int main(void)\n"
		"{\n"
		"	volatile unsigned char* p = (volatile unsigned char*)0x0E0000;\n"
		"	unsigned int i;\n"
		"	for (i = 0; i < 5; i++)\n"
		"	{\n"
		"		p[3] = 0x11;\n"
		"		p[3] = 0x10;\n"
		"	}\n"
		"	return 0;\n"
		"}\n";

	AssertInt(0, TestVolatileRun(szToggle, &sDevice, 1));
	AssertInt(10, (int)sDevice.uiWrites[3]);
}


//////////////////////////////////////////////////////////////////////////
//
// A field of a volatile object is volatile (registers laid out as a
// struct), and a compound assignment to a volatile register is one read
// and one write each time.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816VolatileStructAndCompound(void)
{
	STestVolatileDevice	sDevice;

	static const char* szStruct =
		"struct SRegisters { unsigned char uiStatus; unsigned char uiCounter; };\n"
		"int main(void)\n"
		"{\n"
		"	volatile SRegisters* ps = (volatile SRegisters*)0x0E0000;\n"
		"	unsigned int i;\n"
		"	for (i = 0; i < 60000; i++)\n"
		"	{\n"
		"		if (ps->uiStatus & 0x80) break;\n"
		"	}\n"
		"	return (int)i * 256 + ps->uiCounter + ps->uiCounter;\n"
		"}\n";

	AssertInt(3 * 256 + 3, TestVolatileRun(szStruct, &sDevice, 4));
	AssertInt(4, (int)sDevice.uiReads[0]);
	AssertInt(2, (int)sDevice.uiReads[1]);

	static const char* szCompound =
		"int main(void)\n"
		"{\n"
		"	volatile unsigned char* p = (volatile unsigned char*)0x0E0000;\n"
		"	p[3] |= 0x01;\n"
		"	p[3] |= 0x02;\n"
		"	p[3]++;\n"
		"	return 0;\n"
		"}\n";

	AssertInt(0, TestVolatileRun(szCompound, &sDevice, 1));
	AssertInt(3, (int)sDevice.uiReads[3]);
	AssertInt(3, (int)sDevice.uiWrites[3]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Volatile(void)
{
	BeginTests();

	TestCPP816VolatilePoll();
	TestCPP816VolatileAccesses();
	TestCPP816VolatileStructAndCompound();

	TestStatistics();
}
