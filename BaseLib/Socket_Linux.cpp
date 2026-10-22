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
#ifdef LINUX_GNU
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <unistd.h>
#include "Logger.h"
#include "LinuxError.h"
#include "Chars.h"
#include "Socket.h"


#define INVALID_SOCKET_DESCRIPTOR	-1


static_assert(sizeof(int) <= SOCKET_HANDLE_SIZE, "SOCKET_HANDLE_SIZE is too small for a socket descriptor.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int& SocketHandle(uint8* paSocket)
{
	return *(int*)paSocket;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSocket::Init(int iPort)
{
	mszPortNumber.Init();
	mszPortNumber.Append(iPort);
	SocketHandle(maSocket) = INVALID_SOCKET_DESCRIPTOR;
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
	int		iSocket;

	iSocket = SocketHandle(maSocket);
	if (iSocket != INVALID_SOCKET_DESCRIPTOR)
	{
		close(iSocket);
		SocketHandle(maSocket) = INVALID_SOCKET_DESCRIPTOR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSocket::IsOpen(void)
{
	return SocketHandle(maSocket) != INVALID_SOCKET_DESCRIPTOR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSocket::Receive(void* pvBuffer, int iBufferLength)
{
	ssize_t		iResult;

	iResult = recv(SocketHandle(maSocket), pvBuffer, iBufferLength, 0);
	if (iResult < 0)
	{
		gcLogger.Error2(__METHOD__, " recv failed [", LastSocketErrorToString(), "].", NULL);
		return SOCKET_RESULT_ERROR;
	}
	else
	{
		return (int)iResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSocket::Send(void* pvBuffer, int iBufferLength)
{
	ssize_t		iSendResult;

	//MSG_NOSIGNAL stops a closed connection raising SIGPIPE (which kills the process) instead of returning an error like Windows.
	iSendResult = send(SocketHandle(maSocket), pvBuffer, iBufferLength, MSG_NOSIGNAL);
	if (iSendResult < 0)
	{
		gcLogger.Error2(__METHOD__, " send failed [", LastSocketErrorToString(), "].", NULL);
		return SOCKET_RESULT_ERROR;
	}
	else
	{
		return (int)iSendResult;
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
	ssize_t		iResult;
	uint8*		pucBuffer;

	pucBuffer = (uint8*)pvBuffer;
	while (iLength > 0)
	{
		iResult = recv(SocketHandle(maSocket), pucBuffer, iLength, 0);
		if (iResult > 0)
		{
			pucBuffer += iResult;
			iLength -= (int)iResult;
		}
		else if ((iResult < 0) && (errno == EINTR))
		{
			continue;
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
	ssize_t		iResult;
	uint8*		pucBuffer;

	pucBuffer = (uint8*)pvBuffer;
	while (iLength > 0)
	{
		iResult = send(SocketHandle(maSocket), pucBuffer, iLength, MSG_NOSIGNAL);
		if (iResult > 0)
		{
			pucBuffer += iResult;
			iLength -= (int)iResult;
		}
		else if ((iResult < 0) && (errno == EINTR))
		{
			continue;
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
	int		iNoDelay;

	iNoDelay = bNoDelay ? 1 : 0;
	return setsockopt(SocketHandle(maSocket), IPPROTO_TCP, TCP_NODELAY, &iNoDelay, sizeof(iNoDelay)) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSocket::WaitForData(int iTimeoutMilliseconds)
{
	struct pollfd	sPoll;
	int				iResult;

	sPoll.fd = SocketHandle(maSocket);
	sPoll.events = POLLIN;
	sPoll.revents = 0;
	do
	{
		iResult = poll(&sPoll, 1, iTimeoutMilliseconds);
	}
	while ((iResult < 0) && (errno == EINTR));

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
	return LinuxErrorCodeToString(errno);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SocketsInit(void)
{
	//Linux sockets need no initialisation.
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SocketsKill(void)
{
}


#endif // LINUX_GNU
