/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __ARRAY_BIT_H__
#define __ARRAY_BIT_H__
#include "ChunkFile.h"
#include "Malloc.h"


class CArrayBit : public CMalloc
{
protected:
	size	miNumBits;

	union
	{
		struct SLarge
		{
			void*	mpvData;
		} aLarge;

		struct SSmall
		{
			uint32	ui[3];
		} aSmall;
	} u;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);
	void	Zero(void);
	void	ReInit(void);
	void	GrowBy(size iNumBits);
	void	GrowTo(size iNumBits);

	bool	Get(size iBit);  //return 0 or not 0 (not necessarily 1).
	void	Get(void* pvDest, size iDestOffset, size iOffset, size iNumBits);

	void	Add(bool bBit);
	void	Add(void* pvSource, size iSourceOffset, size iNumBits);

	void	Set(size iBit, bool bBit);  //bBit can only take 1 or 0.
	void	Set(size iOffset, void* pvSource, size iSourceOffset, size iNumBits);

	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);

	size	NumElements(void);
	void*	GetData(void);
	size	ByteSize(size iNumBits);

	bool	IsLarge(void);

	void	Print(CChars* psz);
	void	Dump(void);
};


#endif // __ARRAY_BIT_H__

