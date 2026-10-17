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
#ifndef __LISTEN_SOCKET_H__
#define __LISTEN_SOCKET_H__
#include "Socket.h"


class CListenSocket : public CSocket
{
protected:
	uint8	maListenSocket[SOCKET_HANDLE_SIZE];
	bool	mbListening;
	bool	mbAccepting;

public:
	bool Init(int iPort);
	//Listen on one address only, for example "127.0.0.1" for the loopback network.
	//A port of 0 lets the operating system choose a free port; see GetPort.
	//It is listening when Init returns so connections made before Accept wait.
	bool Init(const char* szAddress, int iPort);
	void Kill(void);

	//Accepts one connection into this socket and stops listening.
	bool Listen(void);

	//Accepts a connection into pcConnection, which must be initialised and not
	//open, and keeps listening for more.  Fails if no connection arrives within
	//iTimeoutMilliseconds.
	bool Accept(CSocket* pcConnection, int iTimeoutMilliseconds);

	int  GetPort(void);

protected:
	bool Bind(const char* szAddress, int iPort);
	bool StartListening(void);
};


#endif // __LISTEN_SOCKET_H__

