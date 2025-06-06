#include "BaseLib/Logger.h"
#include "W65C816.h"
#include "ExecuteIf8Bit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExecuteIf8Bit::Init(CW65C816Func fFunction, EWidthFromRegister eWidth, bool bInitialSide)
{
    COperation::Init();

	mfFunction = fFunction;
	meWidth = eWidth;
    mbInitialSide = bInitialSide;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExecuteIf8Bit::Execute(CW65C816* pcCPU)
{
    if (meWidth == WFR_M)
    {
        if (pcCPU->IsMemory8Bit())
        {
            (pcCPU->*mfFunction)();
        }
    }
    else if (meWidth == WFR_XY)
    {
        if (pcCPU->IsIndex8Bit())
        {
            (pcCPU->*mfFunction)();
        }
    }
    else
    {
        LOG_ERROR("Unknown Width.");
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExecuteIf8Bit::IsInitialSide(void)
{
    return mbInitialSide;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExecuteIf8Bit::Print(CChars* psz)
{
	//Work out some way of getting the function name out.
	psz->Append("Execute_If_8Bit_");
	psz->Append(CWidthFromRegister::GetName(meWidth));
	psz->Append("(HELP!)");
}


