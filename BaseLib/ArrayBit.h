/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
	int				miNumBits;

	union
	{
		struct SLarge
		{
			void*	mpvData;
		} aLarge;

		struct SSmall
		{
			unsigned int	ui[3];
		} aSmall;
	} u;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);
	void	Zero(void);
	void	ReInit(void);
	void	GrowBy(int iNumBits);

	bool	Get(int iBit);  //return 0 or not 0 (not necessarily 1).
	void	Get(void* pvDest, int iDestOffset, int iOffset, int iNumBits);

	void	Add(bool bBit);
	void	Add(void* pvSource, int iSourceOffset, int iNumBits);

	void	Set(int iBit, bool bBit);  //bBit can only take 1 or 0.
	void	Set(int iOffset, void* pvSource, int iSourceOffset, int iNumBits);

	bool	Read(CFileReader* pcFile);
	bool	Write(CFileWriter* pcFile);

	int		NumElements(void);
	void*	GetData(void);
	int		ByteSize(int iNumBits);

	bool	IsLarge(void);
};


#endif // !__ARRAY_BIT_H__

