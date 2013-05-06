/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela DirectXLib

Codaphela DirectXLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela DirectXLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela DirectXLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX and associated libraries are Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __FLEXIBLE_VERTEX_FORMATS_H__
#define __FLEXIBLE_VERTEX_FORMATS_H__


//These are helper classes and are not exhaustive.  Notably the skinned verticies are missing.


//No textures no colour (for debugging)
#define D3DFVF_DEBUGVERTEX (D3DFVF_XYZ)
struct D3DDEBUGVERTEX
{
	float	x,y,z;		//untransformed position
};

#define D3DFVF_CVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)
struct D3DCVERTEX
{
	float	x,y,z;		//untransformed position
	int		color;
};


#define D3DFVF_HCVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
struct D3DHCVERTEX
{
	float	x,y,z,w;		//untransformed position
	int		color;
};


//No textures no colour (for debugging)
#define D3DFVF_NVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL)
struct D3DNVERTEX
{
	float	x,y,z;		//untransformed position
	float	nx,ny,nz;	//vertex normal
};


//Just colour
#define D3DFVF_CNVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE)
struct D3DCNVERTEX
{
	float	x,y,z;		//untransformed position
	float	nx,ny,nz;	//vertex normal
	int		color;
};


#define D3DFVF_HVERTEX		(D3DFVF_XYZRHW)
struct D3DHVERTEX
{
	float x,y,z,w;
};


#define D3DFVF_HT1VERTEX	(D3DFVF_XYZRHW | D3DFVF_TEX1)
struct D3DHT1VERTEX
{
	float x,y,z,w;
	float tu0,tv0;
};


#define D3DFVF_HT2VERTEX	(D3DFVF_XYZRHW | D3DFVF_TEX2)
struct D3DHT2VERTEX
{
	float x,y,z,w;
	float tu0,tv0;
	float tu1,tv2;
};


#define D3DFVF_HT3VERTEX	(D3DFVF_XYZRHW | D3DFVF_TEX3)
struct D3DHT3VERTEX
{
	float x,y,z,w;
	float tu0,tv0;
	float tu1,tv1;
	float tu2,tv2;
};


#define D3DFVF_HCTVERTEX	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct D3DHCTVERTEX
{
	float x,y,z,w;
	int   color;
	float tu0,tv0;
};


#define D3DFVF_TVERTEX	(D3DFVF_XYZ | D3DFVF_TEX1)
struct D3DTVERTEX
{
	float x,y,z;
	float tu0,tv0;
};


#define D3DFVF_CTVERTEX	(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct D3DCTVERTEX
{
	float x,y,z;
	int   color;
	float tu0,tv0;
};


//One Texture and colour
#define D3DFVF_CNT1VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct D3DCNT1VERTEX
{
	float	x,y,z;		//untransformed position
	float	nx,ny,nz;	//vertex normal
	int		color;
	float	tu0,tv0;		//texture coordinate
};


//Two Textures and colour
#define D3DFVF_CNT2VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2)
struct D3DCNT2VERTEX
{
	float	x,y,z;		//untransformed position
	float	nx,ny,nz;	//vertex normal
	int		color;
	float	tu0,tv0;	//texture coordinate 1
	float	tu1,tv1;	//texture coordinate 2
};


//Three Textures and colour
#define D3DFVF_CNT3VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX3)
struct D3DCNT3VERTEX
{
	float	x,y,z;		//untransformed position
	float	nx,ny,nz;	//vertex normal
	int		color;
	float	tu0,tv0;	//texture coordinate 1
	float	tu1,tv1;	//texture coordinate 2
	float	tu2,tv2;	//texture coordinate 3
};


//Four Textures and colour
#define D3DFVF_CNT4VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX4)
struct D3DCNT4VERTEX
{
	float	x,y,z;		//untransformed position
	float	nx,ny,nz;	//vertex normal
	int		color;
	float	tu0,tv0;	//texture coordinate 1
	float	tu1,tv1;	//texture coordinate 2
	float	tu2,tv2;	//texture coordinate 3
	float	tu3,tv3;	//texture coordinate 4
};


#define D3DFVF_NT1VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
struct D3DNT1VERTEX
{
	float	x, y, z;		//untransformed position
	float	nx, ny, nz;	//vertex normal
	float	tu0, tv0;	//texture coordinate
};


//Two Textures
#define D3DFVF_NT2VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2)
struct D3DNT2VERTEX
{
	float	x,y,z;		//untransformed position
	float	nx,ny,nz;	//vertex normal
	float	tu0,tv0;		//texture coordinate 1
	float	tu1,tv1;		//texture coordinate 2
};


//Three Textures
#define D3DFVF_NT3VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3)
struct D3DNT3VERTEX
{
	float	x,y,z;		//untransformed position
	float	nx,ny,nz;	//vertex normal
	float	tu0,tv0;	//texture coordinate 1
	float	tu1,tv1;	//texture coordinate 2
	float	tu2,tv2;	//texture coordinate 3
};


//Four Textures
#define D3DFVF_NT4VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX4)
struct D3DNT4VERTEX
{
	float	x,y,z;		//untransformed position
	float	nx,ny,nz;	//vertex normal
	float	tu0,tv0;	//texture coordinate 1
	float	tu1,tv1;	//texture coordinate 2
	float	tu2,tv2;	//texture coordinate 3
	float	tu3,tv3;	//texture coordinate 4
};


#endif //__FLEXIBLE_VERTEX_FORMATS_H__

