/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/Log.h"
#include "ConnectSocket.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectSocket::Init(char* szServer, int iPort)
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
	mszServer.Kill();
	CSocket::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConnectSocket::Connect(void)
{
	addrinfo*	sResult;
	addrinfo*	ptr;
	addrinfo	hints;
	int			iResult;

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
		return FALSE;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = sResult; ptr != NULL; ptr = ptr->ai_next) 
	{
		// Create a SOCKET for connecting to server
		mSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (mSocket == INVALID_SOCKET) 
		{
			gcLogger.Error("socket failed with error");
			return FALSE;
		}

		// Connect to server.
		iResult = connect(mSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		if (iResult == SOCKET_ERROR)
		{
			closesocket(mSocket);
			mSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(sResult);

	if (mSocket == INVALID_SOCKET) 
	{
		gcLogger.Error("Unable to connect to server!\n");
		return FALSE;
	}

	return TRUE;
}

