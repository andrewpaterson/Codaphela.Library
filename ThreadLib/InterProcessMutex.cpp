#include "BaseLib/Logger.h"
#include "BaseLib/WindowsHeaders.h"
#include "WindowsError.h"
#include "InterProcessMutex.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Init(char* szName)
{
    mszName.Init(szName);
    mhMutex = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Kill(void)
{
    mszName.Kill();
    mhMutex = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInterProcessMutex::Create(void)
{
    mhMutex = CreateMutex(
                NULL,               // default security descriptor
                FALSE,              // mutex not owned
                mszName.Text());    // object name

    if (mhMutex == NULL)
    {
        return gcLogger.Error2(__METHOD__, " Could not create mutex [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
    }
    else
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            return TRUE;
        }
        else
        {
            return TRUE;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInterProcessMutex::Connect(void)
{
    mhMutex = OpenMutex( 
                MUTEX_ALL_ACCESS,   // request full access
                FALSE,              // handle not inheritable
                mszName.Text());    // object name

    if (mhMutex == NULL)
    {
        return gcLogger.Error2(__METHOD__, " Could not open mutex [", mszName.Text(), "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
    }
    else
    {
        return TRUE;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Close(void)
{
    CloseHandle(mhMutex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInterProcessMutex::Lock(void)
{
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
                        mhMutex,    // handle to mutex
                        INFINITE);  // no time-out interval

    if (dwWaitResult == WAIT_OBJECT_0)
    {
        return TRUE;
    }
    else if (dwWaitResult == WAIT_ABANDONED)
    {
        // The thread got ownership of an abandoned mutex
        return FALSE;
    }
    else
    {
        return gcLogger.Error2(__METHOD__, " Lock failed with unknown return code.", NULL);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInterProcessMutex::Unlock(void)
{
    if (!ReleaseMutex(mhMutex))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

