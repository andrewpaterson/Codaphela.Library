# Fonts

Text is drawn with bitmap fonts made from TrueType files by SupportLib's own parser and rasteriser.  A font in one
colour is a [`CFont`]: an image holding its glyphs and a [`CFontDescription`] of where each glyph is, shared by
every colour of the font.  A [`CFontFamily`] makes the faces, sizes and colours text asks for, each the first time
it is asked for, and each glyph's pixels only when that glyph is first drawn.  Text is laid out by [`CTextLayout`]
into a sprite map of glyph cels and a geometry map of the lines drawn with it ([Images and
Drawing](images.md#maps)).  The text area built on all this is WindowLib's ([The Text
Area](../windowlib/text-area.md)).

## TrueType

[`CTrueTypeFont`] reads a whole TrueType file into memory, every read bounds checked:

- glyph outlines from `glyf`, simple and composite, located through `loca`;
- characters to glyphs with `cmap` format 4 or 12 ([`CTrueTypeFont::GetGlyphIndex`]);
- advances from `hmtx`, names from `name`, and optionally underline (`post`), strikeout and superscript and
  subscript sizes and offsets (`OS/2`);
- variable fonts ([`CTrueTypeVariations`]): axes and named instances (`fvar`), axis mapping (`avar`), outline
  deltas with interpolation of untouched points and phantom points (`gvar`), advance widths (`HVAR`) and font wide
  metrics (`MVAR`).  A font starts at its default instance; [`CTrueTypeFont::SetAxis`] or
  [`CTrueTypeFont::SetNamedInstance`] picks another, such as any weight of Inter Variable.

There is no hinting, no kerning and no CFF (PostScript outline) font.

[`CTrueTypeRasteriser`] draws glyphs at a pixel size of one em, anti-aliased by exact area coverage through the
[`CCoverageRasteriser`] the vector drawing uses ([Images and Drawing](images.md#vector-drawing)).  Outlines are in
whole font units, positions in 1/256 pixels ([`SCoverageSubpixel`]), lines and quadratic curves added to the
coverage, and every sum integer, so Debug, Release and every compiler draw the same pixels.  It makes an opacity
image of a glyph or a line of text ([`CTrueTypeRasteriser::RasteriseGlyph`],
[`CTrueTypeRasteriser::RasteriseText`]) or blends text in a colour onto an image
([`CTrueTypeRasteriser::DrawText`]).  Overlapping contours, as in variable fonts, are filled once.

The variations' arithmetic is integer too (16.16 axis values, normalised coordinates, scalars and deltas) and
follows FreeType 2.13's roundings: deltas round halves up ([`TrueTypeRoundDelta`]), normalised coordinates stay
16.16 rather than rounding to 2.14, as FreeType and fontTools do.  Inter's outlines match FreeType's at every point
for most settings and differ by one unit at no more than 6 points in 100,000 elsewhere (2026-11-06).  Advances
always come from `HVAR`, so composite glyphs flagged USE_MY_METRICS can differ from FreeType's; fontTools agrees
with `HVAR`.

## Bitmap fonts

[`CTrueTypeFontFactory`] makes bitmap fonts from a TrueType font at a pixel size, in two steps:

1. [`CTrueTypeFontFactory::Describe`] measures the characters asked for, packs their glyphs into an image of a
   given width and returns a [`CFontDescription`]: the font's metrics, the image's size, and for each character
   where its glyph is in the image ([`SGlyphDescription`]: a [`CSubImage`], its step and the TrueType glyph it came
   from). Each glyph's full size is its advance by the line height, so text sits on a common baseline.  Advances
   are rounded to whole pixels.
2. [`CTrueTypeFontFactory::Rasterise`] draws an image for that description in a colour, as many times as there are
   colours; or [`CTrueTypeFontFactory::CreateImage`] makes an empty one to be drawn into lazily.

A [`CFont`] is a description and one image laid out as it says ([`CFont::Init`] logs an error if the image is
smaller than the description's).  Glyphs are keyed by their character's code point, little endian, as many bytes as
the code point's UTF-8 length; [`CFont::GetGlyph`] takes a code point or a [`CUnicodeReader`].  A glyph object
([`CGlyph`]: a cel and a step) is only made the first time its character is asked for, and kept by the
description's index of it.

The description also holds the lines drawn with text and the script positions, in pixels: underline and strikeout
tops and thicknesses, and superscript and subscript scales and shifts, from `post` and `OS/2` scaled by integer
arithmetic in 1/64 pixels, or worked out from the ascent and descent when the font hasn't got them.  Its average
and fixed widths are worked out when first asked for.  The font's name is [`CFont::GetFontName`], not `GetName`,
which is the object's name: an override of it once crashed the object system while freeing.

[`ReadTrueTypeFont`] is the short way: a white font of the printable ASCII characters.  [`FontWriter`] and
[`FontReader`] save and load a [`CFont`] as a text descriptor beside its cels and image.  [`CFontFactory`] makes a
font from an image of characters and a list of them, the way before TrueType.

## Colours

A font in a colour is a font of its own, sharing the description.  [`CFontColours`] holds a white font (white
pixels, the coverage in the opacity) and makes each colour from it the first time the colour is asked for
([`CFontColours::GetFont`]): the white image copied, its red, green and blue set to the colour, and its opacity
scaled by the colour's alpha.  Asking for white gives the white font itself.  Colours are [`ARGB32`].

## Families

A [`CFontFamily`] is a typeface's faces (regular, bold, italic, bold italic: [`EFontStyle`]), each a TrueType file
([`CFontFamily::SetFace`], optionally at a weight of a variable font) loaded once into a shared [`CTrueTypeFace`].
[`CFontFamily::GetColours`] gives a face at a pixel size, made the first time it is asked for and kept, sizes keyed
in 1/64 pixels; [`CFontFamily::GetFont`] gives it in a [`STextStyle`]'s colour.  A style whose face isn't set is an
error with no fallback.  A family can instead be given fonts already made
([`CFontFamily::Init(Ptr<CFontColours>)`], [`CFontFamily::AddFont`]), each only in the size it was made for.

[`CFontFamily::GetRunFont`] also places superscript and subscript: the face at the size times the description's
script scale, with its baseline shifted up or down by the description's shift.

A font can also be turned a quarter clockwise ([`EFontTurn`]) for text that runs down, as on docked tool windows'
side tabs: the glyphs are packed turned, with the same steps and metrics as the upright font.

## Lazy glyphs

A family's fonts are lazy: their images start empty and each glyph's pixels are drawn the first time the glyph is
made.

- The description has a glyph rasteriser ([`CFontDescription::SetRasteriser`]): a [`CTrueTypeGlyphRasteriser`] for
  the face and size, which draws one glyph white into its rectangle as [`CTrueTypeFontFactory::Rasterise`] would.
- A lazy white font ([`CFont::SetLazy`]) keeps a bit per glyph saying whether it has its pixels;
  [`CFont::EnsurePixels`] draws them before the glyph's cel is made.
- A lazy coloured font ([`CFont::SetLazyColour`]) asks its white font for the glyph's pixels, then recolours just
  that glyph's rectangle.  [`CFontColours`] makes lazy colours of a lazy white font.

Once every glyph has been drawn, a lazy font's image is the eager font's byte for byte
(`TestFontFamilyLazyMatchesRasterise`).  The eager [`CTrueTypeFontFactory::Rasterise`] stays for HCP65's fonts,
[`FontWriter`] and the tests.

## What text costs

Measured on 2026-10-22 in a 64-bit RelWithDebInfo build on Linux, the objects rooted, per font, in ms:

| Font | Load | Describe | Rasterise | Colour order | Recolour | Glyph objects | Total |
|---|---|---|---|---|---|---|---|
| Inter 14 px, 99 characters | 0.43 | 0.07 | 0.55 | 0.27 | 0.18 | 0.33 | 1.84 |
| Inter 18.67 px, 586 characters | | | 3.8 | 2.2 | 1.3 | 2.6 | 10.9 |

Making fonts was cheap; what cost was the object system.  With the objects not reachable from the root, every
pointer change walked the graph's distance to the stack: 99 glyph objects took 2.4 ms and 586 took 67.6 ms (growing
faster than the count), and laying out HCP65's 2.5 KB of text into 2,399 sprites took 1,042 ms rather than 7.2 ms
rooted. Hence fonts are rooted, sprites are plain data ([Images and Drawing](images.md#sprite-maps)), and glyph
objects are made only for characters used.

Lazy glyphs were then measured to save at most about 0.5 ms per HCP65 font (4 to 7 ms with the 586 character set);
the user chose them anyway.  A rooted family of 4 faces in 7 sizes went from 6.39 to 0.54 ms, and a colour of each
from 1.26 to 0.04 ms.

## Laying out text

[`CTextLayout::Layout`] lays out a [`CText`] (runs of characters, each in a font, with a baseline shift and
decorations) for a [`CBaseFontDraw`]: [`CGlyphListFontDraw`] records glyphs and rectangles, [`CMapFontDraw`] puts
the glyphs into a [`CSpriteMap`] and the rectangles into a [`CGeometryMap`].  As far as the fonts drive it:

- Words wrap at spaces to a width; a line's glyphs are held until it ends, then drawn on one baseline.  A row's
  ascent and descent are the largest of the glyphs placed on it (moved by their run's baseline shift), not of every
  run that starts on it.  An empty line takes the font current when it ends.
- Glyphs are placed by their full rectangle at the line's top, so a description's alignment must stay at the top
  left ([`CFontDescription`] once had a `Done` that moved it to the baseline, which shifted every glyph; it was
  removed).
- Underlines, overlines and strike-throughs (the `TD_` flags of [`SupportLib/TextStyle.h`]) are drawn by
  [`CTextLayout::DecorateLine`] in the text's colour over each stretch of glyphs that has them, trailing spaces
  left out.  A stretch that mixes sizes gets one line, placed and thickened by its largest font (with that font's
  baseline shift); one rectangle per colour.  An overline is at the top of the ascent, as thick as the underline.
- [`STextStyle`] says how a run is drawn (bold, italic, pixel size, colour, position, decorations);
  [`STextStyleOverride`] changes only the properties it sets, so overlapping spans combine, the last setting each
  property winning.

## Tests

| Test | Covers |
|---|---|
| [`TestSupportLib/TestTrueTypeFont.cpp`] | parsing, outlines, every glyph, rasterising, the factory, colours, mixed sizes and typefaces, script and line metrics, decorations, variations against FreeType's outlines and widths |
| [`TestSupportLib/TestFontFamily.cpp`] | faces and sizes, a missing face, one-font families, colour order, scripts, lazy glyphs (and lazy matching eager), clockwise fonts, variable weights |
| [`TestSupportLib/TestFont.cpp`] | fonts from images, writing text, TrueType fonts, writing and reading `.font` files |

`Input/TrueType/DrawText.png` was made on Linux.  Variations were checked by dumping every glyph with fontTools and
freetype-py and comparing.

## Decisions

- Integer and fixed point rasterising, so Release builds draw what Debug builds draw (the user asked, 2026-10-21,
  after floats gave different pixels in Release).
- One class per file for the TrueType code (the user, 2026-10-20).
- Points mean pixels at 96 DPI where a size is given in points: Inter at 14 pt is 14 × 96 / 72 px (the user,
  2026-10-20).
- Colour-independent descriptions shared by one font per colour, so per-colour fonts don't repeat glyph data (the
  user asked for the shared description, 2026-10-22; the name [`CFontDescription`], the index with an array of
  glyphs, and the describe and rasterise split were proposed and accepted).
- Coloured text (the user's decisions, 2026-10-22): a white font recoloured; [`ARGB32`] colours; a family of faces
  and sizes keyed by style and size; bold and italic as flags with one TrueType file per face, not weights; sizes
  in pixels; a missing face is an error with no fallback; superscript and subscript as a smaller size and a
  baseline shift; decorations in the text's colour, strike-through included, drawn as geometry; one line for a
  decorated stretch of mixed sizes.  Built in that order: colours and spans, baseline-aligned lines, families,
  script metrics, then the geometry map and decorations.
- Lazy rasterising, a full-size image drawn into glyph by glyph (the user's choice, 2026-10-22, after the
  measurements above showed it would save little).
- Variable fonts follow FreeType's roundings so outlines match it (2026-11-06).

## Not done

- Kerning (`GPOS`) and hinting.
- Choosing a variable font's optical size axis (`opsz`) from the pixel size in [`CFontFamily`].
- A text area's spans can't change typeface: a text area has one family.
- The expected images were made on Linux; the first Windows run may differ.
