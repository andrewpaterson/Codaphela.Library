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
#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "Define.h"
#include "Chars.h"


//Large enough to hold a Windows SOCKET or a Linux socket file descriptor.
#define SOCKET_HANDLE_SIZE		8

//Returned by Receive and Send on failure.
#define SOCKET_RESULT_ERROR		-1

//WaitForData.
#define SOCKET_WAIT_FOREVER		-1
#define SOCKET_WAIT_READY		1
#define SOCKET_WAIT_TIMEOUT		0
#define SOCKET_WAIT_ERROR		-1


class CSocket
{
public:
	CChars	mszPortNumber;
	alignas(8) uint8	maSocket[SOCKET_HANDLE_SIZE];

	void	Init(int iPort);
	void	Kill(void);

	void	Close(void);
	bool	IsOpen(void);

	int		Receive(void* pvBuffer, int iBufferLength);
	int		Send(void* pvBuffer, int iBufferLength);

	//Return false if the connection fails or is closed before every byte is received or sent.
	bool	ReceiveAll(void* pvBuffer, int iLength);
	bool	SendAll(void* pvBuffer, int iLength);

	//Send small writes immediately instead of waiting to combine them (TCP_NODELAY).
	bool	SetNoDelay(bool bNoDelay);

	//Returns SOCKET_WAIT_READY when Receive will not block (data has arrived or
	//the connection was closed), SOCKET_WAIT_TIMEOUT, or SOCKET_WAIT_ERROR.
	//An iTimeoutMilliseconds of SOCKET_WAIT_FOREVER waits until one happens.
	int		WaitForData(int iTimeoutMilliseconds);
};


bool SocketsInit(void);
void SocketsKill(void);

//The text of the last socket error on this thread.
char* LastSocketErrorToString(void);


#endif // __SOCKET_H__

