#include <stdlib.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
// The 65C64 from 658-Computer/W65C64/65C64 Instruction Architecture.xlsx.
// Programs run from reset.  Every test checks the cycles each instruction takes:
// the opcode, its operand words, pointer words and the data words it reads and writes.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Encoding(void)
{
	S65C64Decoded	sDecoded;

	//Worked out by hand from the opcode encoding sheet.
	AssertShortHex((uint16)0x2110, Op65RI(C65_CPY, R65_A, 1));
	AssertShortHex((uint16)0xC1E5, Op65RE(C65_ADD, R65_X, 1));
	AssertShortHex((uint16)0x20AA, Op65Jump(R65_S, OPERAND_MODE_IMMEDIATE, 4, false, true, false));
	AssertShortHex((uint16)0x003B, Op65Stp());
	AssertShortHex((uint16)0x660C, Op65Branch(B65_CLEAR, F65_Z, 0, -8));

	Decode65C64(0x2110, &sDecoded);
	AssertInt(I65_Cpy, sDecoded.eInstruction);
	AssertInt(OK65_Register, sDecoded.sFirst.eKind);
	AssertInt(R65_A, sDecoded.sFirst.eRegister);
	AssertInt(OK65_Immediate, sDecoded.sSecond.eKind);
	AssertInt(1, sDecoded.sSecond.uiImmediateBytes);
	AssertInt(1, sDecoded.uiOperandWords);

	Decode65C64(Op65M(C65_CPY, true, R65_B, OPERAND_MODE_ABSOLUTE_INDIRECT_REGISTER, R65_Y), &sDecoded);
	AssertInt(I65_Cpy, sDecoded.eInstruction);
	AssertInt(OK65_Memory, sDecoded.sFirst.eKind);
	AssertInt(OK65_Register, sDecoded.sSecond.eKind);
	AssertInt(R65_B, sDecoded.sSecond.eRegister);
	AssertTrue(sDecoded.bIndirect);
	AssertInt(R65_Y, sDecoded.ePost);
	AssertInt(R65_Zero, sDecoded.ePre1);
	AssertInt(2, sDecoded.uiOperandWords);
	AssertInt(MA65_Write, sDecoded.eAccess);

	//Arithmetic, shifts and cpyx address memory with none, V, X or Y (the user, 2026-10-28).
	Decode65C64(Op65ME(C65_SUB, -3, OPERAND_MODE_OFFSET, R65_V), &sDecoded);
	AssertInt(I65_Sub, sDecoded.eInstruction);
	AssertInt(OK65_Memory, sDecoded.sFirst.eKind);
	AssertInt(OK65_Embedded, sDecoded.sSecond.eKind);
	AssertLong(-3, sDecoded.sSecond.iEmbedded);
	AssertInt(R65_V, sDecoded.ePre1);
	AssertInt(1, sDecoded.uiOperandWords);
	AssertInt(MA65_ReadWrite, sDecoded.eAccess);

	Decode65C64(Op65M2(C65_AND, false, R65_X, OPERAND_MODE_REGISTER_INDIRECT_REGISTER, R65_S, R65_W), &sDecoded);
	AssertInt(I65_And, sDecoded.eInstruction);
	AssertInt(R65_X, sDecoded.sFirst.eRegister);
	AssertInt(R65_S, sDecoded.ePre1);
	AssertInt(R65_W, sDecoded.ePost);
	AssertTrue(sDecoded.bIndirect);
	AssertInt(0, sDecoded.uiOperandWords);

	Decode65C64(Op65RE(C65_CPY, R65_Y, -16), &sDecoded);
	AssertLong(-16, sDecoded.sSecond.iEmbedded);
	Decode65C64(Op65RE(C65_CPY, R65_Y, 16), &sDecoded);
	AssertLong(16, sDecoded.sSecond.iEmbedded);

	Decode65C64(Op65Push(R65_S, M65_A | M65_Y), &sDecoded);
	AssertInt(I65_PshRegisters, sDecoded.eInstruction);
	AssertInt(M65_A | M65_Y, sDecoded.uiMask);

	//Not implemented: hardware register instructions.  Code 8 is block and vector operations.
	Decode65C64(0x0018, &sDecoded);
	AssertInt(I65_Vcpy, sDecoded.eInstruction);
	Decode65C64(0x000F, &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
	Decode65C64(0x0009, &sDecoded);
	AssertInt(I65_Scp, sDecoded.eInstruction);
	AssertInt(1, sDecoded.uiOperandWords);
	Decode65C64(Op65RI(C65_CPY, R65_A, 0), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);

	//Code 12 extend 2 is brae and brane, extend 3 is cpyx.  The shifts are code 14 extend 2 and 3.
	AssertShortHex((uint16)0x4FAC, Op65BranchEqual(false, F65_C, F65_Z, 8));
	Decode65C64(0x4FAC, &sDecoded);
	AssertInt(I65_Brae, sDecoded.eInstruction);
	AssertInt(F65_C, sDecoded.uiFlagFirst);
	AssertInt(F65_Z - 2, sDecoded.uiFlagSecond);
	AssertInt(OK65_Embedded, sDecoded.sSecond.eKind);
	AssertInt(8, sDecoded.iBranchOffset);
	Decode65C64(Op65BranchEqualImmediate(true, F65_F, F65_G, 3), &sDecoded);
	AssertInt(I65_Brane, sDecoded.eInstruction);
	AssertInt(OK65_Immediate, sDecoded.sSecond.eKind);
	AssertInt(3, sDecoded.sSecond.uiImmediateBytes);
	AssertInt(2, sDecoded.uiOperandWords);
	Decode65C64(Op65BranchEqual(false, F65_TRUE, F65_Z, 4), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
	Decode65C64(Op65BranchEqual(true, F65_Z, F65_FALSE, 4), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);

	AssertShortHex((uint16)0xF03C, Op65Cpyx(true, R65_Y, OPERAND_MODE_ABSOLUTE, R65_X, 1));
	Decode65C64(0xF03C, &sDecoded);
	AssertInt(I65_Cpyx, sDecoded.eInstruction);
	AssertInt(OK65_Memory, sDecoded.sFirst.eKind);
	AssertInt(R65_Y, sDecoded.sSecond.eRegister);
	AssertInt(R65_X, sDecoded.ePre1);
	AssertInt(OPERAND_MODE_ABSOLUTE, sDecoded.uiMode);
	AssertInt(1, sDecoded.uiWidthBytes);
	AssertInt(2, sDecoded.uiOperandWords);
	AssertInt(MA65_Write, sDecoded.eAccess);
	Decode65C64(Op65Cpyx(false, R65_X, OPERAND_MODE_REGISTER_INDIRECT, R65_V, 8), &sDecoded);
	AssertInt(I65_Cpyx, sDecoded.eInstruction);
	AssertInt(R65_X, sDecoded.sFirst.eRegister);
	AssertInt(OK65_Memory, sDecoded.sSecond.eKind);
	AssertInt(R65_V, sDecoded.ePre1);
	AssertTrue(sDecoded.bIndirect);
	AssertInt(8, sDecoded.uiWidthBytes);
	AssertInt(0, sDecoded.uiOperandWords);
	AssertInt(MA65_Read, sDecoded.eAccess);

	//The sign extending cpyx is code 15 extend 6.  Width code 3 is unused.
	AssertShortHex((uint16)0x96EF, Op65CpyxSigned(R65_X, OPERAND_MODE_OFFSET, R65_V, 2));
	Decode65C64(0x96EF, &sDecoded);
	AssertInt(I65_Cpyx, sDecoded.eInstruction);
	AssertTrue(sDecoded.bSigned);
	AssertInt(R65_X, sDecoded.sFirst.eRegister);
	AssertInt(OK65_Memory, sDecoded.sSecond.eKind);
	AssertInt(R65_V, sDecoded.ePre1);
	AssertInt(OPERAND_MODE_OFFSET, sDecoded.uiMode);
	AssertInt(2, sDecoded.uiWidthBytes);
	AssertInt(1, sDecoded.uiOperandWords);
	AssertInt(MA65_Read, sDecoded.eAccess);
	Decode65C64(0x96EF | (3 << 7), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);

	Decode65C64(Op65Shift(true, R65_A, 3), &sDecoded);
	AssertInt(I65_Shl, sDecoded.eInstruction);
	AssertLong(3, sDecoded.sSecond.iEmbedded);
	Decode65C64(Op65ShiftRegister(false, R65_A, R65_X), &sDecoded);
	AssertInt(I65_Shr, sDecoded.eInstruction);
	AssertInt(R65_X, sDecoded.sSecond.eRegister);

	AssertTrue(C65C64InstructionFactory::GetInstance()->NumForms() > 100);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64ResetAndStop(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	STest65C64Cycle*	psCycles;

	Test65C64Begin(&sTest, 0x1000);
	sTest.pcContext->Op(Op65Stp());

	AssertSize(4, Test65C64Reset(&sTest));
	psCycles = sTest.pcContext->masCycles;
	AssertFalse(psCycles[0].bVPA);
	AssertFalse(psCycles[0].bVDA);
	AssertTrue(psCycles[0].bVPB);
	AssertFalse(psCycles[0].bLower);
	AssertIntHex(0xFFFFFFFC, psCycles[2].uiAddress);
	AssertFalse(psCycles[2].bVPB);
	AssertTrue(psCycles[2].bRead);
	AssertShortHex((uint16)0x1000, psCycles[2].uiData);
	AssertIntHex(0xFFFFFFFE, psCycles[3].uiAddress);
	AssertFalse(psCycles[3].bVPB);
	AssertShortHex((uint16)0x0000, psCycles[3].uiData);

	pcMPU = Test65C64MPU(&sTest);
	AssertIntHex(0x1000, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFFF, pcMPU->GetS());
	AssertShortHex((uint16)0x00C0, pcMPU->GetP());
	AssertInt(1, pcMPU->GetInterruptDepth());
	AssertFalse(pcMPU->IsStopped());

	sTest.pcContext->ClearCycles();
	AssertFalse(sTest.cMeta.TickInstruction());
	AssertSize(1, sTest.pcContext->muiNumCycles);
	psCycles = sTest.pcContext->masCycles;
	AssertIntHex(0x1000, psCycles[0].uiAddress);
	AssertTrue(psCycles[0].bOPV);
	AssertTrue(psCycles[0].bVPA);
	AssertFalse(psCycles[0].bVDA);
	AssertTrue(psCycles[0].bRead);
	AssertTrue(psCycles[0].bLower);
	AssertTrue(psCycles[0].bUpper);
	AssertTrue(pcMPU->IsStopped());
	AssertIntHex(0x1002, pcMPU->GetProgramCounter());

	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64CopyRegister(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65RI(C65_CPY, R65_A, 1), 0x0080);
	pcContext->Op(Op65RE(C65_CPY, R65_B, -1));
	pcContext->Op(Op65Wth(R65_X, 2));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_A, true));
	pcContext->Op(Op65Wth(R65_Y, 2));
	pcContext->Op(Op65RR(C65_CPY, R65_Y, R65_A, false));
	pcContext->Op(Op65RE(C65_CPY, R65_V, 16));
	pcContext->Op(Op65RE(C65_CPY, R65_W, -16));
	pcContext->Op(Op65RR(C65_CPY, R65_A, R65_Zero));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x80LL, pcMPU->GetA());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest));
	AssertTrue(pcContext->masCycles[1].bVPA);
	AssertFalse(pcContext->masCycles[1].bOPV);

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFLL, pcMPU->GetB());

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFF80LL, pcMPU->GetX());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x0080LL, pcMPU->GetY());
	AssertShortHex((uint16)0, Test65C64Flags(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(16, pcMPU->GetV());
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0xFFFFFFF0, pcMPU->GetW());

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0LL, pcMPU->GetA());
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64Flags(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Immediate(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wtth(R65_A, R65_B, 8));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_A, 8), 0x0123456789ABCDEFULL);
	pcContext->Op(Op65RR(C65_CPY, R65_B, R65_A));
	pcContext->Op(Op65RE(C65_ADD, R65_B, 1));
	pcContext->Op(Op65Wth(R65_X, 4));
	pcContext->OpLong(Op65RI(C65_CPY, R65_X, 3, true), 0x00800000);
	pcContext->OpLong(Op65RI(C65_CPY, R65_X, 3, false), 0x00800000);
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));
	AssertShortHex((uint16)0xF0C0, pcMPU->GetP());
	AssertSize(5, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x0123456789ABCDEFLL, pcMPU->GetA());
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x0123456789ABCDF0LL, pcMPU->GetB());
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFF800000LL, pcMPU->GetX());
	AssertSize(3, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x00800000LL, pcMPU->GetX());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64SetData(CTest65C64Context* pcContext)
{
	size	i;

	for (i = 0; i < 9; i++)
	{
		pcContext->SetByte((uint32)(0x2000 + i), (uint8)(0x11 * (i + 1)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Load(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	STest65C64Cycle*	psCycle;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	Test65C64SetData(pcContext);
	pcContext->Op(Op65Wth(R65_A, 4));
	pcContext->OpLong(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x2000);
	pcContext->OpLong(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x2001);
	pcContext->Op(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_OFFSET, R65_Zero), 0x2002);
	pcContext->Op(Op65Wth(R65_X, 2));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 3));
	pcContext->OpLong(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ABSOLUTE, R65_X), 0x2000);
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2004);
	pcContext->Op(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ADDRESS_REGISTER, R65_V));
	pcContext->Op(Op65M2(C65_CPY, false, R65_A, OPERAND_MODE_REGISTER_REGISTER, R65_V, R65_X));
	pcContext->Op(Op65Wth(R65_A, 1));
	pcContext->OpLong(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x2001);
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));

	AssertSize(5, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x44332211LL, pcMPU->GetA());
	psCycle = &pcContext->masCycles[3];
	AssertIntHex(0x2000, psCycle->uiAddress);
	AssertTrue(psCycle->bVDA);
	AssertFalse(psCycle->bVPA);
	AssertTrue(psCycle->bLower);
	AssertTrue(psCycle->bUpper);

	AssertSize(6, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x55443322LL, pcMPU->GetA());
	AssertIntHex(0x2000, pcContext->masCycles[3].uiAddress);
	AssertFalse(pcContext->masCycles[3].bLower);
	AssertTrue(pcContext->masCycles[3].bUpper);
	AssertIntHex(0x2004, pcContext->masCycles[5].uiAddress);
	AssertTrue(pcContext->masCycles[5].bLower);
	AssertFalse(pcContext->masCycles[5].bUpper);

	AssertSize(4, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x66554433LL, pcMPU->GetA());

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(6, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x77665544LL, pcMPU->GetA());

	AssertSize(2, Test65C64Step(&sTest));
	AssertIntHex(0x2004, pcMPU->GetV());
	AssertSize(3, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x88776655LL, pcMPU->GetA());

	AssertSize(4, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x00009988LL, pcMPU->GetA());

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(4, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x00009922LL, pcMPU->GetA());
	AssertFalse(pcContext->masCycles[3].bLower);
	AssertTrue(pcContext->masCycles[3].bUpper);

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Indirect(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	Test65C64SetData(pcContext);
	pcContext->SetLong(0x3000, 0x2000);
	pcContext->SetLong(0x3005, 0x2004);

	pcContext->Op(Op65Wth(R65_A, 4));
	pcContext->OpLong(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ABSOLUTE_INDIRECT, R65_Zero), 0x3000);
	pcContext->Op(Op65RE(C65_CPY, R65_X, 5));
	pcContext->OpLong(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ABSOLUTE_REGISTER_INDIRECT, R65_X), 0x3000);
	pcContext->Op(Op65RE(C65_CPY, R65_Y, 1));
	pcContext->OpLong(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ABSOLUTE_INDIRECT_REGISTER, R65_Y), 0x3000);
	pcContext->Op(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_OFFSET_INDIRECT, R65_Zero), 0x3000);
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x3000);
	pcContext->Op(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_REGISTER_INDIRECT, R65_V));
	pcContext->Op(Op65M2(C65_CPY, false, R65_A, OPERAND_MODE_REGISTER_REGISTER_INDIRECT, R65_V, R65_X));
	pcContext->Op(Op65M2(C65_CPY, false, R65_A, OPERAND_MODE_REGISTER_INDIRECT_REGISTER, R65_V, R65_Y));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(7, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x44332211LL, pcMPU->GetA());
	AssertIntHex(0x3000, pcContext->masCycles[3].uiAddress);
	AssertIntHex(0x3002, pcContext->masCycles[4].uiAddress);
	AssertIntHex(0x2000, pcContext->masCycles[5].uiAddress);

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(8, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x88776655LL, pcMPU->GetA());

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(8, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x55443322LL, pcMPU->GetA());

	AssertSize(6, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x44332211LL, pcMPU->GetA());

	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(5, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x44332211LL, pcMPU->GetA());

	AssertSize(6, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x88776655LL, pcMPU->GetA());

	AssertSize(6, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x55443322LL, pcMPU->GetA());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Store(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetWord(0x4012, 0x5555);
	pcContext->Op(Op65Wth(R65_A, 2));
	pcContext->Op(Op65RI(C65_CPY, R65_A, 2), 0xBEEF);
	pcContext->OpLong(Op65M(C65_CPY, true, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x4001);
	pcContext->OpLong(Op65ME(C65_CPY, -1, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x4010);
	pcContext->OpLong(Op65M(C65_CPY, true, R65_Zero, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x4012);
	pcContext->OpLong(Op65ME(C65_ADD, 1, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x4001);
	pcContext->OpLong(Op65M(C65_XOR, true, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x4001);
	pcContext->OpLong(Op65M(C65_TST, false, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x4001);
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));

	AssertSize(5, Test65C64Step(&sTest));
	AssertByteHex((uint8)0xEF, pcContext->GetByte(0x4001));
	AssertByteHex((uint8)0xBE, pcContext->GetByte(0x4002));
	AssertByteHex((uint8)0x00, pcContext->GetByte(0x4000));
	AssertByteHex((uint8)0x00, pcContext->GetByte(0x4003));
	AssertFalse(pcContext->masCycles[3].bRead);
	AssertIntHex(0x4000, pcContext->masCycles[3].uiAddress);
	AssertFalse(pcContext->masCycles[3].bLower);
	AssertTrue(pcContext->masCycles[3].bUpper);
	AssertIntHex(0x4002, pcContext->masCycles[4].uiAddress);
	AssertTrue(pcContext->masCycles[4].bLower);
	AssertFalse(pcContext->masCycles[4].bUpper);

	//Without a register the operation is 32 bits.
	AssertSize(5, Test65C64Step(&sTest));
	AssertIntHex(0xFFFFFFFF, pcContext->GetLong(0x4010));

	AssertSize(5, Test65C64Step(&sTest));
	AssertIntHex(0x00000000, pcContext->GetLong(0x4012));
	AssertShortHex((uint16)0xFFFF, pcContext->GetWord(0x4010));

	AssertSize(9, Test65C64Step(&sTest));
	AssertIntHex(0x0000BEF0, pcContext->GetLong(0x4001));
	AssertShortHex((uint16)0, Test65C64Flags(&sTest));

	AssertSize(7, Test65C64Step(&sTest));
	AssertShortHex((uint16)0x001F, pcContext->GetWord(0x4001));
	AssertShortHex((uint16)0, Test65C64Flags(&sTest));

	AssertSize(5, Test65C64Step(&sTest));
	AssertShortHex((uint16)0x001F, pcContext->GetWord(0x4001));
	AssertLongLongIntHex(0xBEEFLL, pcMPU->GetA());
	AssertShortHex((uint16)0, Test65C64Flags(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Arithmetic(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65RI(C65_CPY, R65_A, 1), 0x7F);
	pcContext->Op(Op65RE(C65_ADD, R65_A, 1));
	pcContext->Op(Op65RI(C65_ADD, R65_A, 1), 0x80);
	pcContext->Op(Op65RE(C65_SUB, R65_A, 1));
	pcContext->Op(Op65RI(C65_CMP, R65_A, 1), 0xFF);
	pcContext->Op(Op65RI(C65_CPY, R65_A, 1), 0xFE);
	pcContext->Op(Op65Wth(R65_X, 2));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65RR(C65_ADD, R65_X, R65_A, true));
	pcContext->Op(Op65RR(C65_ADD, R65_X, R65_A, false));
	pcContext->Op(Op65Wth(R65_B, 8));
	pcContext->Op(Op65RE(C65_CPY, R65_B, -1));
	pcContext->Op(Op65RE(C65_ADD, R65_B, 1));
	pcContext->Op(Op65RE(C65_SUB, R65_B, 1));
	pcContext->Op(Op65RE(C65_SUB, R65_B, -8));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x80LL, pcMPU->GetA());
	AssertShortHex((uint16)(STATUS65_NEGATIVE | STATUS65_OVERFLOW), Test65C64Flags(&sTest));

	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x00LL, pcMPU->GetA());
	AssertShortHex((uint16)(STATUS65_ZERO | STATUS65_CARRY | STATUS65_OVERFLOW), Test65C64Flags(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFLL, pcMPU->GetA());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest));

	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFLL, pcMPU->GetA());
	AssertShortHex((uint16)(STATUS65_ZERO | STATUS65_CARRY), Test65C64Flags(&sTest));

	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFFELL, pcMPU->GetX());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x00FCLL, pcMPU->GetX());
	AssertShortHex((uint16)STATUS65_CARRY, Test65C64Flags(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex((int64)0xFFFFFFFFFFFFFFFFULL, pcMPU->GetB());
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0LL, pcMPU->GetB());
	AssertShortHex((uint16)(STATUS65_ZERO | STATUS65_CARRY), Test65C64Flags(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex((int64)0xFFFFFFFFFFFFFFFFULL, pcMPU->GetB());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(7LL, pcMPU->GetB());
	AssertShortHex((uint16)STATUS65_CARRY, Test65C64Flags(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// cmp sets v like sub (the user, 2026-10-23) but leaves the register.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64CompareOverflow(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65RI(C65_CPY, R65_A, 1), 0x80);
	pcContext->Op(Op65RI(C65_CMP, R65_A, 1), 0x01);
	pcContext->Op(Op65RI(C65_CMP, R65_A, 1), 0x80);
	pcContext->Op(Op65RI(C65_CPY, R65_A, 1), 0x7F);
	pcContext->Op(Op65RI(C65_CMP, R65_A, 1), 0xFF);
	pcContext->Op(Op65RI(C65_CMP, R65_A, 1), 0x01);
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	//-128 - 1 overflows.
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x80LL, pcMPU->GetA());
	AssertShortHex((uint16)(STATUS65_CARRY | STATUS65_OVERFLOW), Test65C64Flags(&sTest));

	AssertSize(2, Test65C64Step(&sTest));
	AssertShortHex((uint16)(STATUS65_ZERO | STATUS65_CARRY), Test65C64Flags(&sTest));

	//127 - -1 overflows.
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x7FLL, pcMPU->GetA());
	AssertShortHex((uint16)(STATUS65_NEGATIVE | STATUS65_OVERFLOW), Test65C64Flags(&sTest));

	//A compare that does not overflow clears v.
	AssertSize(2, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_CARRY, Test65C64Flags(&sTest));

	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Subroutine(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->OpLong(Op65Jump(R65_S, OPERAND_MODE_IMMEDIATE, 4, false, true, false), 0x1100);
	pcContext->Op(Op65Stp());
	pcContext->Org(0x1100);
	pcContext->Op(Op65RI(C65_CPY, R65_A, 1), 0x42);
	pcContext->Op(Op65Return(R65_S, false, true, false));

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(5, Test65C64Step(&sTest));
	AssertIntHex(0x1100, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFFB, pcMPU->GetS());
	AssertIntHex(0x1006, pcContext->GetLong(0x1FFFC));
	AssertFalse(pcContext->masCycles[3].bRead);
	AssertIntHex(0x1FFFC, pcContext->masCycles[3].uiAddress);
	AssertIntHex(0x1FFFE, pcContext->masCycles[4].uiAddress);

	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x42LL, pcMPU->GetA());

	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(0x1006, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFFF, pcMPU->GetS());

	AssertFalse(sTest.cMeta.TickInstruction());
	AssertIntHex(0x1008, pcMPU->GetProgramCounter());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64SubroutinePushStatusAndRelative(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Sep(STATUS65_CARRY | STATUS65_NEGATIVE));
	pcContext->OpLong(Op65Jump(R65_S, OPERAND_MODE_IMMEDIATE, 4, true, true, false), 0x1100);
	pcContext->Op(Op65Jump(R65_S, OPERAND_MODE_IMMEDIATE, 1, false, true, true), 0x0078);
	pcContext->Op(Op65RI(C65_CPY, R65_X, 1), 0x00D4);
	pcContext->Op(Op65Jump(R65_S, OPERAND_MODE_REGISTER, R65_X, true, true, true));
	pcContext->Op(Op65Stp());
	pcContext->Org(0x1100);
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65Return(R65_S, true, true, false));
	pcContext->Org(0x1084);
	pcContext->Op(Op65RE(C65_CPY, R65_Y, 2));
	pcContext->Op(Op65Return(R65_S, false, true, false));
	pcContext->Org(0x0FE6);
	pcContext->Op(Op65Cep(STATUS65_NEGATIVE));
	pcContext->Op(Op65RE(C65_CPY, R65_B, 3));
	pcContext->Op(Op65Return(R65_S, true, true, false));

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));
	AssertShortHex((uint16)0x00C9, pcMPU->GetP());

	//jsr pushing C then P: the opcode, 2 operand words, 2 words of C and 1 of P.
	AssertSize(6, Test65C64Step(&sTest));
	AssertIntHex(0x1100, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFF9, pcMPU->GetS());
	AssertIntHex(0x1008, pcContext->GetLong(0x1FFFC));
	AssertShortHex((uint16)0x00C9, pcContext->GetWord(0x1FFFA));
	AssertFalse(pcContext->masCycles[3].bRead);
	AssertIntHex(0x1FFFC, pcContext->masCycles[3].uiAddress);
	AssertIntHex(0x1FFFE, pcContext->masCycles[4].uiAddress);
	AssertFalse(pcContext->masCycles[5].bRead);
	AssertIntHex(0x1FFFA, pcContext->masCycles[5].uiAddress);

	AssertSize(1, Test65C64Step(&sTest));
	AssertShortHex((uint16)0x00C8, pcMPU->GetP());
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(1LL, pcMPU->GetA());

	//ret pulling P then C.
	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x1008, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFFF, pcMPU->GetS());
	AssertShortHex((uint16)0x00C9, pcMPU->GetP());

	//brsr with an 8 bit immediate: relative to the next instruction (0x100C).
	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x1084, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFFB, pcMPU->GetS());
	AssertIntHex(0x100C, pcContext->GetLong(0x1FFFC));

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(2LL, pcMPU->GetY());
	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(0x100C, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFFF, pcMPU->GetS());

	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xD4LL, pcMPU->GetX());

	//brsr pushing P with an 8 bit register: X (-44) is sign extended, relative to 0x1012.
	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x0FE6, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFF9, pcMPU->GetS());
	AssertIntHex(0x1012, pcContext->GetLong(0x1FFFC));
	AssertShortHex((uint16)0x00C9, pcContext->GetWord(0x1FFFA));

	AssertSize(1, Test65C64Step(&sTest));
	AssertShortHex((uint16)0x00C1, pcMPU->GetP());
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(3LL, pcMPU->GetB());
	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x1012, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFFF, pcMPU->GetS());
	AssertShortHex((uint16)0x00C9, pcMPU->GetP());

	AssertFalse(sTest.cMeta.TickInstruction());
	AssertIntHex(0x1014, pcMPU->GetProgramCounter());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Branch(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65RR(C65_CPY, R65_A, R65_Zero));
	pcContext->Op(Op65Branch(B65_CLEAR, F65_Z, 0, 4));
	pcContext->Op(Op65Branch(B65_SET, F65_Z, 0, 6));
	pcContext->Op(Op65RI(C65_CPY, R65_B, 1), 0x11);
	pcContext->Op(Op65RE(C65_CPY, R65_B, 2));
	AssertIntHex(0x100C, pcContext->Here());
	pcContext->Op(Op65Bal(4));
	pcContext->Op(Op65RE(C65_CPY, R65_Y, 1));
	pcContext->Op(Op65RE(C65_CPY, R65_Y, 2));
	AssertIntHex(0x1012, pcContext->Here());
	pcContext->Op(Op65Nop(3));
	pcContext->Op(Op65BranchImmediate(B65_SET, F65_TRUE, 0, 2), 4);
	pcContext->Op(Op65RE(C65_CPY, R65_X, 7));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 8));
	AssertIntHex(0x101C, pcContext->Here());
	pcContext->OpLong(Op65Jump(R65_S, OPERAND_MODE_IMMEDIATE, 4, false, false, false), 0x1030);
	pcContext->Op(Op65Stp());
	pcContext->Org(0x1030);
	pcContext->Op(Op65Jump(R65_S, OPERAND_MODE_IMMEDIATE, 1, false, false, true), 0x0002);
	pcContext->Op(Op65RE(C65_CPY, R65_X, 9));
	AssertIntHex(0x1036, pcContext->Here());
	pcContext->Op(Op65Branch(B65_AND, F65_Z, 1, 2));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 10));
	pcContext->Op(Op65Branch(B65_XOR, F65_Z, 1, 2));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 1));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0x1004, pcMPU->GetProgramCounter());
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0x100C, pcMPU->GetProgramCounter());
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0x1012, pcMPU->GetProgramCounter());
	AssertSize(4, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertIntHex(0x101C, pcMPU->GetProgramCounter());
	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(0x1030, pcMPU->GetProgramCounter());
	AssertSize(2, Test65C64Step(&sTest));
	AssertIntHex(0x1036, pcMPU->GetProgramCounter());
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0x103A, pcMPU->GetProgramCounter());
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0x103C, pcMPU->GetProgramCounter());
	AssertSize(1, Test65C64Step(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	AssertLongLongIntHex(0LL, pcMPU->GetA());
	AssertLongLongIntHex(0LL, pcMPU->GetB());
	AssertLongLongIntHex(1LL, pcMPU->GetX());
	AssertLongLongIntHex(0LL, pcMPU->GetY());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BranchEqual(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65RR(C65_CPY, R65_A, R65_Zero));
	pcContext->Op(Op65BranchEqual(false, F65_Z, F65_N, 4));
	pcContext->Op(Op65BranchEqual(true, F65_Z, F65_N, 4));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 1));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 2));
	AssertIntHex(0x100A, pcContext->Here());
	pcContext->Op(Op65BranchEqualImmediate(false, F65_N, F65_C, 1), 4);
	pcContext->Op(Op65RE(C65_CPY, R65_Y, 1));
	pcContext->Op(Op65RE(C65_CPY, R65_Y, 2));
	AssertIntHex(0x1012, pcContext->Here());
	pcContext->Op(Op65BranchEqual(true, F65_N, F65_C, -4));
	pcContext->Op(Op65BranchEqual(false, F65_F, F65_G, -8));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	//z is set and n clear, so brae falls through and brane is taken.
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0x1004, pcMPU->GetProgramCounter());
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0x100A, pcMPU->GetProgramCounter());

	//n and c are both clear.
	AssertSize(2, Test65C64Step(&sTest));
	AssertIntHex(0x1012, pcMPU->GetProgramCounter());
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0x1014, pcMPU->GetProgramCounter());

	//f and g are both clear: 0x1016 - 8.
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(0x100E, pcMPU->GetProgramCounter());

	AssertLongLongIntHex(0LL, pcMPU->GetX());
	AssertLongLongIntHex(0LL, pcMPU->GetY());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64CopyExtend(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	Test65C64SetData(pcContext);
	pcContext->SetLong(0x3008, 0xFFFFFFFF);
	pcContext->Op(Op65Wth(R65_A, 4));
	pcContext->OpLong(Op65Cpyx(false, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero, 1), 0x2001);
	pcContext->Op(Op65RE(C65_CPY, R65_A, -1));
	pcContext->Op(Op65Cpyx(false, R65_A, OPERAND_MODE_OFFSET, R65_Zero, 2), 0x2002);
	pcContext->Op(Op65Cpyx(false, R65_A, OPERAND_MODE_OFFSET, R65_Zero, 8), 0x2000);
	pcContext->Op(Op65Wth(R65_B, 4));
	pcContext->OpLong(Op65RI(C65_CPY, R65_B, 4), 0x8899AABB);
	pcContext->Op(Op65Cpyx(true, R65_B, OPERAND_MODE_OFFSET, R65_Zero, 2), 0x3000);
	pcContext->Op(Op65Cpyx(true, R65_B, OPERAND_MODE_OFFSET, R65_Zero, 8), 0x3004);
	pcContext->Op(Op65Wth(R65_X, 2));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2004);
	pcContext->Op(Op65Cpyx(false, R65_X, OPERAND_MODE_ADDRESS_REGISTER, R65_V, 1));
	pcContext->Op(Op65Cpyx(false, R65_X, OPERAND_MODE_OFFSET, R65_Zero, 1), 0x3010);
	pcContext->Op(Op65CpyxSigned(R65_A, OPERAND_MODE_OFFSET, R65_Zero, 1), 0x2007);
	pcContext->Op(Op65CpyxSigned(R65_X, OPERAND_MODE_OFFSET, R65_Zero, 1), 0x2007);
	pcContext->Op(Op65CpyxSigned(R65_A, OPERAND_MODE_OFFSET, R65_Zero, 2), 0x2006);
	pcContext->Op(Op65CpyxSigned(R65_X, OPERAND_MODE_OFFSET, R65_Zero, 4), 0x2005);
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));

	//Memory into a register zero extends.
	AssertSize(4, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x22LL, pcMPU->GetA());
	AssertShortHex((uint16)0, Test65C64Flags(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFFFFFFFLL, pcMPU->GetA());
	AssertSize(3, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x4433LL, pcMPU->GetA());

	//Wider than the register truncates.
	AssertSize(6, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x44332211LL, pcMPU->GetA());

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));

	//The register's low bits into memory, n and z from the value written.
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)0xAABB, pcContext->GetWord(0x3000));
	AssertShortHex((uint16)0, pcContext->GetWord(0x3002));
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest));
	AssertSize(6, Test65C64Step(&sTest));
	AssertIntHex(0x8899AABB, pcContext->GetLong(0x3004));
	AssertIntHex(0, pcContext->GetLong(0x3008));
	AssertShortHex((uint16)0, Test65C64Flags(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x55LL, pcMPU->GetX());
	AssertSize(3, Test65C64Step(&sTest));
	AssertLongLongIntHex(0LL, pcMPU->GetX());
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64Flags(&sTest));

	//Sign extended to the register's width.
	AssertSize(3, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFFFFF88LL, pcMPU->GetA());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFF88LL, pcMPU->GetX());
	AssertSize(3, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFFF8877LL, pcMPU->GetA());
	AssertSize(5, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x7766LL, pcMPU->GetX());
	AssertShortHex((uint16)0, Test65C64Flags(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64PushPull(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint8				uiMask;

	uiMask = M65_A | M65_B | M65_V | M65_X;
	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65RI(C65_CPY, R65_A, 1), 0x11);
	pcContext->Op(Op65Wth(R65_B, 2));
	pcContext->Op(Op65RI(C65_CPY, R65_B, 2), 0x2233);
	pcContext->OpLong(Op65RI(C65_CPY, R65_V, 4), 0x44556677);
	pcContext->Op(Op65Wth(R65_X, 2));
	pcContext->Op(Op65RI(C65_CPY, R65_X, 2), 0x1234);
	pcContext->Op(Op65Push(R65_S, uiMask));
	pcContext->Op(Op65RR(C65_CPY, R65_A, R65_Zero));
	pcContext->Op(Op65RR(C65_CPY, R65_B, R65_Zero));
	pcContext->Op(Op65RR(C65_CPY, R65_V, R65_Zero));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65Pull(R65_S, uiMask));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));

	AssertSize(9, Test65C64Step(&sTest));
	AssertIntHex(0x0001FFF6, pcMPU->GetS());
	AssertByteHex((uint8)0x11, pcContext->GetByte(0x1FFFF));
	AssertShortHex((uint16)0x2233, pcContext->GetWord(0x1FFFD));
	AssertIntHex(0x44556677, pcContext->GetLong(0x1FFF9));
	AssertShortHex((uint16)0x1234, pcContext->GetWord(0x1FFF7));

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0LL, pcMPU->GetX());

	AssertSize(9, Test65C64Step(&sTest));
	AssertIntHex(0x0001FFFF, pcMPU->GetS());
	AssertLongLongIntHex(0x11LL, pcMPU->GetA());
	AssertLongLongIntHex(0x2233LL, pcMPU->GetB());
	AssertIntHex(0x44556677, pcMPU->GetV());
	AssertLongLongIntHex(0x1234LL, pcMPU->GetX());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64State(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Sep(STATUS65_FLAG_F | STATUS65_CARRY));
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65Wth(R65_A, 2));
	pcContext->Op(Op65Gwth(R65_A, true, R65_X));
	pcContext->Op(Op65Gwth(R65_A, false, R65_Y));
	pcContext->Op(Op65RI(C65_CPY, R65_B, 1), 0x80);
	pcContext->Op(Op65Wthx(R65_B, true, 2));
	pcContext->Op(Op65Wthx(R65_B, false, 4));
	pcContext->Op(Op65RI(C65_CPY, R65_A, 2), 0xFF80);
	pcContext->Op(Op65Shift(true, R65_A, 1));
	pcContext->Op(Op65Shift(false, R65_A, 4));
	pcContext->Op(Op65Shift(true, R65_A, 8));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));
	AssertShortHex((uint16)0x00E1, pcMPU->GetP());
	AssertSize(1, Test65C64Step(&sTest));
	AssertShortHex((uint16)0x00E0, pcMPU->GetP());
	AssertSize(1, Test65C64Step(&sTest));
	AssertShortHex((uint16)0x40E0, pcMPU->GetP());
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(15LL, pcMPU->GetX());
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(16LL, pcMPU->GetY());

	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFF80LL, pcMPU->GetB());
	AssertInt(2, pcMPU->GetRegisterBytes(R65_B));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x0000FF80LL, pcMPU->GetB());
	AssertInt(4, pcMPU->GetRegisterBytes(R65_B));

	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFF00LL, pcMPU->GetA());
	AssertShortHex((uint16)(STATUS65_NEGATIVE | STATUS65_CARRY), Test65C64Flags(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x0FF0LL, pcMPU->GetA());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xF000LL, pcMPU->GetA());
	AssertShortHex((uint16)(STATUS65_NEGATIVE | STATUS65_OVERFLOW | STATUS65_CARRY), Test65C64Flags(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BreakReturn(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetVector(5, 0x1200);
	pcContext->Op(Op65Sep(STATUS65_CARRY));
	pcContext->Op(Op65Brk(5));
	pcContext->Op(Op65Stp());
	pcContext->Org(0x1200);
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65Return(R65_S, true, true, true));

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(6, Test65C64Step(&sTest));
	AssertIntHex(0x1200, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFF9, pcMPU->GetS());
	AssertIntHex(0x1004, pcContext->GetLong(0x1FFFC));
	AssertShortHex((uint16)0x00C1, pcContext->GetWord(0x1FFFA));
	AssertInt(2, pcMPU->GetInterruptDepth());
	AssertIntHex(0xFFFFF014, pcContext->masCycles[4].uiAddress);
	AssertFalse(pcContext->masCycles[4].bVPB);

	AssertSize(1, Test65C64Step(&sTest));
	AssertShortHex((uint16)0, Test65C64Flags(&sTest));

	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x1004, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFFF, pcMPU->GetS());
	AssertShortHex((uint16)STATUS65_CARRY, Test65C64Flags(&sTest));
	AssertInt(1, pcMPU->GetInterruptDepth());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// The spreadsheet's block move loop.  Its "cpy B, X+W" cannot be encoded: the first
// register of the register + register modes must be A, B, S or V.  V and A are used here.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockCopy(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	size				i;
	size				uiLoopCycles;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	for (i = 0; i < 16; i++)
	{
		pcContext->SetByte((uint32)(0x2000 + i), (uint8)(0xA0 + i));
	}
	pcContext->Op(Op65Wth(R65_B, 8));
	pcContext->Op(Op65Wth(R65_A, 4));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2000 - 8);
	pcContext->Op(Op65RI(C65_CPY, R65_A, 2), 0x2800 - 8);
	pcContext->Op(Op65RE(C65_CPY, R65_W, 16));
	pcContext->Op(Op65M2(C65_CPY, false, R65_B, OPERAND_MODE_REGISTER_REGISTER, R65_V, R65_W));
	pcContext->Op(Op65M2(C65_CPY, true, R65_B, OPERAND_MODE_REGISTER_REGISTER, R65_A, R65_W));
	pcContext->Op(Op65RE(C65_SUB, R65_W, 8));
	pcContext->Op(Op65Branch(B65_CLEAR, F65_Z, 0, -8));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	for (i = 0; i < 2; i++)
	{
		uiLoopCycles = Test65C64Step(&sTest);
		uiLoopCycles += Test65C64Step(&sTest);
		uiLoopCycles += Test65C64Step(&sTest);
		uiLoopCycles += Test65C64Step(&sTest);
		AssertSize(12, uiLoopCycles);
	}

	AssertFalse(sTest.cMeta.TickInstruction());
	for (i = 0; i < 16; i++)
	{
		AssertByteHex((uint8)(0xA0 + i), pcContext->GetByte((uint32)(0x2800 + i)));
	}
	AssertIntHex(0, pcMPU->GetW());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// The factory's instructions are made by the first Init and freed by the
// last Kill.  A Kill too many does nothing.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64FactoryUsers(void)
{
	C65C64InstructionFactory*	pcFactory;
	C65Instruction*				pcReset;
	size						uiNumForms;

	pcFactory = C65C64InstructionFactory::GetInstance();
	AssertNull(pcFactory->GetReset());

	pcFactory->Init();
	pcReset = pcFactory->GetReset();
	uiNumForms = pcFactory->NumForms();
	AssertNotNull(pcReset);
	pcFactory->Init();
	AssertPointer(pcReset, pcFactory->GetReset());
	AssertSize(uiNumForms, pcFactory->NumForms());

	pcFactory->Kill();
	AssertPointer(pcReset, pcFactory->GetReset());
	AssertSize(uiNumForms, pcFactory->NumForms());
	AssertNotNull(pcFactory->GetInstruction(0x0009));

	pcFactory->Kill();
	AssertNull(pcFactory->GetReset());
	AssertSize(0, pcFactory->NumForms());
	AssertNull(pcFactory->GetInstruction(0x0009));

	pcFactory->Kill();
	pcFactory->Init();
	AssertNotNull(pcFactory->GetReset());
	pcFactory->Kill();
	AssertNull(pcFactory->GetReset());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	Test65C64FactoryUsers();
	C65C64InstructionFactory::GetInstance()->Init();

	Test65C64Encoding();
	Test65C64ResetAndStop();
	Test65C64CopyRegister();
	Test65C64Immediate();
	Test65C64Load();
	Test65C64Indirect();
	Test65C64Store();
	Test65C64Arithmetic();
	Test65C64CompareOverflow();
	Test65C64Subroutine();
	Test65C64SubroutinePushStatusAndRelative();
	Test65C64Branch();
	Test65C64BranchEqual();
	Test65C64CopyExtend();
	Test65C64PushPull();
	Test65C64State();
	Test65C64BreakReturn();
	Test65C64BlockCopy();

	C65C64InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

