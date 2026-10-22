#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "StandardLib/Unknown.h"
#include "W65InstructionCycles.h"
#include "AddressingMode.h"


class CW65Instruction : public CUnknown
{
protected:
    uint16                  muiCode;
    CChars                  mszName;
    EAddressingMode         meAddressingMode;
    CW65InstructionCycles*     mpcCycles;
    CChars                  mszDescription;

public:
    void                    Init(uint16 uiCode, CW65InstructionCycles* pcCycles, char* szName, char* szDescription);
    void                    Kill(void) override;

    uint16                  GetCode(void);
    char*                   GetName(void);
    char*                   GetDescription(void);
    CW65InstructionCycles*     GetCycles(void);
    CW65BusCycle*              GetBusCycle(int16 iCycle);
};


#endif // __INSTRUCTION_H__

