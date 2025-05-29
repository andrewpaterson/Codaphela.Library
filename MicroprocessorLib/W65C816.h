/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela GerberLib

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
#ifndef __W65C816_H__
#define __W65C816_H__
#include "BaseLib/PrimitiveTypes.h"
#include "Address.h"
#include "TraceValue.h"
#include "CPUFlags.h"
#include "Timeline.h"
#include "W65C816Pins.h"


class CInstruction;
class CW65C816State;
class CW65C816
{
protected:
	CW65C816State*	mpcState;
    CW65C816Pins*   mpcPins;

public:
	void			Init(CW65C816Pins* pcPins);
	void			Kill(void);

    CW65C816State*  GetState(void);
    CW65C816Pins*   GetPins(void);

    void            ASL(void);
    void            ASL_A(void);
    void            PER(void);
    void            PLD(void);
    void            PHD(void);
    void            PLB(void);
    void            PHB(void);
    void            PHK(void);
    void            PLP(void);
    void            PHP(void);
    void            BRK(void);
    void            COP(void);
    void            ORA(void);
    void            TSB(void);
    void            TRB(void);
    void            BPL(void);
    void            BMI(void);
    void            CLC(void);
    void            INC(void);
    void            INC_A(void);
    void            TCS(void);
    void            AND(void);
    void            BIT(void);
    void            BIT_A(void);
    void            ROL(void);
    void            ROL_A(void);
    void            SEC(void);
    void            DEC(void);
    void            DEC_A(void);
    void            TSC(void);
    void            EOR(void);
    void            WDM(void);
    void            MVP(void);
    void            MVN(void);
    void            LSR(void);
    void            LSR_A(void);
    void            PHA(void);
    void            BVC(void);
    void            CLI(void);
    void            PHY(void);
    void            TCD(void);
    void            ADC(void);
    void            STZ(void);
    void            PLA(void);
    void            ROR(void);
    void            ROR_A(void);
    void            BVS(void);
    void            SEI(void);
    void            PLY(void);
    void            TDC(void);
    void            BRA(void);
    void            STA(void);
    void            STY(void);
    void            STX(void);
    void            DEY(void);
    void            TXA(void);
    void            BCC(void);
    void            LDY(void);
    void            LDA(void);
    void            LDX(void);
    void            BCS(void);
    void            CLV(void);
    void            TSX(void);
    void            TYX(void);
    void            TYA(void);
    void            TXS(void);
    void            TXY(void);
    void            CPY(void);
    void            CMP(void);
    void            REP(void);
    void            TAY(void);
    void            TAX(void);
    void            PHX(void);
    void            STP(void);
    void            INY(void);
    void            DEX(void);
    void            BNE(void);
    void            CLD(void);
    void            CPX(void);
    void            SBC(void);
    void            SEP(void);
    void            INX(void);
    void            NOP(void);
    void            XBA(void);
    void            BEQ(void);
    void            SED(void);
    void            PLX(void);
    void            XCE(void);

    void            ABORT(void);
    void            IRQ(void);
    void            NMI(void);
    void            RES(void);

    void            InputTransition(CTimeline* pcTimeline);// override;

    void            GetAddressValueHex(CChars* psz, bool bAppend0x = true);
    void            GetAccumulatorValueHex(CChars* psz, bool bAppend0x = true);
    void            GetXValueHex(CChars* psz, bool bAppend0x = true);
    void            GetYValueHex(CChars* psz, bool bAppend0x = true);
    void            GetDataBankValueHex(CChars* psz, bool bAppend0x = true);
    void            GetStackValueHex(CChars* psz, bool bAppend0x = true);
    void            GetDirectPageValueHex(CChars* psz, bool bAppend0x = true);
    void            GetProgramCounterValueHex(CChars* psz, bool bAppend0x = true);
    void            GetDataValueHex(CChars* psz, bool bAppend0x = true);
    void            GetOpcodeValueHex(CChars* psz, int cycle, CInstruction* pcInstruction, bool bAppend0x = true);
    void            GetOpcodeValueHex(CChars* psz, bool bAppend0x = true);
    void            GetOpcodeMnemonicString(CChars* psz);
    void            GetCycleOperationString(CChars* psz);
    void            GetStatusString(CChars* psz);
    char*           GetType(void);
    void            GetCycleString(CChars* psz);
    int16           GetCycle(void);
    bool            IsStopped(void);

protected:
    void            Branch(bool condition);
    void            To8BitHexString(CChars* psz, uint8 ui8);
    void            To16BitHexString(CChars* psz, uint16 ui16);
    void            ToAddressHexString(CChars* psz, CAddress* pcAddress, bool bAppend0x = true);
    void            DisableBuses(void);
    void            Append0x(CChars* psz, bool bAppend0x);

    void            ExecutPhi2Falling(CTimeline* pcTimeline);
    void            ExecutPhi2Rising(CTimeline* pcTimeline);
};



#endif //  __W65C816_H__

