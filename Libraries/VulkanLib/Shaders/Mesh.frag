#version 450
//A pixel of its triangle's colour and its texture's times its shade, as CRasteriser draws one with no texture.  A transparent
//triangle's alpha, times its texture's, blends it.  An untextured triangle's texture is white.

layout(location = 0) in vec4	inColour;
layout(location = 1) in float	inShade;
layout(location = 2) in vec2	inUV;

layout(set = 1, binding = 0) uniform sampler2D	sTexture;

layout(location = 0) out vec4	outColour;


void main()
{
	vec4	vTexel;

	vTexel = texture(sTexture, inUV);
	outColour = vec4(inColour.rgb * vTexel.rgb * inShade, inColour.a * vTexel.a);
}
