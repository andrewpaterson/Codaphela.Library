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
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "Logger.h"
#include "ListenSocket.h"


#define INVALID_SOCKET_DESCRIPTOR	-1


//Defined in Socket_Linux.cpp.
void	SetSocketHandle(uint8* paSocket, int iSocket);
int		GetSocketHandle(uint8* paSocket);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenSocket::Init(int iPort)
{
	int			iResult;
	addrinfo*	psResult;
	addrinfo	psHints;
	int			iListenSocket;
	int			iReuse;

	CSocket::Init(iPort);

	SetSocketHandle(maListenSocket, INVALID_SOCKET_DESCRIPTOR);
	SetSocketHandle(maSocket, INVALID_SOCKET_DESCRIPTOR);
	mbListening = false;

	memset(&psHints, 0, sizeof(psHints));
	psHints.ai_family = AF_INET;
	psHints.ai_socktype = SOCK_STREAM;
	psHints.ai_protocol = IPPROTO_TCP;
	psHints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	psResult = NULL;
	iResult = getaddrinfo(NULL, mszPortNumber.Text(), &psHints, &psResult);
	if (iResult != 0) 
	{
		gcLogger.Error("getaddrinfo failed with error");
		return false;
	}

	// Create a socket for connecting to server
	iListenSocket = socket(psResult->ai_family, psResult->ai_socktype, psResult->ai_protocol);
	if (iListenSocket == INVALID_SOCKET_DESCRIPTOR) 
	{
		gcLogger.Error("socket failed with error");
		freeaddrinfo(psResult);
		return false;
	}

	//Without SO_REUSEADDR Linux will not bind a port that was recently used (TIME_WAIT); Windows allows it.
	iReuse = 1;
	setsockopt(iListenSocket, SOL_SOCKET, SO_REUSEADDR, &iReuse, sizeof(iReuse));

	// Setup the TCP listening socket
	iResult = bind(iListenSocket, psResult->ai_addr, psResult->ai_addrlen);
	if (iResult != 0) 
	{
		gcLogger.Error("bind failed with error");
		freeaddrinfo(psResult);
		close(iListenSocket);
		return false;
	}

	freeaddrinfo(psResult);
	SetSocketHandle(maListenSocket, iListenSocket);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenSocket::Kill(void)
{
	int		iListenSocket;
	int		iSocket;

	iListenSocket = GetSocketHandle(maListenSocket);
	if (iListenSocket != INVALID_SOCKET_DESCRIPTOR)
	{
		close(iListenSocket);
		SetSocketHandle(maListenSocket, INVALID_SOCKET_DESCRIPTOR);
	}

	iSocket = GetSocketHandle(maSocket);
	if (iSocket != INVALID_SOCKET_DESCRIPTOR)
	{
		if (mbListening)
		{
			shutdown(iSocket, SHUT_WR);
			mbListening = false;
		}
		close(iSocket);
		SetSocketHandle(maSocket, INVALID_SOCKET_DESCRIPTOR);
	}

	CSocket::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenSocket::Listen(void)
{
	int		iResult;
	int		iListenSocket;
	int		iSocket;

	iListenSocket = GetSocketHandle(maListenSocket);
	iResult = listen(iListenSocket, SOMAXCONN);
	if (iResult != 0) 
	{
		gcLogger.Error("listen failed with error");
		close(iListenSocket);
		SetSocketHandle(maListenSocket, INVALID_SOCKET_DESCRIPTOR);
		return false;
	}

	// Accept a client socket
	iSocket = accept(iListenSocket, NULL, NULL);  //This blocks
	SetSocketHandle(maSocket, iSocket);
	if (iSocket == INVALID_SOCKET_DESCRIPTOR) 
	{
		gcLogger.Error("accept failed with error");
		close(iListenSocket);
		SetSocketHandle(maListenSocket, INVALID_SOCKET_DESCRIPTOR);
		return false;
	}

	// No longer need server socket
	close(iListenSocket);
	SetSocketHandle(maListenSocket, INVALID_SOCKET_DESCRIPTOR);
	mbListening = true;

	return true;
}


#endif // LINUX_GNU_32
