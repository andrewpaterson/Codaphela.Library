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
#include "CPUFlags.h"


class CW65C816State;
class CW65C816
{
protected:
	CW65C816State*	mpcState;

public:
	void			Init(void);
	void			Kill(void);

	CW65C816State*	GetState(void);

	void            BIT(void);
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
    void            INC_A(void);
    void            TCS(void);
    void            AND(void);
    void            BIT(void);
    void            BIT_I(void);
    void            ROL(void);
    void            ROL_A(void);
    void            SEC(void);
    void            DEC_A(void);
    void            TSC(void);
    void            EOR(void);
    void            WDM(void);
    void            MVP(void);
    void            MVN(void);
    void            LSR(void);
    void            PHA(void);
    void            LSR_A(void);
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
    void            DEC(void);
    void            INY(void);
    void            DEX(void);
    void            BNE(void);
    void            CLD(void);
    void            CPX(void);
    void            SBC(void);
    void            SEP(void);
    void            INC(void);
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

    void GetAddressValueHex(CChars* psz)
    {
        uint16      ui16;
        uint8       ui8;
        CAddress*   pcAddress;

        pcAddress = mpcState->GetAddress();
        ui16 = pcAddress->GetOffset();
        ui8 = pcAddress->GetBank();
        psz->Append("0x");
        psz->AppendHexHiLo(&ui8, 1);
        psz->Append(":");
        psz->AppendHexHiLo(&ui16, 2);
    }

    void GetAccumulaTorValueHex(CChars* psz)
    {
        uint16      ui16;

        ui16 = mpcState->GetA();
        psz->Append("0x");
        psz->AppendHexHiLo(&ui16, 2);
    }

    void GetXValueHex(CChars* psz)
    {
        uint16      ui16;

        ui16 = mpcState->GetX();
        psz->Append("0x");
        psz->AppendHexHiLo(&ui16, 2);
    }

    void GetYValueHex(CChars* psz)
    {
        uint16      ui16;

        ui16 = mpcState->GetY();
        psz->Append("0x");
        psz->AppendHexHiLo(&ui16, 2);
    }

    void GetDataBankValueHex(CChars* psz)
    {
        uint8      ui8;

        ui8 = mpcState->GetDataBank();
        psz->Append("0x");
        psz->AppendHexHiLo(&ui8, 1);
    }

    void GetStackValueHex(CChars* psz)
    {
        uint16      ui16;

        ui16 = mpcState->GetStackPointer();
        psz->Append("0x");
        psz->AppendHexHiLo(&ui16, 2);
    }

    void GetDirectPageValueHex(CChars* psz)
    {
        uint16      ui16;

        ui16 = mpcState->GetDirectPage();
        psz->Append("0x");
        psz->AppendHexHiLo(&ui16, 2);
    }

    void GetProgramCounterValueHex(CChars* psz)
    {
        uint16      ui16;
        uint8       ui8;
        CAddress*   pcAddress;

        pcAddress = mpcState->GetProgramCounter();
        ui16 = pcAddress->GetOffset();
        ui8 = pcAddress->GetBank();
        psz->Append("0x");
        psz->AppendHexHiLo(&ui8, 1);
        psz->Append(":");
        psz->AppendHexHiLo(&ui16, 2);
    }

    void GetDataValueHex(CChars* psz)
    {
        uint16      ui16;

        ui16 = mpcState->GetData16Bit();
        psz->Append("0x");
        psz->AppendHexHiLo(&ui16, 2);
    }

    void GetOpcodeValueHex(CChars* psz, int cycle, CInstruction* opCode)
    {
        if (cycle != 0)
        {
            int code = opCode->GetCode();
            if (code >= 0 && code <= 255)
            {
                return GetByteStringHex(code);
            }
            else
            {
                return "---";
            }
        }
        else
        {
            return "###";
        }
    }

    void GetOpcodeValueHex(CChars* psz)
    {
        return GetOpcodeValueHex(mpcState->GetCycle(), mpcState->GetOpCode());
    }

    void GetOpcodeMnemonicString(CChars* psz)
    {
        return mpcState->GetOpCode().GetName();
    }

private:
    void GetStatusString(CChars* psz)
    {
        String z = mpcState->isZeroFlagSet() ? "Z:1 " : "Z:0 ";
        String n = mpcState->isNegativeSet() ? "N:1 " : "N:0 ";
        String d = mpcState->isDecimal() ? "D:1 " : "D:0 ";
        String i = mpcState->isInterruptDisable() ? "I:1 " : "I:0 ";
        String m = mpcState->isMemory8Bit() ? "M8  " : "M16 ";
        String x = "";
        bool emulation = mpcState->isEmulation();
        if (!emulation)
        {
            x = mpcState->isIndex8Bit() ? "X8  " : "X16 ";
        }
        String c = mpcState->isCarrySet() ? "C1 " : "C0 ";
        String e = emulation ? "E1 " : "E0 ";
        String o = mpcState->isOverflowFlag() ? "O1 " : "O0 ";
        String b = "";
        if (emulation)
        {
            b = mpcState->isBreak() ? "B1 " : "B0 ";
        }
        return z + n + d + i + m + x + c + e + o + b;
    }

    char* GetType(void)
    {
        return "W65C816 Microprocessor";
    }

    void GetCycleString(CChars* psz)
    {
        psz->Append(mpcState->GetCycle());
    }

private:
    void Branch(bool condition);
};



#endif //  __W65C816_H__

