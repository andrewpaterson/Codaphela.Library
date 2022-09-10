/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __P_P_BLOCK_SET_ARRAY_H__
#define __P_P_BLOCK_SET_ARRAY_H__
#include "BaseLib/ArrayTemplate.h"
#include "PPBlockSet.h"


typedef CArrayTemplate<CPPBlockSet> __CArrayPPBlockSet;


//Re-write this to have a freelist of CPPBlockSet rather than being an array of.
class CPPBlockSetArray : public __CArrayPPBlockSet
{
protected:
	bool				mbRawProcessed;

public:
	void 				Init(void);
	void				Kill(void);
	void				RawProcessed(void);
	bool				IsRawProcessed(void);
	CPPBlockSet*		Add(int iLine, bool bTextBlocks);
	void				Print(CChars* psz);
	void				Dump(void);
	void				DumpBlockSets(void);
};


#endif // !__P_P_BLOCK_SET_ARRAY_H__

