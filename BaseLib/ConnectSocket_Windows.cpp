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
#include "ConnectSocket.h"


//Defined in Socket_Windows.cpp.
void	SetSocketHandle(uint8* paSocket, SOCKET hSocket);
SOCKET	GetSocketHandle(uint8* paSocket);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectSocket::Init(const char* szServer, int iPort)
{
	CSocket::Init(iPort);
	mszServer.Init(szServer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectSocket::Kill(void)
{
	SOCKET	hSocket;

	hSocket = GetSocketHandle(maSocket);
	if (hSocket != INVALID_SOCKET)
	{
		closesocket(hSocket);
		SetSocketHandle(maSocket, INVALID_SOCKET);
	}

	mszServer.Kill();
	CSocket::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CConnectSocket::Connect(void)
{
	addrinfo*	sResult;
	addrinfo*	pAddrInfo;
	addrinfo	hints;
	int			iResult;
	SOCKET		hSocket;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	sResult = NULL;
	iResult = getaddrinfo(mszServer.Text(), mszPortNumber.Text(), &hints, &sResult);
	if (iResult != 0) 
	{
		gcLogger.Error("getaddrinfo failed with error");
		return false;
	}

	// Attempt to connect to an address until one succeeds
	hSocket = INVALID_SOCKET;
	for (pAddrInfo = sResult; pAddrInfo != NULL; pAddrInfo = pAddrInfo->ai_next) 
	{
		// Create a SOCKET for connecting to server
		hSocket = socket(pAddrInfo->ai_family, pAddrInfo->ai_socktype, pAddrInfo->ai_protocol);
		if (hSocket == INVALID_SOCKET) 
		{
			gcLogger.Error("socket failed with error");
			freeaddrinfo(sResult);
			return false;
		}

		// Connect to server.
		iResult = connect(hSocket, pAddrInfo->ai_addr, (int)pAddrInfo->ai_addrlen);

		if (iResult == SOCKET_ERROR)
		{
			closesocket(hSocket);
			hSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(sResult);
	SetSocketHandle(maSocket, hSocket);

	if (hSocket == INVALID_SOCKET) 
	{
		gcLogger.Error("Unable to connect to server!\n");
		return false;
	}

	return true;
}


#endif // _WIN32
