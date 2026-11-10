/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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

** ------------------------------------------------------------------------ **/
#ifndef __SCENE_FILE_GENERAL_H__
#define __SCENE_FILE_GENERAL_H__
#include "BaseLib/ChunkFileName.h"


//A scene file is a chunk file of plain numbers, needing nothing but BaseLib to write or read.
//
//Each attribute is a chunk of its own, holding only its values, so how many there are comes from the chunk's size.  A new
//attribute is a new chunk: a reader asks for chunks by name, so an old file lacks it and gets none, and an old reader ignores
//it.  SCENE_FILE_VERSION only changes for what that can't do.
//
//No struct is written whole, and only fixed size types are written, little endian.  An index refers to a list in the same
//file, counts from 0, and is -1 for none.
//
//A scene is right handed with y up, and matrices are row major with vectors on their left.  A polygon's corners go anticlockwise
//seen from outside.  A UV's v of 0 is an image's top row.  A camera looks along its connection's -z with y up, and a light shines
//along its connection's -z.
//
//	Version		int32
//	Mesh		Name
//				Positions		float32 x, y, z
//				Normals			float32 x, y, z
//				Colours			uint32 0xAARRGGBB
//				PolygonSizes	int32 corners of each polygon: a triangle is a polygon of 3
//				CornerPositions	int32 for each corner of each polygon, around it in order
//				CornerNormals	int32 for each corner, -1 for the polygon's flat normal
//				CornerColours	int32 for each corner
//				UVLayer			UVs				float32 u, v
//								CornerUVs		int32 for each corner
//				MaterialSlots	int32 materials, the mesh's own list
//				PolygonSlots	int32 for each polygon, an index in MaterialSlots
//				SmoothGroups	uint32 for each polygon
//				SkinCounts		int32 for each position, how many of the weights are its
//				SkinMatrices	int32 for each weight, an index in InverseMatrices
//				SkinWeights		float32 for each weight
//				InverseMatrices	float32 x 16 for each matrix
//	Image		Name
//				Size			int32 width, height
//				Channels		char for each channel, as "RGBA"
//				Pixels			uint8 for each channel of each pixel, with the top row first
//	Material	Name
//				Diffuse, Ambient, Emissive, Specular	float32 r, g, b, a
//				Shininess		float32 power, base
//				One chunk for each image used, of an int32 image: see gaszSceneFileTextures
//	Connection	Name
//				Parent			int32 connection
//				World, Local	float32 x 16, row major with vectors on the left
//				Parity			int32 1 if its matrix mirrors
//	Light		Name
//				LightType		int32 ESceneFileLightType
//				Colour			float32 r, g, b, a
//				LightValues		float32 decay start, range, intensity
//				Shadows			int32 1 if it casts them
//				LightDecay		int32 ESceneFileLightDecay
//				Attached		int32 connection
//	Camera		Name
//				CameraType		int32 ESceneFileCameraType
//				Lens			float32 horizontal field of view in radians, near, far
//				Attached		int32 connection
//	Animation	Name
//				Track			Attached		int32 connection
//								KeyTimes		float32 seconds from the start of the animation
//								KeyMatrices		float32 x 16 for each key, the connection's world matrix
//	Instance	Name
//				Connections		int32 connections, the instance's own list in slot order
//				InstanceMesh	MeshIndex		int32 mesh
//								ConnectionSlots	int32 for each of the mesh's matrices, one for a rigid mesh, an index in Connections
//								Materials		int32 materials in place of the mesh's MaterialSlots
//	Particles	Name
//				Positions		float32 x, y, z
//				Colours			float32 r, g, b
//				Radii			float32
//				Intensities		float32


//"SCN\0" as a little endian int32.
#define SCENE_FILE_ID		((chunkName)'S' | ((chunkName)'C' << 8) | ((chunkName)'N' << 16))
#define SCENE_FILE_VERSION	1
#define SCENE_FILE_NONE		-1


#define SCENE_FILE_FLOATS_IN_MATRIX		16

#define SCENE_CHUNK_VERSION				"Version"
#define SCENE_CHUNK_NAME				"Name"

#define SCENE_CHUNK_MESH				"Mesh"
#define SCENE_CHUNK_POSITIONS			"Positions"
#define SCENE_CHUNK_NORMALS				"Normals"
#define SCENE_CHUNK_COLOURS				"Colours"
#define SCENE_CHUNK_POLYGON_SIZES		"PolygonSizes"
#define SCENE_CHUNK_CORNER_POSITIONS	"CornerPositions"
#define SCENE_CHUNK_CORNER_NORMALS		"CornerNormals"
#define SCENE_CHUNK_CORNER_COLOURS		"CornerColours"
#define SCENE_CHUNK_UV_LAYER			"UVLayer"
#define SCENE_CHUNK_UVS					"UVs"
#define SCENE_CHUNK_CORNER_UVS			"CornerUVs"
#define SCENE_CHUNK_MATERIAL_SLOTS		"MaterialSlots"
#define SCENE_CHUNK_POLYGON_SLOTS		"PolygonSlots"
#define SCENE_CHUNK_SMOOTH_GROUPS		"SmoothGroups"
#define SCENE_CHUNK_SKIN_COUNTS			"SkinCounts"
#define SCENE_CHUNK_SKIN_MATRICES		"SkinMatrices"
#define SCENE_CHUNK_SKIN_WEIGHTS		"SkinWeights"
#define SCENE_CHUNK_INVERSE_MATRICES	"InverseMatrices"

#define SCENE_CHUNK_IMAGE				"Image"
#define SCENE_CHUNK_SIZE				"Size"
#define SCENE_CHUNK_CHANNELS			"Channels"
#define SCENE_CHUNK_PIXELS				"Pixels"

#define SCENE_CHUNK_MATERIAL			"Material"
#define SCENE_CHUNK_SHININESS			"Shininess"

#define SCENE_CHUNK_CONNECTION			"Connection"
#define SCENE_CHUNK_PARENT				"Parent"
#define SCENE_CHUNK_WORLD				"World"
#define SCENE_CHUNK_LOCAL				"Local"
#define SCENE_CHUNK_PARITY				"Parity"

#define SCENE_CHUNK_LIGHT				"Light"
#define SCENE_CHUNK_LIGHT_TYPE			"LightType"
#define SCENE_CHUNK_LIGHT_VALUES		"LightValues"
#define SCENE_CHUNK_LIGHT_DECAY			"LightDecay"
#define SCENE_CHUNK_COLOUR				"Colour"
#define SCENE_CHUNK_SHADOWS				"Shadows"
#define SCENE_CHUNK_ATTACHED			"Attached"

#define SCENE_CHUNK_CAMERA				"Camera"
#define SCENE_CHUNK_CAMERA_TYPE			"CameraType"
#define SCENE_CHUNK_LENS				"Lens"

#define SCENE_CHUNK_ANIMATION			"Animation"
#define SCENE_CHUNK_TRACK				"Track"
#define SCENE_CHUNK_KEY_TIMES			"KeyTimes"
#define SCENE_CHUNK_KEY_MATRICES		"KeyMatrices"

#define SCENE_CHUNK_INSTANCE			"Instance"
#define SCENE_CHUNK_CONNECTIONS			"Connections"
#define SCENE_CHUNK_INSTANCE_MESH		"InstanceMesh"
#define SCENE_CHUNK_MESH_INDEX			"MeshIndex"
#define SCENE_CHUNK_CONNECTION_SLOTS	"ConnectionSlots"
#define SCENE_CHUNK_MATERIALS			"Materials"

#define SCENE_CHUNK_PARTICLES			"Particles"
#define SCENE_CHUNK_RADII				"Radii"
#define SCENE_CHUNK_INTENSITIES			"Intensities"


//Each of these is a chunk's name: see gaszSceneFileMaterialColours.
enum ESceneFileMaterialColour
{
	SFMC_Diffuse,
	SFMC_Ambient,
	SFMC_Emissive,
	SFMC_Specular,

	SFMC_NumColours,
};


//Each of these is a chunk's name: see gaszSceneFileTextures.
enum ESceneFileTexture
{
	SFTX_Diffuse,
	SFTX_DiffuseLevel,
	SFTX_Specular,
	SFTX_Illumination,
	SFTX_Opacity,
	SFTX_Bump,
	SFTX_Detail,
	SFTX_Decal,
	SFTX_Reflection,

	SFTX_NumTextures,
};


//The values of these are in files, and are ELightType's.  An ambient light lights everything the same, so where it's attached
//doesn't matter.
enum ESceneFileLightType
{
	SFLT_Omni,
	SFLT_Directional,
	SFLT_Spot,
	SFLT_Ambient,
};


//The values of these are in files.
enum ESceneFileLightDecay
{
	SFLD_None,
	SFLD_Linear,
	SFLD_Square,
};


//The values of these are in files.
enum ESceneFileCameraType
{
	SFCT_Perspective,
	SFCT_Orthographic,
};


//Indexed by ESceneFileMaterialColour and ESceneFileTexture.
extern const char* const gaszSceneFileMaterialColours[SFMC_NumColours];
extern const char* const gaszSceneFileTextures[SFTX_NumTextures];


#endif // __SCENE_FILE_GENERAL_H__

