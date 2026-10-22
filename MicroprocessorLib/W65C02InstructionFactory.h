#ifndef __W65C02_INSTRUCTION_FACTORY_H__
#define __W65C02_INSTRUCTION_FACTORY_H__
#include "W65C02InstructionCodes.h"
#include "Instruction.h"
#include "W65C02InstructionCycleFactory.h"


#define W65C02_INSTRUCTIONS_SIZE 256 + 4


class CW65C02InstructionFactory
{
protected:
	CInstruction*	mapcInstructions[W65C02_INSTRUCTIONS_SIZE];
	CInstruction*	mpcReset;
	CInstruction*	mpcIRQ;
	CInstruction*	mpcNMI;
	CInstruction*	mpcFetchNext;
	uint16			muiInstructions;

public:
	static	CW65C02InstructionFactory*	GetInstance(void);

			void			Init(void);
			void			Kill(void);

			CInstruction*	GetInstruction(uint16 uiOpcode);
			CInstruction*	GetReset(void);
			CInstruction*	GetIRQ(void);
			CInstruction*	GetNMI(void);
			CInstruction*	GetFetchNext(void);

protected:
			void			CreateInstructions(void);
			CInstruction*	Instruct(uint16 uiCode, CInstructionCycles* pcInstructionCycles, char* szName, char* szDescription);
			CInstruction*	Instruct(CInstructionCycles* pcInstructionCycles, char* szName, char* szDescription);
};


#endif // __W65C02_INSTRUCTION_FACTORY_H__

