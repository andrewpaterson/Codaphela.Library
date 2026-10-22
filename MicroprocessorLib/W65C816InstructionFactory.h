#ifndef __INSTRUCTION_FACTORY_H__
#define __INSTRUCTION_FACTORY_H__
#include "W65C816InstructionCodes.h"
#include "W65Instruction.h"
#include "W65C816InstructionCycleFactory.h"
#include "W65C816.h"


#define INSTRUCTIONS_SIZE 256 + 5


class CW65C816InstructionFactory
{
protected:
	CW65Instruction*	mapcInstructions[INSTRUCTIONS_SIZE];
	CW65Instruction*	mpcReset;
	CW65Instruction*	mpcIRQ;
	CW65Instruction*	mpcNMI;
	CW65Instruction*	mpcAbort;
	CW65Instruction*	mpcFetchNext;

    uint16          muiInstructions;

public:
	static	CW65C816InstructionFactory*	GetInstance(void);

			void					Init(void);
            void                    Kill(void);

			CW65Instruction*			GetInstruction(uint16 uiOpcode);
			CW65Instruction*			GetReset(void);
			CW65Instruction*			GetIRQ(void);
			CW65Instruction*			GetNMI(void);
			CW65Instruction*			GetAbort(void);
			CW65Instruction*			GetFetchNext(void);

protected:
            CW65Instruction*           Instruct(CW65Instruction* pcInstruction);
            void                    CreateInstructions(void);
            void                    ValidateOpcodes(void);
            CW65Instruction*           CreateBEQ(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateXBA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateNOP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateINX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSEP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBNE(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateWAI(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateDEX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateINY(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePEA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateINC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateXCE(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePLX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSED(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSBC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateCPX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSTP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePHX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateCLD(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePEI(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateDEC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateREP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateCMP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateCPY(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTYX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTSX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateCLV(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBCS(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePLB(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTAX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTAY(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateLDX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBRL(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBRA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTDC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePLY(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSEI(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBVS(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateRTL(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateRRA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePLA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateROR(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePER(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateADC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateRTS(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTCD(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePHY(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateCLI(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateMVN(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBVC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateJMP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePHK(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSRA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePHA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateLSR(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateMVP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateWDM(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateEOR(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateRTI(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTSC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateDEC_A(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSEC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBMI(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePLP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateRLA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePLD(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateROL(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateLDA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateLDY(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePHB(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTXA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSTY(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBCC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSTX(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTYA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTXS(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTXY(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSTZ(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSTA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateDEY(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBIT(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateJSL(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateAND(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateJSR(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTCS(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateINC_A(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateCLC(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTRB(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBPL(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePHD(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateSLA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateASL(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreatePHP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateTSB(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateCOP(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateORA(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);
            CW65Instruction*           CreateBRK(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles);

           CW65Instruction*            CreateReset(void);
           CW65Instruction*            CreateIRQ(void);
           CW65Instruction*            CreateNMI(void);
           CW65Instruction*            CreateAbort(void);
           CW65Instruction*            CreateFetchNext(void);
};


#endif // __INSTRUCTION_FACTORY_H__

