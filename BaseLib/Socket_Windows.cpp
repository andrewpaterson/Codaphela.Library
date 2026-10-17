/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Logger.h"
#include "WindowsError.h"
#include "Chars.h"
#include "Socket.h"


static_assert(sizeof(SOCKET) <= SOCKET_HANDLE_SIZE, "SOCKET_HANDLE_SIZE is too small for a SOCKET.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SOCKET& SocketHandle(uint8* paSocket)
{
	return *(SOCKET*)paSocket;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSocket::Init(int iPort)
{
	mszPortNumber.Init();
	mszPortNumber.Append(iPort);
	SocketHandle(maSocket) = INVALID_SOCKET;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSocket::Kill(void)
{
	Close();
	mszPortNumber.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSocket::Close(void)
{
	SOCKET	hSocket;

	hSocket = SocketHandle(maSocket);
	if (hSocket != INVALID_SOCKET)
	{
		closesocket(hSocket);
		SocketHandle(maSocket) = INVALID_SOCKET;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSocket::IsOpen(void)
{
	return SocketHandle(maSocket) != INVALID_SOCKET;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSocket::Receive(void* pvBuffer, int iBufferLength)
{
	int		iResult;

	iResult = recv(SocketHandle(maSocket), (char*)pvBuffer, iBufferLength, 0);
	if (iResult < 0)
	{
		gcLogger.Error2(__METHOD__, " recv failed [", LastSocketErrorToString(), "].", NULL);
		return SOCKET_RESULT_ERROR;
	}
	else
	{
		return iResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSocket::Send(void* pvBuffer, int iBufferLength)
{
	int		iSendResult;

	iSendResult = send(SocketHandle(maSocket), (char*)pvBuffer, iBufferLength, 0);
	if (iSendResult == SOCKET_ERROR) 
	{
		gcLogger.Error2(__METHOD__, " send failed [", LastSocketErrorToString(), "].", NULL);
		return SOCKET_RESULT_ERROR;
	}
	else
	{
		return iSendResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSocket::ReceiveAll(void* pvBuffer, int iLength)
{
	int		iResult;
	char*	pcBuffer;

	pcBuffer = (char*)pvBuffer;
	while (iLength > 0)
	{
		iResult = recv(SocketHandle(maSocket), pcBuffer, iLength, 0);
		if (iResult > 0)
		{
			pcBuffer += iResult;
			iLength -= iResult;
		}
		else
		{
			//0 is the other end closing the connection.
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSocket::SendAll(void* pvBuffer, int iLength)
{
	int		iResult;
	char*	pcBuffer;

	pcBuffer = (char*)pvBuffer;
	while (iLength > 0)
	{
		iResult = send(SocketHandle(maSocket), pcBuffer, iLength, 0);
		if (iResult > 0)
		{
			pcBuffer += iResult;
			iLength -= iResult;
		}
		else
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSocket::SetNoDelay(bool bNoDelay)
{
	BOOL	bValue;

	bValue = bNoDelay ? TRUE : FALSE;
	return setsockopt(SocketHandle(maSocket), IPPROTO_TCP, TCP_NODELAY, (char*)&bValue, sizeof(bValue)) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSocket::WaitForData(int iTimeoutMilliseconds)
{
	fd_set		sReadSet;
	timeval		sTimeout;
	timeval*	psTimeout;
	int			iResult;

	FD_ZERO(&sReadSet);
	FD_SET(SocketHandle(maSocket), &sReadSet);
	if (iTimeoutMilliseconds == SOCKET_WAIT_FOREVER)
	{
		psTimeout = NULL;
	}
	else
	{
		sTimeout.tv_sec = iTimeoutMilliseconds / 1000;
		sTimeout.tv_usec = (iTimeoutMilliseconds % 1000) * 1000;
		psTimeout = &sTimeout;
	}

	iResult = select(0, &sReadSet, NULL, NULL, psTimeout);
	if (iResult > 0)
	{
		return SOCKET_WAIT_READY;
	}
	else if (iResult == 0)
	{
		return SOCKET_WAIT_TIMEOUT;
	}
	else
	{
		return SOCKET_WAIT_ERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* LastSocketErrorToString(void)
{
	return WindowsErrorCodeToString(WSAGetLastError());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SocketsInit(void)
{
	int			iResult;
	WSADATA		wsaData;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) 
	{
		//printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SocketsKill(void)
{
	WSACleanup();
}


#endif // _WIN32
