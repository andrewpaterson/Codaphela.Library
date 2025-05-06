#ifndef __W65C816_State_H__
#define __W65C816_State_H__
#include "BaseLib/PrimitiveTypes.h"
#include "Address.h"
#include "Operation.h"
#include "Instruction.h"
#include "CPUFlags.h"
#include "BusCycle.h"
#include "W65C816.h" // ?!?


class CW65C816State : public CUnknown
{
public:
    bool        mbEmulationFlag;

    // Status register
    bool        mbZeroFlag;
    bool        mbNegativeFlag;
    bool        mbDecimalFlag;
    bool        mbInterruptDisableFlag;
    bool        mbAccumulatorWidthFlag;
    bool        mbIndexWidthFlag;
    bool        mbCarryFlag;
    bool        mbOverflowFlag;
    bool        mbBreakFlag;

    // Registers
    uint16      muiAccumulator;
    uint16      muiXIndex;
    uint16      muiYIndex;
    uint8       muiDataBank;
    uint16      muiDirectPage;
    CAddress    mcProgramCounter;
    uint16      muiStackPointer;

    int16       miCycle;
    uint16      muiOpCodeIndex;
    bool        mbStopped;

    uint8       muiAbortProcessRegister;
    uint16      muiAbortAccumulator;
    uint16      muiAbortXIndex;
    uint16      muiAbortYIndex;
    uint8       muiAbortDataBank;
    uint16      muiAbortDirectPage;
    CAddress    mcAbortProgramCounter;
    uint16      muiAbortStackPointer;

    //These are not the values on the pins, they are internal data.
    CAddress    mcAddress;
    uint16      muiInternal16BitData;
    uint16      muiDirectOffset;
    CAddress    mcNewProgramCounter;
    bool        mbBusEnable;
    bool        mbIrq;
    bool        mbAbort;
    bool        mbNextInstruction;
    uint8       muiData;
    bool        mbNmi;

public:
    void            Init(void);
    void            Init(CW65C816State state);
        
    CInstruction*   GetInstruction(uint16 uiOpcode);
    CInstruction*   GetResetOpcode(void);
    CInstruction*   GetIrqOpcode(void);
    CInstruction*   GetNmiOpcode(void);
    CInstruction*   GetAbortOpcode(void);
    CInstruction*   GetFetchNextOpcode(void);

    void            CreateAbortValues(void);
    void            CreatePartialAbortValues(void);
    void            RestoreAbortValues(void);
    void            ResetPulled(void);
    uint8           GetProcessorRegisterValue(void);
    void            SetZeroFlag(bool bZeroFlag);
    void            SetNegativeFlag(bool bSignFlag);
    void            SetDecimalFlag(bool bDecimalFlag);
    void            SetInterruptDisableFlag(bool bInterruptDisableFlag);
    void            SetAccumulatorWidthFlag(bool bAccumulatorWidthFlag);
    void            SetIndexWidthFlag(bool bIndexWidthFlag);
    void            SetCarryFlag(bool bCarryFlag);
    void            SetEmulationFlag(bool bEmulationFlag);
    bool            IsZeroFlagSet(void);
    bool            IsNegativeSet(void);
    bool            IsDecimal(void);
    bool            IsInterruptDisable(void);
    bool            IsCarrySet(void);
    bool            IsEmulation(void);
    bool            IsBreak(void);
    bool            IsOverflowFlag(void);
    void            SetBreakFlag(bool bBreakFlag);
    void            SetOverflowFlag(bool bOverflowFlag);
    void            Cycle(CW65C816* pcCPU);

    void ExecuteOperation(CW65C816* pcCPU)
    {
       CBusCycle*           pcBusCycle;
       COperationArray*     pcOperations;
       size                 uiNumElements;
       size                 i;
       COperation*          pcOperation;

       pcBusCycle = GetBusCycle();
       pcOperations = pcBusCycle->GetOperations();
       uiNumElements = pcOperations->NumElements();
       for (i = 0; i < uiNumElements; i++)
       {
           pcOperation = pcOperations->GetPtr(i);
           pcOperation->Execute(pcCPU);
       }
    }

    CBusCycle* GetBusCycle(void)
    {
        CInstruction*           pcInstruction;
        CInstructionCycles*     pcCycles;

        pcInstruction = CInstructionFactory::GetInstance()->GetInstruction(muiOpCodeIndex);

       pcCycles = pcInstruction->GetCycles();
       return pcCycles->GetBusCycle(miCycle);
    }

    void NextInstruction()
    {
       if (!mbAbort)
       {
          CreateAbortValues();
       }

       if (mbNmi)
       {
          muiOpCodeIndex = GetNmiOpcode()->GetCode();
          mbNmi = false;
       }
       else if (abort)
       {
          muiOpCodeIndex = GetAbortOpcode()->GetCode();
          mbAbort = false;
       }
       else if (mbIrq && !mbInterruptDisableFlag)
       {
          muiOpCodeIndex = GetIrqOpcode()->GetCode();
       }
       else
       {
          muiOpCodeIndex = GetFetchNextOpcode()->GetCode();
       }
    }

    void DoneInstruction()
    {
       mbNextInstruction = true;
    }

    void BNextCycle()
    {
        miCycle++;
    }

    int16 GetCycle()
    {
       return miCycle;
    }

    void SetOpCode(int uiOpCodeIndex)
    {
       if ((uiOpCodeIndex >= 0) && (uiOpCodeIndex <= 255))
       {
          muiOpCodeIndex = uiOpCodeIndex;
       }
       else
       {
           gcLogger.Error2(__METHOD__, " Invalid Op-code", NULL);
       }
    }

    CDataOperation* GetDataOperation()
    {
       CBusCycle* pcBusCycle = GetBusCycle();
       if (pcBusCycle != NULL)
       {
          return pcBusCycle->GetDataOperation();
       }
       else
       {
          CInstruction* pcInstruction = CInstructionFactory::GetInstance()->GetInstruction(muiOpCodeIndex);
          System.out.println("W65C816.GetDataOperation: CBusCycle for OpCode [" + instruction.getName() + "] Cycle [" + cycle + "] cannot be fetch.  OpCode cycles size [" + instruction.getCycles().size() + "].");
          return NULL;
       }
    }

    bool IsStopped()
    {
       return mbStopped;
    }

    bool IsRead()
    {
       return GetDataOperation()->IsRead();
    }

    bool IsMemory8Bit()
    {
       if (IsEmulation())
       {
          return true;
       }
       else
       {
          return mbAccumulatorWidthFlag;
       }
    }

    bool IsMemory16Bit()
    {
       return !IsMemory8Bit();
    }

    bool IsIndex8Bit()
    {
       if (IsEmulation())
       {
          return true;
       }
       else
       {
          return mbIndexWidthFlag;
       }
    }

    void SetX(int uiXIndex)
    {
       if (IsIndex16Bit())
       {
          Assert16Bit(uiXIndex, "X Index register");
          muiXIndex = uiXIndex;
       }
       else
       {
          Assert8Bit(uiXIndex, "X Index register");
          muiXIndex = SetLowByte(muiXIndex, uiXIndex);
       }
       SetSignAndZeroFromIndex(muiXIndex);
    }

        void SetY(int uiYIndex)
    {
       if (IsIndex16Bit())
       {
          Assert16Bit(uiYIndex, "Y Index register");
          muiYIndex = uiYIndex;
       }
       else
       {
          Assert8Bit(uiYIndex, "Y Index register");
          muiYIndex = SetLowByte(muiYIndex, uiYIndex);
       }
       SetSignAndZeroFromIndex(muiYIndex);
    }

        void SetA(int accumulator)
    {
       if (isMemory16Bit())
       {
          Assert16Bit(accumulator, "Accumulator");
          this->accumulator = accumulator;
       }
       else
       {
          Assert8Bit(accumulator, "Accumulator");
          this->accumulator = SetLowByte(this->accumulator, accumulator);
       }

       setSignAndZeroFromMemory(accumulator);
    }

        void SetC(int accumulator)
    {
       Assert16Bit(accumulator, "Accumulator");
       this->accumulator = accumulator;
       setSignAndZeroFlagFrom16BitValue(accumulator);
    }

        void SetData(int data, bool updateFlags)
    {
       if (isMemory16Bit())
       {
          Assert16Bit(data, "Data");
          this->internal16BitData = data;
       }
       else
       {
          Assert8Bit(data, "Data");
          this->internal16BitData = SetLowByte(this->internal16BitData, data);
       }
       if (updateFlags)
       {
          setSignAndZeroFromMemory(data);
       }
    }

        void SetIndexData(int data, bool updateFlags)
    {
       if (isIndex16Bit())
       {
          Assert16Bit(data, "Data");
          this->internal16BitData = data;
       }
       else
       {
          Assert8Bit(data, "Data");
          this->internal16BitData = SetLowByte(this->internal16BitData, data);
       }
       if (updateFlags)
       {
          setSignAndZeroFromIndex(data);
       }
    }

        void SetSignAndZeroFromMemory(int value)
    {
       if (isMemory16Bit())
       {
          setSignAndZeroFlagFrom16BitValue(value);
       }
       else
       {
          setSignAndZeroFlagFrom8BitValue(value);
       }
    }

        void SetSignAndZeroFromIndex(int value)
    {
       if (isIndex16Bit())
       {
          setSignAndZeroFlagFrom16BitValue(value);
       }
       else
       {
          setSignAndZeroFlagFrom8BitValue(value);
       }
    }

        void SetInternal16BitData(int internal16BitData)
    {
       setData(internal16BitData, true);
    }

        int getA()
    {
       if (isMemory16Bit())
       {
          return this->accumulator;
       }
       else
       {
          return ToByte(this->accumulator);
       }
    }

        int getC()
    {
       return this->accumulator;
    }

        int getX()
    {
       if (isIndex16Bit())
       {
          return this->xIndex;
       }
       else
       {
          return ToByte(this->xIndex);
       }
    }

        int getY()
    {
       if (isIndex16Bit())
       {
          return this->yIndex;
       }
       else
       {
          return ToByte(this->yIndex);
       }
    }

        int getDataBank()
    {
       return this->dataBank;
    }

        void SetDataBank(int dataBank)
    {
       Assert8Bit(dataBank, "Data Bank");
       this->dataBank = dataBank;
    }

        void SetDirectPage(int directPage)
    {
       Assert16Bit(directPage, "Direct Page");
       this->directPage = directPage;
    }

        Address getProgramCounter()
    {
       return this->programCounter;
    }

        void SetProcessorRegisterValue(int value)
    {
       setCarryFlag((value & STATUS_CARRY) != 0);
       setZeroFlag((value & STATUS_ZERO) != 0);
       setInterruptDisableFlag((value & STATUS_INTERRUPT_DISABLE) != 0);
       setDecimalFlag((value & STATUS_DECIMAL) != 0);

       if (isEmulation())
       {
          setBreakFlag((value & STATUS_BREAK) != 0);
       }
       else
       {
          setIndexWidthFlag((value & STATUS_INDEX_WIDTH) != 0);
       }

       setAccumulatorWidthFlag(!isEmulation() && ((value & STATUS_ACCUMULATOR_WIDTH) != 0));
       setOverflowFlag((value & STATUS_OVERFLOW) != 0);

       setNegativeFlag((value & STATUS_NEGATIVE) != 0);
    }

        void SetZeroFlagFrom8BitValue(int value)
    {
       setZeroFlag(is8bitValueZero(value));
    }

        void SetZeroFlagFrom16BitValue(int value)
    {
       setZeroFlag(is16bitValueZero(value));
    }

        void SetNegativeFlagFrom8BitValue(int value)
    {
       setNegativeFlag(is8bitValueNegative(value));
    }

        void SetNegativeFlagFrom16BitValue(int value)
    {
       setNegativeFlag(is16bitValueNegative(value));
    }

        void SetSignAndZeroFlagFrom8BitValue(int value)
    {
       setNegativeFlagFrom8BitValue(value);
       setZeroFlagFrom8BitValue(value);
    }

        void SetSignAndZeroFlagFrom16BitValue(int value)
    {
       setNegativeFlagFrom16BitValue(value);
       setZeroFlagFrom16BitValue(value);
    }

        bool Is8bitValueNegative(int value)
    {
       return (value & 0x80) != 0;
    }

        bool Is16bitValueNegative(int value)
    {
       return (value & 0x8000) != 0;
    }

        bool Is8bitValueZero(int value)
    {
       return (ToByte(value) == 0);
    }

        bool Is16bitValueZero(int value)
    {
       return (ToShort(value) == 0);
    }

        bool IsMemoryNegative(int operand)
    {
       if (isMemory16Bit())
       {
          return is16bitValueNegative(operand);
       }
       else
       {
          return is8bitValueNegative(operand);
       }
    }

        void blockMoveNext()
    {
       if (accumulator != 0xffff)
       {
          accumulator = ToShort(--accumulator);
          xIndex = trimIndex(++xIndex);
          yIndex = trimIndex(++yIndex);
          programCounter.offset(-3, true);
       }
    }

        void blockMovePrevious()
    {
       if (accumulator != 0xffff)
       {
          accumulator = ToShort(--accumulator);
          xIndex = trimIndex(--xIndex);
          yIndex = trimIndex(--yIndex);
          programCounter.offset(-3, true);
       }
    }

        int trimMemory(int value)
    {
       if (isMemory16Bit())
       {
          value = ToShort(value);
       }
       else
       {
          value = ToByte(value);
       }
       return value;
    }

        int trimIndex(int value)
    {
       if (isIndex16Bit())
       {
          value = ToShort(value);
       }
       else if (isIndex8Bit())
       {
          value = ToByte(value);
       }
       return value;
    }

        bool IsIndex16Bit()
    {
       return !isIndex8Bit();
    }

    int getCarry()
    {
       return isCarrySet() ? 1 : 0;
    }

        bool IsSignSet()
    {
       return isNegativeSet();
    }

        bool IsOverflowSet()
    {
       return isOverflowFlag();
    }

        void SetProgramCounter(Address address)
    {
       programCounter = address;
    }

        void SetProgramAddressBank(int bank)
    {
       Assert8Bit(bank, "Program Address Bank");
       programCounter.bank = bank;
    }

        void SetAddressLow(int addressLow)
    {
       Assert8Bit(addressLow, "Address Low");
       this->address.setOffsetLow(addressLow);
    }

        void SetAddressHigh(int addressHigh)
    {
       Assert8Bit(addressHigh, "Address High");
       this->address.setOffsetHigh(addressHigh);
    }

        void SetAddressBank(int addressBank)
    {
       Assert8Bit(addressBank, "Address Bank");
       this->address.setBank(addressBank);
    }

        int getDataLow()
    {
       return GetLowByte(internal16BitData);
    }

        int getDataHigh()
    {
       return GetHighByte(internal16BitData);
    }

        int getInternal16BitData()
    {
       if (isMemory16Bit())
       {
          return internal16BitData;
       }
       else
       {
          return ToByte(internal16BitData);
       }
    }

        int getIndexData()
    {
       if (isIndex16Bit())
       {
          return internal16BitData;
       }
       else
       {
          return ToByte(internal16BitData);
       }
    }

        int getData16Bit()
    {
       return internal16BitData;
    }

        int getDirectPage()
    {
       return directPage;
    }

        int getDirectOffset()
    {
       return directOffset;
    }

        int getStackPointer()
    {
       if (!isEmulation())
       {
          return stackPointer;
       }
       else
       {
          return GetLowByte(stackPointer) | 0x100;
       }
    }

        Address getAddress()
    {
       return address;
    }

        Instruction getOpCode()
    {
       return opCodeTable[muiOpCodeIndex];
    }

        Address getNewProgramCounter()
    {
       return newProgramCounter;
    }

        void incrementProgramAddress()
    {
       this->programCounter.offset(1, true);
    }

        void decrementProgramCounter()
    {
       this->programCounter.offset(-1, true);
    }

        void incrementStackPointer()
    {
       stackPointer = ToShort(++stackPointer);
    }

        void decrementStackPointer()
    {
       stackPointer = ToShort(--stackPointer);
    }

        void SetDirectOffset(int data)
    {
       Assert8Bit(data, "Direct Offset");
       directOffset = data;
    }

        void SetDataLow(int data)
    {
       Assert8Bit(data, "Data Low");
       this->internal16BitData = SetLowByte(this->internal16BitData, data);
    }

        void SetDataHigh(int data)
    {
       Assert8Bit(data, "Data High");
       this->internal16BitData = SetHighByte(this->internal16BitData, data);
    }

        void SetStackPointer(int data)
    {
       Assert16Bit(data, "Stack Pointer");
       stackPointer = data;
    }

        void SetNewProgramCounterLow(int data)
    {
       Assert8Bit(data, "Program Counter Low");
       newProgramCounter.setOffsetLow(data);

    }

        void SetNewProgramCounterHigh(int data)
    {
       Assert8Bit(data, "Program Counter High");
       newProgramCounter.setOffsetHigh(data);
    }

        void SetNewProgramCounterBank(int data)
    {
       Assert8Bit(data, "Program Counter Bank");
       newProgramCounter.setBank(data);
    }

        void SetEmulationMode(bool emulation)
    {
       setEmulationFlag(emulation);
       if (emulation)
       {
          xIndex = ToByte(xIndex);
          yIndex = ToByte(yIndex);
          setAccumulatorWidthFlag(true);
          setIndexWidthFlag(true);
          stackPointer = ToByte(stackPointer) | 0x100;
       }
    }

        void processorStatusChanged()
    {
       if (indexWidthFlag)
       {
          xIndex = ToByte(xIndex);
          yIndex = ToByte(yIndex);
       }
       if (isEmulation())
       {
          setIndexWidthFlag(true);
          setAccumulatorWidthFlag(true);
       }
    }

    void execute8BitADC()
    {
       int operand = getInternal16BitData();
       int accumulator = getA();
       int carryValue = getCarry();

       int result = accumulator + operand + carryValue;

       accumulator &= 0x7F;
       operand &= 0x7F;
       bool carryOutOfPenultimateBit = isMemoryNegative(trimMemory(accumulator + operand + carryValue));

       bool carry = (result & 0x0100) != 0;
       setCarryFlag(carry);
       setOverflowFlag(carry ^ carryOutOfPenultimateBit);
       setA(trimMemory(result));
    }

    void execute16BitADC()
    {
       int operand = getInternal16BitData();
       int accumulator = getA();
       int carryValue = getCarry();

       int result = accumulator + operand + carryValue;

       accumulator &= 0x7FFF;
       operand &= 0x7FFF;
       bool carryOutOfPenultimateBit = isMemoryNegative(trimMemory(accumulator + operand + carryValue));

       bool carry = (result & 0x010000) != 0;
       setCarryFlag(carry);
       setOverflowFlag(carry ^ carryOutOfPenultimateBit);
       setA(trimMemory(result));
    }

    void execute8BitBCDADC()
    {
       int operand = getDataLow();
       int accumulator = getA();

       BCDResult bcdResult = bcdAdd8Bit(operand, accumulator, isCarrySet());
       setCarryFlag(bcdResult.carry);
       setA(bcdResult.value);
    }

    void execute16BitBCDADC()
    {
       int operand = getInternal16BitData();
       int accumulator = getA();

       BCDResult bcdResult = bcdAdd16Bit(operand, accumulator, isCarrySet());
       setCarryFlag(bcdResult.carry);
       setA(bcdResult.value);
    }

        void execute8BitSBC()
    {
       int value = getInternal16BitData();
       int accumulator = getA();
       int borrowValue = 1 - getCarry();

       int result = accumulator - value - borrowValue;

       accumulator &= 0x7F;
       value &= 0x7F;
       bool borrowFromPenultimateBit = isMemoryNegative(trimMemory(accumulator - value - borrowValue));
       bool borrow = (result & 0x0100) != 0;

       setCarryFlag(!borrow);
       setOverflowFlag(borrow ^ borrowFromPenultimateBit);
       setA(trimMemory(result));
    }

    void execute16BitSBC()
    {
       int value = getInternal16BitData();
       int accumulator = getA();
       int borrowValue = 1 - getCarry();

       int result = accumulator - value - borrowValue;

       accumulator &= 0x7FFF;
       value &= 0x7FFF;
       bool borrowFromPenultimateBit = isMemoryNegative(trimMemory(accumulator - value - borrowValue));
       bool borrow = (result & 0x010000) != 0;

       setCarryFlag(!borrow);
       setOverflowFlag(borrow ^ borrowFromPenultimateBit);
       setA(trimMemory(result));
    }

    void execute8BitBCDSBC()
    {
       int value = getDataLow();
       int accumulator = GetLowByte(getA());

       BCDResult bcdResult = bcdSubtract8Bit(value, accumulator, !isCarrySet());
       setCarryFlag(!bcdResult.carry);
       setA(bcdResult.value);
    }

    void execute16BitBCDSBC()
    {
       int value = getInternal16BitData();
       int accumulator = getA();

       BCDResult bcdResult = bcdSubtract16Bit(value, accumulator, !isCarrySet());
       setCarryFlag(!bcdResult.carry);
       setA(bcdResult.value);
    }

        BCDResult bcdAdd8Bit(int bcdFirst, int bcdSecond, bool carry)
    {
       int shift = 0;
       int result = 0;

       while (shift < 8)
       {
          int digitOfFirst = (bcdFirst & 0xF);
          int digitOfSecond = (bcdSecond & 0xF);
          int sumOfDigits = ToByte(digitOfFirst + digitOfSecond + (carry ? 1 : 0));
          carry = sumOfDigits > 9;
          if (carry)
          {
             sumOfDigits += 6;
          }
          sumOfDigits &= 0xF;
          result |= sumOfDigits << shift;

          shift += 4;
          bcdFirst >>= shift;
          bcdSecond >>= shift;
       }

       return new BCDResult(result, carry);
    }

        BCDResult bcdAdd16Bit(int bcdFirst, int bcdSecond, bool carry)
    {
       int result = 0;
       int shift = 0;
       while (shift < 16)
       {
          int digitOfFirst = (bcdFirst & 0xFF);
          int digitOfSecond = (bcdSecond & 0xFF);
          BCDResult bcd8BitResult = bcdAdd8Bit(digitOfFirst, digitOfSecond, carry);
          carry = bcd8BitResult.carry;
          int partialResult = bcd8BitResult.value;
          result = ToShort(result | (partialResult << shift));
          shift += 8;
          bcdFirst = ToShort(bcdFirst >> shift);
          bcdSecond = ToShort(bcdSecond >> shift);
       }
       return new BCDResult(result, carry);
    }

        BCDResult bcdSubtract8Bit(int bcdFirst, int bcdSecond, bool borrow)
    {
       int shift = 0;
       int result = 0;

       while (shift < 8)
       {
          int digitOfFirst = (bcdFirst & 0xF);
          int digitOfSecond = (bcdSecond & 0xF);
          int diffOfDigits = ToByte(digitOfFirst - digitOfSecond - (borrow ? 1 : 0));
          borrow = diffOfDigits > 9;
          if (borrow)
          {
             diffOfDigits -= 6;
          }
          diffOfDigits &= 0xF;
          result |= diffOfDigits << shift;

          shift += 4;
          bcdFirst >>= shift;
          bcdSecond >>= shift;
       }

       return new BCDResult(result, borrow);
    }

        BCDResult bcdSubtract16Bit(int bcdFirst, int bcdSecond, bool borrow)
    {
       int result = 0;
       int shift = 0;
       while (shift < 16)
       {
          int digitOfFirst = (bcdFirst & 0xFF);
          int digitOfSecond = (bcdSecond & 0xFF);
          BCDResult bcd8BitResult = bcdSubtract8Bit(digitOfFirst, digitOfSecond, borrow);
          borrow = bcd8BitResult.carry;
          int partialResult = bcd8BitResult.value;
          result = ToShort(result | (partialResult << shift));
          shift += 8;
          bcdFirst = ToShort(bcdFirst >> shift);
          bcdSecond = ToShort(bcdSecond >> shift);
       }
       return new BCDResult(result, borrow);
    }

    private int setBit(int value, bool bitValue, int bitNumber)
    {
       if (bitValue)
       {
          value = setBit(value, bitNumber);
       }
       else
       {
          value = clearBit(value, bitNumber);
       }
       return value;
    }

        int clearBit(int value, int bitNumber)
    {
       return trimMemory(value & ~(1 << bitNumber));
    }

        int setBit(int value, int bitNumber)
    {
       return trimMemory(value | (1 << bitNumber));
    }

        bool IsBusEnable()
    {
       return busEnable;
    }

        int getData()
    {
       return data;
    }

        void SetData(int data)
    {
       this->data = data;
    }

    void perInternal()
    {
       internal16BitData = ToShort(internal16BitData + programCounter.GetOffset());  // + Carry?
    }

    void phdInternal()
    {
       internal16BitData = directPage;
    }

    void plpInternal()
    {
       setProcessorRegisterValue(getDataLow());
       processorStatusChanged();
    }

    void brkInternal()
    {
       setInterruptDisableFlag(true);
       setDecimalFlag(false);
       if (isEmulation())
       {
          setBreakFlag(true);
       }
    }

    void oraInternal()
    {
       setA(getA() | getInternal16BitData());
    }

    void tsbInternal()
    {
       int value = getInternal16BitData();
       setData((value | getA()), false);
       setZeroFlag((value & getA()) == 0);
    }

    void trbInternal()
    {
       int value = getInternal16BitData();
       setData(value & trimMemory(~getA()), false);
       setZeroFlag((value & getA()) == 0);
    }

        void incrementA()
    {
       int a = trimMemory(getA() + 1);
       setA(a);
       setSignAndZeroFromMemory(a);
    }

        void incrementX()
    {
       int x = trimIndex(getX() + 1);
       setX(x);
       setSignAndZeroFromIndex(x);
    }

        void incrementY()
    {
       int y = trimIndex(getY() + 1);
       setY(y);
       setSignAndZeroFromIndex(y);
    }

        void decrementA()
    {
       int a = trimMemory(getA() - 1);
       setA(a);
       setSignAndZeroFromMemory(a);
    }

        void decrementY()
    {
       int y = trimIndex(getY() - 1);
       setY(y);
       setSignAndZeroFromIndex(y);
    }

        void decrementX()
    {
       int x = trimIndex(getX() - 1);
       setX(x);
       setSignAndZeroFromIndex(x);
    }

    int rotateLeft(int value)
    {
       bool carryWillBeSet = isMemoryNegative(value);
       value = trimMemory(value << 1);
       value = setBit(value, isCarrySet(), 0);
       setCarryFlag(carryWillBeSet);
       return value;
    }

    int rotateRight(int value)
    {
       bool carryWillBeSet = (value & 1) != 0;
       value = trimMemory(value >> 1);
       value = setBit(value, isCarrySet(), isMemory16Bit() ? 15 : 7);
       setCarryFlag(carryWillBeSet);
       return value;
    }

    void bitInternal()
    {
       int value = getInternal16BitData();

       if (isMemory16Bit())
       {
          setNegativeFlag(is16bitValueNegative(value));
          setOverflowFlag((value & 0x4000) != 0);
          setZeroFlagFrom16BitValue((value & getA()));
       }
       else
       {
          setNegativeFlag(is8bitValueNegative(value));
          setOverflowFlag((value & 0x40) != 0);
          setZeroFlagFrom8BitValue((value & getA()));
       }
    }

    void bitIInternal()
    {
       if (isMemory16Bit())
       {
          setZeroFlagFrom16BitValue((getInternal16BitData() & getA()));
       }
       else
       {
          setZeroFlagFrom8BitValue((getInternal16BitData() & getA()));
       }
    }

    void rotateRightInternal()
    {
       setInternal16BitData(rotateRight(getInternal16BitData()));
    }

    void rotateRightAInternal()
    {
       setA(rotateRight(getA()));
    }

    void rotateLeftInternal()
    {
       setInternal16BitData(rotateLeft(getInternal16BitData()));
    }

    void rotateLeftAInternal()
    {
       setA(rotateLeft(getA()));
    }

    int shiftRight(int value)
    {
       setCarryFlag((value & 1) != 0);
       return trimMemory(value >> 1);
    }

    void ShiftRightInternal()
    {
       setInternal16BitData(shiftRight(getInternal16BitData()));
    }

    void ShiftRightAInternal()
    {
       setA(shiftRight(getA()));
    }

    void aslInternal()
    {
       int operand = getInternal16BitData();
       bool carry = isMemoryNegative(operand);
       operand = trimMemory(operand << 1);
       setCarryFlag(carry);
       setInternal16BitData(operand);
    }

    void aslAInternal()
    {
       int operand = getA();
       bool carry = isMemoryNegative(operand);
       operand = trimMemory(operand << 1);
       setCarryFlag(carry);
       setA(operand);
    }

    void tscInternal()
    {
       setA(getStackPointer());
       setSignAndZeroFromMemory(getA());
    }

    void eorInternal()
    {
       int result = trimMemory(getA() ^ getInternal16BitData());
       setSignAndZeroFromMemory(result);
       setA(result);
    }

    void tcdInternal()
    {
       setDirectPage(accumulator);
       setSignAndZeroFlagFrom16BitValue(accumulator);
    }

    void adcInternal()
    {
       if (isMemory16Bit())
       {
          if (!isDecimal())
          {
             execute16BitADC();
          }
          else
          {
             execute16BitBCDADC();
          }
       }
       else
       {
          if (!isDecimal())
          {
             execute8BitADC();
          }
          else
          {
             execute8BitBCDADC();
          }
       }
    }

    void txaInternal()
    {
       if (isMemory8Bit() && isIndex16Bit())
       {
          setA(GetLowByte(getX()));
       }
       else if (isMemory16Bit() && isIndex8Bit())
       {
          setA(SetLowByte(getA(), getX()));
       }
       else
       {
          setA(getX());
       }
    }

    void ldaInternal()
    {
       setA(getInternal16BitData());
    }

    void ldyInternal()
    {
       if (isIndex16Bit())
       {
          setY(internal16BitData);
       }
       else
       {
          setY(ToByte(internal16BitData));
       }
    }

    void ldxInternal()
    {
       if (isIndex16Bit())
       {
          setX(internal16BitData);
       }
       else
       {
          setX(ToByte(internal16BitData));
       }
    }

    void tsxInternal()
    {
       int stackPointer = getStackPointer();
       if (isIndex8Bit())
       {
          int stackPointerLower8Bits = GetLowByte(stackPointer);
          setX(SetLowByte(getX(), stackPointerLower8Bits));
       }
       else
       {
          setX(stackPointer);
       }
    }

    void tyaInternal()
    {
       if (isMemory8Bit() && isIndex16Bit())
       {
          setA(GetLowByte(getY()));
       }
       else if (isMemory16Bit() && isIndex8Bit())
       {
          setA(SetLowByte(getA(), getY()));
       }
       else
       {
          setA(getY());
       }
    }

    void txsInternal()
    {
       if (isEmulation())
       {
          int newStackPointer = 0x100 | GetLowByte(getX());
          setStackPointer(newStackPointer);
       }
       else
       {
          setStackPointer(getX());
       }
    }

    void cpyInternal()
    {
       int value = getIndexData();
       setSignAndZeroFromIndex(trimMemory(getY() - value));
       setCarryFlag(getY() >= value);
    }

    void cmpInternal()
    {
       int value = getInternal16BitData();
       setSignAndZeroFromMemory(trimMemory(getA() - value));
       setCarryFlag(getA() >= value);
    }

    void repInternal()
    {
       int value = ToByte(~getDataLow());
       setProcessorRegisterValue(getProcessorRegisterValue() & value);
       processorStatusChanged();
    }

    void tayInternal()
    {
       if ((isMemory8Bit() && isIndex8Bit()) ||
          (isMemory16Bit() && isIndex8Bit()))
       {
          int lower8BitsOfA = GetLowByte(getA());
          setY(SetLowByte(getY(), lower8BitsOfA));
       }
       else
       {
          setY(getA());
       }
    }

    void taxInternal()
    {
       if ((isMemory8Bit() && isIndex8Bit()) ||
          (isMemory16Bit() && isIndex8Bit()))
       {
          int lower8BitsOfA = GetLowByte(getA());
          setX(SetLowByte(getX(), lower8BitsOfA));
       }
       else
       {
          setX(getA());
       }
    }

    void decInternal()
    {
       setInternal16BitData(trimMemory(getInternal16BitData() - 1));
    }

    void cpxInternal()
    {
       int value = getIndexData();
       setSignAndZeroFromIndex(trimMemory(getX() - value));
       setCarryFlag(getX() >= value);
    }

    void SbcInternal()
    {
       if (isMemory16Bit())
       {
          if (!isDecimal())
          {
             execute16BitSBC();
          }
          else
          {
             execute16BitBCDSBC();
          }
       }
       else
       {
          if (!isDecimal())
          {
             execute8BitSBC();
          }
          else
          {
             execute8BitBCDSBC();
          }
       }
    }

    void SepInternal()
    {
       int value = getDataLow();
       if (isEmulation())
       {
          value = value & 0xCF;
       }
       setProcessorRegisterValue(getProcessorRegisterValue() | value);
       processorStatusChanged();
    }

    void incInternal()
    {
       setInternal16BitData(trimMemory(getInternal16BitData() + 1));
    }

    void xbaInternal()
    {
       int lowerA = IntUtil.GetLowByte(getA());
       int higherA = IntUtil.GetHighByte(getA());
       accumulator = higherA | (lowerA << 8);
    }

    void xceInternal()
    {
       bool oldCarry = isCarrySet();
       bool oldEmulation = isEmulation();
       setEmulationMode(oldCarry);
       setCarryFlag(oldEmulation);

       setAccumulatorWidthFlag(isEmulation());
       setIndexWidthFlag(isEmulation());
    }

    void resetInternal()
    {
       setEmulationMode(true);
       dataBank = 0;
       directPage = 0;
       programCounter.setBank(0);
       setDecimalFlag(false);
       setInterruptDisableFlag(true);
    }

    void tcsInternal()
    {
       int stackPointer = getStackPointer();
       if (isEmulation())
       {
          stackPointer = SetLowByte(stackPointer, GetLowByte(getA()));
       }
       else
       {
          stackPointer = getA();
       }
       setStackPointer(stackPointer);
    }

    void andInternal()
    {
       setA((getA() & getInternal16BitData()));
    }

        void doneIfIndex8Bit()
    {
       if (isIndex8Bit())
       {
          doneInstruction();
       }
    }

        void doneIfIndex16Bit()
    {
       if (isIndex16Bit())
       {
          doneInstruction();
       }
    }

        void doneIfMemory8Bit()
    {
       if (isMemory8Bit())
       {
          doneInstruction();
       }
    }

        void doneIfMemory16Bit()
    {
       if (isMemory16Bit())
       {
          doneInstruction();
       }
    }

        void writeProcessorStatus()
    {
       setData(getProcessorRegisterValue());
    }

        void readProcessorStatus()
    {
       setProcessorRegisterValue(getData());
    }

        int getData16BitOffset()
    {
       int dataLow = getData16Bit();
       if (is16bitValueNegative(dataLow))
       {
          return dataLow | 0xffff0000;
       }
       else
       {
          return dataLow;
       }
    }

        int getData8BitOffset()
    {
       int dataLow = getDataLow();
       if (is8bitValueNegative(dataLow))
       {
          return dataLow | 0xffffff00;
       }
       else
       {
          return dataLow;
       }
    }

        void writeDataLow()
    {
       setData(getDataLow());
    }

        void writeDataHigh()
    {
       setData(getDataHigh());
    }

        void readOpCode()
    {
       setOpCode(getData());
    }

        bool noteFourX(bool nextWillRead)
    {
       return (GetLowByte(getAddress().GetOffset()) + GetLowByte(getX())) > 0xFF ||
          !nextWillRead ||
          isIndex16Bit();
    }

        bool noteFourY(bool nextWillRead)
    {
       return (GetLowByte(getAddress().GetOffset()) + GetLowByte(getY())) > 0xFF ||
          !nextWillRead ||
          isIndex16Bit();
    }

        bool noteSix()
    {
       if (isEmulation())
       {
          int pcOffset = getProgramCounter().GetOffset();
          return AreOffsetsOnDifferentPages(pcOffset, pcOffset + getData16Bit());
       }
       return false;
    }

        int getAddressOffsetX()
    {
       return ToByte(GetLowByte(getAddress().GetOffset()) + GetLowByte(getX()));
    }

        int getAddressOffsetY()
    {
       return ToByte(GetLowByte(getAddress().GetOffset()) + GetLowByte(getY()));
    }

        void writeProgramBank()
    {
       setData(getProgramCounter().GetBank());
    }

        void writeProgramCounterHigh()
    {
       setData(IntUtil.GetHighByte(getProgramCounter().GetOffset()));
    }

        void writeProgramCounterLow()
    {
       setData(IntUtil.GetLowByte(getProgramCounter().GetOffset()));
    }
};


#endif // __W65C816_State_H__

