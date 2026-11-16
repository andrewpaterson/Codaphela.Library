#include <stdlib.h>
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Box.h"
#include "SupportLib/Image.h"
#include "SupportLib/PNGWriter.h"
#include "SupportLib/Sphere.h"
#include "TestLib/Assert.h"
#include "ShapeSceneHelper.h"
#include "TextureGallery.h"


#define TEXTURE_GALLERY_WIDTH		384
#define TEXTURE_GALLERY_HEIGHT		240
#define TEXTURE_GALLERY_TEXELS		(64.0f)		//To the unit.


//////////////////////////////////////////////////////////////////////////
//
// The renderer's pixels (0xAARRGGBB) as an image of 8 bit red, green and
// blue.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CImage> TextureGalleryImage(uint32* puiPixels)
{
	Ptr<CImage>	pImage;
	uint8*		puiData;
	int			i;

	pImage = OMalloc<CImage>((uint32)TEXTURE_GALLERY_WIDTH, (uint32)TEXTURE_GALLERY_HEIGHT, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	puiData = (uint8*)pImage->GetData();
	for (i = 0; i < TEXTURE_GALLERY_WIDTH * TEXTURE_GALLERY_HEIGHT; i++)
	{
		puiData[(i * 3)] = (uint8)(puiPixels[i] >> 16);
		puiData[(i * 3) + 1] = (uint8)(puiPixels[i] >> 8);
		puiData[(i * 3) + 2] = (uint8)puiPixels[i];
	}
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// A unit sphere and a box turned a sixth of a turn about z, side by side,
// seen from in front and above, so a texture is seen on a curve, on three
// flat faces and across the box's edges.
//
// Nothing is drawn with a 16 bit size: RenderLib's cooked mesh lists count
// with a size, so a mesh this big passes 64KB, and RenderLib isn't built
// for the W65C816.
//
//////////////////////////////////////////////////////////////////////////
void TextureGallery(const char* szName, Ptr<CSolidTexture> pTexture)
{
	Ptr<CScene>		pScene;
	CFileUtil		cFileUtil;
	CChars			szFile;
	SFloat32Box		sBox;
	SFloat32Vec3	sCentre;
	SFloat32Vec3	asAxes[3];
	SFloat32Vec3	sEye;
	uint32*			puiPixels;
	int				i;
	int				iDrawn;

#ifdef BASELIB_SIZE16
	return;
#endif

	pScene = OMalloc<CScene>();
	sCentre.Init(-1.3f, 0.0f, 0.0f);
	ShapeSceneAddTextured(pScene, OMalloc<CSphere>(&sCentre, 1.0f), pTexture, TEXTURE_GALLERY_TEXELS);

	sCentre.Init(1.3f, 0.0f, -0.2f);
	asAxes[0].Init(0.5f, 0.8660254f, 0.0f);
	asAxes[1].Init(-0.8660254f, 0.5f, 0.0f);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 0.8f, &asAxes[1], 0.8f, &asAxes[2], 0.8f);
	ShapeSceneAddTextured(pScene, OMalloc<CBox>(&sBox), pTexture, TEXTURE_GALLERY_TEXELS);

	puiPixels = (uint32*)malloc(TEXTURE_GALLERY_WIDTH * TEXTURE_GALLERY_HEIGHT * sizeof(uint32));
	sEye.Init(0.0f, -4.6f, 2.4f);
	AssertSize(2, ShapeSceneDraw(pScene, &sEye, puiPixels, TEXTURE_GALLERY_WIDTH, TEXTURE_GALLERY_HEIGHT));

	iDrawn = 0;
	for (i = 0; i < TEXTURE_GALLERY_WIDTH * TEXTURE_GALLERY_HEIGHT; i++)
	{
		iDrawn += (puiPixels[i] != 0xFF000000) ? 1 : 0;
	}
	AssertTrue(iDrawn > (TEXTURE_GALLERY_WIDTH * TEXTURE_GALLERY_HEIGHT / 10));

	cFileUtil.TouchDir("Output" _FS_ "Gallery");
	szFile.Init("Output" _FS_ "Gallery" _FS_);
	szFile.Append(szName);
	szFile.Append(".png");
	AssertTrue(SavePNG(TextureGalleryImage(puiPixels), szFile.Text()));
	szFile.Kill();

	free(puiPixels);
	pScene = NULL;
}

