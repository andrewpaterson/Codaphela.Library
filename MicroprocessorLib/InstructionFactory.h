#ifndef __INSTRUCTION_FACTORY_H__
#define __INSTRUCTION_FACTORY_H__
#include "Instruction.h"


class CInstructionFactory
{
protected:
	CInstruction*	mapcInstructions[256];
	CInstruction*	mpcReset;
	CInstruction*	mpcIRQ;
	CInstruction*	mpcNMI;
	CInstruction*	mpcAbort;
	CInstruction*	mpcFetchNext;

public:
	static	CInstructionFactory*	GetInstance(void);

			void					Init(void);

			CInstruction*			GetInstruction(uint16 uiOpcode);
			CInstruction*			GetReset(void);
			CInstruction*			GetIRQ(void);
			CInstruction*			GetNMI(void);
			CInstruction*			GetAbort(void);
			CInstruction*			GetFetchNext(void);
};


#endif // __INSTRUCTION_FACTORY_H__

