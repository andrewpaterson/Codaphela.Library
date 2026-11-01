#ifndef __TEST_SINGLE_STEP_H__
#define __TEST_SINGLE_STEP_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArraySparseTemplate.h"
#include "MicroprocessorLib/W65CMeta.h"


//////////////////////////////////////////////////////////////////////////
//
// Runs single instruction tests from https://github.com/SingleStepTests (MIT licence)
// converted to binary by ConvertSingleStepTests.py.  Each test sets the registers and
// memory, runs one instruction and compares every bus cycle and the final state.  A
// CSingleStepProcessor adapts the runner to the W65C816S or the W65C02S.
//
//////////////////////////////////////////////////////////////////////////


#define SINGLE_STEP_VDA			0x01
#define SINGLE_STEP_VPA			0x02
#define SINGLE_STEP_VPB			0x04
#define SINGLE_STEP_READ		0x08
#define SINGLE_STEP_E			0x10
#define SINGLE_STEP_M			0x20
#define SINGLE_STEP_X			0x40
#define SINGLE_STEP_MLB			0x80

#define SINGLE_STEP_VALUE_VALID		0x01
#define SINGLE_STEP_ADDRESS_VALID	0x02

#define SINGLE_STEP_MAX_CYCLES	1024


struct SSingleStepCycle
{
	uint32	uiAddress;
	uint8	uiValue;
	uint8	uiSignals;
	uint8	uiValid;
};


struct SSingleStepState
{
	uint16	uiPC;
	uint16	uiS;
	uint8	uiP;
	uint16	uiA;
	uint16	uiX;
	uint16	uiY;
	uint8	uiDBR;
	uint16	uiD;
	uint8	uiPBR;
	uint8	uiE;
	uint16	uiRamCount;
	uint8*	puiRam;  //uint24 address, uint8 value.
};


class CSingleStepReader
{
protected:
	uint8*	mpuiPos;

public:
	void	Init(uint8* puiData)	{ mpuiPos = puiData; }
	uint8	ReadByte(void)			{ return *mpuiPos++; }
	uint16	ReadShort(void)			{ uint16 ui = mpuiPos[0] | (mpuiPos[1] << 8); mpuiPos += 2; return ui; }
	uint32	ReadAddress(void)		{ uint32 ui = mpuiPos[0] | (mpuiPos[1] << 8) | (mpuiPos[2] << 16); mpuiPos += 3; return ui; }
	uint8*	GetPos(void)			{ return mpuiPos; }
	void	Skip(size uiBytes)		{ mpuiPos += uiBytes; }
};


class CTestSingleStepContext
{
public:
	CArraySparseTemplate<uint8>	mcMemory;
	SSingleStepCycle			masCycles[SINGLE_STEP_MAX_CYCLES];
	size						muiCycles;

	void Init(void)
	{
		mcMemory.Init();
		muiCycles = 0;
	}

	void Kill(void)
	{
		mcMemory.Kill();
	}

	void SetByte(uint32 uiAddress, uint8 uiValue)
	{
		*mcMemory.Set(uiAddress) = uiValue;
	}

	uint8 GetByte(uint32 uiAddress)
	{
		uint8*	puiValue;

		puiValue = mcMemory.Get(uiAddress);
		if (puiValue)
		{
			return *puiValue;
		}
		return 0;
	}
};


class CSingleStepProcessor
{
public:
	virtual void		Init(CTestSingleStepContext* pcContext) = 0;
	virtual void		Kill(void) = 0;
	virtual CW65CMeta*	GetMeta(void) = 0;
	virtual CW65CCore*	GetCPU(void) = 0;
	virtual void		SetState(SSingleStepState* psState) = 0;  //Registers the core does not set.
	virtual uint8		GetSignalMask(void) = 0;  //The signals the tests give.
	virtual uint8		GetStatusMask(void) = 0;  //The bits of P the processor has.
	virtual bool		IsWaitOrStop(uint8 uiOpcode) = 0;
};


void TestSingleStepRecordCycle(CW65CMeta* pcMPU, CTestSingleStepContext* pcContext, uint8 uiSignals, bool bEnabled);
void TestSingleStepReadState(CSingleStepReader* pcReader, SSingleStepState* psState);
void TestSingleStepAppendHex(CChars* psz, uint32 uiValue, int iDigits);
bool TestSingleStepCompareRegister(CChars* pszError, char* szName, uint32 uiExpected, uint32 uiActual, int iDigits);
void TestSingleStepFile(CSingleStepProcessor* pcProcessor, char* szDirectory, char* szName, uint8 uiOpcode);


#endif // __TEST_SINGLE_STEP_H__

