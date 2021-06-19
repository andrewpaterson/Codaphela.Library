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
#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <winsock.h>
#include "BaseLib/Define.h"
#include "BaseLib/Chars.h"


class CSocket
{
public:
	CChars	mszPortNumber;
	SOCKET	mSocket;

	void	Init(int iPort);
	void	Kill(void);

	int		Receive(void* pvBuffer, int iBufferLength);
	int		Send(void* pvBuffer, int iBufferLength);
};


BOOL SocketsInit(void);
void SocketsKill(void);


#endif // !__SOCKET_H__

