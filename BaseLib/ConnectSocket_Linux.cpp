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
#include <errno.h>
#include "Logger.h"
#include "LinuxError.h"
#include "ConnectSocket.h"


#define INVALID_SOCKET_DESCRIPTOR	-1


//Defined in Socket_Linux.cpp.
void	SetSocketHandle(uint8* paSocket, int iSocket);
int		GetSocketHandle(uint8* paSocket);


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
	int			iSocket;
	int			iError;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	sResult = NULL;
	iResult = getaddrinfo(mszServer.Text(), mszPortNumber.Text(), &hints, &sResult);
	if (iResult != 0) 
	{
		gcLogger.Error2(__METHOD__, " getaddrinfo failed for [", mszServer.Text(), ":", mszPortNumber.Text(), "] [", gai_strerror(iResult), "].", NULL);
		return false;
	}

	// Attempt to connect to an address until one succeeds
	iSocket = INVALID_SOCKET_DESCRIPTOR;
	iError = 0;
	for (pAddrInfo = sResult; pAddrInfo != NULL; pAddrInfo = pAddrInfo->ai_next) 
	{
		// Create a socket for connecting to server
		iSocket = socket(pAddrInfo->ai_family, pAddrInfo->ai_socktype, pAddrInfo->ai_protocol);
		if (iSocket == INVALID_SOCKET_DESCRIPTOR) 
		{
			gcLogger.Error2(__METHOD__, " socket failed [", LastSocketErrorToString(), "].", NULL);
			freeaddrinfo(sResult);
			return false;
		}

		// Connect to server.
		iResult = connect(iSocket, pAddrInfo->ai_addr, pAddrInfo->ai_addrlen);

		if (iResult != 0)
		{
			iError = errno;
			close(iSocket);
			iSocket = INVALID_SOCKET_DESCRIPTOR;
			continue;
		}
		break;
	}

	freeaddrinfo(sResult);
	SetSocketHandle(maSocket, iSocket);

	if (iSocket == INVALID_SOCKET_DESCRIPTOR) 
	{
		gcLogger.Error2(__METHOD__, " Could not connect to [", mszServer.Text(), ":", mszPortNumber.Text(), "] [", LinuxErrorCodeToString(iError), "].", NULL);
		return false;
	}

	return true;
}


#endif // LINUX_GNU_32
