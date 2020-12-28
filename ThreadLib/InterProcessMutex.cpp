#include "BaseLib/Logger.h"
#include "BaseLib/WindowsHeaders.h"
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
        return gcLogger.Error2(__METHOD__, " Could not create mutex [", GetLastError(), "].", NULL);
    }
    else
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            gcLogger.Info2(__METHOD__, " CreateMutex opened an existing mutex.", NULL);
        }
        else
        {
            gcLogger.Info2(__METHOD__, " CreateMutex created a new mutex.", NULL);
        }
        return TRUE;
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
        return gcLogger.Error2(__METHOD__, " Could not open mutex [", GetLastError(), "].", NULL);
    }
    else
    {
        gcLogger.Info2(__METHOD__, " OpenMutex successfully opened the mutex.", NULL);
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

