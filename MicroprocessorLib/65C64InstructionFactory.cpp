/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/ConstructorCall.h"
#include "65C64.h"
#include "65C64InstructionFactory.h"


C65C64InstructionFactory	gc65C64InstructionFactory;


struct S65C64InstructionName
{
	E65Instruction	eInstruction;
	char*			szName;
	char*			szDescription;
};


static S65C64InstructionName gasInstructionNames[NUM_65_INSTRUCTIONS] =
{
	{ I65_Illegal,			"ill",	"Not an instruction (or not implemented yet)" },
	{ I65_Cpy,				"cpy",	"Copy" },
	{ I65_And,				"and",	"And" },
	{ I65_Or,				"or",	"Or" },
	{ I65_Xor,				"xor",	"Xor" },
	{ I65_Tst,				"tst",	"Test" },
	{ I65_Add,				"add",	"Add" },
	{ I65_Sub,				"sub",	"Subtract" },
	{ I65_Cmp,				"cmp",	"Compare" },
	{ I65_Jmp,				"jmp",	"Jump absolute" },
	{ I65_Jsr,				"jsr",	"Jump absolute subroutine" },
	{ I65_Bra,				"bra",	"Jump relative" },
	{ I65_Brsr,				"brsr",	"Jump relative subroutine" },
	{ I65_Psh,				"psh",	"Push processor state / program count" },
	{ I65_Ret,				"ret",	"Return" },
	{ I65_Pul,				"pul",	"Pull processor state" },
	{ I65_Rti,				"rti",	"Return from interrupt" },
	{ I65_Sep,				"sep",	"Set state bits" },
	{ I65_Cep,				"cep",	"Clear state bits" },
	{ I65_Wth,				"wth",	"Set register width" },
	{ I65_Wtth,				"wtth",	"Set two register widths" },
	{ I65_Brk,				"brk",	"Break" },
	{ I65_Stp,				"stp",	"Stop" },
	{ I65_Gwth,				"gwth",	"Get register width" },
	{ I65_Wthx,				"wthx",	"Extend register width" },
	{ I65_Bras,				"bras",	"Branch on set" },
	{ I65_Brac,				"brac",	"Branch on clear" },
	{ I65_Brasa,			"brasa","Branch on set and set" },
	{ I65_Braso,			"braso","Branch on set xor set" },
	{ I65_Bal,				"bra",	"Branch always" },
	{ I65_Nop,				"nop",	"No operation" },
	{ I65_Shl,				"shl",	"Shift left" },
	{ I65_Shr,				"shr",	"Shift right" },
	{ I65_PshRegisters,		"psh",	"Push registers" },
	{ I65_PulRegisters,		"pul",	"Pull registers" },
	{ I65_Mul,				"mul",	"Multiply (partial)" },
	{ I65_Div,				"div",	"Divide (partial)" },
	{ I65_Scp,				"scp",	"Subcopy bytes" },
	{ I65_Scb,				"scb",	"Subcopy bits" },
	{ I65_Neg,				"neg",	"Negate" },
	{ I65_Abs,				"abs",	"Absolute" },
	{ I65_Fsbi,				"fsbi",	"Find set bit" },
	{ I65_Fcbi,				"fcbi",	"Find clear bit" },
	{ I65_Fscr,				"fscr",	"Find non-zero crumb" },
	{ I65_Fccr,				"fccr",	"Find zero crumb" },
	{ I65_Fsny,				"fsny",	"Find non-zero nybble" },
	{ I65_Fcny,				"fcny",	"Find zero nybble" },
	{ I65_Fsby,				"fsby",	"Find non-zero byte" },
	{ I65_Fcby,				"fcby",	"Find zero byte" },
	{ I65_Bist,				"bist",	"Bit to status" },
	{ I65_Stab,				"stab",	"Status to bit" },
	{ I65_Stat,				"stat",	"Status to status" },
	{ I65_PshOperand,		"psh",	"Push operand" },
	{ I65_PulOperand,		"pul",	"Pull into address" },
	{ I65_Band,				"band",	"Block and" },
	{ I65_Btst,				"btst",	"Block test" },
	{ I65_Bor,				"bor",	"Block or" },
	{ I65_Bxor,				"bxor",	"Block xor" },
	{ I65_Badd,				"badd",	"Block add" },
	{ I65_Bsub,				"bsub",	"Block subtract" },
	{ I65_Bcmp,				"bcmp",	"Block compare" },
	{ I65_Bcpy,				"bcpy",	"Block copy" },
	{ I65_Bpcpy,			"bpcpy","Block precopy" },
	{ I65_Bpand,			"bpand","Block preand" },
	{ I65_Bpor,				"bpor",	"Block preor" },
	{ I65_Bpxor,			"bpxor","Block prexor" },
	{ I65_Vcpy,				"vcpy",	"Vector copy" },
	{ I65_Vnot,				"vnot",	"Vector invert" },
	{ I65_Vand,				"vand",	"Vector and" },
	{ I65_Vtst,				"vtst",	"Vector test" },
	{ I65_Vor,				"vor",	"Vector or" },
	{ I65_Vxor,				"vxor",	"Vector xor" },
	{ I65_Vadd,				"vadd",	"Vector add" },
	{ I65_Vsub,				"vsub",	"Vector subtract" },
	{ I65_Vcmp,				"vcmp",	"Vector compare" },
	{ I65_Vpcpy,			"vpcpy","Vector precopy" },
	{ I65_Vpand,			"vpand","Vector preand" },
	{ I65_Vpor,				"vpor",	"Vector preor" },
	{ I65_Vpxor,			"vpxor","Vector prexor" },
	{ I65_Reset,			"RES",	"Reset" },
	{ I65_FetchNext,		"---",	"Fetch the next opcode" },
	{ I65_Interrupt,		"INT",	"Hardware interrupt" },
	{ I65_Wait,				"WAIT",	"Waiting for an interrupt (stp with wake bits)" },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* Get65C64InstructionName(E65Instruction eInstruction)
{
	if (eInstruction < NUM_65_INSTRUCTIONS)
	{
		return gasInstructionNames[eInstruction].szName;
	}
	return "???";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65C64InstructionFactory* C65C64InstructionFactory::GetInstance(void)
{
	return &gc65C64InstructionFactory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64InstructionFactory::Init(void)
{
	uint32			uiOpcode;
	S65C64Decoded	sDecoded;
	S65C64Form		sForm;
	size			i;

	memset(this, 0, sizeof(C65C64InstructionFactory));

	for (i = 0; i < NUM_65_INSTRUCTIONS; i++)
	{
		if (gasInstructionNames[i].eInstruction != (E65Instruction)i)
		{
			LOG_ERROR("65C64 instruction names are out of order.");
		}
	}

	for (uiOpcode = 0; uiOpcode < NUM_65C64_OPCODES; uiOpcode++)
	{
		Decode65C64((uint16)uiOpcode, &sDecoded);
		GetForm(&sDecoded, &sForm);
		mapcInstructions[uiOpcode] = GetOrCreateInstruction(&sForm);
	}

	mpcReset = CreateInstruction(0, C65C64InstructionCycleFactory::CreateResetCycles(), I65_Reset);
	mpcFetchNext = CreateInstruction(0, C65C64InstructionCycleFactory::CreateFetchNextCycles(), I65_FetchNext);
	mpcInterrupt = CreateInstruction(0, C65C64InstructionCycleFactory::CreateInterruptCycles(), I65_Interrupt);
	mpcWait = CreateInstruction(0, C65C64InstructionCycleFactory::CreateWaitCycles(), I65_Wait);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64InstructionFactory::Kill(void)
{
	size			i;
	C65Instruction*	pcInstruction;

	for (i = 0; i < FORM_65C64_HASH_SIZE; i++)
	{
		pcInstruction = mapcForms[i];
		SafeKill(pcInstruction);
		mapcForms[i] = NULL;
	}

	SafeKill(mpcReset);
	SafeKill(mpcFetchNext);
	SafeKill(mpcInterrupt);
	SafeKill(mpcWait);
	memset(mapcInstructions, 0, sizeof(mapcInstructions));
	muiNumForms = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65Instruction* C65C64InstructionFactory::GetInstruction(uint16 uiOpcode)
{
	return mapcInstructions[uiOpcode];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65Instruction* C65C64InstructionFactory::GetReset(void)
{
	return mpcReset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65Instruction* C65C64InstructionFactory::GetFetchNext(void)
{
	return mpcFetchNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65Instruction* C65C64InstructionFactory::GetInterrupt(void)
{
	return mpcInterrupt;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65Instruction* C65C64InstructionFactory::GetWait(void)
{
	return mpcWait;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size C65C64InstructionFactory::NumForms(void)
{
	return muiNumForms;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint8 GetRegisterMaskWords(uint8 uiMask)
{
	uint8	uiWords;
	size	i;

	//A, B, X and Y can be 8 bytes (5 words when not aligned).  S, V and W are 4 bytes (3 words).
	uiWords = 0;
	for (i = 0; i < 7; i++)
	{
		if (uiMask & (1 << i))
		{
			switch (R65_A + i)
			{
			case R65_S:
			case R65_V:
			case R65_W:
				uiWords += 3;
				break;
			default:
				uiWords += 5;
				break;
			}
		}
	}
	return uiWords;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64InstructionFactory::GetForm(S65C64Decoded* psDecoded, S65C64Form* psForm)
{
	memset(psForm, 0, sizeof(S65C64Form));
	psForm->eInstruction = psDecoded->eInstruction;
	psForm->uiOperandWords = psDecoded->uiOperandWords;
	psForm->bIndirect = psDecoded->bIndirect;
	psForm->eAccess = psDecoded->eAccess;
	psForm->fOperand = &C65C64::CalculateOperand;

	switch (psDecoded->eInstruction)
	{
	case I65_Cpy:	psForm->fExecute = &C65C64::CPY;	break;
	case I65_And:	psForm->fExecute = &C65C64::AND;	break;
	case I65_Or:	psForm->fExecute = &C65C64::OR;		break;
	case I65_Xor:	psForm->fExecute = &C65C64::XOR;	break;
	case I65_Tst:	psForm->fExecute = &C65C64::TST;	break;
	case I65_Add:	psForm->fExecute = &C65C64::ADD;	break;
	case I65_Sub:	psForm->fExecute = &C65C64::SUB;	break;
	case I65_Cmp:	psForm->fExecute = &C65C64::CMP;	break;
	case I65_Shl:	psForm->fExecute = &C65C64::SHL;	break;
	case I65_Shr:	psForm->fExecute = &C65C64::SHR;	break;

	case I65_Jmp:
	case I65_Jsr:
	case I65_Bra:
	case I65_Brsr:
	case I65_Psh:
		psForm->fExecute = &C65C64::JMP;
		psForm->uiMaxTransferWords = (psDecoded->bPushC ? 3 : 0) + (psDecoded->bPushP ? 2 : 0);
		break;

	case I65_Ret:
	case I65_Pul:
	case I65_Rti:
		psForm->fOperand = &C65C64::ReturnSetup;
		psForm->fExecute = &C65C64::RET;
		psForm->uiMaxTransferWords = (psDecoded->bPullC ? 3 : 0) + (psDecoded->bPullP ? 2 : 0);
		break;

	case I65_Sep:	psForm->fExecute = &C65C64::SEP;	break;
	case I65_Cep:	psForm->fExecute = &C65C64::CEP;	break;
	case I65_Wth:
	case I65_Wtth:	psForm->fExecute = &C65C64::WTH;	break;
	case I65_Stp:	psForm->fExecute = &C65C64::STP;	break;
	case I65_Gwth:	psForm->fExecute = &C65C64::GWTH;	break;
	case I65_Wthx:	psForm->fExecute = &C65C64::WTHX;	break;

	case I65_Brk:
		psForm->fExecute = &C65C64::BRK;
		psForm->uiMaxTransferWords = 5;
		psForm->bVector = true;
		break;

	case I65_Bras:
	case I65_Brac:
	case I65_Brasa:
	case I65_Braso:
		psForm->fExecute = &C65C64::BRANCH;
		break;

	case I65_Bal:	psForm->fExecute = &C65C64::BAL;	break;
	case I65_Mul:	psForm->fExecute = &C65C64::MUL;	break;
	case I65_Div:	psForm->fExecute = &C65C64::DIV;	break;
	case I65_Scp:
	case I65_Scb:	psForm->fExecute = &C65C64::SCP;	break;
	case I65_Neg:	psForm->fExecute = &C65C64::NEG;	break;
	case I65_Abs:	psForm->fExecute = &C65C64::ABS;	break;
	case I65_Fsbi:
	case I65_Fcbi:
	case I65_Fscr:
	case I65_Fccr:
	case I65_Fsny:
	case I65_Fcny:
	case I65_Fsby:
	case I65_Fcby:	psForm->fExecute = &C65C64::FIND;	break;
	case I65_Bist:	psForm->fExecute = &C65C64::BIST;	break;
	case I65_Stat:	psForm->fExecute = &C65C64::STAT;	break;

	case I65_Stab:
		//32 or more repeated bits take a cycle more.
		psForm->fExecute = &C65C64::STAB;
		psForm->uiMaxWait = (psDecoded->uiOperandWords != 0) ? 1 : 0;
		break;

	case I65_PshOperand:
		psForm->fExecute = &C65C64::PushOperand;
		psForm->uiMaxTransferWords = 5;
		break;

	case I65_PulOperand:
		psForm->fOperand = &C65C64::PullOperandSetup;
		psForm->fExecute = &C65C64::PullOperand;
		psForm->uiMaxTransferWords = 3;
		break;

	case I65_Band:
	case I65_Btst:
	case I65_Bor:
	case I65_Bxor:
	case I65_Badd:
	case I65_Bsub:
	case I65_Bcmp:
		psForm->fOperand = &C65C64::BlockSetup;
		psForm->fExecute = &C65C64::BLOCK;
		psForm->uiMaxTransferWords = 5;
		break;

	case I65_Bcpy:
		psForm->fOperand = &C65C64::BlockSetup;
		psForm->fExecute = &C65C64::BCPY;
		psForm->uiMaxTransferWords = 5;
		break;

	case I65_Bpcpy:
	case I65_Bpand:
	case I65_Bpor:
	case I65_Bpxor:
		psForm->fOperand = &C65C64::BlockSetup;
		psForm->fExecute = &C65C64::BPRE;
		psForm->uiMaxTransferWords = 5;
		break;

	case I65_Vcpy:
	case I65_Vnot:
	case I65_Vand:
	case I65_Vtst:
	case I65_Vor:
	case I65_Vxor:
	case I65_Vadd:
	case I65_Vsub:
	case I65_Vcmp:
	case I65_Vpcpy:
	case I65_Vpand:
	case I65_Vpor:
	case I65_Vpxor:
		//[W] then [V] are read (up to 5 words each), [V] is written.
		psForm->fOperand = &C65C64::VectorSetup;
		psForm->fExecute = &C65C64::VECTOR;
		psForm->uiMaxTransferWords = 10;
		break;

	case I65_Nop:
		psForm->fExecute = &C65C64::NOP;
		psForm->uiMaxWait = psDecoded->uiWait;
		break;

	case I65_PshRegisters:
		psForm->fExecute = &C65C64::PushRegisters;
		psForm->uiMaxTransferWords = GetRegisterMaskWords(psDecoded->uiMask);
		break;

	case I65_PulRegisters:
		psForm->fOperand = &C65C64::PullRegistersSetup;
		psForm->fExecute = &C65C64::PullRegisters;
		psForm->uiMaxTransferWords = GetRegisterMaskWords(psDecoded->uiMask);
		break;

	default:
		psForm->fOperand = NULL;
		psForm->fExecute = &C65C64::ILL;
		break;
	}

	//A data operand is at most 8 bytes, 5 words when it is not aligned.
	if (psDecoded->bMemoryData && (psForm->uiMaxTransferWords < 5))
	{
		psForm->uiMaxTransferWords = 5;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64InstructionFactory::GetFormKey(S65C64Form* psForm)
{
	uint32	uiKey;

	uiKey = psForm->eInstruction;
	uiKey |= (uint32)psForm->uiOperandWords << 6;
	uiKey |= (uint32)psForm->bIndirect << 9;
	uiKey |= (uint32)psForm->eAccess << 10;
	uiKey |= (uint32)psForm->uiMaxTransferWords << 12;
	uiKey |= (uint32)psForm->uiMaxWait << 18;
	uiKey |= (uint32)psForm->bVector << 22;

	//Zero marks an empty slot.
	return uiKey + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65Instruction* C65C64InstructionFactory::GetOrCreateInstruction(S65C64Form* psForm)
{
	uint32			uiKey;
	uint32			uiSlot;
	size			i;
	C65Instruction*	pcInstruction;

	uiKey = GetFormKey(psForm);
	uiSlot = (uiKey * 2654435761U) >> 20;
	for (i = 0; i < FORM_65C64_HASH_SIZE; i++)
	{
		if (mauiFormKeys[uiSlot] == uiKey)
		{
			return mapcForms[uiSlot];
		}
		if (mauiFormKeys[uiSlot] == 0)
		{
			pcInstruction = CreateInstruction(uiKey, C65C64InstructionCycleFactory::CreateCycles(psForm), psForm->eInstruction);
			mauiFormKeys[uiSlot] = uiKey;
			mapcForms[uiSlot] = pcInstruction;
			muiNumForms++;
			return pcInstruction;
		}
		uiSlot = (uiSlot + 1) & (FORM_65C64_HASH_SIZE - 1);
	}

	LOG_ERROR("Too many 65C64 instruction forms.");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65Instruction* C65C64InstructionFactory::CreateInstruction(uint32 uiKey, C65InstructionCycles* pcCycles, E65Instruction eInstruction)
{
	C65Instruction*	pcInstruction;

	pcInstruction = NewMalloc<C65Instruction>();
	pcInstruction->Init(uiKey, pcCycles, gasInstructionNames[eInstruction].szName, gasInstructionNames[eInstruction].szDescription);
	return pcInstruction;
}

