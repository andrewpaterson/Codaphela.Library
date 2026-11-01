#include "BaseLib/Logger.h"
#include "W65CCore.h"
#include "W65ExecuteIf8Bit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65ExecuteIf8Bit::Init(CW65CFunc fFunction, EWidthFromRegister eWidth, bool bInitialSide)
{
    CW65Operation::Init();

	mfFunction = fFunction;
	meWidth = eWidth;
    mbInitialSide = bInitialSide;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65ExecuteIf8Bit::Execute(CW65CCore* pcCPU)
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
bool CW65ExecuteIf8Bit::IsInitialSide(void)
{
    return mbInitialSide;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65ExecuteIf8Bit::Print(CChars* psz)
{
	//Work out some way of getting the function name out.
	psz->Append("Execute_If_8Bit_");
	psz->Append(CW65WidthFromRegister::GetName(meWidth));
	psz->Append("(HELP!)");
}


