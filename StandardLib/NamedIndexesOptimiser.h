/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __NAMED_INDEXES_OPTIMISER_H__
#define __NAMED_INDEXES_OPTIMISER_H__
#include "BaseLib/ErrorTypes.h"
#include "NamedIndexedSorterSource.h"


class CNamedIndexesBlocks;
class CIndexedFile;
class CNamedIndexes;
class CNamedIndexesOptimiser
{
public:
	CIndexedFiles*						mpcFiles;
	CArrayNamedIndexesBlocks*			mpacBlocks;
	CArrayNamesIndexedSorterSource		macSources;

	void			Init(CNamedIndexes* pcNamedIndexes);
	void			Kill(void);

	BOOL			Optimise(void);

private:
	TRISTATE		OptimiseBlock(CNamedIndexesBlocks* pcBlocks, CIndexedFile* pcIndexedFile);
	CIndexedFile*	GetFile(int iDataSize, int iFileNumber);
	BOOL			OpenFiles(void);
	void			AssignBlockNumbers(void);
	BOOL			CloseFiles(TRISTATE tOptimiseResult);
	TRISTATE		AllocateSources(CNamedIndexesBlocks* pcBlocks, CIndexedFile* pcIndexedFile);
	void			KillSources(void);
};


#endif // __NAMED_INDEXES_OPTIMISER_H__

