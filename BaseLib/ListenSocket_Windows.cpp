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
#include "ListenSocket.h"


//Defined in Socket_Windows.cpp.
void	SetSocketHandle(uint8* paSocket, SOCKET hSocket);
SOCKET	GetSocketHandle(uint8* paSocket);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenSocket::Init(int iPort)
{
	int			iResult;
	addrinfo*	psResult;
	addrinfo	psHints;
	SOCKET		hListenSocket;

	CSocket::Init(iPort);

	SetSocketHandle(maListenSocket, INVALID_SOCKET);
	SetSocketHandle(maSocket, INVALID_SOCKET);
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

	// Create a SOCKET for connecting to server
	hListenSocket = socket(psResult->ai_family, psResult->ai_socktype, psResult->ai_protocol);
	if (hListenSocket == INVALID_SOCKET) 
	{
		gcLogger.Error("socket failed with error");
		freeaddrinfo(psResult);
		return false;
	}

	// Setup the TCP listening socket
	iResult = bind(hListenSocket, psResult->ai_addr, (int)psResult->ai_addrlen);
	if (iResult == SOCKET_ERROR) 
	{
		gcLogger.Error("bind failed with error");
		freeaddrinfo(psResult);
		closesocket(hListenSocket);
		return false;
	}

	freeaddrinfo(psResult);
	SetSocketHandle(maListenSocket, hListenSocket);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenSocket::Kill(void)
{
	int		iResult;
	SOCKET	hListenSocket;
	SOCKET	hSocket;

	hListenSocket = GetSocketHandle(maListenSocket);
	if (hListenSocket != INVALID_SOCKET)
	{
		closesocket(hListenSocket);
		SetSocketHandle(maListenSocket, INVALID_SOCKET);
	}

	hSocket = GetSocketHandle(maSocket);
	if (hSocket != INVALID_SOCKET)
	{
		if (mbListening)
		{
			iResult = shutdown(hSocket, SD_SEND);
			mbListening = false;
		}
		closesocket(hSocket);
		SetSocketHandle(maSocket, INVALID_SOCKET);
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
	SOCKET	hListenSocket;
	SOCKET	hSocket;

	hListenSocket = GetSocketHandle(maListenSocket);
	iResult = listen(hListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) 
	{
		gcLogger.Error("listen failed with error");
		closesocket(hListenSocket);
		SetSocketHandle(maListenSocket, INVALID_SOCKET);
		return false;
	}

	// Accept a client socket
	hSocket = accept(hListenSocket, NULL, NULL);  //This blocks
	SetSocketHandle(maSocket, hSocket);
	if (hSocket == INVALID_SOCKET) 
	{
		gcLogger.Error("accept failed with error");
		closesocket(hListenSocket);
		SetSocketHandle(maListenSocket, INVALID_SOCKET);
		return false;
	}

	// No longer need server socket
	closesocket(hListenSocket);
	SetSocketHandle(maListenSocket, INVALID_SOCKET);
	mbListening = true;

	return true;
}


#endif // _WIN32
