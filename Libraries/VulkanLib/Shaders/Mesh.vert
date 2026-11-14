#version 450
//A mesh's vertex, shaded as CRasteriser shades one: ambient, lights far away and point lights, added up and no more than one,
//with the colour of its triangle.  Matrices are row major with vectors on the left, which a mat4 read from them is the
//transpose of, so they multiply a column vector here.

#define MAX_DIRECTIONAL_LIGHTS	8
#define MAX_POINT_LIGHTS		64
#define DECAY_LINEAR			1
#define DECAY_SQUARE			2


layout(location = 0) in vec3	inPosition;
layout(location = 1) in vec3	inNormal;
layout(location = 2) in vec4	inColour;
layout(location = 3) in vec2	inUV;

//mWorld places the mesh.  avNormal are the rows of what turns its normals (its inverse transpose).  vColour multiplies the
//vertex's colour: the submesh's for a mesh with no vertex colours, and white for one with.  A skinned mesh has a 1 in
//avNormal[0].w, and is put in the world by its palette, not mWorld.
layout(push_constant) uniform SDraw
{
	mat4	mWorld;
	vec4	avNormal[3];
	vec4	vColour;
} sDraw;

//vTowards is towards a light far away, a unit, with its amount in w.  A point light's vPosition has its amount in w, and
//vValues are where it starts to fall off, how far it reaches and how it falls off.  Amounts are of RASTER_MAX_SHADE.
struct SDirectionalLight
{
	vec4	vTowards;
};

struct SPointLight
{
	vec4	vPosition;
	vec4	vValues;
};

//vLights is the ambient amount and how many of each kind of light there are.
layout(set = 0, binding = 0) uniform SFrame
{
	mat4				mViewProjection;
	vec4				vLights;
	SDirectionalLight	asDirectional[MAX_DIRECTIONAL_LIGHTS];
	SPointLight			asPoint[MAX_POINT_LIGHTS];
} sFrame;

//A skinned mesh's vertex has uvFirstCount.y weights from uvFirstCount.x, each a matrix of the palette and how much of the vertex it
//moves.  The palette is its instance's: each matrix is where it puts the mesh from where it was bound.
struct SWeight
{
	uint	uiMatrix;
	float	fWeight;
};

layout(std430, set = 2, binding = 0) readonly buffer SSkinVertices
{
	uvec2	auvFirstCount[];
} sSkinVertices;

layout(std430, set = 2, binding = 1) readonly buffer SSkinWeights
{
	SWeight	asWeights[];
} sSkinWeights;

layout(std430, set = 2, binding = 2) readonly buffer SPalette
{
	mat4	amMatrices[];
} sPalette;

layout(location = 0) out vec4	outColour;
layout(location = 1) out float	outShade;
layout(location = 2) out vec2	outUV;


float PointShade(SPointLight sLight, vec3 vNormal, vec3 vWorld)
{
	vec3	vTowards;
	float	fDistance;
	float	fStart;
	float	fShade;

	vTowards = sLight.vPosition.xyz - vWorld;
	fDistance = length(vTowards);
	if ((fDistance <= 0.0) || (fDistance >= sLight.vValues.y))
	{
		return 0.0;
	}
	fShade = max(dot(vNormal, vTowards) / fDistance, 0.0) * sLight.vPosition.w;
	fStart = sLight.vValues.x;
	if (fDistance > fStart)
	{
		if (int(sLight.vValues.z) == DECAY_LINEAR)
		{
			fShade *= fStart / fDistance;
		}
		else if (int(sLight.vValues.z) == DECAY_SQUARE)
		{
			fShade *= (fStart / fDistance) * (fStart / fDistance);
		}
	}
	return fShade;
}


//Where a skinned vertex is: where each of its matrices puts it, by its weight, and its normal turned by each.  One with no
//weights stays where the mesh was bound.
void Skin(out vec4 vWorld, out vec3 vNormal)
{
	uvec2	uvFirstCount;
	SWeight	sWeight;
	mat4	mMatrix;
	uint	i;

	uvFirstCount = sSkinVertices.auvFirstCount[gl_VertexIndex];
	if (uvFirstCount.y == 0)
	{
		vWorld = vec4(inPosition, 1.0);
		vNormal = inNormal;
		return;
	}
	vWorld = vec4(0.0);
	vNormal = vec3(0.0);
	for (i = 0; i < uvFirstCount.y; i++)
	{
		sWeight = sSkinWeights.asWeights[uvFirstCount.x + i];
		mMatrix = sPalette.amMatrices[sWeight.uiMatrix];
		vWorld += sWeight.fWeight * (mMatrix * vec4(inPosition, 1.0));
		vNormal += sWeight.fWeight * (mat3(mMatrix) * inNormal);
	}
}


void main()
{
	vec4	vWorld;
	vec3	vNormal;
	float	fShade;
	int		i;

	if (sDraw.avNormal[0].w > 0.5)
	{
		Skin(vWorld, vNormal);
	}
	else
	{
		vWorld = sDraw.mWorld * vec4(inPosition, 1.0);
		vNormal = mat3(sDraw.avNormal[0].xyz, sDraw.avNormal[1].xyz, sDraw.avNormal[2].xyz) * inNormal;
	}
	gl_Position = sFrame.mViewProjection * vWorld;

	if (dot(vNormal, vNormal) > 0.0)
	{
		vNormal = normalize(vNormal);
	}

	fShade = sFrame.vLights.x;
	for (i = 0; i < int(sFrame.vLights.y); i++)
	{
		fShade += max(dot(vNormal, sFrame.asDirectional[i].vTowards.xyz), 0.0) * sFrame.asDirectional[i].vTowards.w;
	}
	for (i = 0; i < int(sFrame.vLights.z); i++)
	{
		fShade += PointShade(sFrame.asPoint[i], vNormal, vWorld.xyz);
	}

	outShade = min(fShade, 256.0) / 256.0;
	outColour = inColour * sDraw.vColour;
	outUV = inUV;
}
