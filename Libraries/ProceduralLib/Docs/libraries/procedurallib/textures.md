# Texture Generators

ProceduralLib's textures are solid textures: every one is a function of a point in space, sampled a point at a time
on a surface by SupportLib's bake ([`CMeshBake`], [`BakeMeshTexture`]), every texel its own. A texture is a small graph
of objects, generators at the bottom and operators that read other textures above them, and the graph is what is
saved: texels can always be made again from it. Materials (marble, wood, stone, rust, bricks, tiles, weathering) are
graphs built by functions. The noise they are made from is plain arithmetic in BaseLib. HCP65-Sculpt shows a
material's graph as a Blender-style node editor over a 3D view of the bake, and saves and loads materials as files.

The idea is .kkrieger's (Farbrausch's .werkkzeug 3): a texture stored as how it was made, a few generators, many
filters, colour beside height, floats between operators so a long chain doesn't band, and a colour ramp and a mask as
the operators used most. Not taken: 2D images and tiling.

ProceduralLib isn't in an indexed solution, so its classes are named here by their headers.

## Where it lives

| Library | What |
|---|---|
| BaseLib | Noise ([`BaseLib/Noise.h`]): functions of a point and a seed, no objects, nothing to set up |
| SupportLib | [`CSolidTexture`] and its sample [`SSolidSample`], [`CSolidChecker`], the bake ([`CMeshBake`], [`SMeshBakeSettings`]) |
| ProceduralLib | every other generator and operator, the materials, materials as files |

## What a texture gives

A sample, not a colour: an [`SSolidSample`] holds a colour (red, green, blue, alpha), a height (how far the surface
is raised, in the texture's units), shine (how strong a highlight is), gloss (how tight it is) and glow (how much of
its colour it gives off as light). All are floats, not clamped between operators. All five were carried from the
start, as carrying one is an operation or so against a noise's hundreds. Shine and gloss are a pair: wet stone and
polished metal both shine, one's highlight broad, the other's a point.

[`SSolidSample::Channels`] is the sample as eight numbers in a row, so operators that treat every channel alike loop
over them, and [`SSolidSample::Blend`] moves it towards another. The one number an operator reads from a sample (a
ramp's place, a mask's amount) is [`SSolidSample::Value`], its colour's brightness, not its height, so a texture
adjusted or ramped is a mask as it looks. A texture that is only a number (a noise, a gradient) gives it as a grey and
as the height ([`SSolidSample::InitValue`]), so any texture plugs into any operator.

Left out of the sample: opacity apart from alpha, a normal (made from the height by the bake; both would disagree),
ambient occlusion (from the height and the shape: CSolidCavity), and metalness (physically based rendering, which
neither renderer does).

## Placement

A [`CSolidTexture`] has a placement: an origin, three axes and a scale ([`CSolidTexture::Place`]). A point's sample is
the texture's own ([`CSolidTexture::TextureSample`]) at the point taken into its space ([`CSolidTexture::ToOwn`]),
the normal turned the same way and the texel's width divided by the scale. The height is multiplied by the scale on
the way out, so a texture placed half the size is raised half as far and slopes as it did. An operator samples its
inputs at its own point, so an input's placement is inside its operator's: a graph is placed in the world once, at its
top, and each texture in it can still be moved against the rest. [`CSolidTexture::Changed`] remakes what Init and
Place derive when an editor sets fields from outside.

## Noise

[`BaseLib/Noise.h`], the same on every machine (TestBaseLib's [`TestBaseLib/TestNoise.cpp`] runs on the 65C64 and the
W65C816 too). A unit of space is one lattice cube.

| Function | Is |
|---|---|
| [`NoiseHash`] | a lattice point and a seed to 32 bits by multiplying and xoring: no permutation table, so no 256 period |
| [`GradientNoise`] | Perlin's improved noise (2002), -1 to 1, 0 at lattice points, with its gradient if asked |
| [`FractalNoise`] | octaves, each twice as fine and half as strong, as a sum ([`FN_Sum`]), turbulence ([`FN_Turbulence`]) or ridged ([`FN_Ridged`]) |
| [`CellularNoise`] | Worley's (1996): the nearest two points' distances (F1, F2) and the nearest's own number |
| [`NoiseKept`] | how much of a noise's difference from its mean is left over a width |

Each takes the width of the texel looking at it and gives its average over that width, near enough, so a coarse mip
level is the fine level's blur, not its flicker. An octave is its mean plus what [`NoiseKept`] keeps of the rest, so an
octave much finer than the texel is just its mean and isn't worked out. [`NoiseKept`] is fitted to gradient noise
averaged over squares (0.87 kept at half a unit, 0.58 at one, 0.15 at two): one less a smoothstep to two and a half
units. Turbulence, ridges and cells go to their measured means (0.2204, 0.6322; 0.5201 and 0.7167 for F1 and F2), not
to 0.

A fractal sum is over the sum of its octaves' strengths, so each kind keeps its range whatever the octaves, and each
octave's lattice is moved so their zeros don't all fall on the first's lattice points. Cellular noise is exact: the 27
cubes round a point aren't always enough, so if the second nearest is past the nearest face of the block searched, a
block a cube bigger is searched (1.7% of points). A texel within half its width of the plane between the two nearest
points' cells is partly the second cell, so a cell's number crosses an edge in a texel's width.

## Generators and operators

Each is a [`CSolidTexture`]; those with inputs are CSolidOperators ([`ProceduralLib/SolidOperator.h`]), whose inputs
are `Ptr<CSolidTexture>` fields. A texture can feed several operators (the graph isn't a tree); it can't feed itself,
however far up. An input that isn't linked gives opaque black, so every operator gives something with an input
missing.

| Header | Class | Gives |
|---|---|---|
| [`ProceduralLib/SolidFlat.h`] | CSolidFlat | one sample everywhere |
| [`ProceduralLib/SolidGradient.h`] | CSolidGradient | a value rising from a plane, round an axis (rings) or out from a point; once, as a saw or as a wave; its exact mean over the texel |
| [`ProceduralLib/SolidNoise.h`] | CSolidNoise | a fractal sum as a value, 0 to 1 |
| [`ProceduralLib/SolidCells.h`] | CSolidCells | F1, F2 - F1 (0 along cells' edges) or the cell's own number |
| [`ProceduralLib/SolidBricks.h`] | CSolidBricks | a lattice of boxes with mortar, rows offset: a brick's own number, or how far inside its brick the point is |
| [`ProceduralLib/SolidRamp.h`] | CSolidRamp | its input's value through a table of keys, each a whole sample, a straight blend between; two at one value a hard edge |
| [`ProceduralLib/SolidAdjust.h`] | CSolidAdjust | invert, hue (turned about the line of greys, no conversion to hue and back), saturation, contrast, brightness |
| [`ProceduralLib/SolidMerge.h`] | CSolidMerge | two inputs added, multiplied, the lesser, the greater or the difference, every channel alike |
| [`ProceduralLib/SolidMask.h`] | CSolidMask | two inputs blended by a third's value; where it's all one input the other isn't sampled |
| [`ProceduralLib/SolidWarp.h`] | CSolidWarp | its input at the point moved by a second input's value (read at the point and at two far points, one per axis) |
| [`ProceduralLib/SolidSlope.h`] | CSolidSlope | how nearly the normal faces a direction: moss on top, dust in the lee |
| [`ProceduralLib/SolidCavity.h`] | CSolidCavity | from a [`CShape`]'s distances a reach out and in along the normal: towards 0 in hollows, 1 on edges, a half on a flat |
| [`ProceduralLib/SolidChannel.h`] | CSolidChannel | one channel of its input as a grey (Blender's Separate) |
| [`ProceduralLib/SolidCombine.h`] | CSolidCombine | a sample from a colour input and four inputs' brightnesses for height, shine, gloss and glow (Blender's Combine) |

[`CSolidChecker`], box filtered cubes of two colours, is in SupportLib. CSolidCavity is something a solid texture can
do and a 2D one never could, as the shape is to hand; the shape is in the space the texture is placed in, so the
texture takes its point back out of its own space ([`CSolidTexture::FromOwn`]). On a mesh of flat faces its value on a
curve varies a little across each face, which shows only when the raw value is baked with relief.

## Materials

A material is a function that builds a graph and returns its top texture, not a class
([`ProceduralLib/SolidMaterials.h`]): MakeSolidMarble, MakeSolidWood, MakeSolidStone, MakeSolidRust, MakeSolidBricks,
MakeSolidTiles, and MakeSolidWeathered(material, shape, seed), which pales the material along the shape's edges, puts
earth in its hollows and moss in patches on what faces up. Each is sized for things a unit or so across and differs by
its seed. Their numbers are first guesses, to be tuned in HCP65-Sculpt.

[`ProceduralLib/SolidMaterialFile.h`] keeps a material as a configuration: WriteSolidMaterial writes the graph under a
named CSolidMaterial object ("Material") to a chunk file of objects (DRG), and ReadSolidMaterial reads it back as
textures that sample as they did, NULL if the file isn't a material's. The functions in SolidMaterials stay as the way
to make one in code.

## Filtering, and the bake's samples

Noise is filtered by its width, but a ramp of a filtered noise isn't the filtered ramp: a hard key, a mask's edge, min
and max put back detail finer than the texel, and the mean moves (a marble's veins fade, so coarse levels are paler).
Measured by [`TestProceduralLib/TestSolidFiltering.cpp`] (`TEXTURE_FILTER_REPORT` set in the environment prints its
tables), as the root mean square, over 144 texels and red, green and blue, of a bake's distance from the mean of 64
samples across each texel, at widths from a 64th of a unit to a half (2026-11-16):

| Texture | One sample a texel, finest | coarsest | The bake's samples, finest | coarsest |
|---|---|---|---|---|
| Noise (sum) | 0.001 | 0.020 | | |
| Marble | 0.003 | 0.091 | 0.003 | 0.014 |
| Wood | 0.025 | 0.010 | 0.025 | 0.010 |
| Stone | 0.017 | 0.097 | 0.017 | 0.022 |
| Rust | 0.005 | 0.100 | 0.005 | 0.011 |

The generators are near the truth. With one sample the materials were a tenth of the whole range out at coarse
levels, mostly from the mean moving; wood is worst in between (0.033 at a 16th, 0.006 with the bake's samples).
Level 0's own errors are detail finer than a texel and hard edges, as any texture's finest level has.

So the bake takes several samples in a texel ([`SMeshBakeSettings`]'s iSamples each way, each as wide as its share):
[`CMeshBake`]'s level 0 is one sample, and each level after has twice as many each way up to [`CMeshBake::miSamples`]
(4), for about two and a half times the samples. [`CMeshBake::SetJobs`] gives it threads to bake on. Soft ramp keys (a
key's step no sharper than the input's change across the texel) were measured and not built: they mend only a steep
key on an evenly changing input (stone's cracks), need two more samples of the input for every ramp, and the bake's
samples mend every case for less.

A sample of each material at a 64th of a unit cost, in microseconds, Release x64 (2026-11-16): a six octave noise
0.16, cells 0.12, marble 0.46, wood 0.42, stone 0.72, rust 0.30. A page of 128 by 128 texels with its share of coarser
levels is 5 to 30 milliseconds on one thread, so pages can be baked when first wanted if a frame needn't wait.

## Height

The bake turns height into a normal (Blinn's bump mapping): the height's slope on a face is the best fit through the
heights of the texel's own samples, so no image neighbours are used and seams don't show, and the normal less the
slope, made a unit, is the tilted normal. With [`SMeshBakeSettings`]'s fRelief the colour is lit by it from a fixed
light (sLight) as it is baked: times 1 + fRelief times how much more of the light the tilted normal faces than the
smooth one, so a flat height changes nothing; a glowing texel is changed that much less. [`CMeshBake::mfRelief`] is 1
and its light above, left and in front. [`CMeshBake::mbNormals`] also bakes the tilted normals, in the mesh's own space
(every texel is unique, so no tangent frames), to the material's bump image; it's off by default, as neither renderer
reads it.

## What a solid can't do

Blur, sharpen, grow and shrink by a chosen amount (they need a texel's neighbours: roundness is made in the
generator), text and loaded pictures, and patterns that follow the surface. Bricks and tiles are a lattice in space,
right only on faces square to it: a slanted or curved surface cuts them into slivers, and a face lying in one of the
lattice's planes is all mortar, so the gallery places them along its box ([`TestProceduralLib/TextureGallery.cpp`]).

## Tests

TestBaseLib's [`TestBaseLib/TestNoise.cpp`] (2088 checks when built), and in TestProceduralLib
[`TestProceduralLib/TestSolidGenerators.cpp`], [`TestProceduralLib/TestSolidOperators.cpp`] (a graph saved, loaded and
sampling the same; channel and combine; every operator unlinked), [`TestProceduralLib/TestSolidMaterials.cpp`]
(placement inside a parent, warp, material files), [`TestProceduralLib/TestSolidSurface.cpp`] and
[`TestProceduralLib/TestSolidFiltering.cpp`]. The look is judged by eye: [`TestProceduralLib/TextureGallery.h`] bakes a
texture onto a sphere and a box and draws them to `Output/Gallery/<name>.png`. The gallery is skipped in Size16
builds (RenderLib).

[`ProceduralLib/TexturePlan.txt`] is the plan this was built from, steps 1 to 39. The code's comments (ProceduralLib's,
[`SupportLib/SolidTexture.h`], [`SupportLib/MeshBake.h`], [`BaseLib/Noise.h`]), SupportLib's unwrap and virtual texture
plans and HCP65-Sculpt's plan cite its sections and steps, so it is kept as that record.

## Decisions

- Texture generators before the virtual texture (the user, 2026-11-16).
- Solid textures only, every texture a function of a 3D point. A family of 2D operators, as .werkkzeug's, waits until a
  scene needs what a solid can't do (the user, 2026-11-16).
- A sample holds colour, height, shine, gloss and glow; gloss and glow were the user's picks from the recommendations
  (2026-11-16).
- Noise in BaseLib; [`CSolidTexture`], the sample and the bake in SupportLib; generators, operators and materials in
  ProceduralLib.
- A material is a function building a graph of ordinary objects, not a class; as a configuration it is the graph in a
  file, with no C++ written out.
- The one number an operator reads is the colour's brightness.
- Several samples a texel in the bake, not soft ramp keys (measured, 2026-11-16).
- A texture read by several operators is worked out each time, not cached per point: a sample costs a few tenths of
  a microsecond.

## Not done

- Whether colours between operators are in the display's space or linear: as the renderers take them now, to be
  looked at again with baked lighting.
- Shine, gloss and glow are carried and not baked: nothing reads them (glow only holds off the relief's shading).
  Neither renderer reads the normal image.
- Relief makes level 0 take four samples a texel where it took one, about five times the samples; a texture with no
  height gains nothing (set [`CMeshBake::mfRelief`] to 0). A graph can't tell the bake from its top what it needs
  (whether it has height, whether it averages itself), so the bake's samples are the same for every texture.
- The virtual texture, whose pages are baked from these ([`SupportLib/VirtualTexturePlan.txt`]), and loaded textures on
  a second UV layer ([`SupportLib/UnwrapPlan.txt`], step 10).
- Bricks on slanted surfaces and text, which would bring in 2D operators projected by the normal: not planned.
