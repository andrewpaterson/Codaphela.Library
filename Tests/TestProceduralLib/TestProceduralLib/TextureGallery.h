#ifndef __TEXTURE_GALLERY_H__
#define __TEXTURE_GALLERY_H__
#include "SupportLib/SolidTexture.h"


//pTexture baked onto a sphere and a box and drawn to Output/Gallery/<szName>.png (ProceduralLib's TexturePlan.txt, Steps): what
//a texture looks like is judged by eye, so the tests of textures draw them to be looked at.  It asserts only that they drew
//(and with a 16 bit size, where RenderLib can't hold the meshes, draws nothing).
void TextureGallery(const char* szName, Ptr<CSolidTexture> pTexture);


#endif // __TEXTURE_GALLERY_H__

