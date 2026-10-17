#include "BaseLib/Numbers.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "ProcessFork.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, bool bWaitForProcessToExit)
{
    CChars  sz;
    bool    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, bool bWaitForProcessToExit)
{
    CChars  sz;
    bool    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);
    sz.Append(" ");
    sz.Append(szParameter3);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, bool bWaitForProcessToExit)
{
    CChars  sz;
    bool    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);
    sz.Append(" ");
    sz.Append(szParameter3);
    sz.Append(" ");
    sz.Append(szParameter4);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, bool bWaitForProcessToExit)
{
    CChars  sz;
    bool    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);
    sz.Append(" ");
    sz.Append(szParameter3);
    sz.Append(" ");
    sz.Append(szParameter4);
    sz.Append(" ");
    sz.Append(szParameter5);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkProcess(char* szParameter1, char* szParameter2, char* szParameter3, char* szParameter4, char* szParameter5, char* szParameter6, bool bWaitForProcessToExit)
{
    CChars  sz;
    bool    bResult;

    sz.Init(szParameter1);
    sz.Append(" ");
    sz.Append(szParameter2);
    sz.Append(" ");
    sz.Append(szParameter3);
    sz.Append(" ");
    sz.Append(szParameter4);
    sz.Append(" ");
    sz.Append(szParameter5);
    sz.Append(" ");
    sz.Append(szParameter6);

    bResult = ForkProcess(sz.Text(), bWaitForProcessToExit);

    sz.Kill();
    return bResult;
}

