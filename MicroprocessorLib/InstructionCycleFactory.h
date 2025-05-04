#ifndef __INSTRUCTION_CYCLE_FACTORY_H__
#define __INSTRUCTION_CYCLE_FACTORY_H__


public:
class InstructionCycleFactory
{
  public:
static InstructionCycles createFetchOpCodeCycles()
  {
    //0*
    return new InstructionCycles(AddressingMode.OpCode,
                                 new BusCycle(Address(PBR(), PC()), new ReadOpCode(), PC_inc()));
  }

  public:
static InstructionCycles createAbsoluteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //1a
    return new InstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AA()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  public:
static InstructionCycles createAbsoluteWriteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //1a
    return new InstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AA()), E(operation), Write_DataLow(), DONE8Bit(width)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Write_DataHigh(), DONE16Bit(width)));
  }

  public:
static InstructionCycles createAbsoluteJMPCycles()
  {
    //1b
    return new InstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc(), PC_e(PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createAbsoluteJSRCycles()
  {
    //1c
    return new InstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec(), PC_e(PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createAbsoluteRMWCycles(Executor<W65C816> operation)
  {
    //1d
    return new InstructionCycles(Absolute,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(RMW), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(RMW), PC_inc()),
                                 new BusCycle(Address(DBR(), AA()), Read_DataLow(RMW)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), IO(RMW), E(operation)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DBR(), AA()), new NoteThree(), Write_DataLow(RMW), DONE()));
  }

  public:
static InstructionCycles createAbsoluteIndexedIndirectWithXJMPCycles()
  {
    //2a
    return new InstructionCycles(AbsoluteIndexedIndirectWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), AA(), X()), Read_NewPCL()),
                                 new BusCycle(Address(PBR(), AA(), X(), o(1)), Read_NewPCH(), PC_e(PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createAbsoluteIndexedIndirectWithXJSRCycles()
  {
    //2b
    return new InstructionCycles(AbsoluteIndexedIndirectWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH()),
                                 new BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
                                 new BusCycle(Address(PBR(), AA(), X()), Read_NewPCL()),
                                 new BusCycle(Address(PBR(), AA(), X(), o(1)), Read_NewPCH(), PC_e(PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createAbsoluteIndirectJMLCycles()
  {
    //3a
    return new InstructionCycles(AbsoluteIndirectLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(AA()), Read_NewPCL()),
                                 new BusCycle(Address(AA(), o(1)), Read_NewPCH()),
                                 new BusCycle(Address(AA(), o(2)), Read_NewPBR(), PC_e(New_PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createAbsoluteIndirectJMPCycles()
  {
    //3b
    return new InstructionCycles(AbsoluteIndirect,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(AA()), Read_NewPCL()),
                                 new BusCycle(Address(AA(), o(1)), Read_NewPCH(), PC_e(PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createAbsoluteLongWriteCycles(Executor<W65C816> operation)
  {
    //4a
    return new InstructionCycles(AbsoluteLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), AA()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createAbsoluteLongCycles(Executor<W65C816> operation)
  {
    //4a
    return new InstructionCycles(AbsoluteLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), AA()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  public:
static InstructionCycles createAbsoluteLongJMLCycles()
  {
    //4b
    return new InstructionCycles(AbsoluteLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPBR(), PC_inc(), PC_e(PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createAbsoluteLongJSLCycles()
  {
    //4c
    return new InstructionCycles(AbsoluteLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc()),
                                 new BusCycle(Address(S()), Write_PBR(), SP_dec()),
                                 new BusCycle(Address(S()), IO()),
                                 new BusCycle(Address(PBR(), PC()), Read_NewPBR(), PC_inc()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec(), PC_e(PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createAbsoluteLongIndexedWithXCycles(Executor<W65C816> operation)
  {
    //5
    return new InstructionCycles(AbsoluteLongIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), AA(), X()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), X(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  public:
static InstructionCycles createAbsoluteLongIndexedWithXWriteCycles(Executor<W65C816> operation)
  {
    //5
    return new InstructionCycles(AbsoluteLongIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), AA(), X()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), X(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createAbsoluteIndexedWithXWriteCycles(Executor<W65C816> operation)
  {
    //6a
    return new InstructionCycles(AbsoluteIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_XL()), IO(), new NoteFourX(false)),
                                 new BusCycle(Address(DBR(), AA(), X()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), X(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createAbsoluteIndexedWithXCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //6a
    return new InstructionCycles(AbsoluteIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_XL()), IO(), new NoteFourX(true)),
                                 new BusCycle(Address(DBR(), AA(), X()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DBR(), AA(), X(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  public:
static InstructionCycles createAbsoluteIndexedWithXRMWCycles(Executor<W65C816> operation)
  {
    //6b
    return new InstructionCycles(AbsoluteIndexedWithX,
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

  public:
static InstructionCycles createAbsoluteIndexedWithYWriteCycles(Executor<W65C816> operation)
  {
    //7
    return new InstructionCycles(AbsoluteIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(false)),
                                 new BusCycle(Address(DBR(), AA(), Y()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createAbsoluteIndexedWithYCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //7
    return new InstructionCycles(AbsoluteIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(true)),
                                 new BusCycle(Address(DBR(), AA(), Y()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  public:
static InstructionCycles createAccumulatorCycles(Executor<W65C816> operation)
  {
    //8
    return new InstructionCycles(Accumulator,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation), DONE()));
  }

  public:
static InstructionCycles createBlockMoveCycles(Executor<W65C816> operation)
  {
    //9a & 9b
    return new InstructionCycles(AddressingMode.BlockMove,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DBR(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
                                 new BusCycle(Address(AAB(), X()), Read_DataLow()),
                                 new BusCycle(Address(DBR(), Y()), Write_DataLow(), E(operation)),
                                 new BusCycle(Address(DBR(), Y()), IO()),
                                 new BusCycle(Address(DBR(), Y()), IO(), DONE()));
  }

  public:
static InstructionCycles createDirectCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //10a
    return new InstructionCycles(Direct,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  public:
static InstructionCycles createDirectWriteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //10a
    return new InstructionCycles(Direct,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), E(operation), Write_DataLow(), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), o(1)), Write_DataHigh(), DONE16Bit(width)));
  }

  public:
static InstructionCycles createDirectRMWCycles(Executor<W65C816> operation)
  {
    //10b
    return new InstructionCycles(Direct,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(RMW), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_DataLow(RMW)),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DP(), D0(), o(1)), IO(RMW), E(operation)),
                                 new BusCycle(Address(DP(), D0(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
                                 new BusCycle(Address(DP(), D0()), new NoteThree(), Write_DataLow(RMW), DONE()));
  }

  public:
static InstructionCycles createDirectIndexedIndirectWithXCycles(Executor<W65C816> operation)
  {
    //11
    return new InstructionCycles(DirectIndexedIndirectWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), X()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AA()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndexedIndirectWithXWriteCycles(Executor<W65C816> operation)
  {
    //11
    return new InstructionCycles(DirectIndexedIndirectWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), X()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AA()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndirectCycles(Executor<W65C816> operation)
  {
    //12
    return new InstructionCycles(DirectIndirect,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AA()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndirectWriteCycles(Executor<W65C816> operation)
  {
    //12
    return new InstructionCycles(DirectIndirect,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AA()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndirectIndexedWithYCycles(Executor<W65C816> operation)
  {
    //13
    return new InstructionCycles(DirectIndirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(true)),
                                 new BusCycle(Address(DBR(), AA(), Y()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndirectIndexedWithYWriteCycles(Executor<W65C816> operation)
  {
    //13
    return new InstructionCycles(DirectIndirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(false)),
                                 new BusCycle(Address(DBR(), AA(), Y()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndirectLongIndexedWithYCycles(Executor<W65C816> operation)
  {
    //14
    return new InstructionCycles(DirectIndirectLongIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
                                 new BusCycle(Address(AAB(), AA(), Y()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndirectLongIndexedWithYWriteCycles(Executor<W65C816> operation)
  {
    //14
    return new InstructionCycles(DirectIndirectLongIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
                                 new BusCycle(Address(AAB(), AA(), Y()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndirectLongCycles(Executor<W65C816> operation)
  {
    //15
    return new InstructionCycles(DirectIndirectLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
                                 new BusCycle(Address(AAB(), AA()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndirectLongWriteCycles(Executor<W65C816> operation)
  {
    //15
    return new InstructionCycles(DirectIndirectLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
                                 new BusCycle(Address(AAB(), AA()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(AAB(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createDirectIndexedWithXCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //16a
    return new InstructionCycles(DirectIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), X()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  public:
static InstructionCycles createDirectIndexedWithXWriteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //16a
    return new InstructionCycles(DirectIndexedWithX,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), X()), E(operation), Write_DataLow(), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), X(), o(1)), Write_DataHigh(), DONE16Bit(width)));
  }

  public:
static InstructionCycles createDirectIndexedWithXRMWCycles(Executor<W65C816> operation)
  {
    //16b
    return new InstructionCycles(DirectIndexedWithX,
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

  public:
static InstructionCycles createDirectIndexedWithYCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //17
    return new InstructionCycles(DirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), Y()), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  public:
static InstructionCycles createDirectIndexedWithYWriteCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //17
    return new InstructionCycles(DirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(DP(), D0(), Y()), E(operation), Write_DataLow(), DONE8Bit(width)),
                                 new BusCycle(Address(DP(), D0(), Y(), o(1)), Write_DataHigh(), DONE16Bit(width)));
  }

  public:
static InstructionCycles createImmediateREPSEPCycles(Executor<W65C816> operation)
  {
    //18
    return new InstructionCycles(Immediate,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), PC_inc(), Read_DataLow()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataHigh(), E(operation), DONE()));
  }

  public:
static InstructionCycles createImmediateCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //18
    return new InstructionCycles(Immediate,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), PC_inc(), Read_DataLow(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(PBR(), PC()), PC_inc(), Read_DataHigh(), E16Bit(operation, width), DONE16Bit(width)));
  }

  public:
static InstructionCycles createImpliedCycles(Executor<W65C816> operation)
  {
    //19a
    return new InstructionCycles(Implied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation), DONE()));
  }

  public:
static InstructionCycles createWDMImpliedCycles(Executor<W65C816> operation)
  {
    return new InstructionCycles(Implied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), PC_inc(), IO(), E(operation), DONE()));
  }

  public:
static InstructionCycles createImpliedXBACycles(Executor<W65C816> operation)
  {
    //19b
    return new InstructionCycles(Implied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation), DONE()));
  }

  public:
static InstructionCycles createStopTheClockCycles(Executor<W65C816> operation)
  {
    //19c
    return new InstructionCycles(StopTheClock,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation), DONE()));
  }

  public:
static InstructionCycles createWaitForInterruptCycles()
  {
    //19d
    return new InstructionCycles(WaitForInterrupt,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), new WaitOperation(), DONE()));
  }

  public:
static InstructionCycles createRelativeShortCycles(Executor<W65C816> operation)
  {
    //20
    return new InstructionCycles(Relative,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataLow(), PC_inc(), E(operation)),  //Done if branch not taken
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteSix()),
                                 new BusCycle(Address(PBR(), PC()), PC_e(PBR(), PC(), new SignedDataLow()), IO(), DONE()));
  }

  public:
static InstructionCycles createRelativeLongCycles(Executor<W65C816> operation)
  {
    //21
    return new InstructionCycles(RelativeLong,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataLow(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataHigh(), PC_inc(), E(operation)),  //Done if branch not taken
                                 new BusCycle(Address(PBR(), PC()), PC_e(PBR(), PC(), new SignedData()), IO(), DONE()));
  }

  public:
static InstructionCycles createStackHardwareInterruptCycles(InterruptVector interruptVector, Executor<W65C816> operation)
  {
    //22a
    return new InstructionCycles(StackInterruptHardware,
                                 new BusCycle(Address(PBR(), PC()), new InternalOperation(true, true, true)),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), Write_PBR(), SP_dec(), PBR_e(0), new NoteSeven()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PS(), SP_dec(), E(operation)),
                                 new BusCycle(Address(VA(interruptVector)), Read_AAVL()),
                                 new BusCycle(Address(VA(interruptVector), o(1)), Read_AAVH(), PC_e(AA()), DONE()));
  }

  public:
static InstructionCycles createStackAbortInterruptCycles(InterruptVector interruptVector, Executor<W65C816> operation)
  {
    //22a
    return new InstructionCycles(StackInterruptHardware,
                                 new BusCycle(Address(PBR(), PC()), new InternalOperation(true, true, true)),
                                 new BusCycle(Address(PBR(), PC()), IO(), new RestoreAbortValues()),
                                 new BusCycle(Address(S()), Write_PBR(), SP_dec(), PBR_e(0), new NoteSeven()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PS(), SP_dec(), E(operation)),
                                 new BusCycle(Address(VA(interruptVector)), Read_AAVL()),
                                 new BusCycle(Address(VA(interruptVector), o(1)), Read_AAVH(), PC_e(AA()), DONE()));
  }

  public:
static InstructionCycles createStackResetCycles(InterruptVector interruptVector, Executor<W65C816> operation)
  {
    //22a
    return new InstructionCycles(StackInterruptHardware,
                                 new BusCycle(Address(PBR(), PC()), new InternalOperation(true, true, true)),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation)),
                                 new BusCycle(Address(S()), IO(), SP_dec()),
                                 new BusCycle(Address(S()), IO(), SP_dec()),
                                 new BusCycle(Address(S()), IO(), SP_dec()),
                                 new BusCycle(Address(VA(interruptVector)), Read_AAVL()),
                                 new BusCycle(Address(VA(interruptVector), o(1)), Read_AAVH(), PC_e(PBR(), AA()), DONE()));
  }

  public:
static InstructionCycles createStackPullCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //22b
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_DataLow(), SP_inc(), E8Bit(operation, width), DONE8Bit(width)),
                                 new BusCycle(Address(S(), o(1)), Read_DataHigh(), SP_inc(), E16Bit(operation, width), DONE16Bit(width)));
  }

  public:
static InstructionCycles createStackPLBCycles(Executor<W65C816> operation)
  {
    //22b
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), SP_inc(), Read_DataLow(), E(operation), DONE()));
  }

  public:
static InstructionCycles createStackPLDCycles(Executor<W65C816> operation)
  {
    //22b
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_DataLow(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_DataHigh(), SP_inc(), E(operation), DONE()));
  }

  public:
static InstructionCycles createStackPLPCycles(Executor<W65C816> operation)
  {
    //22b
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_DataLow(), SP_inc(), E(operation), DONE()));
  }

  public:
static InstructionCycles createStackImpliedPHPCycles(Executor<W65C816> operation)
  {
    //22c
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataLow(), SP_dec(), DONE()));
  }

  public:
static InstructionCycles createStackPushCycles(Executor<W65C816> operation, WidthFromRegister width)
  {
    //22c
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), E(operation)),
                                 new BusCycle(Address(S()), Write_DataHigh(), SP_dec(), new NoteOne(width)),
                                 new BusCycle(Address(S()), Write_DataLow(), SP_dec(), DONE()));
  }

  public:
static InstructionCycles createStackPHBCycles(Executor<W65C816> operation)
  {
    //22c
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataLow(), SP_dec(), DONE()));
  }

  public:
static InstructionCycles createStackPHDCycles(Executor<W65C816> operation)
  {
    //22c
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataHigh(), SP_dec()),
                                 new BusCycle(Address(S()), Write_DataLow(), SP_dec(), DONE()));
  }

  public:
static InstructionCycles createStackPHKCycles(Executor<W65C816> operation)
  {
    //22c
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataLow(), SP_dec(), DONE()));
  }

  public:
static InstructionCycles createStackPEACycles()
  {
    //22d
    return new InstructionCycles(StackImmediate,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_AAH()),
                                 new BusCycle(Address(S()), Write_AAH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_AAL(), SP_dec(), DONE()));
  }

  public:
static InstructionCycles createStackPEICycles()
  {
    //22e
    return new InstructionCycles(StackDirectIndirect,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
                                 new BusCycle(Address(DP(), D0()), Read_AAL()),
                                 new BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(S()), Write_AAH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_AAL(), SP_dec(), DONE()));
  }

  public:
static InstructionCycles createStackPERCycles(Executor<W65C816> operation)
  {
    //22f
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataLow(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_DataHigh(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S()), E(operation), Write_DataHigh(), SP_dec()),
                                 new BusCycle(Address(S()), Write_DataLow(), SP_dec(), DONE()));
  }

  public:
static InstructionCycles createStackRTICycles()
  {
    //22g
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_PS(), SP_inc()),  //Processor status
                                 new BusCycle(Address(S(), o(1)), Read_NewPCL(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCH(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPBR(), SP_inc(), PC_e(New_PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createStackRTSCycles()
  {
    //22h
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCL(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCH(), SP_inc()),
                                 new BusCycle(Address(S()), IO(), PC_e(PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createStackRTLCycles()
  {
    //22i
    return new InstructionCycles(StackImplied,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCL(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPCH(), SP_inc()),
                                 new BusCycle(Address(S(), o(1)), Read_NewPBR(), SP_inc(), PC_e(New_PBR(), New_PC()), DONE()));
  }

  public:
static InstructionCycles createStackSoftwareInterruptCycles(InterruptVector interruptVector, Executor<W65C816> operation)
  {
    //22j
    return new InstructionCycles(StackInterruptSoftware,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), new InternalOperation(true, false, true), PC_inc()),  //Read and ignore the signature byte.
                                 new BusCycle(Address(S()), Write_PBR(), SP_dec(), PBR_e(0), new NoteSeven()),
                                 new BusCycle(Address(S()), Write_PCH(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PCL(), SP_dec()),
                                 new BusCycle(Address(S()), Write_PS(), SP_dec(), E(operation)),
                                 new BusCycle(Address(VA(interruptVector)), Read_AAL()),
                                 new BusCycle(Address(VA(interruptVector), o(1)), Read_AAH(), PC_e(PBR(), AA()), DONE()));
  }

  public:
static InstructionCycles createStackRelativeCycles(Executor<W65C816> operation)
  {
    //23
    return new InstructionCycles(StackRelative,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), D0()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(S(), D0(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  public:
static InstructionCycles createStackRelativeWriteCycles(Executor<W65C816> operation)
  {
    //23
    return new InstructionCycles(StackRelative,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), D0()), E(operation), Write_DataLow(), DONE8Bit(M)),
                                 new BusCycle(Address(S(), D0(), o(1)), Write_DataHigh(), DONE16Bit(M)));
  }

  public:
static InstructionCycles createStackRelativeIndirectIndexedWithYCycles(Executor<W65C816> operation)
  {
    //24
    return new InstructionCycles(StackRelativeIndirectIndexedWithY,
                                 new BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
                                 new BusCycle(Address(PBR(), PC()), IO()),
                                 new BusCycle(Address(S(), D0()), Read_AAL()),
                                 new BusCycle(Address(S(), D0(), o(1)), Read_AAH()),
                                 new BusCycle(Address(S(), D0(), o(1)), IO()),
                                 new BusCycle(Address(DBR(), AA(), Y()), Read_DataLow(), E8Bit(operation, M), DONE8Bit(M)),
                                 new BusCycle(Address(DBR(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(operation, M), DONE16Bit(M)));
  }

  public:
static InstructionCycles createStackRelativeIndirectIndexedWithYWriteCycles(Executor<W65C816> operation)
  {
    //24
    return new InstructionCycles(StackRelativeIndirectIndexedWithY,
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

