/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __ARRAY_BLOCK_H__
#define __ARRAY_BLOCK_H__
#include "DataTypes.h"
#include "ArrayTemplate.h"


//This is an where the sizeof the elements are not know at compile time.
//It is not an array of void*
class CArrayBlock : public __CArrayTemplate<void>
{
public:
	void	Init(int iSize);
	void	Init(int iSize, int iChunkSize);
	void 	Allocate(int iSize, int iChunkSize);
	void 	Allocate(int iSize, int iChunkSize, int iNumElements);
	void 	ReInit(void);
	void	Fake(void* pvData, int iSize, int iNum, int iChunkSize = 1);
	void	FakeSetUsedElements(int iUsedElements);

	BOOL	WriteArrayUnknown(CFileWriter* pcFileWriter);
	BOOL	ReadArrayUnknown(CFileReader* pcFileReader);
};


#endif // __ARRAY_BLOCK_H__

