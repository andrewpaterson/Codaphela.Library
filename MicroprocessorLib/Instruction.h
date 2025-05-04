#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "InstructionCycles.h"
#include "AddressingMode.h"


class CInstruction
{
protected:
    uint16                  muiCode;
    CChars                  mszName;
    EAddressingMode*        meAddressingMode;
    CInstructionCycles*     mpcCycles;
    CChars                  mszDescription;

public:
    void                    Init(uint16 uiCode, CInstructionCycles* pcCycles, char* szName, char* szDescription);
    uint16                  GetCode(void);
    char*                   GetName(void);
    CInstructionCycles*     GetCycles(void);
};


#endif // __INSTRUCTION_H__

