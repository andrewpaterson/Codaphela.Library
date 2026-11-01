#include "W65C02InstructionCycleFactory.h"


//The W65C02S bus cycles from SingleStepTests/65x02 (wdc65c02), which differ from the data
//sheet's table 6-5 in places.  Every cycle reads or writes memory.


//The W65C02S instruction functions are members of CW65C02, the operations call CW65CCore.
CW65Execute* E(CW65C02Func fConsumer, bool bInitialSide)
{
	return E(static_cast<CW65CFunc>(fConsumer), bInitialSide);
}


CW65Operation* DONE(bool bDecimal)
{
	//ADC and SBC in decimal mode are done one cycle later.
	if (bDecimal)
	{
		return DONE_Binary();
	}
	return DONE();
}


CW65BusCycle* DecimalCycle(CW65AddressOffsetArray* papcAddressOffsets, bool bDecimal)
{
	//The extra decimal mode cycle reads its address again.  It ends the cycle list when not needed.
	if (bDecimal)
	{
		return BusCycle(papcAddressOffsets, Operation(NoteDecimal(), IO(), DONE(), NULL));
	}
	papcAddressOffsets->Kill();
	free(papcAddressOffsets);
	return NULL;
}


CW65AddressOffset* Index(bool bIndexY)
{
	if (bIndexY)
	{
		return Y();
	}
	return X();
}


CW65Operation* PageCross(bool bIndexY)
{
	if (bIndexY)
	{
		return NoteFourY(true);
	}
	return NoteFourX(true);
}


CW65InstructionCycles* CreateW65C02ImpliedCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, false), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02ImmediateCycles(CW65C02Func fOperation, bool bDecimal, uint16 uiDecimalAddress)
{
	//ADC and SBC take one more cycle in decimal mode.  With immediate data it reads a fixed
	//address, 007F for ADC and 0000 for SBC.
	return InstructionCycles(AM_Immediate,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataLow(), PC_inc(), E(fOperation, false), DONE(bDecimal), NULL)),
		DecimalCycle(Address(Constant(uiDecimalAddress), NULL), bDecimal),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageCycles(CW65C02Func fOperation, bool bDecimal)
{
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(Read_DataLow(), E(fOperation, false), DONE(bDecimal), NULL)),
		DecimalCycle(Address(DP(), D0(), NULL), bDecimal),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageWriteCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(E(fOperation, true), Write_DataLow(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageRMWCycles(CW65C02Func fOperation)
{
	//The W65C02S reads the address twice and writes it once.
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(IO(RMW), E(fOperation, false), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageIndexedCycles(bool bIndexY, CW65C02Func fOperation, bool bDecimal)
{
	//The zero page address is read before it is indexed.  The index wraps within page 0.
	return InstructionCycles(bIndexY ? AM_DirectIndexedWithY : AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), Index(bIndexY), NULL),	Operation(Read_DataLow(), E(fOperation, false), DONE(bDecimal), NULL)),
		DecimalCycle(Address(DP(), D0(), Index(bIndexY), NULL), bDecimal),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageIndexedWriteCycles(bool bIndexY, CW65C02Func fOperation)
{
	return InstructionCycles(bIndexY ? AM_DirectIndexedWithY : AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), Index(bIndexY), NULL),	Operation(E(fOperation, true), Write_DataLow(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageIndexedWithXRMWCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),		Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),		Operation(IO(RMW), E(fOperation, false), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),		Operation(Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteCycles(CW65C02Func fOperation, bool bDecimal)
{
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(Read_DataLow(), E(fOperation, false), DONE(bDecimal), NULL)),
		DecimalCycle(Address(AA(), NULL), bDecimal),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteWriteCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(E(fOperation, true), Write_DataLow(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteRMWCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(IO(RMW), E(fOperation, false), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteIndexedCycles(EAddressingMode eMode, bool bIndexY, CW65C02Func fOperation, bool bDecimal)
{
	//Crossing a page adds a cycle that reads the last byte of the instruction again.
	return InstructionCycles(eMode,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), PageCross(bIndexY), NULL)),
		BusCycle(	Address(AA(), Index(bIndexY), NULL),	Operation(Read_DataLow(), E(fOperation, false), DONE(bDecimal), NULL)),
		DecimalCycle(Address(AA(), Index(bIndexY), NULL), bDecimal),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteIndexedWithXCycles(CW65C02Func fOperation, bool bDecimal)
{
	return CreateW65C02AbsoluteIndexedCycles(AM_AbsoluteIndexedWithX, false, fOperation, bDecimal);
}


CW65InstructionCycles* CreateW65C02AbsoluteIndexedWithYCycles(CW65C02Func fOperation, bool bDecimal)
{
	return CreateW65C02AbsoluteIndexedCycles(AM_AbsoluteIndexedWithY, true, fOperation, bDecimal);
}


CW65InstructionCycles* CreateW65C02AbsoluteIndexedWriteCycles(EAddressingMode eMode, bool bIndexY, CW65C02Func fOperation)
{
	return InstructionCycles(eMode,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NULL)),
		BusCycle(	Address(AA(), Index(bIndexY), NULL),	Operation(E(fOperation, true), Write_DataLow(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteIndexedWithXWriteCycles(CW65C02Func fOperation)
{
	return CreateW65C02AbsoluteIndexedWriteCycles(AM_AbsoluteIndexedWithX, false, fOperation);
}


CW65InstructionCycles* CreateW65C02AbsoluteIndexedWithYWriteCycles(CW65C02Func fOperation)
{
	return CreateW65C02AbsoluteIndexedWriteCycles(AM_AbsoluteIndexedWithY, true, fOperation);
}


CW65InstructionCycles* CreateW65C02AbsoluteIndexedWithXRMWCycles(CW65C02Func fOperation, bool bAlwaysIndexCycle)
{
	//ASL, LSR, ROL and ROR only take the index cycle when crossing a page, INC and DEC always do.
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), bAlwaysIndexCycle ? NULL : PageCross(false), NULL)),
		BusCycle(	Address(AA(), X(), NULL),			Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(AA(), X(), NULL),			Operation(IO(RMW), E(fOperation, false), NULL)),
		BusCycle(	Address(AA(), X(), NULL),			Operation(Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageIndexedIndirectWithXCycles(CW65C02Func fOperation, bool bDecimal)
{
	return InstructionCycles(AM_DirectIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),		Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(AA(), NULL),				Operation(Read_DataLow(), E(fOperation, false), DONE(bDecimal), NULL)),
		DecimalCycle(Address(AA(), NULL), bDecimal),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageIndexedIndirectWithXWriteCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_DirectIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),		Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(AA(), NULL),				Operation(E(fOperation, true), Write_DataLow(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageIndirectIndexedWithYCycles(CW65C02Func fOperation, bool bDecimal)
{
	//Crossing a page adds a cycle that reads the zero page operand again.
	return InstructionCycles(AM_DirectIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), PageCross(true), NULL)),
		BusCycle(	Address(AA(), Y(), NULL),			Operation(Read_DataLow(), E(fOperation, false), DONE(bDecimal), NULL)),
		DecimalCycle(Address(AA(), Y(), NULL), bDecimal),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageIndirectIndexedWithYWriteCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_DirectIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NULL)),
		BusCycle(	Address(AA(), Y(), NULL),			Operation(E(fOperation, true), Write_DataLow(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageIndirectCycles(CW65C02Func fOperation, bool bDecimal)
{
	return InstructionCycles(AM_DirectIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(AA(), NULL),				Operation(Read_DataLow(), E(fOperation, false), DONE(bDecimal), NULL)),
		DecimalCycle(Address(AA(), NULL), bDecimal),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageIndirectWriteCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_DirectIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(AA(), NULL),				Operation(E(fOperation, true), Write_DataLow(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02RelativeCycles(CW65C02Func fOperation)
{
	//Taken: one cycle reading the next instruction.  Crossing a page: one more reading the
	//target's low byte in the old page.
	return InstructionCycles(AM_Relative,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataLow(), PC_inc(), E(fOperation, false), NULL)),  //Done if branch not taken
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), DONE_SamePage(), PC_e(Address(PBR(), PC(), SignedDataLow(), NULL)), NULL)),
		BusCycle(	Address(BranchPageCross(), NULL),	Operation(IO(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02ZeroPageRelativeCycles(void)
{
	//BBRx and BBSx.  A page crossing reads the next instruction again.
	return InstructionCycles(AM_DirectRelative,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(Read_DataLow(), E(&CW65C02::BitBranchTest, false), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataLow(), PC_inc(), E(&CW65C02::BitBranch, false), NULL)),  //Done if branch not taken
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), DONE_SamePage(), PC_e(Address(PBR(), PC(), SignedDataLow(), NULL)), NULL)),
		BusCycle(	Address(BranchOrigin(), NULL),	Operation(IO(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteJMPCycles(void)
{
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteIndirectJMPCycles(void)
{
	//The W65C02S reads the high byte of the new PC twice, the pointer may cross a page.
	return InstructionCycles(AM_AbsoluteIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(AA(), o(1), NULL),		Operation(Read_NewPCH(), NULL)),
		BusCycle(	Address(AA(), o(1), NULL),		Operation(IO(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteIndexedIndirectWithXJMPCycles(void)
{
	return InstructionCycles(AM_AbsoluteIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAH(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NULL)),
		BusCycle(	Address(AA(), X(), NULL),			Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(AA(), X(), o(1), NULL),		Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02AbsoluteJSRCycles(void)
{
	//JSR pushes the address of its last byte and reads that byte after the push.
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_PCL(), SP_dec(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02StackRTSCycles(void)
{
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_NewPCL(), SP_inc(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_NewPCH(), SP_inc(), NULL)),
		BusCycle(	Address(New_PC(), NULL),		Operation(IO(), PC_e(Address(PBR(), New_PC(), NULL)), PC_inc(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02StackRTICycles(void)
{
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_DataLow(), SP_inc(), NULL)),  //Processor status, applied at the end.
		BusCycle(	Address(S1(), NULL),			Operation(Read_NewPCL(), SP_inc(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_NewPCH(), SP_inc(), PC_e(Address(PBR(), New_PC(), NULL)), E(&CW65CCore::RTIStatus, false), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02StackSoftwareInterruptCycles(CW65InterruptVector* pcInterruptVector, CW65C02Func fOperation)
{
	return InstructionCycles(AM_StackInterruptSoftware,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(), PC_inc(), NULL)),  //Read and ignore the signature byte.
		BusCycle(	Address(S(), NULL),								Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCL(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PSB(), SP_dec(), E(fOperation, false), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), NULL),			Operation(Read_AAVL(), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), o(1), NULL),		Operation(Read_AAVH(), PC_e(Address(AA(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02StackPushCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(E(fOperation, true), Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02StackPullCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_DataLow(), SP_inc(), E(fOperation, false), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02WaitForInterruptCycles(void)
{
	return InstructionCycles(AM_WaitForInterrupt,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(WaitOperation(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02StopTheClockCycles(CW65C02Func fOperation)
{
	return InstructionCycles(AM_StopTheClock,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, false), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02NOPCycles(int iBytes)
{
	//The reserved opcodes X3 and XB take one cycle, X2 two cycles (they skip a byte).
	if (iBytes == 1)
	{
		return InstructionCycles(AM_Implied,
			BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), DONE(), NULL)),
			NULL);
	}
	return InstructionCycles(AM_Immediate,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), PC_inc(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02NOPZeroPageCycles(void)
{
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(IO(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02NOPZeroPageIndexedCycles(void)
{
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),	Operation(IO(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateW65C02NOPAbsoluteCycles(void)
{
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), DONE(), NULL)),
		NULL);
}

