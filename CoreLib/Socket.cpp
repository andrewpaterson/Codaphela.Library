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
#include "BaseLib/Logger.h"
#include "BaseLib/Chars.h"
#include "Socket.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSocket::Init(int iPort)
{
	mszPortNumber.Init();
	mszPortNumber.Append(iPort);
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
	int		iResult;

	iResult = recv(mSocket, (char*)pvBuffer, iBufferLength, 0);
	if (iResult < 0)
	{
		gcLogger.Error("recv failed with error");
		return SOCKET_ERROR;
	}
	else
	{
		return iResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSocket::Send(void* pvBuffer, int iBufferLength)
{
	int		iSendResult;

	iSendResult = send(mSocket, (char*)pvBuffer, iBufferLength, 0);
	if (iSendResult == SOCKET_ERROR) 
	{
		gcLogger.Error("send failed with error");
		return SOCKET_ERROR;
	}
	else
	{
		return iSendResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SocketsInit(void)
{
	int			iResult;
	WSADATA		wsaData;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) 
	{
		//printf("WSAStartup failed with error: %d\n", iResult);
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SocketsKill(void)
{
	WSACleanup();
}
