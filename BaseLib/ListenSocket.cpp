/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Logger.h"
#include "ListenSocket.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenSocket::Init(int iPort)
{
	int			iResult;
	addrinfo*	psResult;
	addrinfo	psHints;

	CSocket::Init(iPort);

	mListenSocket = INVALID_SOCKET;
	mSocket = INVALID_SOCKET;
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
	mListenSocket = socket(psResult->ai_family, psResult->ai_socktype, psResult->ai_protocol);
	if (mListenSocket == INVALID_SOCKET) 
	{
		gcLogger.Error("socket failed with error");
		freeaddrinfo(psResult);
		return false;
	}

	// Setup the TCP listening socket
	iResult = bind(mListenSocket, psResult->ai_addr, (int)psResult->ai_addrlen);
	if (iResult == SOCKET_ERROR) 
	{
		gcLogger.Error("bind failed with error");
		freeaddrinfo(psResult);
		closesocket(mListenSocket);
		return false;
	}

	freeaddrinfo(psResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenSocket::Kill(void)
{
	int		iResult;

	if (mListenSocket != INVALID_SOCKET)
	{
		closesocket(mListenSocket);
		mListenSocket = INVALID_SOCKET;
	}

	if (mSocket != INVALID_SOCKET)
	{
		if (mbListening)
		{
			iResult = shutdown(mSocket, SD_SEND);
			mbListening = false;
		}
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
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

	iResult = listen(mListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) 
	{
		gcLogger.Error("listen failed with error");
		closesocket(mListenSocket);
		return false;
	}

	// Accept a client socket
	mSocket = accept(mListenSocket, NULL, NULL);  //This blocks
	if (mSocket == INVALID_SOCKET) 
	{
		gcLogger.Error("accept failed with error");
		closesocket(mListenSocket);
		return false;
	}

	// No longer need server socket
	closesocket(mListenSocket);
	mListenSocket = INVALID_SOCKET;
	mbListening = true;

	return true;
}

