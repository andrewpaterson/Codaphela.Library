#ifdef _WIN32
#include "BaseLib/Logger.h"
#include "BaseLib/WindowsHeaders.h"
#include "BaseLib/WindowsError.h"
#include "InterProcessMutex.h"


static_assert(sizeof(HANDLE) <= INTER_PROCESS_MUTEX_HANDLE_SIZE, "INTER_PROCESS_MUTEX_HANDLE_SIZE is too small for a HANDLE.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static HANDLE& MutexHandle(uint8* paMutex)
{
    return *(HANDLE*)paMutex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Init(char* szName)
{
    mszName.Init(szName);
    MutexHandle(maMutex) = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Kill(void)
{
    mszName.Kill();
    MutexHandle(maMutex) = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Create(void)
{
    HANDLE  hMutex;

    hMutex = CreateMutex(
                NULL,               // default security descriptor
                false,              // mutex not owned
                mszName.Text());    // object name

    MutexHandle(maMutex) = hMutex;
    if (hMutex == NULL)
    {
        return gcLogger.Error2(__METHOD__, " Could not create mutex [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
    }
    else
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            return true;
        }
        else
        {
            return true;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Connect(void)
{
    HANDLE  hMutex;

    hMutex = OpenMutex( 
                MUTEX_ALL_ACCESS,   // request full access
                false,              // handle not inheritable
                mszName.Text());    // object name

    MutexHandle(maMutex) = hMutex;
    if (hMutex == NULL)
    {
        return gcLogger.Error2(__METHOD__, " Could not open mutex [", mszName.Text(), "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
    }
    else
    {
        return true;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessMutex::Close(void)
{
    CloseHandle(MutexHandle(maMutex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessMutex::Lock(void)
{
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
                        MutexHandle(maMutex),    // handle to mutex
                        INFINITE);  // no time-out interval

    if (dwWaitResult == WAIT_OBJECT_0)
    {
        return true;
    }
    else if (dwWaitResult == WAIT_ABANDONED)
    {
        // The thread got ownership of an abandoned mutex
        return false;
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
bool CInterProcessMutex::Unlock(void)
{
    if (!ReleaseMutex(MutexHandle(maMutex)))
    {
        return false;
    }
    else
    {
        return true;
    }
}


#endif // _WIN32

