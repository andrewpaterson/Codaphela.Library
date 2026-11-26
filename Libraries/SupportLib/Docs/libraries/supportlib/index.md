# SupportLib

SupportLib is the project's media library: images and drawing, fonts and text layout, sound, meshes, shapes and
scenes.  It sits on BaseLib and StandardLib, and almost everything in it is an object
([Objects](../standardlib/objects.md)): an image, a font or a sound is held by [`Ptr`]s and freed when nothing
reaches it.  WindowLib draws its windows with it, the HCP65 IDE, LogiCim, HCP65-Sculpt and the HCP65 emulator draw
and play with it, and GerberLib draws circuit boards with its vector rasteriser.  It also compiles for the 65C64
and the W65C816, and its tests run there (TestSupportLib65); HCP65's `modplay.exe` plays modules with it on the
W65C816.

Its arithmetic is integer wherever pixels or samples must come out the same in every build and on every target:
coverage, TrueType outlines, blends and the module mixer.

| Page | Covers |
|---|---|
| [Images and Drawing](images.md) | [`CImage`] and cels, blitters and each image's blitter cache, [`CImageDraw`], the shared vector drawing, tile, sprite and geometry maps, rectangles |
| [Fonts](fonts.md) | the TrueType parser and rasteriser, font descriptions and coloured fonts, families, lazy glyphs, what text costs, text layout |
| [Sound](sound.md) | [`CSound`], WAVE files, the resampler, the MOD player and its mixer on the W65C816 |
| [Shapes](shapes.md) | [`CShape`] solids and the geometry under them |
| [Scenes](scenes.md) | [`CScene`]: instances, skeletons, cameras, lights and scene files |
| [Unwrapping](unwrapping.md) | laying meshes flat for textures ([`CMeshUnwrap`]) |

## Its parts

| Part | Main classes | Is |
|---|---|---|
| Images | [`CImage`], [`CImageCel`], [`CSubImage`], [`CImageAccessor`], [`ReadImage`], [`WriteImage`] | pixels in any channels, cels of them, files ([Images and Drawing](images.md)) |
| Image modifiers | [`CImageModifierStack`], [`CImageResampler`], [`CImageRecolour`], [`CImageCombiner`] | changing images in place, combining cels into atlases |
| Blitters | [`CImageBlitter`], [`CImageCelBlitterCache`], [`CImageRowBlitterFactory`] | drawing cels onto images ([Images and Drawing](images.md#blitters)) |
| Drawing | [`CImageDraw`], [`CVectorPath`], [`CVectorRasteriser`], [`CCoverageRasteriser`] | boxes, lines and anti-aliased shapes ([Images and Drawing](images.md#vector-drawing)) |
| Maps | [`CMaps`], [`CTileMap`], [`CSpriteMap`], [`CGeometryMap`] | what canvases draw ([Images and Drawing](images.md#maps)) |
| Fonts | [`CTrueTypeFont`], [`CTrueTypeRasteriser`], [`CFontDescription`], [`CFont`], [`CFontColours`], [`CFontFamily`] | text in any face, size and colour ([Fonts](fonts.md)) |
| Text | [`CText`], [`CTextLayout`], [`STextStyle`], [`CMapFontDraw`] | runs of styled text laid out into maps ([Fonts](fonts.md#laying-out-text)) |
| Sound | [`CSound`], [`CSoundCel`], [`CSoundCelGroup`], [`CSoundResampler`], [`LoadWAV`] | samples in any channels ([Sound](sound.md)) |
| Modules | [`CModSong`], [`LoadMOD`], [`CModPlayer`] | ProTracker modules ([Sound](sound.md#mod-player)) |
| Meshes | [`CMesh`], [`CMeshEditor`], [`CMeshModifierStack`], [`LoadOBJ`] | polygon meshes, made and changed by editors and modifiers |
| Shapes | [`CShape`], [`CConvexHull`] | solids ([Shapes](shapes.md)) |
| Scenes | [`CScene`], [`CInstance`], [`CSkeleton`], [`CCamera`], [`CLight`], [`CMaterial`] | what a renderer draws ([Scenes](scenes.md)) |
| Rectangles | [`CRectangle`], [`CInt32Rect`], [`CRectangleBestPacker`] | pixel and real rectangles, packing them ([Images and Drawing](images.md#rectangles)) |

The tests are TestSupportLib ([`TestSupportLib/TestSupportLib.cpp`]); after a StandardLib change, run it with
TestStandardLib and TestWindowLib.
