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
#ifdef LINUX_GNU_32
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Logger.h"
#include "Chars.h"
#include "Socket.h"


#define INVALID_SOCKET_DESCRIPTOR	-1


static_assert(sizeof(int) <= SOCKET_HANDLE_SIZE, "SOCKET_HANDLE_SIZE is too small for a socket descriptor.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetSocketHandle(uint8* paSocket, int iSocket)
{
	memcpy(paSocket, &iSocket, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetSocketHandle(uint8* paSocket)
{
	int		iSocket;

	memcpy(&iSocket, paSocket, sizeof(int));
	return iSocket;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSocket::Init(int iPort)
{
	mszPortNumber.Init();
	mszPortNumber.Append(iPort);
	SetSocketHandle(maSocket, INVALID_SOCKET_DESCRIPTOR);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSocket::Kill(void)
{
	mszPortNumber.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSocket::Receive(void* pvBuffer, int iBufferLength)
{
	ssize_t		iResult;

	iResult = recv(GetSocketHandle(maSocket), pvBuffer, iBufferLength, 0);
	if (iResult < 0)
	{
		gcLogger.Error("recv failed with error");
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
	iSendResult = send(GetSocketHandle(maSocket), pvBuffer, iBufferLength, MSG_NOSIGNAL);
	if (iSendResult < 0)
	{
		gcLogger.Error("send failed with error");
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


#endif // LINUX_GNU_32
