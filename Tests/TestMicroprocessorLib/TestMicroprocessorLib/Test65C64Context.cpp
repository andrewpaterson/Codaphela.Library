#include <string.h>
#include <stdlib.h>
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::Init(uint32 uiResetAddress)
{
	memset(mauiLow, 0, sizeof(mauiLow));
	memset(mauiHigh, 0, sizeof(mauiHigh));
	muiBadAccesses = 0;
	muiTotalCycles = 0;
	ClearCycles();

	SetVector(VECTOR_65C64_RESET, uiResetAddress);
	muiNext = uiResetAddress;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CTest65C64Context::Byte(uint32 uiAddress)
{
	if (uiAddress < TEST_65C64_LOW_SIZE)
	{
		return &mauiLow[uiAddress];
	}
	if (uiAddress >= TEST_65C64_HIGH_BASE)
	{
		return &mauiHigh[uiAddress - TEST_65C64_HIGH_BASE];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CTest65C64Context::GetByte(uint32 uiAddress)
{
	uint8*	puiByte;

	puiByte = Byte(uiAddress);
	if (puiByte)
	{
		return *puiByte;
	}
	muiBadAccesses++;
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::SetByte(uint32 uiAddress, uint8 uiValue)
{
	uint8*	puiByte;

	puiByte = Byte(uiAddress);
	if (puiByte)
	{
		*puiByte = uiValue;
	}
	else
	{
		muiBadAccesses++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CTest65C64Context::GetWord(uint32 uiAddress)
{
	return (uint16)(GetByte(uiAddress) | (GetByte(uiAddress + 1) << 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CTest65C64Context::GetLong(uint32 uiAddress)
{
	return (uint32)GetWord(uiAddress) | ((uint32)GetWord(uiAddress + 2) << 16);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CTest65C64Context::GetQuad(uint32 uiAddress)
{
	return (uint64)GetLong(uiAddress) | ((uint64)GetLong(uiAddress + 4) << 32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::SetWord(uint32 uiAddress, uint16 uiValue)
{
	SetByte(uiAddress, (uint8)uiValue);
	SetByte(uiAddress + 1, (uint8)(uiValue >> 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::SetLong(uint32 uiAddress, uint32 uiValue)
{
	SetWord(uiAddress, (uint16)uiValue);
	SetWord(uiAddress + 2, (uint16)(uiValue >> 16));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::SetQuad(uint32 uiAddress, uint64 uiValue)
{
	SetLong(uiAddress, (uint32)uiValue);
	SetLong(uiAddress + 4, (uint32)(uiValue >> 32));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::SetVector(uint16 uiVector, uint32 uiAddress)
{
	SetLong(VECTOR_65C64_BASE + 4 * uiVector, uiAddress);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::Org(uint32 uiAddress)
{
	muiNext = uiAddress;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::Op(uint16 uiOpcode)
{
	Word(uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::Op(uint16 uiOpcode, uint16 uiWord)
{
	Word(uiOpcode);
	Word(uiWord);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::OpLong(uint16 uiOpcode, uint32 uiLong)
{
	Word(uiOpcode);
	Long(uiLong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::OpQuad(uint16 uiOpcode, uint64 uiQuad)
{
	Word(uiOpcode);
	Long((uint32)uiQuad);
	Long((uint32)(uiQuad >> 32));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::Word(uint16 uiWord)
{
	SetWord(muiNext, uiWord);
	muiNext += 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::Long(uint32 uiLong)
{
	Word((uint16)uiLong);
	Word((uint16)(uiLong >> 16));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CTest65C64Context::Here(void)
{
	return muiNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTest65C64Context::ClearCycles(void)
{
	muiNumCycles = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Memory responds while PHI2 is high.  Only cycles with RWB driven are recorded.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64TickHigh(C65C64Meta* pcMPU, void* pvContext)
{
	CTest65C64Context*	pcContext;
	STest65C64Cycle		sCycle;
	bool				bValid;
	uint16				uiData;

	pcContext = (CTest65C64Context*)pvContext;
	if (!pcMPU->GetRWB()->IsHigh() && !pcMPU->GetRWB()->IsLow())
	{
		return;
	}

	sCycle.uiAddress = pcMPU->GetAddress()->Get();
	sCycle.bRead = pcMPU->GetRWB()->IsHigh();
	sCycle.bLower = pcMPU->GetLowerByte()->IsHigh();
	sCycle.bUpper = pcMPU->GetUpperByte()->IsHigh();
	sCycle.bOPV = pcMPU->GetOPV()->IsHigh();
	sCycle.bVPA = pcMPU->GetVPA()->IsHigh();
	sCycle.bVDA = pcMPU->GetVDA()->IsHigh();
	sCycle.bVPB = pcMPU->GetVPB()->IsHigh();

	bValid = sCycle.bVPA || sCycle.bVDA || !sCycle.bVPB;
	uiData = 0;
	if (bValid)
	{
		if (sCycle.bRead)
		{
			uiData = pcContext->GetWord(sCycle.uiAddress);
			pcMPU->GetData()->Set(uiData);
		}
		else
		{
			uiData = pcMPU->GetData()->Get();
			if (sCycle.bLower)
			{
				pcContext->SetByte(sCycle.uiAddress, (uint8)uiData);
			}
			if (sCycle.bUpper)
			{
				pcContext->SetByte(sCycle.uiAddress + 1, (uint8)(uiData >> 8));
			}
		}
	}
	sCycle.uiData = uiData;

	if (pcContext->muiNumCycles < TEST_65C64_MAX_CYCLES)
	{
		pcContext->masCycles[pcContext->muiNumCycles] = sCycle;
		pcContext->muiNumCycles++;
	}
	pcContext->muiTotalCycles++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64TickLow(C65C64Meta* pcMPU, void* pvContext)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Runs one instruction (the first call runs reset).  Returns its bus cycles.
//
//////////////////////////////////////////////////////////////////////////
size Test65C64Step(C65C64Meta* pcMPU, CTest65C64Context* pcContext)
{
	pcContext->ClearCycles();
	pcMPU->TickInstruction();
	return pcContext->muiNumCycles;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs until the processor stops.  Returns the number of instructions.
//
//////////////////////////////////////////////////////////////////////////
size Test65C64Run(C65C64Meta* pcMPU, CTest65C64Context* pcContext, size uiMaxInstructions)
{
	size	uiInstructions;

	for (uiInstructions = 0; uiInstructions < uiMaxInstructions; uiInstructions++)
	{
		pcContext->ClearCycles();
		if (!pcMPU->TickInstruction())
		{
			return uiInstructions + 1;
		}
	}
	return uiInstructions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint16 ImmediateIndex(uint8 uiBytes)
{
	return (uiBytes == 8) ? 7 : uiBytes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsArithmetic(uint8 uiCode)
{
	return (uiCode >= C65_ADD) && (uiCode <= C65_CMP);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65RR(uint8 uiCode, E65Register eFirst, E65Register eSecond, bool bSigned)
{
	if (IsArithmetic(uiCode))
	{
		return (uint16)((eFirst << 13) | (eSecond << 9) | (OPERAND_MODE_REGISTER << 5) | (bSigned << 4) | uiCode);
	}
	return (uint16)((eFirst << 13) | (bSigned << 11) | (eSecond << 8) | (OPERAND_MODE_REGISTER << 4) | uiCode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65RI(uint8 uiCode, E65Register eFirst, uint8 uiBytes, bool bSigned)
{
	if (IsArithmetic(uiCode))
	{
		return (uint16)((eFirst << 13) | (ImmediateIndex(uiBytes) << 9) | (OPERAND_MODE_IMMEDIATE << 5) | (bSigned << 4) | uiCode);
	}
	return (uint16)((eFirst << 13) | (bSigned << 11) | (ImmediateIndex(uiBytes) << 8) | (OPERAND_MODE_IMMEDIATE << 4) | uiCode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65RE(uint8 uiCode, E65Register eFirst, int iValue)
{
	uint16	uiSign;
	uint16	uiField;

	uiSign = iValue < 0;
	if (IsArithmetic(uiCode))
	{
		uiField = (uint16)(uiSign ? iValue + 8 : iValue - 1);
		return (uint16)((eFirst << 13) | (uiSign << 12) | (uiField << 9) | (OPERAND_MODE_EMBEDDED << 5) | uiCode);
	}
	uiField = (uint16)(uiSign ? iValue + 16 : iValue - 1);
	return (uint16)((eFirst << 13) | (uiSign << 12) | (uiField << 8) | (OPERAND_MODE_EMBEDDED << 4) | uiCode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65M(uint8 uiCode, bool bSwap, E65Register eRegister, uint8 uiMode, E65Register eAddress, bool bSigned)
{
	if (IsArithmetic(uiCode))
	{
		return (uint16)((bSwap << 15) | (eRegister << 11) | ((uint16)Get65C64RegisterIndex(eAddress, gae65NoneVXY, 4) << 9) | (uiMode << 5) | (bSigned << 4) | uiCode);
	}
	return (uint16)((bSwap << 15) | (eRegister << 11) | (eAddress << 8) | (uiMode << 4) | uiCode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65ME(uint8 uiCode, int iValue, uint8 uiMode, E65Register eAddress)
{
	uint16	uiSign;
	uint16	uiField;

	uiSign = iValue < 0;
	uiField = (uint16)(uiSign ? iValue + 8 : iValue - 1);
	if (IsArithmetic(uiCode))
	{
		return (uint16)((uiSign << 15) | (1 << 14) | (uiField << 11) | ((uint16)Get65C64RegisterIndex(eAddress, gae65NoneVXY, 4) << 9) | (uiMode << 5) | uiCode);
	}
	return (uint16)((uiSign << 15) | (1 << 14) | (uiField << 11) | (eAddress << 8) | (uiMode << 4) | uiCode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65M2(uint8 uiCode, bool bSwap, E65Register eRegister, uint8 uiMode, E65Register eFirst, E65Register eSecond, bool bSigned)
{
	if (IsArithmetic(uiCode))
	{
		return (uint16)((bSwap << 15) | (eRegister << 12) | ((uint16)Get65C64RegisterIndex(eSecond, gae65XY, 2) << 11) | ((uint16)Get65C64RegisterIndex(eFirst, gae65ABSV, 4) << 9) | (uiMode << 5) | (bSigned << 4) | uiCode);
	}
	return (uint16)((bSwap << 15) | (eRegister << 12) | ((uint16)Get65C64RegisterIndex(eSecond, gae65VWXY, 4) << 10) | ((uint16)Get65C64RegisterIndex(eFirst, gae65ABSV, 4) << 8) | (uiMode << 4) | uiCode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Jump(E65Register eStack, uint8 uiMode, uint8 uiField, bool bPushP, bool bPushC, bool bRelative)
{
	return (uint16)(((uint16)Get65C64RegisterIndex(eStack, gae65SVWY, 4) << 14) | (uiField << 11) | (uiMode << 7) | (bPushP << 6) | (bPushC << 5) | (bRelative << 4) | 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Return(E65Register eStack, bool bPullP, bool bPullC, bool bRti)
{
	return (uint16)(((uint16)Get65C64RegisterIndex(eStack, gae65SVWY, 4) << 14) | (bPullP << 9) | (bPullC << 8) | (bRti << 7) | (0 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Sep(uint8 uiMask)
{
	return (uint16)((uiMask << 8) | (0 << 6) | (1 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Cep(uint8 uiMask)
{
	return (uint16)((uiMask << 8) | (1 << 6) | (1 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Wth(E65Register eRegister, uint8 uiBytes)
{
	return (uint16)(((uint16)Get65C64RegisterIndex(eRegister, gae65ABXY, 4) << 14) | (1 << 11) | ((uiBytes - 1) << 8) | (2 << 6) | (1 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Wtth(E65Register eFirst, E65Register eSecond, uint8 uiBytes)
{
	return (uint16)(((uint16)Get65C64RegisterIndex(eFirst, gae65ABXY, 4) << 14) | ((uint16)Get65C64RegisterIndex(eSecond, gae65ABXY, 4) << 12) | (1 << 11) | ((uiBytes - 1) << 8) | (3 << 6) | (1 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Brk(uint16 uiVector)
{
	return (uint16)((uiVector << 6) | (2 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Stp(void)
{
	return (uint16)((0 << 6) | (3 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65StpWake(uint8 uiWakeMask)
{
	return (uint16)((uiWakeMask << 8) | (0 << 6) | (3 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Mul(void)
{
	return (uint16)((0 << 6) | (0 << 4) | 14);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Div(void)
{
	return (uint16)((1 << 6) | (0 << 4) | 14);
}


//////////////////////////////////////////////////////////////////////////
//
// The count is a register or embedded: 1 to 8 bytes, 1 to 64 bits (the top two bits
// of a bit count go in the operand word, see Op65SubcopyIndices).
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Subcopy(bool bBits, E65Register eSource, E65Register eDestination, bool bCountRegister, uint8 uiCountOrRegister)
{
	uint16	uiCount;

	if (bCountRegister)
	{
		uiCount = (uint16)((0 << 9) | (uiCountOrRegister << 6));
	}
	else if (bBits)
	{
		uiCount = (uint16)((1 << 9) | (((uiCountOrRegister - 1) & 0xF) << 5));
	}
	else
	{
		uiCount = (uint16)((1 << 9) | (((uiCountOrRegister - 1) & 0x7) << 6));
	}
	return (uint16)((eSource << 13) | (eDestination << 10) | uiCount | (bBits << 4) | 9);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Neg(E65Register eDestination, E65Register eSource)
{
	return (uint16)((eDestination << 13) | (eSource << 8) | (2 << 6) | (0 << 4) | 14);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Abs(E65Register eDestination, E65Register eSource)
{
	return (uint16)((eDestination << 13) | (eSource << 8) | (3 << 6) | (0 << 4) | 14);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Find(bool bSet, uint8 uiUnit, E65Register eRegister, E65Register eIndex, bool bDecrement)
{
	return (uint16)((eRegister << 13) | (eIndex << 10) | (bDecrement << 9) | (bSet << 8) | (uiUnit << 6) | (1 << 4) | 14);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Bist(uint8 uiFlag, E65Register eRegister, uint8 uiIndexStyle)
{
	return (uint16)((uiFlag << 13) | (eRegister << 10) | (uiIndexStyle << 7) | (4 << 4) | 15);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Stab(uint8 uiFlag, E65Register eRegister, uint8 uiIndexStyle)
{
	return (uint16)((uiFlag << 13) | (eRegister << 10) | (uiIndexStyle << 7) | (5 << 4) | 15);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Stat(uint8 uiDestinationFlag, uint8 uiSourceFlag)
{
	return (uint16)((uiDestinationFlag << 13) | (uiSourceFlag << 10) | (7 << 4) | 15);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65ShiftRegister(bool bLeft, E65Register eFirst, E65Register eSecond)
{
	return (uint16)((eFirst << 13) | (eSecond << 10) | (OPERAND_MODE_REGISTER << 6) | ((bLeft ? 2 : 3) << 4) | 14);
}


//////////////////////////////////////////////////////////////////////////
//
// uiField is the addressing register, or the immediate width index for mode 1.
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65PushOperand(E65Register eStack, uint8 uiMode, uint8 uiField)
{
	return (uint16)((eStack << 13) | (uiField << 10) | (uiMode << 6) | (1 << 4) | 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65PullOperand(E65Register eStack, uint8 uiMode, uint8 uiField)
{
	return (uint16)((eStack << 13) | (uiField << 10) | (uiMode << 6) | (3 << 4) | 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Vector(uint8 uiSub, bool bDecrementV, bool bAllowStop, uint8 uiStopStyle, bool bDecrementW, uint8 uiWidthCode)
{
	return (uint16)((bDecrementV << 15) | (bAllowStop << 14) | (uiStopStyle << 12) | (bDecrementW << 11) | (uiWidthCode << 9) | (uiSub << 5) | (1 << 4) | 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Block(uint8 uiSub, bool bDecrement, bool bAllowStop, uint8 uiStopStyle, E65Register eRegister)
{
	return (uint16)((bDecrement << 15) | (bAllowStop << 14) | (uiStopStyle << 12) | (eRegister << 9) | (uiSub << 5) | (0 << 4) | 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65SubcopyIndices(bool bBits, bool bSourceRegister, uint8 uiSource, bool bDestinationRegister, uint8 uiDestination, uint8 uiEmbeddedBitCount)
{
	uint16	uiWord;
	uint16	uiCount;

	uiWord = 0;
	if (bSourceRegister)
	{
		uiWord |= (uint16)(uiSource << 12);
	}
	else
	{
		uiWord |= (uint16)((1 << 15) | (bBits ? (uiSource << 9) : (uiSource << 12)));
	}
	if (bDestinationRegister)
	{
		uiWord |= (uint16)(uiDestination << 4);
	}
	else
	{
		uiWord |= (uint16)((1 << 7) | (bBits ? (uiDestination << 1) : (uiDestination << 4)));
	}
	if (uiEmbeddedBitCount != 0)
	{
		uiCount = (uint16)(uiEmbeddedBitCount - 1);
		uiWord |= (uint16)((((uiCount >> 4) & 1) << 8) | ((uiCount >> 5) & 1));
	}
	return uiWord;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Gwth(E65Register eQuery, bool bDecrement, E65Register eDestination)
{
	return (uint16)(((uint16)Get65C64RegisterIndex(eQuery, gae65ABXY, 4) << 14) | (bDecrement << 13) | (eDestination << 10) | (2 << 6) | (3 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Wthx(E65Register eRegister, bool bSigned, uint8 uiBytes)
{
	return (uint16)(((uint16)Get65C64RegisterIndex(eRegister, gae65ABXY, 4) << 14) | (bSigned << 11) | ((uiBytes - 1) << 8) | (3 << 6) | (3 << 4) | 11);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Branch(uint8 uiCondition, uint8 uiFirst, uint8 uiSecond, int iOffset)
{
	uint16	uiIndex;

	for (uiIndex = 0; uiIndex < 8; uiIndex++)
	{
		if (gai65BranchOffsets[uiIndex] == iOffset)
		{
			break;
		}
	}
	return (uint16)((uiFirst << 13) | (uiSecond << 11) | (uiIndex << 8) | (uiCondition << 6) | (0 << 4) | 12);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65BranchImmediate(uint8 uiCondition, uint8 uiFirst, uint8 uiSecond, uint8 uiBytes)
{
	return (uint16)((uiFirst << 13) | (uiSecond << 11) | ((uiBytes - 1) << 9) | (0 << 8) | (uiCondition << 6) | (1 << 4) | 12);
}


//////////////////////////////////////////////////////////////////////////
//
// brae and brane with an embedded offset of -8, -4, 4 or 8.
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65BranchEqual(bool bNotEqual, uint8 uiFirst, uint8 uiSecond, int iOffset)
{
	uint16	uiIndex;

	for (uiIndex = 0; uiIndex < 4; uiIndex++)
	{
		if (gai65EqualBranchOffsets[uiIndex] == iOffset)
		{
			break;
		}
	}
	return (uint16)((uiFirst << 13) | (uiSecond << 10) | (uiIndex << 8) | (1 << 7) | (bNotEqual << 6) | (2 << 4) | 12);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65BranchEqualImmediate(bool bNotEqual, uint8 uiFirst, uint8 uiSecond, uint8 uiBytes)
{
	return (uint16)((uiFirst << 13) | (uiSecond << 10) | ((uiBytes - 1) << 8) | (0 << 7) | (bNotEqual << 6) | (2 << 4) | 12);
}


//////////////////////////////////////////////////////////////////////////
//
// uiMode is a full addressing mode; it must have a minimal mode.  eRegister is A, B, X
// or Y and eAddress None, V, X or Y.
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Cpyx(bool bSwap, E65Register eRegister, uint8 uiMode, E65Register eAddress, uint8 uiBytes)
{
	uint16	uiMinimal;
	uint16	uiWidth;

	for (uiMinimal = 0; uiMinimal < 7; uiMinimal++)
	{
		if (gaui65MinimalModes[uiMinimal] == uiMode)
		{
			break;
		}
	}
	for (uiWidth = 0; uiWidth < 3; uiWidth++)
	{
		if (gaui65CopyExtendBytes[uiWidth] == uiBytes)
		{
			break;
		}
	}
	return (uint16)((bSwap << 15) | (Get65C64RegisterIndex(eRegister, gae65ABXY, 4) << 13) | (Get65C64RegisterIndex(eAddress, gae65NoneVXY, 4) << 11) | (uiMinimal << 8) | (uiWidth << 6) | (3 << 4) | 12);
}


//////////////////////////////////////////////////////////////////////////
//
// The sign extending cpyx (instruction code 15 extend 6).  uiBytes is 1, 2 or 4.
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65CpyxSigned(E65Register eRegister, uint8 uiMode, E65Register eAddress, uint8 uiBytes)
{
	uint16	uiMinimal;
	uint16	uiWidth;

	for (uiMinimal = 0; uiMinimal < 7; uiMinimal++)
	{
		if (gaui65MinimalModes[uiMinimal] == uiMode)
		{
			break;
		}
	}
	for (uiWidth = 0; uiWidth < 3; uiWidth++)
	{
		if (gaui65CopyExtendBytes[uiWidth] == uiBytes)
		{
			break;
		}
	}
	return (uint16)((Get65C64RegisterIndex(eRegister, gae65ABXY, 4) << 14) | (Get65C64RegisterIndex(eAddress, gae65NoneVXY, 4) << 12) | (uiMinimal << 9) | (uiWidth << 7) | (6 << 4) | 15);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Bal(int iOffset)
{
	return (uint16)(((iOffset & 0x7F) << 9) | (1 << 8) | (0 << 6) | (1 << 4) | 12);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Nop(uint8 uiWait)
{
	return (uint16)(((uiWait - 1) << 9) | (1 << 8) | (1 << 6) | (1 << 4) | 12);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Shift(bool bLeft, E65Register eFirst, uint8 uiAmount)
{
	return (uint16)((eFirst << 13) | ((uiAmount - 1) << 10) | (OPERAND_MODE_EMBEDDED << 6) | ((bLeft ? 2 : 3) << 4) | 14);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Push(E65Register eStack, uint8 uiMask)
{
	return (uint16)((eStack << 13) | (uiMask << 6) | (0 << 4) | 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Op65Pull(E65Register eStack, uint8 uiMask)
{
	return (uint16)((eStack << 13) | (uiMask << 6) | (2 << 4) | 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Begin(STest65C64* psTest, uint32 uiResetAddress)
{
	psTest->pcContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
	psTest->pcContext->Init(uiResetAddress);
}


//////////////////////////////////////////////////////////////////////////
//
// Starts the processor and runs reset.  Returns the cycles reset took.
//
//////////////////////////////////////////////////////////////////////////
size Test65C64Reset(STest65C64* psTest)
{
	psTest->cMeta.Init(Test65C64TickHigh, Test65C64TickLow, psTest->pcContext);
	return Test65C64Step(&psTest->cMeta, psTest->pcContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size Test65C64Step(STest65C64* psTest)
{
	return Test65C64Step(&psTest->cMeta, psTest->pcContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65C64* Test65C64MPU(STest65C64* psTest)
{
	return psTest->cMeta.GetMPU();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64End(STest65C64* psTest)
{
	AssertSize(0, psTest->pcContext->muiBadAccesses);
	psTest->cMeta.Kill();
	free(psTest->pcContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Test65C64Flags(STest65C64* psTest)
{
	return Test65C64MPU(psTest)->GetP() & (STATUS65_NEGATIVE | STATUS65_OVERFLOW | STATUS65_ZERO | STATUS65_CARRY);
}

