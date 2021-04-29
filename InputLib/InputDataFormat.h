/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INPUT_DATA_FORMAT_H__
#define __INPUT_DATA_FORMAT_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/LinkedListTemplate.h"
#include "StandardLib/Channels.h"
#include "StandardLib/Unknown.h"
#include "StandardLib/SetType.h"


class CInputDataFormat : public CUnknown
{
CONSTRUCTABLE(CInputDataFormat);
protected:
	CChannels			mcExternalChannels;
	CArrayChars		mcChannelNames;
	BOOL				mbCommon;
	CChars				mszCommonName;

public:
	void 		Init(void);
	void 		Init(char* szCommonName);
	void 		Kill(void);

	void		Add(EPrimitiveType eType, char* szName);
	void		Done(void);
	int			GetIndex(char* szName);
	CChannel*	Get(int iIndex);
	float		GetConvertToFloat(int iIndex, void* pvData);
	float		GetCastToFloat(int iIndex, void* pvData);
	void		GetAsNative(void* pvDest, int iIndex, void* pvData);

	BOOL		IsCommon(void);
	BOOL		IsCommonName(char* szName);
	char*		GetCommonName(void);
};


typedef CSetType<CInputDataFormat> CSetInputDataFormat;


#endif // __INPUT_DATA_FORMAT_H__

