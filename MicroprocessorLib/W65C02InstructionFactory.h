#ifndef __W65C02_INSTRUCTION_FACTORY_H__
#define __W65C02_INSTRUCTION_FACTORY_H__
#include "W65C02InstructionCodes.h"
#include "W65Instruction.h"
#include "W65C02InstructionCycleFactory.h"


#define W65C02_INSTRUCTIONS_SIZE 256 + 4


class CW65C02InstructionFactory
{
protected:
	CW65Instruction*	mapcInstructions[W65C02_INSTRUCTIONS_SIZE];
	CW65Instruction*	mpcReset;
	CW65Instruction*	mpcIRQ;
	CW65Instruction*	mpcNMI;
	CW65Instruction*	mpcFetchNext;
	uint16			muiInstructions;

public:
	static	CW65C02InstructionFactory*	GetInstance(void);

			void			Init(void);
			void			Kill(void);

			CW65Instruction*	GetInstruction(uint16 uiOpcode);
			CW65Instruction*	GetReset(void);
			CW65Instruction*	GetIRQ(void);
			CW65Instruction*	GetNMI(void);
			CW65Instruction*	GetFetchNext(void);

protected:
			void			CreateInstructions(void);
			CW65Instruction*	Instruct(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles, char* szName, char* szDescription);
			CW65Instruction*	Instruct(CW65InstructionCycles* pcInstructionCycles, char* szName, char* szDescription);
};


#endif // __W65C02_INSTRUCTION_FACTORY_H__

