/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/FastFunctions.h"
#include "ImageColour.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SImageColour::Full(void)
{
	//This is just a testing method.  Float 1.0f is not 'full' with a bit value of 0xffffffff.
	*(uint32*)(&c[0]) = 0xffffffff;
	*(uint32*)(&c[4]) = 0xffffffff;
	*(uint32*)(&c[8]) = 0xffffffff;
	*(uint32*)(&c[12]) = 0xffffffff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SImageColour::Zero(void)
{
	*(int*)(&c[0]) = 0;
	*(int*)(&c[4]) = 0;
	*(int*)(&c[8]) = 0;
	*(int*)(&c[12]) = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SImageColour::IsZero(int iDataSize)
{
	if (iDataSize == 1)
	{
		return c[0] == 0;
	}
	if (iDataSize == 2)
	{
		return *(short*)(&c[0]) == 0;
	}
	if (iDataSize == 3)
	{
		if (*(short*)(&c[0]) == 0)
		{
			return c[2] == 0;
		}
		return false;
	}
	if (iDataSize > 8)
	{
		int	i;

		for (i = 0; i < iDataSize; i++)
		{
			if (c[i] != 0)
			{
				return false;
			}
		}
		return true;
	}
	if (iDataSize == 4)
	{
		return *(int*)(&c[0]) == 0;
	}
	if (iDataSize == 5)
	{
		if (*(int*)(&c[0]) == 0)
		{
			return c[4] == 0;
		}
		return false;
	}
	if (iDataSize == 6)
	{
		if (*(int*)(&c[0]) == 0)
		{
			return *(short*)(&c[4]) == 0;
		}
		return false;
	}
	if (iDataSize == 7)
	{
		if (*(int*)(&c[0]) == 0)
		{
			if (*(short*)(&c[4]) == 0)
			{
				return c[6] == 0;
			}
		}
		return false;

	}
	if (iDataSize == 8)
	{
		if (*(int*)(&c[0]) == 0)
		{
			return *(int*)(&c[4]) == 0;
		}
		return false;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SImageColour::Equals(SImageColour* psColour, int iDataSize)
{
	return ga_memcmp_fast[iDataSize](this, psColour) == 0;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SImageColour::Equals(void* pvColour, int iDataSize)
{
	return ga_memcmp_fast[iDataSize](this, pvColour) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColour::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColour::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourRGB::Init(float r, float g, float b)
{
	this->r = r;
	this->g = g;
	this->b = b;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CImageColourRGB::GetValue(EChannel eChannel)
{
	if (eChannel == IMAGE_DIFFUSE_RED)
	{
		return r;
	}
	if (eChannel == IMAGE_DIFFUSE_GREEN)
	{
		return g;
	}
	if (eChannel == IMAGE_DIFFUSE_BLUE)
	{
		return b;
	}
	return gfNaN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourRGB::GetChannels(CArrayInt* pai)
{
	pai->Add(IMAGE_DIFFUSE_RED);
	pai->Add(IMAGE_DIFFUSE_GREEN);
	pai->Add(IMAGE_DIFFUSE_BLUE);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourRGB::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteFloat(r));
	ReturnOnFalse(pcFile->WriteFloat(g));
	ReturnOnFalse(pcFile->WriteFloat(b));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourRGB::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadFloat(&r));
	ReturnOnFalse(pcFile->ReadFloat(&g));
	ReturnOnFalse(pcFile->ReadFloat(&b));
	return true;}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourARGB::Init(float a, float r, float g, float b)
{
	this->a = a;
	this->r = r;
	this->g = g;
	this->b = b;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourARGB::Zero(void)
{
	a = r = g = b = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CImageColourARGB::GetValue(EChannel eChannel)
{
	if (eChannel == IMAGE_OPACITY)
	{
		return a;
	}
	if (eChannel == IMAGE_DIFFUSE_RED)
	{
		return r;
	}
	if (eChannel == IMAGE_DIFFUSE_GREEN)
	{
		return g;
	}
	if (eChannel == IMAGE_DIFFUSE_BLUE)
	{
		return b;
	}
	return gfNaN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourARGB::GetChannels(CArrayInt* pai)
{
	pai->Add(IMAGE_OPACITY);
	pai->Add(IMAGE_DIFFUSE_RED);
	pai->Add(IMAGE_DIFFUSE_GREEN);
	pai->Add(IMAGE_DIFFUSE_BLUE);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourARGB::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteFloat(a));
	ReturnOnFalse(pcFile->WriteFloat(r));
	ReturnOnFalse(pcFile->WriteFloat(g));
	ReturnOnFalse(pcFile->WriteFloat(b));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourARGB::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadFloat(&a));
	ReturnOnFalse(pcFile->ReadFloat(&r));
	ReturnOnFalse(pcFile->ReadFloat(&g));
	ReturnOnFalse(pcFile->ReadFloat(&b));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourOpacity::Init(float a)
{
	this->a = a;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CImageColourOpacity::GetValue(EChannel eChannel)
{
	if (eChannel == IMAGE_OPACITY)
	{
		return a;
	}
	return gfNaN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourOpacity::GetChannels(CArrayInt* pai)
{
	pai->Add(IMAGE_OPACITY);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourOpacity::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteFloat(a));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourOpacity::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadFloat(&a));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourSpecular::Init(float s)
{
	this->s = s;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CImageColourSpecular::GetValue(EChannel eChannel)
{
	if (eChannel == IMAGE_OPACITY)
	{
		return s;
	}
	return gfNaN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourSpecular::GetChannels(CArrayInt* pai)
{
	pai->Add(IMAGE_SPECULAR);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourSpecular::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteFloat(s));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourSpecular::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadFloat(&s));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourNormal::Init(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CImageColourNormal::GetValue(EChannel eChannel)
{
	if (eChannel == IMAGE_NORMAL_X)
	{
		return x;
	}
	if (eChannel == IMAGE_NORMAL_Y)
	{
		return y;
	}
	if (eChannel == IMAGE_NORMAL_Z)
	{
		return z;
	}
	return gfNaN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourNormal::GetChannels(CArrayInt* pai)
{
	pai->Add(IMAGE_NORMAL_X);
	pai->Add(IMAGE_NORMAL_Y);
	pai->Add(IMAGE_NORMAL_Z);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourNormal::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteFloat(x));
	ReturnOnFalse(pcFile->WriteFloat(y));
	ReturnOnFalse(pcFile->WriteFloat(z));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CImageColourNormal::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadFloat(&x));
	ReturnOnFalse(pcFile->ReadFloat(&y));
	ReturnOnFalse(pcFile->ReadFloat(&z));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourCombo2::Init(CImageColour* pc1, CImageColour* pc2)
{
	cImageColours[0] = pc1;
	cImageColours[1] = pc2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CImageColourCombo2::GetValue(EChannel eChannel)
{
	float	f;

	f = cImageColours[0]->GetValue(eChannel);
	if (!IsNotANumber(&f))
	{
		return f;
	}
	f = cImageColours[1]->GetValue(eChannel);
	if (!IsNotANumber(&f))
	{
		return f;
	}
	return gfNaN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageColourCombo2::GetChannels(CArrayInt* pai)
{
	cImageColours[0]->GetChannels(pai);
	cImageColours[1]->GetChannels(pai);
}
