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
#include <string.h>
#include "Define.h"
#include "FastMemset.h"


memset_fast_func ga_memset_fast[65];


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitMemsetFast(void)
{
	memset(ga_memset_fast, 0, sizeof(ga_memset_fast));

	ga_memset_fast[0] = memset_fast_0bytes;
	ga_memset_fast[1] = memset_fast_1byte;
	ga_memset_fast[2] = memset_fast_2bytes;
	ga_memset_fast[3] = memset_fast_3bytes;
	ga_memset_fast[4] = memset_fast_4bytes;
	ga_memset_fast[5] = memset_fast_5bytes;
	ga_memset_fast[6] = memset_fast_6bytes;
	ga_memset_fast[7] = memset_fast_7bytes;
	ga_memset_fast[8] = memset_fast_8bytes;
	ga_memset_fast[9] = memset_fast_9bytes;

	ga_memset_fast[10] = memset_fast_10bytes;
	ga_memset_fast[11] = memset_fast_11bytes;
	ga_memset_fast[12] = memset_fast_12bytes;
	ga_memset_fast[13] = memset_fast_13bytes;
	ga_memset_fast[14] = memset_fast_14bytes;
	ga_memset_fast[15] = memset_fast_15bytes;
	ga_memset_fast[16] = memset_fast_16bytes;
	ga_memset_fast[17] = memset_fast_17bytes;
	ga_memset_fast[18] = memset_fast_18bytes;
	ga_memset_fast[19] = memset_fast_19bytes;

	ga_memset_fast[20] = memset_fast_20bytes;
	ga_memset_fast[21] = memset_fast_21bytes;
	ga_memset_fast[22] = memset_fast_22bytes;
	ga_memset_fast[23] = memset_fast_23bytes;
	ga_memset_fast[24] = memset_fast_24bytes;
	ga_memset_fast[25] = memset_fast_25bytes;
	ga_memset_fast[26] = memset_fast_26bytes;
	ga_memset_fast[27] = memset_fast_27bytes;
	ga_memset_fast[28] = memset_fast_28bytes;
	ga_memset_fast[29] = memset_fast_29bytes;

	ga_memset_fast[30] = memset_fast_30bytes;
	ga_memset_fast[31] = memset_fast_31bytes;
	ga_memset_fast[32] = memset_fast_32bytes;
	ga_memset_fast[33] = memset_fast_33bytes;
	ga_memset_fast[34] = memset_fast_34bytes;
	ga_memset_fast[35] = memset_fast_35bytes;
	ga_memset_fast[36] = memset_fast_36bytes;
	ga_memset_fast[37] = memset_fast_37bytes;
	ga_memset_fast[38] = memset_fast_38bytes;
	ga_memset_fast[39] = memset_fast_39bytes;

	ga_memset_fast[40] = memset_fast_40bytes;
	ga_memset_fast[41] = memset_fast_41bytes;
	ga_memset_fast[42] = memset_fast_42bytes;
	ga_memset_fast[43] = memset_fast_43bytes;
	ga_memset_fast[44] = memset_fast_44bytes;
	ga_memset_fast[45] = memset_fast_45bytes;
	ga_memset_fast[46] = memset_fast_46bytes;
	ga_memset_fast[47] = memset_fast_47bytes;
	ga_memset_fast[48] = memset_fast_48bytes;
	ga_memset_fast[49] = memset_fast_49bytes;

	ga_memset_fast[50] = memset_fast_50bytes;
	ga_memset_fast[51] = memset_fast_51bytes;
	ga_memset_fast[52] = memset_fast_52bytes;
	ga_memset_fast[53] = memset_fast_53bytes;
	ga_memset_fast[54] = memset_fast_54bytes;
	ga_memset_fast[55] = memset_fast_55bytes;
	ga_memset_fast[56] = memset_fast_56bytes;
	ga_memset_fast[57] = memset_fast_57bytes;
	ga_memset_fast[58] = memset_fast_58bytes;
	ga_memset_fast[59] = memset_fast_59bytes;

	ga_memset_fast[60] = memset_fast_60bytes;
	ga_memset_fast[61] = memset_fast_61bytes;
	ga_memset_fast[62] = memset_fast_62bytes;
	ga_memset_fast[63] = memset_fast_63bytes;
	ga_memset_fast[64] = memset_fast_64bytes;
}

void memset_fast_0bytes(void* pvDest, uint8 cVal)
{
}


void memset_fast_1byte(void* pvDest, uint8 cVal)
{
	((char*)pvDest)[0] = cVal;
}


void memset_fast_2bytes(void* pvDest, uint8 cVal)
{
	((char*)pvDest)[0] = cVal;
	((char*)pvDest)[1] = cVal;
}


void memset_fast_3bytes(void* pvDest, uint8 cVal)
{
	((char*)pvDest)[0] = cVal;
	((char*)pvDest)[1] = cVal;
	((char*)pvDest)[2] = cVal;
}


void memset_fast_4bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
}


void memset_fast_5bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((char*)pvDest)[4] = cVal;
}


void memset_fast_6bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((char*)pvDest)[4] = cVal;
	((char*)pvDest)[5] = cVal;
}


void memset_fast_7bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((char*)pvDest)[4] = cVal;
	((char*)pvDest)[5] = cVal;
	((char*)pvDest)[6] = cVal;
}



void memset_fast_8bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
}


void memset_fast_9bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;

	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((char*)pvDest)[8] = cVal;
}


void memset_fast_10bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((char*)pvDest)[8] = cVal;
	((char*)pvDest)[9] = cVal;
}


void memset_fast_11bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((char*)pvDest)[8] = cVal;
	((char*)pvDest)[9] = cVal;
	((char*)pvDest)[10] = cVal;
}


void memset_fast_12bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
}


void memset_fast_13bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((char*)pvDest)[12] = cVal;
}


void memset_fast_14bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((char*)pvDest)[12] = cVal;
	((char*)pvDest)[13] = cVal;
}


void memset_fast_15bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((char*)pvDest)[12] = cVal;
	((char*)pvDest)[13] = cVal;
	((char*)pvDest)[14] = cVal;
}


void memset_fast_16bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
}


void memset_fast(void* pvDest, uint8 cVal, size iByteSize)
{
    if (iByteSize <= 64)
    {
        ga_memset_fast[iByteSize](pvDest, cVal);
    }
    else
    {
        memset_fast_65bytesUp(pvDest, cVal, (uint32)iByteSize);
    }
}


void memset_fast_17bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((char*)pvDest)[16] = cVal;
}


void memset_fast_18bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((char*)pvDest)[16] = cVal;
	((char*)pvDest)[17] = cVal;
}


void memset_fast_19bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((char*)pvDest)[16] = cVal;
	((char*)pvDest)[17] = cVal;
	((char*)pvDest)[18] = cVal;
}


void memset_fast_20bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
}


void memset_fast_21bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((char*)pvDest)[20] = cVal;
}


void memset_fast_22bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((char*)pvDest)[20] = cVal;
	((char*)pvDest)[21] = cVal;
}


void memset_fast_23bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((char*)pvDest)[20] = cVal;
	((char*)pvDest)[21] = cVal;
	((char*)pvDest)[22] = cVal;
}


void memset_fast_24bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
}


void memset_fast_25bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((char*)pvDest)[24] = cVal;
}


void memset_fast_26bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((char*)pvDest)[24] = cVal;
	((char*)pvDest)[25] = cVal;
}


void memset_fast_27bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((char*)pvDest)[24] = cVal;
	((char*)pvDest)[25] = cVal;
	((char*)pvDest)[26] = cVal;
}


void memset_fast_28bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
}


void memset_fast_29bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((char*)pvDest)[28] = cVal;
}


void memset_fast_30bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((char*)pvDest)[28] = cVal;
	((char*)pvDest)[29] = cVal;
}


void memset_fast_31bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((char*)pvDest)[28] = cVal;
	((char*)pvDest)[29] = cVal;
	((char*)pvDest)[30] = cVal;
}


void memset_fast_32bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
}


void memset_fast_33bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((char*)pvDest)[32] = cVal;
}


void memset_fast_34bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((char*)pvDest)[32] = cVal;
	((char*)pvDest)[33] = cVal;
}


void memset_fast_35bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((char*)pvDest)[32] = cVal;
	((char*)pvDest)[33] = cVal;
	((char*)pvDest)[34] = cVal;
}


void memset_fast_36bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
}


void memset_fast_37bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((char*)pvDest)[36] = cVal;
}


void memset_fast_38bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((char*)pvDest)[36] = cVal;
	((char*)pvDest)[37] = cVal;
}



void memset_fast_39bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((char*)pvDest)[36] = cVal;
	((char*)pvDest)[37] = cVal;
	((char*)pvDest)[38] = cVal;}


void memset_fast_40bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
}


void memset_fast_41bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((char*)pvDest)[40] = cVal;
}


void memset_fast_42bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((char*)pvDest)[40] = cVal;
	((char*)pvDest)[41] = cVal;
}


void memset_fast_43bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((char*)pvDest)[40] = cVal;
	((char*)pvDest)[41] = cVal;
	((char*)pvDest)[42] = cVal;
}


void memset_fast_44bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
}


void memset_fast_45bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((char*)pvDest)[44] = cVal;
}


void memset_fast_46bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((char*)pvDest)[44] = cVal;
	((char*)pvDest)[45] = cVal;
}


void memset_fast_47bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((char*)pvDest)[44] = cVal;
	((char*)pvDest)[45] = cVal;
	((char*)pvDest)[46] = cVal;
}


void memset_fast_48bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
}


void memset_fast_49bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((char*)pvDest)[48] = cVal;
}


void memset_fast_50bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((char*)pvDest)[48] = cVal;
	((char*)pvDest)[49] = cVal;
}


void memset_fast_51bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((char*)pvDest)[48] = cVal;
	((char*)pvDest)[49] = cVal;
	((char*)pvDest)[50] = cVal;
}


void memset_fast_52bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
}


void memset_fast_53bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((char*)pvDest)[52] = cVal;
}


void memset_fast_54bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((char*)pvDest)[52] = cVal;
	((char*)pvDest)[53] = cVal;
}


void memset_fast_55bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((char*)pvDest)[52] = cVal;
	((char*)pvDest)[53] = cVal;
	((char*)pvDest)[54] = cVal;
}


void memset_fast_56bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((int*)pvDest)[13] = iVal;
}


void memset_fast_57bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((int*)pvDest)[13] = iVal;
	((char*)pvDest)[56] = cVal;
}


void memset_fast_58bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((int*)pvDest)[13] = iVal;
	((char*)pvDest)[56] = cVal;
	((char*)pvDest)[57] = cVal;
}


void memset_fast_59bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((int*)pvDest)[13] = iVal;
	((char*)pvDest)[56] = cVal;
	((char*)pvDest)[57] = cVal;
	((char*)pvDest)[58] = cVal;
}


void memset_fast_60bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((int*)pvDest)[13] = iVal;
	((int*)pvDest)[14] = iVal;
}


void memset_fast_61bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((int*)pvDest)[13] = iVal;
	((int*)pvDest)[14] = iVal;
	((char*)pvDest)[60] = cVal;
}


void memset_fast_62bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((int*)pvDest)[13] = iVal;
	((int*)pvDest)[14] = iVal;
	((char*)pvDest)[60] = cVal;
	((char*)pvDest)[61] = cVal;
}


void memset_fast_63bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((int*)pvDest)[13] = iVal;
	((int*)pvDest)[14] = iVal;
	((char*)pvDest)[60] = cVal;
	((char*)pvDest)[61] = cVal;
	((char*)pvDest)[62] = cVal;
}


void memset_fast_64bytes(void* pvDest, uint8 cVal)
{
	uint32 iVal;
	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;
	((int*)pvDest)[0] = iVal;
	((int*)pvDest)[1] = iVal;
	((int*)pvDest)[2] = iVal;
	((int*)pvDest)[3] = iVal;
	((int*)pvDest)[4] = iVal;
	((int*)pvDest)[5] = iVal;
	((int*)pvDest)[6] = iVal;
	((int*)pvDest)[7] = iVal;
	((int*)pvDest)[8] = iVal;
	((int*)pvDest)[9] = iVal;
	((int*)pvDest)[10] = iVal;
	((int*)pvDest)[11] = iVal;
	((int*)pvDest)[12] = iVal;
	((int*)pvDest)[13] = iVal;
	((int*)pvDest)[14] = iVal;
	((int*)pvDest)[15] = iVal;
}


void memset_fast_65bytesUp(void* pvDest, uint8 cVal, uint32 iByteSize)
{
	uint32	i;
	uint32	iVal;
	uint32	iInts;
	uint32	iChars;
	uint32	iDone;

	iInts = iByteSize >> 2;
	iDone = iInts << 2;
	iChars = iByteSize - iDone;

	iVal = (cVal << 24) | (cVal << 16) | (cVal << 8) | cVal;

	for (i = 0; i < iInts; i++)
	{
		((int*)pvDest)[i] = iVal;
	}
	ga_memset_fast[iChars]((void*)(size) ((int)(size) pvDest + iDone), cVal);
}

