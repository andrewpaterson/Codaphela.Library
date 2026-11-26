# Images and Drawing

An image is a [`CImage`]: a width and height of pixels whose channels (red, green, blue, opacity, or anything else)
are interleaved in a StandardLib [`CChannels`], each channel of any primitive type.  A cel ([`CImageCel`]) is a
rectangle of an image drawn as a unit: a glyph, a tile, a sprite, a button's border.  Cels are drawn onto images by
blitters, which each image keeps in a cache; shapes are drawn as vectors, filled by exact area coverage.  This page
covers those, the maps of cels and shapes that canvases draw, and the rectangles all of it is measured in.  How a
window decides what to draw is [Drawing and Speed](../windowlib/drawing.md); PNG is in [Deflate and
PNG](../standardlib/compression.md).

## Images

A [`CImage`] is made by one of its `Init`s (a size and a list of channels, a colour format, or the channels of
another image), or changed between [`CImage::BeginChange`] and [`CImage::EndChange`]: add, remove or rename
channels and set the size, and the data is laid out again once, at the end.  Each channel has a purpose
([`EImagePurpose`]: diffuse colour, opacity, normals, masks, palettes) and a primitive type, down to bit sized
ones, so an image can be 8 bits a pixel (R3G3B2) or 64.  Pixels are read and written whatever their format through
an accessor ([`CImageAccessorCreator`], [`CImageAccessor`]).

Images are read by [`ReadImage`] (PNG, BMP, TGA, RAD) and [`LoadSFT`] (Andre's computer's format), written by
[`WriteImage`] (those, raw and SFT), and changed by modifiers on a [`CImageModifierStack`] (resampling,
recolouring, adding and renaming channels, drawing boxes) in place, so everything pointing at the image sees the
change.

## Cels

A cel is a [`CSubImage`] of a source image: the rectangle its pixels come from, and offsets around it to the cel's
full size, so a sprite whose transparent border was cropped ([`CImageCel::CropTransparentBorders`]) still lines up
with its neighbours.  The full size is what a cel occupies; the image rectangle is what is drawn.  Its alignment
puts its origin at its top left, centre or bottom right.  Many cels share a source image: a font's glyphs, a tile
set, an atlas.  [`CImageCelGroup`] holds cels found by index; [`CImageDivider`] cuts an image into cels: by
rectangles, by a grid ([`CImageDividerNumbers`]), or by a border colour between them
([`CImageDivider::GenerateFromBorder`]).

## Blitters

A [`CImageBlitter`] draws one cel onto one destination image.  It is made once for the pair
([`CImageBlitter::Configure`]) and then blits wherever it is told, clipped to the destination and to a rectangle.
Making it reads the formats of the cel's image and the destination and cuts each row of the cel into runs, each run
drawn by a row blitter suited to it:

| Source | Row blitter | Does |
|---|---|---|
| Opaque, same layout | [`CImageRowBlitterContiguous`] | copies the row's bytes |
| Opaque, different layout | [`CImageRowBlitterByteAlignedOpaque`] | copies each channel |
| 8 bit opacity, 24 bit colour | [`CImageRowBlitterRGBByteAlphaByteTranslucent`] for translucent runs, the opaque one for opaque runs | blends by the source's opacity; fully transparent runs are skipped |

Each has a variant that also writes the destination's opacity when it has one (`DestAlpha`), and a sizing variant
that stretches the cel to a width and height ([`CImageDraw::DrawCel`]'s second overload,
[`CImageBlitter::EnableSizing`]). The row blitters are stateless and shared: every blitter points at the ones in
[`gcImageRowBlitterFactory`].  A pair of formats with no row blitter has no blitter: [`CImageDraw::DrawCel`] logs
that the formats don't match and draws nothing.

Blends round to the nearest ([`SupportLib/ColourBlend.h`]): [`BlendOverByte`] adds opacity, [`BlendLerpByte`] mixes
a colour by a coverage, [`BlendStraightOverPixel`] draws a straight alpha pixel over another, and
[`BlendPremultipliedByte`] is what a window's transparent canvases are copied with.  They are integer, worked in
[`size`], so they are 16 bits on the W65C816.

## The blitter cache

Making a blitter is far dearer than using it, so blitters are kept.  Each destination image owns its blitters: a
[`CImageCelBlitterCache`] made the first time it is asked for ([`CImage::GetBlitterCache`]), with one blitter per
cel ([`CImageCelBlitterCache::CreateImageBlitter`], [`CImageCelBlitterCache::GetImageBlitter`]).  The blitters were
made for the image as it was, so the image lets go of its cache whenever its format or size can change:
[`CImage::EndChange`], [`CImage::ReInit`] and [`CImage::CopyIntoInitialised`].

The lifetimes:

- **The destination goes first.**  The image holds its cache, the cache its blitters, each blitter the image: an
  unreachable cycle, freed together when nothing else holds the image.  No global map of caches exists to keep
  them.
- **The cel goes first.**  Cels can be freed while their destination lives on: a window's canvas image outlives the
  cels of child canvases made again on a resize.  So a blitter holds its cel weakly
  ([`CImageBlitter::mwSourceCel`], [Weak References](../standardlib/weak-references.md)): whoever draws with a
  blitter keeps its cel, and once the cel is freed the blitter's cel is NULL.  The cache drops such blitters the
  next time it is used, never while something is being freed.

[`CImageDraw::DrawCel`], WindowLib's image and maps canvas draws, and the maps below all take their blitters from
the destination's cache.  Before the cache, [`CImageDraw::DrawCel`] made a blitter for every draw.

## Drawing on an image

[`CImageDraw`] draws on an image it is given (a raw pointer: it isn't an object).  Its old calls set pixels in one
colour without blending: [`CImageDraw::DrawPixel`], [`CImageDraw::DrawBox`] (filled or outlined),
[`CImageDraw::DrawLine`].  Its anti-aliased calls fill a vector path and blend it with straight alpha
([`BlendStraightOverPixel`]); the image must have an opacity channel:

- [`CImageDraw::DrawPath`]: any [`CVectorPath`], in pixels y down or placed by an origin, a scale and which way y
  goes;
- [`CImageDraw::DrawCircle`], [`CImageDraw::DrawSector`], [`CImageDraw::DrawRing`], [`CImageDraw::DrawStroke`],
  [`CImageDraw::DrawPolygon`], [`CImageDraw::DrawRectangle`], [`CImageDraw::DrawRoundedRectangle`]: paths made for
  you.  Angles are degrees, 0 right and 90 up, anticlockwise.

[`CImageDraw::DrawSemicircle`] is a [`CImageDraw::DrawRing`] (a filled one has no hole): WindowLib's jewel buttons
and scroll bars are drawn with it.

## Vector drawing

One vector path and one coverage rasteriser draw every 2D shape: GerberLib's apertures and regions,
[`CImageDraw`]'s shapes and [`CGeometryMap`]'s lines, circles and polygons.

- A [`CVectorPath`] is parts, each a set of closed contours of straight edges and circular arcs filled by the
  non-zero rule.  A part with exposure on adds to what the parts before it drew; exposure off erases it (Gerber's
  clear polarity).  Cubic Béziers ([`CVectorPath::CubicTo`]) are flattened to lines as they are added.
  [`SupportLib/VectorShapes.h`] adds strokes, arc strokes, obrounds, regular polygons, rectangles, rounded
  rectangles, sectors and rings.
- A [`CVectorRasteriser`] places a path in pixels (an origin, a scale and y up or down,
  [`CVectorRasteriser::SetTransform`]) and fills it into a grid of coverage, 0 to 255.
- A [`CCoverageRasteriser`] does the filling, as font-rs and FreeType do: each pixel cell accumulates the cover and
  area of the edges crossing it, and a sweep along each row turns them into exact area coverage.  Positions are
  1/256 of a pixel ([`SCoverageSubpixel`]) and every sum is integer, so every build and target makes the same
  pixels.  The TrueType rasteriser uses it too ([Fonts](fonts.md)).

Arcs are flattened onto a grid of angles shared by every arc of a circle: a multiple of 8 steps a turn, chosen from
a tolerance of 0.05 pixels (at most 65,536), the cosines and sines worked out for one octant and mirrored, so the
grid is exactly symmetric about the axes and diagonals.  Vertices sit at r·sqrt(a/sin a) from the centre so the
flattened arc has the circle's area, and an arc ending on a grid angle gets that grid vertex.  So a quarter circle
covers exactly the pixels of the half it is part of, and mirrored shapes cover mirrored pixels; quarter turns can
still differ by 1 in 255, as the rasteriser cuts rows and columns differently.

## Maps

A canvas draws [`CMaps`]: an ordered list of maps blitted onto a viewport image at a viewport position
([`CMaps::Blit`]).  A map is a [`CBaseMap`]; the maps of cels are [`CBlockMap`]s, which make blitters from the
viewport's cache ([`CBlockMap::CreateCelBlitters`]).

| Map | Holds | Used for |
|---|---|---|
| [`CTileMap`] | layers of tiles in a grid ([`CTileLayerCel`]) | fixed width text, tile sets |
| [`CSpriteMap`] | cels at positions | proportional text, icons |
| [`CGeometryMap`] | shapes in colours, and borrowed cels | text decorations, LogiCim's circuits |

### Sprite maps

A sprite is plain data, an [`SSprite`]: a position and the index of its cel.  The map keeps each cel it draws once
(its cel table) with one blitter for it, and finds a cel's index through a hash map, so laid out text has a cel and
a blitter for each character it uses, however many glyphs it has.  [`CSpriteMap::AddSprite`] adds one;
[`CSpriteMap::RemoveSprites`] cuts the list.  A NULL cel isn't added.

Sprites were objects (CSprite, deleted 2026-10-24).  The cels stay objects: they are shared between maps,
polymorphic (transparent and masked cels), and weakly held by their blitters.  A sprite map can't be saved yet
([`CSpriteMap::Save`] returns false), as with geometry maps.

### Geometry maps

A [`CGeometryMap`] is a list of plain commands ([`SGeometryCommand`]), each with its bounds in map pixels (right
and bottom exclusive) and a colour:

- rectangles ([`CGeometryMap::AddRectangle`]), filled without anti-aliasing;
- lines with square, round or butt caps ([`EGeometryLineCap`]), circles, filled circles, polygons (non-zero) and
  polylines (mitred joins, bevelled past a mitre limit of ten, as Java's BasicStroke), anti-aliased through
  [`CVectorRasteriser`].  Points are pixel centres: a whole number is the middle of a pixel;
- sprites ([`CGeometryMap::AddSprite`]): a cel of premultiplied colour blitted over what is there, and tiles
  ([`CGeometryMap::AddTile`]): a cel copied over what is there.  The map doesn't hold these cels; they must last
  until it is blitted.

A shape is blended as a translucent cel is: its colour is mixed with what is there by its alpha
([`BlendLerpByte`]), and where the viewport has opacity the shape adds to it ([`BlendOverByte`]), which on a canvas
cleared to nothing leaves premultiplied pixels like a cel's.  [`CImageDraw`] couldn't be used for this: its boxes
overwrite and its shapes blend straight alpha.

## Rectangles

A rectangle is the header-only template [`CRectangle`]`<V, M, N>`: V a 2D vector type, M its component and N a type
twice as wide, so [`CRectangle::GetArea`] can't overflow (as [`SVec2`]'s N).  Right and bottom are exclusive: left
0 to right 1 is a width of 1.  The concrete rectangles are named after their vector:

| Class | Vector | For |
|---|---|---|
| [`CInt32Rect`] | [`SInt32Vec2`] | pixels: images, cels, clips, damage |
| CFloat64Rect ([`SupportLib/Float64Rect.h`]) | [`SFloat64Vec2`] | real units: GerberLib's bounds in millimetres |

The methods that treat a point as the pixel below and right of it are on [`CInt32Rect`] only, not the template:
[`CInt32Rect::GrowToContain`] (which grows by one past the point) and the texture coordinate getters.  A new kind
is a subclass named after its vector, `C<Type>Rect`.

## Tests

| Test | Covers |
|---|---|
| [`TestSupportLib/TestImageDraw.cpp`] | boxes, lines, shapes, semicircle symmetry, cels onto every format, sizing, the blitter cache's lifetimes (cel freed first, destination first, shared cels, freed together) |
| [`TestSupportLib/TestVectorPath.cpp`] | cubic flattening and its area |
| [`TestSupportLib/TestMaps.cpp`] | tile and sprite maps, shared cels, clipped blits |
| [`TestSupportLib/TestGeometryMap.cpp`] | every command, opacity, viewports, colour orders, bounds |
| [`TestSupportLib/TestRectangle.cpp`] | clipping, area, points, 16 bit and float rectangles |

## Decisions

- One vector path and coverage rasteriser for Gerber panel checking and WindowLib's widget images; new 2D drawing
  goes through [`CVectorPath`] and [`CImageDraw::DrawPath`], never another arc flattener (the user asked for the
  shared drawing, 2026-10-21).
- Blends round exactly (/255 to the nearest) in one place, [`SupportLib/ColourBlend.h`] (2026-10-21).  Changing the
  blitters' rounding moved many expected PNGs by 1, which were written again.
- Coverage arithmetic is integer, so Debug, Release and every target draw the same pixels (2026-10-21, after
  Release builds drew different pixels with floats; the user prefers fixed point for anything that must match
  across builds).
- Each destination image owns its blitter cache, so its death frees its blitters with it; any global lookup must
  not keep anything (the user's direction, 2026-10-17).  The user said a field on the image isn't clean and will
  need rework if another cache needs attaching to images.
- Cels can outlive their destinations, and destinations their cels (the user, 2026-10-17).  The cel and the cache
  first kept raw lists of each other, each `Free` unhooking the other side; since 2026-11-16 the blitter holds its
  cel by a weak reference instead.
- Sprite maps made like geometry maps, plain sprites and a table of cels, rather than making cels plain data (the
  user's choice, 2026-10-24): one object per glyph made the object system's distance walks the cost of laying text
  out ([Fonts](fonts.md#what-text-costs)).
- One rectangle template for pixels and real units, with an overflow-safe area (the user, 2026-10-22).

## Not done

- A global map of blitter caches (by destination, or destination and cel), which would make the sprite and tile
  layers' blitter pointers and [`CBlockMap::ClearCelBlitters`] unnecessary.  The user is designing it; eviction and
  lifetimes come first.
- Formats without a row blitter have no blitter at all: an accessor based fallback is a stub in
  [`CImageBlitter::CreateImageRowBlitters`].
- Saving sprite and geometry maps.
- Whether plain sprites made laying text out faster wasn't measured.
