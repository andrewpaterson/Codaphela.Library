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
#include "ListenSocket.h"


//Defined in Socket_Windows.cpp.
SOCKET&	SocketHandle(uint8* paSocket);


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
	SOCKET		hListenSocket;

	CSocket::Init(iPort);

	SocketHandle(maListenSocket) = INVALID_SOCKET;
	SocketHandle(maSocket) = INVALID_SOCKET;
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
		gcLogger.Error2(__METHOD__, " getaddrinfo failed for port [", mszPortNumber.Text(), "] [", WindowsErrorCodeToString(iResult), "].", NULL);
		return false;
	}

	// Create a SOCKET for connecting to server
	hListenSocket = socket(psResult->ai_family, psResult->ai_socktype, psResult->ai_protocol);
	if (hListenSocket == INVALID_SOCKET) 
	{
		gcLogger.Error2(__METHOD__, " socket failed [", LastSocketErrorToString(), "].", NULL);
		freeaddrinfo(psResult);
		return false;
	}

	// Setup the TCP listening socket
	iResult = bind(hListenSocket, psResult->ai_addr, (int)psResult->ai_addrlen);
	if (iResult == SOCKET_ERROR) 
	{
		gcLogger.Error2(__METHOD__, " bind failed [", LastSocketErrorToString(), "].", NULL);
		freeaddrinfo(psResult);
		closesocket(hListenSocket);
		return false;
	}

	freeaddrinfo(psResult);
	SocketHandle(maListenSocket) = hListenSocket;
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
		shutdown(SocketHandle(maSocket), SD_SEND);
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
	SOCKET	hListenSocket;
	SOCKET	hSocket;

	hListenSocket = SocketHandle(maListenSocket);
	iResult = listen(hListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) 
	{
		gcLogger.Error2(__METHOD__, " listen failed [", LastSocketErrorToString(), "].", NULL);
		CloseListenSocket();
		return false;
	}

	// Accept a client socket
	hSocket = accept(hListenSocket, NULL, NULL);  //This blocks
	SocketHandle(maSocket) = hSocket;
	if (hSocket == INVALID_SOCKET) 
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
	int		iResult;
	SOCKET	hListenSocket;
	SOCKET	hSocket;
	fd_set	sReadSet;
	timeval	sTimeout;

	hListenSocket = SocketHandle(maListenSocket);
	if (hListenSocket == INVALID_SOCKET)
	{
		gcLogger.Error2(__METHOD__, " Could not accept, the socket is not bound.", NULL);
		return false;
	}

	if (!StartListening())
	{
		return false;
	}

	FD_ZERO(&sReadSet);
	FD_SET(hListenSocket, &sReadSet);
	sTimeout.tv_sec = iTimeoutMilliseconds / 1000;
	sTimeout.tv_usec = (iTimeoutMilliseconds % 1000) * 1000;
	iResult = select(0, &sReadSet, NULL, NULL, &sTimeout);
	if (iResult <= 0)
	{
		//Timed out or failed.
		return false;
	}

	hSocket = accept(hListenSocket, NULL, NULL);
	if (hSocket == INVALID_SOCKET)
	{
		gcLogger.Error2(__METHOD__, " accept failed [", LastSocketErrorToString(), "].", NULL);
		return false;
	}

	SocketHandle(pcConnection->maSocket) = hSocket;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CListenSocket::GetPort(void)
{
	sockaddr_in		sAddress;
	int				iLength;

	iLength = sizeof(sAddress);
	if (getsockname(SocketHandle(maListenSocket), (sockaddr*)&sAddress, &iLength) == SOCKET_ERROR)
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

	iResult = listen(SocketHandle(maListenSocket), SOMAXCONN);
	if (iResult == SOCKET_ERROR)
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
	if (SocketHandle(maListenSocket) != INVALID_SOCKET)
	{
		closesocket(SocketHandle(maListenSocket));
		SocketHandle(maListenSocket) = INVALID_SOCKET;
	}
}


#endif // _WIN32
