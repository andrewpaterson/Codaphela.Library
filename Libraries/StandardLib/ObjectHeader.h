/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __OBJECT_HEADER_H__
#define __OBJECT_HEADER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexedGeneral.h"


#define OBJECT_IDENTIFIER_SIZE_NOT_SET	0xFFFF


class CObjectIdentifier
{
public:
	int32	meType;  //See ObjectFileGeneral.h
	CChars	mszObjectName;
	OIndex	moi;
	uint16	muiSize;  //This is the in memory allocated size (or -1).  Not the serialised size.

	void	Init(void);
	void	Init(OIndex oi, uint16 uiSize);
	void	Init(const char* szName, OIndex oi, uint16 uiSize);
	void	Kill(void);

	bool	IsNamed(void);
	bool	IsIndexed(void);
	bool	IsFat(void);

	const char*	GetName(void);
	OIndex	GetIndex();
	const char*	GetType(void);
	uint16	GetFatSize(void);
};


class CObjectHeader : public CObjectIdentifier
{
public:
	CChars	mszClassName;

	void	Init(void);
	void	Kill(void);

	const char*	ClassName(void);
	const char*	GetIdentifier(CChars* psz);
};


class CPointerHeader : public CObjectIdentifier
{
public:
	uint16	miNumEmbedded;
	uint16	miEmbeddedIndex;

	void Init(void);
	void Kill(void);
};


#endif // __OBJECT_HEADER_H__

