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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include "Logger.h"
#include "LinuxError.h"
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
	return Bind(NULL, iPort);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenSocket::Init(const char* szAddress, int iPort)
{
	if (!Bind(szAddress, iPort))
	{
		return false;
	}
	return StartListening();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenSocket::Bind(const char* szAddress, int iPort)
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
	mbAccepting = false;

	memset(&psHints, 0, sizeof(psHints));
	psHints.ai_family = AF_INET;
	psHints.ai_socktype = SOCK_STREAM;
	psHints.ai_protocol = IPPROTO_TCP;
	psHints.ai_flags = (szAddress == NULL) ? AI_PASSIVE : 0;

	// Resolve the server address and port
	psResult = NULL;
	iResult = getaddrinfo(szAddress, mszPortNumber.Text(), &psHints, &psResult);
	if (iResult != 0) 
	{
		gcLogger.Error2(__METHOD__, " getaddrinfo failed for port [", mszPortNumber.Text(), "] [", gai_strerror(iResult), "].", NULL);
		return false;
	}

	// Create a socket for connecting to server
	iListenSocket = socket(psResult->ai_family, psResult->ai_socktype, psResult->ai_protocol);
	if (iListenSocket == INVALID_SOCKET_DESCRIPTOR) 
	{
		gcLogger.Error2(__METHOD__, " socket failed [", LastSocketErrorToString(), "].", NULL);
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
		gcLogger.Error2(__METHOD__, " bind failed [", LastSocketErrorToString(), "].", NULL);
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
	CloseListenSocket();

	if (mbListening && IsOpen())
	{
		shutdown(GetSocketHandle(maSocket), SHUT_WR);
	}
	mbListening = false;

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
		gcLogger.Error2(__METHOD__, " listen failed [", LastSocketErrorToString(), "].", NULL);
		CloseListenSocket();
		return false;
	}

	// Accept a client socket
	iSocket = accept(iListenSocket, NULL, NULL);  //This blocks
	SetSocketHandle(maSocket, iSocket);
	if (iSocket == INVALID_SOCKET_DESCRIPTOR) 
	{
		gcLogger.Error2(__METHOD__, " accept failed [", LastSocketErrorToString(), "].", NULL);
		CloseListenSocket();
		return false;
	}

	// No longer need server socket
	CloseListenSocket();
	mbListening = true;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenSocket::Accept(CSocket* pcConnection, int iTimeoutMilliseconds)
{
	int				iResult;
	int				iListenSocket;
	int				iSocket;
	struct pollfd	sPoll;

	iListenSocket = GetSocketHandle(maListenSocket);
	if (iListenSocket == INVALID_SOCKET_DESCRIPTOR)
	{
		gcLogger.Error2(__METHOD__, " Could not accept, the socket is not bound.", NULL);
		return false;
	}

	if (!StartListening())
	{
		return false;
	}

	sPoll.fd = iListenSocket;
	sPoll.events = POLLIN;
	sPoll.revents = 0;
	do
	{
		iResult = poll(&sPoll, 1, iTimeoutMilliseconds);
	}
	while ((iResult < 0) && (errno == EINTR));
	if (iResult <= 0)
	{
		//Timed out or failed.
		return false;
	}

	iSocket = accept(iListenSocket, NULL, NULL);
	if (iSocket == INVALID_SOCKET_DESCRIPTOR)
	{
		gcLogger.Error2(__METHOD__, " accept failed [", LastSocketErrorToString(), "].", NULL);
		return false;
	}

	SetSocketHandle(pcConnection->maSocket, iSocket);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CListenSocket::GetPort(void)
{
	struct sockaddr_in	sAddress;
	socklen_t			iLength;

	iLength = sizeof(sAddress);
	if (getsockname(GetSocketHandle(maListenSocket), (struct sockaddr*)&sAddress, &iLength) != 0)
	{
		return 0;
	}
	return ntohs(sAddress.sin_port);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenSocket::StartListening(void)
{
	int		iResult;

	if (mbAccepting)
	{
		return true;
	}

	iResult = listen(GetSocketHandle(maListenSocket), SOMAXCONN);
	if (iResult != 0)
	{
		gcLogger.Error2(__METHOD__, " listen failed [", LastSocketErrorToString(), "].", NULL);
		return false;
	}
	mbAccepting = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenSocket::CloseListenSocket(void)
{
	if (GetSocketHandle(maListenSocket) != INVALID_SOCKET_DESCRIPTOR)
	{
		close(GetSocketHandle(maListenSocket));
		SetSocketHandle(maListenSocket, INVALID_SOCKET_DESCRIPTOR);
	}
}


#endif // LINUX_GNU_32
