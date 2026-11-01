#ifndef __TEST_W65C816_CONTEXT_H__
#define __TEST_W65C816_CONTEXT_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "MicroprocessorLib/W65C816Meta.h"


class CTestW65C816Context
{
protected:
	uint8*			mauiMemory;
	CChars			mszSequence;
	uint32			muiMemorySize;
	uint8			muiFill;
	size			muiStartOfPreviousLine;
	size			muiStartOfPrevPrevLine;

public:
	bool			mbPrintSet;

	bool			mbPrintReset;
	bool			mbPrintStop;
	bool			mbPrintMnemonic;
	bool			mbPrintCycle;
	bool			mbPrintOperation;
	bool			mbPrintA;
	bool			mbPrintX;
	bool			mbPrintY;
	bool			mbPrintPC;
	bool			mbPrintS;
	bool			mbPrintDP; 
	bool			mbPrintDB;
	bool			mbPrintP;
	bool			mbPrintMergedCycles;

	bool			mbPrintAddressBus;
	bool			mbPrintDataBus;
	bool			mbPrintBank;
	bool			mbPrintVPB;
	bool			mbPrintRDY;
	bool			mbPrintMLB;
	bool			mbPrintVPA;
	bool			mbPrintVDA;
	bool			mbPrintWM;
	bool			mbPrintWX;
	bool			mbPrintE;
	bool			mbPrintRWB;

	bool			mbAddressOperation;
	bool			mbDataOperation;
	bool			mbDescription;

	bool			mbEnablePrint;
	bool			mbDebugSpew;

	uint32			muiLastAddress;
	size			muiInstructions;
	uint64			muiMaxInstructions;
	CW65C816Meta*	mpcMPU;

public:
	void		Init(uint32 uiMemorySize, uint8 uiFill);
	void		Init(uint32 uiMemorySize, uint8 uiFill, uint16 uiResetAddress);
	void		Init(char* szMemoryFileame);
	void		SetPrint(bool bReset, bool bStop, bool bMnemonic, bool bCycle, bool bOperation, bool bA, bool bX, bool bY, bool bPC, bool bS, bool bDP, bool bDB, bool bP, bool mbMerged);
	void		SetSignalPrint(bool bAddressBus, bool bDataBus, bool bBank, bool bVPB, bool bRDY, bool bMLB, bool bVPA, bool bVDA, bool bWM, bool bWX, bool bE, bool bRWB);
	void		SetOperationPrint(bool bAddressOperation, bool bDataOperation, bool bDescription);
	void		DebugSpew(void);
	void		Kill(void);

	uint64		Run(CW65C816Meta* pcMPU);
	void		StripToInstruction(char* szInstruction);

	uint8*		GetMemory(void);
	uint8		GetByte(uint32 uiAddress);
	void		SetByte(uint32 uiAddress, uint8 uiValue);
	void		SetByte(uint8 uiValue);
	void		SetOpcd(uint32 uiAddress, uint8 uiOpcode);
	void		SetOpcd(uint8 uiOpcode);
	CChars*		GetSequence(void);
	char*		SequenceText(void);
	void		SetStartOfPreviousLine(size uiStartOfPreviousLine);
	size		GetStartOfPrevPrevLine(void);
	uint16		GetShort(uint32 uiAddress);
	void		SetShort(uint32 uiAddress, uint16 uiValue);
	void		SetShort(uint16 uiValue);
};


void TestW65C81ContextTickLow(CW65C816Meta* pcMPU, void* pvContext);
void TestW65C81ContextTickHigh(CW65C816Meta* pcMPU, void* pvContext);


void TestW65C816RunInContext(CTestW65C816Context* pcContext, char* szExpected);
void TestW65C816InContext(CTestW65C816Context* pcContext, char* szExpected);


#endif // __TEST_W65C816_CONTEXT_H__

