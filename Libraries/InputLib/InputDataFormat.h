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
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"


//The layout of a device's data: its channels (mcExternalChannels describes data that isn't its own) and their names.
class CInputDataFormat : public CObject
{
CONSTRUCTABLE(CInputDataFormat);
DESTRUCTABLE(CInputDataFormat);
protected:
	CChannels		mcExternalChannels;
	CArrayChars		mcChannelNames;
	bool			mbCommon;
	CChars			mszCommonName;

public:
	void 		Init(void);
	void 		Init(const char* szCommonName);
	void		Class(void) override;
	void		Free(void) override;

	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;

	void		Add(EPrimitiveType eType, const char* szName);
	void		Done(void);
	size		GetIndex(const char* szName);
	CChannel*	Get(size iIndex);
	float		GetConvertToFloat(size iIndex, void* pvData);
	float		GetCastToFloat(size iIndex, void* pvData);
	void		GetAsNative(void* pvDest, size iIndex, void* pvData);

	bool		IsCommon(void);
	bool		IsCommonName(const char* szName);
	char*		GetCommonName(void);
};


typedef CArray<CInputDataFormat> CArrayInputDataFormat;


#endif // __INPUT_DATA_FORMAT_H__

