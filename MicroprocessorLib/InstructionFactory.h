#ifndef __INSTRUCTION_FACTORY_H__
#define __INSTRUCTION_FACTORY_H__
#include "InstructionCodes.h"
#include "Instruction.h"
#include "InstructionCycleFactory.h"
#include "W65C816.h"


#define INSTRUCTIONS_SIZE 256 + 5


class CInstructionFactory
{
protected:
	CInstruction*	mapcInstructions[INSTRUCTIONS_SIZE];
	CInstruction*	mpcReset;
	CInstruction*	mpcIRQ;
	CInstruction*	mpcNMI;
	CInstruction*	mpcAbort;
	CInstruction*	mpcFetchNext;

    uint16          muiInstructions;

public:
	static	CInstructionFactory*	GetInstance(void);

			void					Init(void);
            void                    Kill(void);

			CInstruction*			GetInstruction(uint16 uiOpcode);
			CInstruction*			GetReset(void);
			CInstruction*			GetIRQ(void);
			CInstruction*			GetNMI(void);
			CInstruction*			GetAbort(void);
			CInstruction*			GetFetchNext(void);

protected:
            CInstruction*           Instruct(CInstruction* pcInstruction);
            void                    CreateInstructions(void);
            void                    ValidateOpcodes(void);
            CInstruction*           CreateBEQ(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateXBA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateNOP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateINX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSEP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBNE(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateWAI(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateDEX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateINY(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePEA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateINC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateXCE(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePLX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSED(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSBC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateCPX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSTP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePHX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateCLD(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePEI(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateDEC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateREP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateCMP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateCPY(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTYX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTSX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateCLV(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBCS(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePLB(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTAX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTAY(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateLDX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBRL(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBRA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTDC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePLY(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSEI(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBVS(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateRTL(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateRRA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePLA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateROR(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePER(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateADC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateRTS(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTCD(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePHY(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateCLI(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateMVN(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBVC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateJMP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePHK(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSRA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePHA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateLSR(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateMVP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateWDM(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateEOR(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateRTI(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTSC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateDEC_A(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSEC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBMI(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePLP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateRLA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePLD(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateROL(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateLDA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateLDY(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePHB(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTXA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSTY(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBCC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSTX(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTYA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTXS(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTXY(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSTZ(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSTA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateDEY(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBIT(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateJSL(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateAND(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateJSR(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTCS(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateINC_A(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateCLC(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTRB(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBPL(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePHD(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateSLA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateASL(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreatePHP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateTSB(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateCOP(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateORA(uint16 uiCode, CInstructionCycles* pcInstructionCycles);
            CInstruction*           CreateBRK(uint16 uiCode, CInstructionCycles* pcInstructionCycles);

           CInstruction*            CreateReset(void);
           CInstruction*            CreateIRQ(void);
           CInstruction*            CreateNMI(void);
           CInstruction*            CreateAbort(void);
           CInstruction*            CreateFetchNext(void);
};


#endif // __INSTRUCTION_FACTORY_H__

