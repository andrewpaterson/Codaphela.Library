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
#ifndef __CRC_32_H__
#define __CRC_32_H__
#include "BaseLib/PrimitiveTypes.h"


//The CRC-32 (ISO 3309, polynomial 0xEDB88320 reflected) used by PNG chunks and gzip.
class CCRC32
{
protected:
	uint32	muiCRC;

public:
	void	Init(void);
	void	Update(uint8* puiData, size uiLength);
	uint32	Get(void);
};


#endif // __CRC_32_H__
