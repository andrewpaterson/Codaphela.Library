#include "BaseLib/ArrayChars.h"
#include "ProcessFork.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, bool bWaitForProcessToExit)
{
    CArrayChars aszParameters;
    bool        bResult;

    aszParameters.Init(szParameter1, NULL);
    bResult = ForkProcess(&aszParameters, bWaitForProcessToExit);
    aszParameters.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, bool bWaitForProcessToExit)
{
    CArrayChars aszParameters;
    bool        bResult;

    aszParameters.Init(szParameter1, szParameter2, NULL);
    bResult = ForkProcess(&aszParameters, bWaitForProcessToExit);
    aszParameters.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, bool bWaitForProcessToExit)
{
    CArrayChars aszParameters;
    bool        bResult;

    aszParameters.Init(szParameter1, szParameter2, szParameter3, NULL);
    bResult = ForkProcess(&aszParameters, bWaitForProcessToExit);
    aszParameters.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, bool bWaitForProcessToExit)
{
    CArrayChars aszParameters;
    bool        bResult;

    aszParameters.Init(szParameter1, szParameter2, szParameter3, szParameter4, NULL);
    bResult = ForkProcess(&aszParameters, bWaitForProcessToExit);
    aszParameters.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, bool bWaitForProcessToExit)
{
    CArrayChars aszParameters;
    bool        bResult;

    aszParameters.Init(szParameter1, szParameter2, szParameter3, szParameter4, szParameter5, NULL);
    bResult = ForkProcess(&aszParameters, bWaitForProcessToExit);
    aszParameters.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, char* szParameter6, bool bWaitForProcessToExit)
{
    CArrayChars aszParameters;
    bool        bResult;

    aszParameters.Init(szParameter1, szParameter2, szParameter3, szParameter4, szParameter5, szParameter6, NULL);
    bResult = ForkProcess(&aszParameters, bWaitForProcessToExit);
    aszParameters.Kill();
    return bResult;
}

