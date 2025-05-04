#ifndef __ADDRESSING_MODE_H__
#define __ADDRESSING_MODE_H__


enum EAddressingMode
{
	//PB is 8bit
	//DB is 8bit
	//AA is 16Bit
	//DP is 16Bit
	//SP is 16bit

	//Operand is [Address]

	AM_Absolute,                         //AA = [PB: PC + 2]00 + [PB: PC + 1];                       Address =  DB: AA
	AM_AbsoluteLong,                     //AA = [PB: PC + 2]00 + [PB: PC + 1];  AAB = [PB: PC + 3];  Address = AAB: AA
	AM_AbsoluteIndexedWithX,             //AA = [PB: PC + 2]00 + [PB: PC + 1];                       Address =  DB: AA + X
	AM_AbsoluteLongIndexedWithX,         //AA = [PB: PC + 2]00 + [PB: PC + 1];  AAB = [PB: PC + 3];  Address = AAB: AA + X
	AM_AbsoluteIndexedWithY,             //AA = [PB: PC + 2]00 + [PB: PC + 1];                       Address =  DB: AA + Y
	
	AM_AbsoluteIndirect,                 //AA = [PB: PC + 2]00 + [PB: PC + 1];   PC =           PB: [00:     AA + 1]00 + [00:     AA]
	AM_AbsoluteIndirectLong,             //AA = [PB: PC + 2]00 + [PB: PC + 1];   PC = [00: AA + 2]: [00:     AA + 1]00 + [00:     AA]
	AM_AbsoluteIndexedIndirectWithX,     //AA = [PB: PC + 2]00 + [PB: PC + 1];   PC =           PB: [PB: AA + X + 1]00 + [PB: AA + X]
	
	AM_Accumulator,                      //Operand is Accumulator,
	AM_Implied,                          //Operand is implied by Opcode,
	AM_BlockMove,
	AM_StopTheClock,
	AM_OpCode,                           //Operand is (next) Opcode
	
	AM_Direct,                           //D0 = [PB: PC + 1]                                                 Address =  00: DP + D0
	AM_DirectIndexedWithX,               //D0 = [PB: PC + 1]                                                 Address =  00: DP + D0 + X
	AM_DirectIndexedWithY,               //D0 = [PB: PC + 1]                                                 Address =  00: DP + D0 + Y
	AM_DirectIndirect,                   //D0 = [PB: PC + 1];  AA = [00: DP + D0];                           Address =  DB: AA
	AM_DirectIndirectLong,               //D0 = [PB: PC + 1];  AA = [00: DP + D0]  AAB = [00: DP + D0 + 2];  Address = AAB: AA
	AM_DirectIndexedIndirectWithX,       //D0 = [PB: PC + 1];  AA = [00: DP + D0 + X];                       Address =  DB: AA
	AM_DirectIndirectIndexedWithY,       //D0 = [PB: PC + 1];  AA = [00: DP + D0];                           Address =  DB: AA + Y
	AM_DirectIndirectLongIndexedWithY,   //D0 = [PB: PC + 1];  AA = [00: DP + D0]  AAB = [00: DP + D0 + 2];  Address = AAB: AA + Y
	
	AM_Immediate,                        //Operand is [PB: PC + 1]00 + [PB: PC + 2]
	
	AM_StackImplied,                     //Address = SP;   Operand is implied by Opcode
	AM_StackImmediate,                   //Address = SP;   Operand is [PB: PC + 1]00 + [PB: PC + 2]
	AM_StackDirectIndirect,
	AM_StackRelative,                    //S0 = [PB: PC + 1];                       Address = 00: SP + S0
	AM_StackRelativeIndirectIndexedWithY,//S0 = [PB: PC + 1];  AA = [00: SP + S0];  Address = DB: AA + Y
	
	AM_Relative,                         //R0 = [PB: PC + 1];                    (*)PC = PB: PC + R0
	AM_RelativeLong,                     // R = [PB: PC + 2]00 + [PB: PC + 1];   (*)PC = PB: PC + R
	
	AM_StackInterruptSoftware,
	AM_StackInterruptHardware,
	AM_WaitForInterrupt
};


#endif // __ADDRESSING_MODE_H__

