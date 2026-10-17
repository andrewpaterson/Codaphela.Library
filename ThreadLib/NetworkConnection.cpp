#include <string.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/PointerRemapper.h"
#include "NetworkConnection.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNetworkConnection::Init(void)
{
	mcSocket.Init("", 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNetworkConnection::Kill(void)
{
	mcSocket.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNetworkConnection::Connect(const char* szAddress, int iPort)
{
	mcSocket.Kill();
	mcSocket.Init(szAddress, iPort);
	if (!mcSocket.Connect())
	{
		return false;
	}

	//Messages are small and each one is usually answered so send them immediately.
	mcSocket.SetNoDelay(true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNetworkConnection::Accept(CListenSocket* pcListen, int iTimeoutMilliseconds)
{
	if (mcSocket.IsOpen())
	{
		return gcLogger.Error2(__METHOD__, " Could not accept a connection, this connection is already open.", NULL);
	}

	if (!pcListen->Accept(&mcSocket, iTimeoutMilliseconds))
	{
		return false;
	}

	mcSocket.SetNoDelay(true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNetworkConnection::Close(void)
{
	mcSocket.Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNetworkConnection::IsOpen(void)
{
	return mcSocket.IsOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNetworkConnection::Send(uint32 uiType, void* pvData, size uiSize)
{
	CStackMemory<>			cTemp;
	SProcessChannelHeader*	psHeader;
	bool					bResult;

	//The header and the data are sent together so they arrive together.
	psHeader = BuildMessage(&cTemp, uiType, pvData, uiSize);
	if (psHeader == NULL)
	{
		return false;
	}

	bResult = mcSocket.SendAll(psHeader, (int)(sizeof(SProcessChannelHeader) + uiSize));
	cTemp.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EChannelResult CNetworkConnection::Receive(uint32* puiType, void* pvData, size uiMaxSize, size* puiSize, int iTimeoutMilliseconds)
{
	SProcessChannelHeader	sHeader;
	int						iWait;

	if (!mcSocket.IsOpen())
	{
		return CR_Closed;
	}

	iWait = mcSocket.WaitForData(iTimeoutMilliseconds == CHANNEL_WAIT_FOREVER ? SOCKET_WAIT_FOREVER : iTimeoutMilliseconds);
	if (iWait == SOCKET_WAIT_TIMEOUT)
	{
		return CR_Timeout;
	}
	else if (iWait != SOCKET_WAIT_READY)
	{
		return CR_Error;
	}

	//Fails when the other process has closed the connection.
	if (!mcSocket.ReceiveAll(&sHeader, sizeof(SProcessChannelHeader)))
	{
		return CR_Closed;
	}

	*puiType = sHeader.uiType;
	if (puiSize)
	{
		*puiSize = sHeader.uiSize;
	}

	if (sHeader.uiSize > uiMaxSize)
	{
		//Read past the data so the next message can still be received.
		return Discard(sHeader.uiSize) ? CR_TooLarge : CR_Closed;
	}

	if (sHeader.uiSize > 0)
	{
		if (!mcSocket.ReceiveAll(pvData, (int)sHeader.uiSize))
		{
			return CR_Closed;
		}
	}
	return CR_Message;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNetworkConnection::Discard(size uiSize)
{
	uint8	auiBuffer[512];
	size	uiChunk;

	while (uiSize > 0)
	{
		uiChunk = (uiSize > 512) ? 512 : uiSize;
		if (!mcSocket.ReceiveAll(auiBuffer, (int)uiChunk))
		{
			return false;
		}
		uiSize -= uiChunk;
	}
	return true;
}

