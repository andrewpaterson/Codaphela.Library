#ifndef __INSTRUCTION_CYCLE_FACTORY_H__
#define __INSTRUCTION_CYCLE_FACTORY_H__


class CInstructionCycleFactory
{
public:
static CInstructionCycles* createFetchOpCodeCycles()
  {
    //0*
    return new CInstructionCycles(AddressingMode.OpCode,
                                 new BusCycle(Address(PBR(), PC()), new ReadOpCode(), PC_inc()));
  }

  
static CInstructionCycles* createAbsoluteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //1a
    return new CInstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AA()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  
static CInstructionCycles* createAbsoluteWriteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //1a
    return new CInstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AA()), E(operation), Write_DataLow(), DONE8Bit(width)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Write_DataHigh(), DONE16Bit(width)));
  }

  
static CInstructionCycles* createAbsoluteJMPCycles()
  {
    //1b
    return new CInstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc(), PC_e(PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createAbsoluteJSRCycles()
  {
    //1c
    return new CInstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec(), PC_e(PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createAbsoluteRMWCycles(Executor<W65C816> operation)
  {
    //1d
    return new CInstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(RMW), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(RMW), PC_inc()),
                                 new BusCycle(Address(DBR(), AA()), Read_DataLow(RMW)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), IO(RMW), E(operation)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DBR(), AA()), new NoteThree(), Write_DataLow(RMW), DONE()));
  }

  
static CInstructionCycles* createAbsoluteIndexedIndirectWithXJMPCycles()
  {
    //2a
    return new CInstructionCycles(AbsoluteIndexedIndirectWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), AA(), X()), Read_NewPCL()),
                                 new BusCycle(Address(PBR(), AA(), X(), o(1)), Read_NewPCH(), PC_e(PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createAbsoluteIndexedIndirectWithXJSRCycles()
  {
    //2b
    return new CInstructionCycles(AbsoluteIndexedIndirectWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH()),
                                 new BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
                                 new BusCycle(Address(PBR(), AA(), X()), Read_NewPCL()),
                                 new BusCycle(Address(PBR(), AA(), X(), o(1)), Read_NewPCH(), PC_e(PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createAbsoluteIndirectJMLCycles()
  {
    //3a
    return new CInstructionCycles(AbsoluteIndirectLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(AA()), Read_NewPCL()),
                                 new BusCycle(Address(AA(), o(1)), Read_NewPCH()),
                                 new BusCycle(Address(AA(), o(2)), Read_NewPBR(), PC_e(New_PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createAbsoluteIndirectJMPCycles()
  {
    //3b
    return new CInstructionCycles(AbsoluteIndirect,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(AA()), Read_NewPCL()),
                                 new BusCycle(Address(AA(), o(1)), Read_NewPCH(), PC_e(PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createAbsoluteLongWriteCycles(Executor<W65C816> operation)
  {
    //4a
    return new CInstructionCycles(AbsoluteLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), AA()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createAbsoluteLongCycles(Executor<W65C816> operation)
  {
    //4a
    return new CInstructionCycles(AbsoluteLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), AA()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  
static CInstructionCycles* createAbsoluteLongJMLCycles()
  {
    //4b
    return new CInstructionCycles(AbsoluteLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPBR(), PC_inc(), PC_e(PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createAbsoluteLongJSLCycles()
  {
    //4c
    return new CInstructionCycles(AbsoluteLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc()),
                                 new BusCycle(Address(S()), Write_PBR(), SP_dec()),
                                 new BusCycle(Address(S()), IO()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPBR(), PC_inc()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec(), PC_e(PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createAbsoluteLongIndexedWithXCycles(Executor<W65C816> operation)
  {
    //5
    return new CInstructionCycles(AbsoluteLongIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), AA(), X()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), X(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  
static CInstructionCycles* createAbsoluteLongIndexedWithXWriteCycles(Executor<W65C816> operation)
  {
    //5
    return new CInstructionCycles(AbsoluteLongIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), AA(), X()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), X(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createAbsoluteIndexedWithXWriteCycles(Executor<W65C816> operation)
  {
    //6a
    return new CInstructionCycles(AbsoluteIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_XL()), IO(), new NoteFourX(false)),
                                 new BusCycle(Address(DBR(), AA(), X()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), X(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createAbsoluteIndexedWithXCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //6a
    return new CInstructionCycles(AbsoluteIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_XL()), IO(), new NoteFourX(true)),
                                 new BusCycle(Address(DBR(), AA(), X()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DBR(), AA(), X(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  
static CInstructionCycles* createAbsoluteIndexedWithXRMWCycles(Executor<W65C816> operation)
  {
    //6b
    return new CInstructionCycles(AbsoluteIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(RMW), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(RMW), PC_inc()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_XL()), IO(RMW)),
                                 new BusCycle(Address(DBR(), AA(), X()), Read_DataLow(RMW)),
                                 new BusCycle(Address(DBR(), AA(), X(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DBR(), AA(), X(), o(1)), IO(RMW), E(operation)),
                                 new BusCycle(Address(DBR(), AA(), X(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DBR(), AA(), X()), new NoteThree(), Write_DataLow(RMW), DONE()));
  }

  
static CInstructionCycles* createAbsoluteIndexedWithYWriteCycles(Executor<W65C816> operation)
  {
    //7
    return new CInstructionCycles(AbsoluteIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(false)),
                                 new BusCycle(Address(DBR(), AA(), Y()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createAbsoluteIndexedWithYCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //7
    return new CInstructionCycles(AbsoluteIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(true)),
                                 new BusCycle(Address(DBR(), AA(), Y()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  
static CInstructionCycles* createAccumulatorCycles(Executor<W65C816> operation)
  {
    //8
    return new CInstructionCycles(Accumulator,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation), DONE()));
  }

  
static CInstructionCycles* createBlockMoveCycles(Executor<W65C816> operation)
  {
    //9a & 9b
    return new CInstructionCycles(AddressingMode.BlockMove,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DBR(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), X()), Read_DataLow()),
                                 new BusCycle(Address(DBR(), Y()), Write_DataLow(), E(operation)),
                                 new BusCycle(Address(DBR(), Y()), IO()),
                                 new BusCycle(Address(DBR(), Y()), IO(), DONE()));
  }

  
static CInstructionCycles* createDirectCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //10a
    return new CInstructionCycles(Direct,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  
static CInstructionCycles* createDirectWriteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //10a
    return new CInstructionCycles(Direct,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), E(operation), Write_DataLow(), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), o(1)), Write_DataHigh(), DONE16Bit(width)));
  }

  
static CInstructionCycles* createDirectRMWCycles(Executor<W65C816> operation)
  {
    //10b
    return new CInstructionCycles(Direct,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(RMW), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_DataLow(RMW)),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DP(), D0(), o(1)), IO(RMW), E(operation)),
                                 new BusCycle(Address(DP(), D0(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DP(), D0()), new NoteThree(), Write_DataLow(RMW), DONE()));
  }

  
static CInstructionCycles* createDirectIndexedIndirectWithXCycles(Executor<W65C816> operation)
  {
    //11
    return new CInstructionCycles(DirectIndexedIndirectWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), X()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AA()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndexedIndirectWithXWriteCycles(Executor<W65C816> operation)
  {
    //11
    return new CInstructionCycles(DirectIndexedIndirectWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), X()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AA()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndirectCycles(Executor<W65C816> operation)
  {
    //12
    return new CInstructionCycles(DirectIndirect,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AA()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndirectWriteCycles(Executor<W65C816> operation)
  {
    //12
    return new CInstructionCycles(DirectIndirect,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AA()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndirectIndexedWithYCycles(Executor<W65C816> operation)
  {
    //13
    return new CInstructionCycles(DirectIndirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(true)),
                                 new BusCycle(Address(DBR(), AA(), Y()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndirectIndexedWithYWriteCycles(Executor<W65C816> operation)
  {
    //13
    return new CInstructionCycles(DirectIndirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(false)),
                                 new BusCycle(Address(DBR(), AA(), Y()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndirectLongIndexedWithYCycles(Executor<W65C816> operation)
  {
    //14
    return new CInstructionCycles(DirectIndirectLongIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
                                 new BusCycle(Address(AAB(), AA(), Y()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndirectLongIndexedWithYWriteCycles(Executor<W65C816> operation)
  {
    //14
    return new CInstructionCycles(DirectIndirectLongIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
                                 new BusCycle(Address(AAB(), AA(), Y()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndirectLongCycles(Executor<W65C816> operation)
  {
    //15
    return new CInstructionCycles(DirectIndirectLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
                                 new BusCycle(Address(AAB(), AA()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndirectLongWriteCycles(Executor<W65C816> operation)
  {
    //15
    return new CInstructionCycles(DirectIndirectLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
                                 new BusCycle(Address(AAB(), AA()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createDirectIndexedWithXCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //16a
    return new CInstructionCycles(DirectIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), X()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  
static CInstructionCycles* createDirectIndexedWithXWriteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //16a
    return new CInstructionCycles(DirectIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), X()), E(operation), Write_DataLow(), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), Write_DataHigh(), DONE16Bit(width)));
  }

  
static CInstructionCycles* createDirectIndexedWithXRMWCycles(Executor<W65C816> operation)
  {
    //16b
    return new CInstructionCycles(DirectIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(RMW), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(RMW), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO(RMW)),
                                 new BusCycle(Address(DP(), D0(), X()), Read_DataLow(RMW)),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), IO(RMW), E(operation)),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DP(), D0(), X()), new NoteThree(), Write_DataLow(RMW), DONE()));
  }

  
static CInstructionCycles* createDirectIndexedWithYCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //17
    return new CInstructionCycles(DirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), Y()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  
static CInstructionCycles* createDirectIndexedWithYWriteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //17
    return new CInstructionCycles(DirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), Y()), E(operation), Write_DataLow(), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), Y(), o(1)), Write_DataHigh(), DONE16Bit(width)));
  }

  
static CInstructionCycles* createImmediateREPSEPCycles(Executor<W65C816> operation)
  {
    //18
    return new CInstructionCycles(Immediate,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), PC_inc(), Read_DataLow()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataHigh(), E(operation), DONE()));
  }

  
static CInstructionCycles* createImmediateCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //18
    return new CInstructionCycles(Immediate,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), PC_inc(), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(PBR(), PC()), PC_inc(), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  
static CInstructionCycles* createImpliedCycles(Executor<W65C816> operation)
  {
    //19a
    return new CInstructionCycles(Implied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation), DONE()));
  }

  
static CInstructionCycles* createWDMImpliedCycles(Executor<W65C816> operation)
  {
    return new CInstructionCycles(Implied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), PC_inc(), IO(), E(operation), DONE()));
  }

  
static CInstructionCycles* createImpliedXBACycles(Executor<W65C816> operation)
  {
    //19b
    return new CInstructionCycles(Implied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation), DONE()));
  }

  
static CInstructionCycles* createStopTheClockCycles(Executor<W65C816> operation)
  {
    //19c
    return new CInstructionCycles(StopTheClock,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation), DONE()));
  }

  
static CInstructionCycles* createWaitForInterruptCycles()
  {
    //19d
    return new CInstructionCycles(WaitForInterrupt,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), new WaitOperation(), DONE()));
  }

  
static CInstructionCycles* createRelativeShortCycles(Executor<W65C816> operation)
  {
    //20
    return new CInstructionCycles(Relative,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataLow(), PC_inc(), E(operation)),  //Done if branch not taken
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteSix()),
                                 new BusCycle(Address(PBR(), PC()), PC_e(PBR(), PC(), new SignedDataLow()), IO(), DONE()));
  }

  
static CInstructionCycles* createRelativeLongCycles(Executor<W65C816> operation)
  {
    //21
    return new CInstructionCycles(RelativeLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataLow(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataHigh(), PC_inc(), E(operation)),  //Done if branch not taken
                                 new BusCycle(Address(PBR(), PC()), PC_e(PBR(), PC(), new SignedData()), IO(), DONE()));
  }

  
static CInstructionCycles* createStackHardwareInterruptCycles(InterruptVector interruptVector, Executor<W65C816> operation)
  {
    //22a
    return new CInstructionCycles(StackInterruptHardware,
                                 new BusCycle(Address(PBR(), PC()), new InternalOperation(true, true, true)),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), Write_PBR(), SP_dec(), PBR_e(0), new NoteSeven()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PS(), SP_dec(), E(operation)),
                                 new BusCycle(Address(VA(interruptVector)), Read_AAVL()),
                                 new BusCycle(Address(VA(interruptVector), o(1)), Read_AAVH(), PC_e(AA()), DONE()));
  }

  
static CInstructionCycles* createStackAbortInterruptCycles(InterruptVector interruptVector, Executor<W65C816> operation)
  {
    //22a
    return new CInstructionCycles(StackInterruptHardware,
                                 new BusCycle(Address(PBR(), PC()), new InternalOperation(true, true, true)),
                                 new BusCycle(Address(PBR(), PC()), IO(), new RestoreAbortValues()),
                                 new BusCycle(Address(S()), Write_PBR(), SP_dec(), PBR_e(0), new NoteSeven()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PS(), SP_dec(), E(operation)),
                                 new BusCycle(Address(VA(interruptVector)), Read_AAVL()),
                                 new BusCycle(Address(VA(interruptVector), o(1)), Read_AAVH(), PC_e(AA()), DONE()));
  }

  
static CInstructionCycles* createStackResetCycles(InterruptVector interruptVector, Executor<W65C816> operation)
  {
    //22a
    return new CInstructionCycles(StackInterruptHardware,
                                 new BusCycle(Address(PBR(), PC()), new InternalOperation(true, true, true)),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation)),
                                 new BusCycle(Address(S()), IO(), SP_dec()),
                                 new BusCycle(Address(S()), IO(), SP_dec()),
                                 new BusCycle(Address(S()), IO(), SP_dec()),
                                 new BusCycle(Address(VA(interruptVector)), Read_AAVL()),
                                 new BusCycle(Address(VA(interruptVector), o(1)), Read_AAVH(), PC_e(PBR(), AA()), DONE()));
  }

  
static CInstructionCycles* createStackPullCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //22b
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_DataLow(), SP_inc(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(S(), o(1)), Read_DataHigh(), SP_inc(), E16Bit(operation, width), DONE16Bit(width)));
  }

  
static CInstructionCycles* createStackPLBCycles(Executor<W65C816> operation)
  {
    //22b
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), SP_inc(), Read_DataLow(), E(operation), DONE()));
  }

  
static CInstructionCycles* createStackPLDCycles(Executor<W65C816> operation)
  {
    //22b
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_DataLow(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_DataHigh(), SP_inc(), E(operation), DONE()));
  }

  
static CInstructionCycles* createStackPLPCycles(Executor<W65C816> operation)
  {
    //22b
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_DataLow(), SP_inc(), E(operation), DONE()));
  }

  
static CInstructionCycles* createStackImpliedPHPCycles(Executor<W65C816> operation)
  {
    //22c
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataLow(), SP_dec(), DONE()));
  }

  
static CInstructionCycles* createStackPushCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //22c
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation)),
                                 new BusCycle(Address(S()), Write_DataHigh(), SP_dec(), new NoteOne(width)),
                                 new BusCycle(Address(S()), Write_DataLow(), SP_dec(), DONE()));
  }

  
static CInstructionCycles* createStackPHBCycles(Executor<W65C816> operation)
  {
    //22c
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataLow(), SP_dec(), DONE()));
  }

  
static CInstructionCycles* createStackPHDCycles(Executor<W65C816> operation)
  {
    //22c
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataHigh(), SP_dec()),
                                 new BusCycle(Address(S()), Write_DataLow(), SP_dec(), DONE()));
  }

  
static CInstructionCycles* createStackPHKCycles(Executor<W65C816> operation)
  {
    //22c
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataLow(), SP_dec(), DONE()));
  }

  
static CInstructionCycles* createStackPEACycles()
  {
    //22d
    return new CInstructionCycles(StackImmediate,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH()),
                                 new BusCycle(Address(S()), Write_AAH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_AAL(), SP_dec(), DONE()));
  }

  
static CInstructionCycles* createStackPEICycles()
  {
    //22e
    return new CInstructionCycles(StackDirectIndirect,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(S()), Write_AAH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_AAL(), SP_dec(), DONE()));
  }

  
static CInstructionCycles* createStackPERCycles(Executor<W65C816> operation)
  {
    //22f
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataLow(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataHigh(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataHigh(), SP_dec()),
                                 new BusCycle(Address(S()), Write_DataLow(), SP_dec(), DONE()));
  }

  
static CInstructionCycles* createStackRTICycles()
  {
    //22g
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_PS(), SP_inc()),  //Processor status
                                 new BusCycle(Address(S(), o(1)), Read_NewPCL(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCH(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPBR(), SP_inc(), PC_e(New_PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createStackRTSCycles()
  {
    //22h
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCL(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCH(), SP_inc()),
                                 new BusCycle(Address(S()), IO(), PC_e(PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createStackRTLCycles()
  {
    //22i
    return new CInstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCL(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCH(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPBR(), SP_inc(), PC_e(New_PBR(), New_PC()), DONE()));
  }

  
static CInstructionCycles* createStackSoftwareInterruptCycles(InterruptVector interruptVector, Executor<W65C816> operation)
  {
    //22j
    return new CInstructionCycles(StackInterruptSoftware,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), new InternalOperation(true, false, true), PC_inc()),  //Read and ignore the signature byte.
                                 new BusCycle(Address(S()), Write_PBR(), SP_dec(), PBR_e(0), new NoteSeven()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PS(), SP_dec(), E(operation)),
                                 new BusCycle(Address(VA(interruptVector)), Read_AAL()),
                                 new BusCycle(Address(VA(interruptVector), o(1)), Read_AAH(), PC_e(PBR(), AA()), DONE()));
  }

  
static CInstructionCycles* createStackRelativeCycles(Executor<W65C816> operation)
  {
    //23
    return new CInstructionCycles(StackRelative,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), D0()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(S(), D0(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  
static CInstructionCycles* createStackRelativeWriteCycles(Executor<W65C816> operation)
  {
    //23
    return new CInstructionCycles(StackRelative,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), D0()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(S(), D0(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  
static CInstructionCycles* createStackRelativeIndirectIndexedWithYCycles(Executor<W65C816> operation)
  {
    //24
    return new CInstructionCycles(StackRelativeIndirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), D0()), Read_AAL()),
                                 new BusCycle(Address(S(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(S(), D0(), o(1)), IO()),
                                 new BusCycle(Address(DBR(), AA(), Y()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  
static CInstructionCycles* createStackRelativeIndirectIndexedWithYWriteCycles(Executor<W65C816> operation)
  {
    //24
    return new CInstructionCycles(StackRelativeIndirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), D0()), Read_AAL()),
                                 new BusCycle(Address(S(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(S(), D0(), o(1)), IO()),
                                 new BusCycle(Address(DBR(), AA(), Y()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }
};



#endif // __INSTRUCTION_CYCLE_FACTORY_H__

