#version 450
//A pixel of its triangle's colour times its shade, as CRasteriser draws one.  A transparent triangle's alpha blends it.

layout(location = 0) in vec4	inColour;
layout(location = 1) in float	inShade;

layout(location = 0) out vec4	outColour;


void main()
{
	outColour = vec4(inColour.rgb * inShade, inColour.a);
}
