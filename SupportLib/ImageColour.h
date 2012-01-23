/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __IMAGE_COLOUR_H__
#define __IMAGE_COLOUR_H__
#include "BaseLib/Define.h"
#include "BaseLib/FloatHelper.h"
#include "BaseLib/ArrayInt.h"
#include "StandardLib/Unknown.h"
#include "ImageChannel.h"


#define MAX_IMAGE_COLOUR_BYTES 16


struct SImageColour
{
	unsigned char	c[MAX_IMAGE_COLOUR_BYTES];

	void Zero(void);
	void Full(void);
	BOOL IsZero(int iDataSize);
	BOOL Equals(SImageColour* psColour, int iDataSize);
	BOOL Equals(void* pvColour, int iDataSize);
};


class CImageColour : public CUnknown
{
BASE_FUNCTIONS(CImageColour);
public:
	void			Init(void);
	void			Kill(void);
	
	virtual float	GetValue(EChannel eChannel) =0;
	virtual void	GetChannels(CArrayInt* pai) =0;
};


//Need separate R, G and B colours also.
class CImageColourRGB : public CImageColour
{
BASE_FUNCTIONS(CImageColourRGB);
public:
	float	r;
	float	g;
	float	b;

	void	Init(float r, float g, float b);

	float	GetValue(EChannel eChannel);
	void	GetChannels(CArrayInt* pai);

	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
};


class CImageColourARGB : public CImageColour
{
BASE_FUNCTIONS(CImageColourARGB);
public:
	float	a;
	float	r;
	float	g;
	float	b;

	void	Init(float a, float r, float g, float b);
	void	Zero(void);

	float	GetValue(EChannel eChannel);
	void	GetChannels(CArrayInt* pai);
	
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
};


class CImageColourOpacity : public CImageColour
{
BASE_FUNCTIONS(CImageColourOpacity);
public:
	float	a;

	void	Init(float a);

	float	GetValue(EChannel eChannel);
	void	GetChannels(CArrayInt* pai);

	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
};


class CImageColourSpecular : public CImageColour
{
BASE_FUNCTIONS(CImageColourSpecular);
public:
	float	s;

	void	Init(float s);

	float	GetValue(EChannel eChannel);
	void	GetChannels(CArrayInt* pai);

	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
};


class CImageColourNormal : public CImageColour
{
BASE_FUNCTIONS(CImageColourNormal);
public:
	float	x;
	float	y;
	float	z;

	void	Init(float x, float y, float z);

	float	GetValue(EChannel eChannel);
	void	GetChannels(CArrayInt* pai);

	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
};


class CImageColourCombo2 : public CImageColour
{
BASE_FUNCTIONS(CImageColourCombo2);
public:
	CImageColour*	cImageColours[2];

	//Can't have a '1' combo and more than 3 is getting silly (You can chain them if needed).
	void	Init(CImageColour* pc1, CImageColour* pc2);

	float	GetValue(EChannel eChannel);
	void	GetChannels(CArrayInt* pai);
};


#endif // __IMAGE_COLOUR_H__

