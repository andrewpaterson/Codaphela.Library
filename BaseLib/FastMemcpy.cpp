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
#include <immintrin.h>
#include "FastMemcpy.h"


memcpy_fast_func ga_memcpy_fast[65];


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitMemcpyFast(void)
{
	memset(ga_memcpy_fast, 0, sizeof(ga_memcpy_fast));

	ga_memcpy_fast[0] = memcpy_fast_0bytes;
	ga_memcpy_fast[1] = memcpy_fast_1byte;
	ga_memcpy_fast[2] = memcpy_fast_2bytes;
	ga_memcpy_fast[3] = memcpy_fast_3bytes;
	ga_memcpy_fast[4] = memcpy_fast_4bytes;
	ga_memcpy_fast[5] = memcpy_fast_5bytes;
	ga_memcpy_fast[6] = memcpy_fast_6bytes;
	ga_memcpy_fast[7] = memcpy_fast_7bytes;
	ga_memcpy_fast[8] = memcpy_fast_8bytes;
	ga_memcpy_fast[9] = memcpy_fast_9bytes;

	ga_memcpy_fast[10] = memcpy_fast_10bytes;
	ga_memcpy_fast[11] = memcpy_fast_11bytes;
	ga_memcpy_fast[12] = memcpy_fast_12bytes;
	ga_memcpy_fast[13] = memcpy_fast_13bytes;
	ga_memcpy_fast[14] = memcpy_fast_14bytes;
	ga_memcpy_fast[15] = memcpy_fast_15bytes;
	ga_memcpy_fast[16] = memcpy_fast_16bytes;
	ga_memcpy_fast[17] = memcpy_fast_17bytes;
	ga_memcpy_fast[18] = memcpy_fast_18bytes;
	ga_memcpy_fast[19] = memcpy_fast_19bytes;

	ga_memcpy_fast[20] = memcpy_fast_20bytes;
	ga_memcpy_fast[21] = memcpy_fast_21bytes;
	ga_memcpy_fast[22] = memcpy_fast_22bytes;
	ga_memcpy_fast[23] = memcpy_fast_23bytes;
	ga_memcpy_fast[24] = memcpy_fast_24bytes;
	ga_memcpy_fast[25] = memcpy_fast_25bytes;
	ga_memcpy_fast[26] = memcpy_fast_26bytes;
	ga_memcpy_fast[27] = memcpy_fast_27bytes;
	ga_memcpy_fast[28] = memcpy_fast_28bytes;
	ga_memcpy_fast[29] = memcpy_fast_29bytes;

	ga_memcpy_fast[30] = memcpy_fast_30bytes;
	ga_memcpy_fast[31] = memcpy_fast_31bytes;
	ga_memcpy_fast[32] = memcpy_fast_32bytes;
	ga_memcpy_fast[33] = memcpy_fast_33bytes;
	ga_memcpy_fast[34] = memcpy_fast_34bytes;
	ga_memcpy_fast[35] = memcpy_fast_35bytes;
	ga_memcpy_fast[36] = memcpy_fast_36bytes;
	ga_memcpy_fast[37] = memcpy_fast_37bytes;
	ga_memcpy_fast[38] = memcpy_fast_38bytes;
	ga_memcpy_fast[39] = memcpy_fast_39bytes;

	ga_memcpy_fast[40] = memcpy_fast_40bytes;
	ga_memcpy_fast[41] = memcpy_fast_41bytes;
	ga_memcpy_fast[42] = memcpy_fast_42bytes;
	ga_memcpy_fast[43] = memcpy_fast_43bytes;
	ga_memcpy_fast[44] = memcpy_fast_44bytes;
	ga_memcpy_fast[45] = memcpy_fast_45bytes;
	ga_memcpy_fast[46] = memcpy_fast_46bytes;
	ga_memcpy_fast[47] = memcpy_fast_47bytes;
	ga_memcpy_fast[48] = memcpy_fast_48bytes;
	ga_memcpy_fast[49] = memcpy_fast_49bytes;

	ga_memcpy_fast[50] = memcpy_fast_50bytes;
	ga_memcpy_fast[51] = memcpy_fast_51bytes;
	ga_memcpy_fast[52] = memcpy_fast_52bytes;
	ga_memcpy_fast[53] = memcpy_fast_53bytes;
	ga_memcpy_fast[54] = memcpy_fast_54bytes;
	ga_memcpy_fast[55] = memcpy_fast_55bytes;
	ga_memcpy_fast[56] = memcpy_fast_56bytes;
	ga_memcpy_fast[57] = memcpy_fast_57bytes;
	ga_memcpy_fast[58] = memcpy_fast_58bytes;
	ga_memcpy_fast[59] = memcpy_fast_59bytes;

	ga_memcpy_fast[60] = memcpy_fast_60bytes;
	ga_memcpy_fast[61] = memcpy_fast_61bytes;
	ga_memcpy_fast[62] = memcpy_fast_62bytes;
	ga_memcpy_fast[63] = memcpy_fast_63bytes;
	ga_memcpy_fast[64] = memcpy_fast_64bytes;
}


void memcpy_fast_0bytes(void* pvDest, void* pvSource)
{
}


void memcpy_fast_1byte(void* pvDest, void* pvSource)
{
	((char*)pvDest)[0] = ((char*)pvSource)[0];
}


void memcpy_fast_2bytes(void* pvDest, void* pvSource)
{
	((short*)pvDest)[0] = ((short*)pvSource)[0];
}


void memcpy_fast_3bytes(void* pvDest, void* pvSource)
{
	((short*)pvDest)[0] = ((short*)pvSource)[0];
	((char*)pvDest)[2] = ((char*)pvSource)[2];
}


void memcpy_fast_4bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
}


void memcpy_fast_5bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((char*)pvDest)[4] = ((char*)pvSource)[4];
}


void memcpy_fast_6bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((short*)pvDest)[2] = ((short*)pvSource)[2];
}


void memcpy_fast_7bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((short*)pvDest)[2] = ((short*)pvSource)[2];
	((char*)pvDest)[6] = ((char*)pvSource)[6];
}



void memcpy_fast_8bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
}


void memcpy_fast_9bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((char*)pvDest)[8] = ((char*)pvSource)[8];
}


void memcpy_fast_10bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((short*)pvDest)[4] = ((short*)pvSource)[4];
}


void memcpy_fast_11bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((short*)pvDest)[4] = ((short*)pvSource)[4];
	((char*)pvDest)[10] = ((char*)pvSource)[10];
}


void memcpy_fast_12bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[2] = ((int*)pvSource)[2];
}


void memcpy_fast_13bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[2] = ((int*)pvSource)[2];
	((char*)pvDest)[12] = ((char*)pvSource)[12];
}


void memcpy_fast_14bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[2] = ((int*)pvSource)[2];
	((short*)pvDest)[6] = ((short*)pvSource)[6];
}


void memcpy_fast_15bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[2] = ((int*)pvSource)[2];
	((short*)pvDest)[6] = ((short*)pvSource)[6];
	((char*)pvDest)[14] = ((char*)pvSource)[14];
}


void memcpy_fast_16bytes(void* pvDest, void* pvSource)
{
	((int*)pvDest)[0] = ((int*)pvSource)[0];
	((int*)pvDest)[1] = ((int*)pvSource)[1];
	((int*)pvDest)[2] = ((int*)pvSource)[2];
	((int*)pvDest)[3] = ((int*)pvSource)[3];
}


void memcpy_fast(void* pvDest, void* pvSource, size_t uiByteSize)
{
	if (uiByteSize <= 64)
	{
		ga_memcpy_fast[uiByteSize](pvDest, pvSource);
	}
	else
	{
		memcpy_large(pvDest, pvSource, uiByteSize);
	}
}


void memcpy_fast_17bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_1byte(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_18bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_2bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_19bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_3bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_20bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_4bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_21bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_5bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_22bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_6bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_23bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_7bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_24bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_8bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_25bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_9bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_26bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_10bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_27bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_11bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_28bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_12bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_29bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_13bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_30bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_14bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_31bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_15bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_32bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
}


void memcpy_fast_33bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_1byte(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}


void memcpy_fast_34bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_2bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}



void memcpy_fast_35bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_3bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}



void memcpy_fast_36bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_4bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}



void memcpy_fast_37bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_5bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}



void memcpy_fast_38bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_6bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}



void memcpy_fast_39bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_7bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}



void memcpy_fast_40bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_8bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}



void memcpy_fast_41bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_9bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}



void memcpy_fast_42bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_10bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}



void memcpy_fast_43bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_11bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}


void memcpy_fast_44bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_12bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}


void memcpy_fast_45bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_13bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}


void memcpy_fast_46bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_14bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}


void memcpy_fast_47bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_15bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}


void memcpy_fast_48bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
}


void memcpy_fast_49bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_1byte(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_50bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_2bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_51bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_3bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_52bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_4bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_53bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_5bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_54bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_6bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_55bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_7bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_56bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_8bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_57bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_9bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_58bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_10bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_59bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_11bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_60bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_12bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_61bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_13bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_62bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_14bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_63bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_15bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_fast_64bytes(void* pvDest, void* pvSource)
{
	memcpy_fast_16bytes(pvDest, pvSource);
	memcpy_fast_16bytes(&((char*)pvDest)[16], &((char*)pvSource)[16]);
	memcpy_fast_16bytes(&((char*)pvDest)[32], &((char*)pvSource)[32]);
	memcpy_fast_16bytes(&((char*)pvDest)[48], &((char*)pvSource)[48]);
}


void memcpy_large(void* pvDest, void* pvSource, size_t uiByteSize)
{
	memcpy(pvDest, pvSource, uiByteSize);
}


void memcpy_avx2(void* pvDest, void* pvSource, int iLength)
{
	float* pfSource = (float*)pvSource;
	float* pfDest = (float*)pvDest;

	while (iLength > 31)
	{
		_mm256_storeu_ps(pfDest, _mm256_loadu_ps(pfSource));
		pfDest += 8;
		pfSource += 8;
		iLength -= 32;
	}
}

