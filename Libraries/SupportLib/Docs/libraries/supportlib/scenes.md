# Scenes

A scene is what is in a world, in floats: instances of meshes, the connections that place them, skeletons and
animations, lights, cameras and particles.  SupportLib holds the scene and reads and writes scene files.  A renderer
draws what is converted from it, one way: from the description to what is drawn.  Two renderers do: the fixed point
raster renderer (RasterLib, through RasterRenderLib) and a Vulkan one (VulkanLib), both behind RenderLib.  This page
is the scene model, the file format, how a scene reaches a renderer, and the rulings.

The plan, [`SupportLib/ScenePlan.txt`], is not finished: in-place updates (its step 10), a 3ds Max exporter (format
step F) and its Open list remain, and Vulkan in a window is planned in [`WindowLib/Vulkan.txt`].  The plan also holds
what converting and rendering the old scene files found, file by file.  Where it and this page differ, this page
follows the code (see Not done for what the plan still says that the code has moved past).

## The objects

| Class | What it is |
|---|---|
| [`CScene`] | Instances, lights, cameras, animations and particles.  Everything else is reached through what uses it. |
| [`CInstance`] | One thing in the game, one character say: its connections, its meshes and, shared, its skeleton. |
| [`CInstanceMesh`] | A mesh in an instance: a shared [`CMeshEditor`], the instance's connections the mesh's matrices use, and materials in place of the mesh's defaults. |
| [`CConnection`] | Something spatial that meshes, lights, cameras or game information hang from: a local and a world matrix, a parent, and the bone it plays, if any. |
| [`CSkeleton`], [`CBone`] | Bones shared by every instance of a character, parents first. |
| [`CSceneAnimation`], [`CConnectionAnimation`] | Tracks of keys ([`SConnectionKey`]: a time and a local matrix) that play together. |
| [`CAnimationPlayer`] | Plays an animation: its time, speed and whether it loops.  The app's, not the file's. |
| [`CLight`], [`CCamera`] | Not instanced: each has its own connection. |
| [`CSceneParticles`] | Points, as a field of stars is. |

Two characters are two [`CInstance`]s that share their meshes and their skeleton and are drawn in different places.
Shared data can't point at a connection, as each instance has its own, so what an instance uses of shared data is in
an object of the instance's: a [`CInstanceMesh`] holds the shared editor and the instance's connections that its
matrices use, one for a rigid mesh and one per matrix for a skinned one.  A skinned mesh's weights number its own
matrices, which is why the connections are per mesh.  Lights and cameras are too small to be worth sharing: two
lanterns are two [`CLight`]s, and a lantern carried by a character has a connection whose parent is the hand's.

Names are [`CChars`], not object names, as many things in a file share one.  A scene reaches meshes, materials,
images and connections only through what uses them, so one nothing uses isn't kept from a file.

### Connections

Matrices are row major with vectors on their left.  [`CConnection::UpdateWorldMatrix`] makes the world matrix the
local one times the parent's world matrix, parents first, and sets the parity (whether it mirrors);
[`CScene::UpdateConnections`] does all of a scene's, each shared parent once.

### Skeletons

A [`CSkeleton`] is shared and holds its [`CBone`]s parents first; a bone is added only at the end, so none moves.  A
[`CBone`] doesn't change once built: its name, its parent bone, its rest local matrix and its index.  Bones are known
by what they are, not where they are: a [`CConnection`] has the [`CBone`] it plays (none for what isn't a bone), and a
skin holds the [`CBone`] each of its matrices is, so a bone added later breaks no clip or skin.

[`CInstance::MakeBones`] gives an instance a connection for each bone, in the skeleton's order, starting at the
bone's rest matrix, its root bones hanging from a connection of the instance's own.  [`CInstanceMesh::BindBones`]
finds the connections a skinned mesh's bones are played by; a bone the instance hasn't got is logged once and the
mesh isn't drawn.  The bound connections are a cache: a file doesn't hold them.

### Animation

A key is a connection's local matrix, relative to its parent's, so a parent carries what is animated: Goober walks
on the spot and its instance's connection moves it.  Between two keys each is split into scale, rotation and position
([`Float4x4Decompose`]), blended (the rotation by slerp) and put back; what the split loses, a slight shear, is
blended straight, so each key is itself at its time and nothing jumps.

A [`CSceneAnimation`] is one of two kinds, never mixed:

- a skeleton's, each track moving a [`CBone`]: it moves nothing until a [`CAnimationPlayer`] is bound to an
  instance with that skeleton, so any number of instances share one walk;
- the scene's own, each track moving its [`CConnection`]: ships flying about, planets, a camera's dolly.

A fixed connection in a shared clip would move the first instance's whatever instance played it, which is why they
aren't mixed.  [`CAnimationPlayer::Apply`] sets local matrices, and [`CScene::UpdateConnections`] then the world ones.

### Mesh editors

A [`CMeshEditor`] holds a mesh and its modifier stack ([`CMeshModifierStack`]).  [`CMeshEditor::GetOutput`] is the
mesh with its modifiers applied (the mesh itself with none), and three counters
([`CMeshEditor::NumStructureChanges`], [`CMeshEditor::NumShapeChanges`], [`CMeshEditor::NumModifierChanges`]) say
what changed.  [`CMeshEditor::ClearOutput`] lets the output go once it is cooked, and [`CMeshEditor::Collapse`] makes
the output the editor's mesh (the same mesh object) and removes the modifiers.  A shape is a modifier, collapsed to
edit its faces.

Modifiers are [`CObject`]s, made by [`CMeshModifierStack::AddModifier`] and freed with their stack, so one can hold a
[`Ptr`] (an isosurface holds its shape: [Shapes](shapes.md)).  Polygons and smoothing groups are opt-in details of
[`CMesh`], so big meshes don't build connectivity for every face; an edge is hidden when the faces on both sides are
in one polygon ([`CMeshPolygons::GetFaceEdges`]).  Edge visibility and selections stay on the editor: they are what
the user sees and does.

Modifiers that build a mesh of other faces (weld, corner merge, edge collapse, merge normals) keep only positions,
normals and colours: [`CMeshModifier::CanRebuild`] refuses a mesh with UVs, skin or materials, and the modifier then
returns it as it was.  Modifiers that only change normals, and shapes, keep everything.

Pose (skinning) is per instance and never changes the shared [`CMesh`].  [`CInstanceMesh::Pose`] poses the editor's
output in floats, positions only, for what needs a posed mesh in main memory (physics), with or without a renderer;
[`CInstanceMesh::GetPalette`] gives the matrices a renderer poses with.

## The scene file

The goal behind the plan is a file that meshes and whole scenes can be saved to, simpler than the object graph, so
that an exporter can write it with no object system.  The old exporter filled engine objects and called their Save,
so its format was whatever those classes were at the time.

[`CSceneFileWriter`] and [`CSceneFileReader`] are calls, not a model in memory (begin a mesh, write its positions,
its faces, end it), over [`CChunkFile`], and use only BaseLib.  [`SupportLib/SceneFileGeneral.h`] describes the
format; [`CScene::ReadFile`] and [`CScene::WriteFile`] fill and write a scene, and [`SupportLib/SceneFileMesh.h`] a
mesh in an editor.  The extension is .DRG; the chunk file's user ID says it is a scene file and a required chunk holds
its version (2 since skeletons; a reader refuses a version it doesn't know, including 1).

The rules that keep it from going the way of the old files:

- Each attribute is a chunk of its own (Positions, Normals, UVs).  A new attribute is a new chunk; readers ask for
  chunks by name, so an old file gets a default and an old reader ignores a new chunk.  The version changes only
  when that isn't enough.
- No struct is written whole: only fixed size types, no sizes, no pointers.
- References are indices into lists in the same file, checked by the writer when the file closes.  The writer also
  checks that every bone referred to is in its skeleton.
- Every result is checked.  A failure sticks, and Close then fails and leaves no file.  (Melee.DRG's faces were lost
  because a Save's result was ignored.)
- [`CChunkFile`]'s own layout must stay put: TestSupportLib's Input/Scene/Sample.DRG uses every chunk and fails its
  test if the bytes change.

Faces are polygons, a triangle being a polygon of 3.  Modifier stacks are not in the format: it holds final meshes,
as a Max scene arrives with its modifiers applied.  The conventions, settled by rendering the old files: a scene is
right handed with y up, a polygon goes anticlockwise, a v of 0 is an image's top row, cameras look along their
connection's -z with y up, lights shine along -z, and a field of view is horizontal.  Going through a [`CScene`] and
back, a file stays the same byte for byte.

OBJ is kept for plain meshes ([`SaveOBJ`] and the OBJ reader: positions, normals and one UV layer).  It carries no
smoothing groups, materials or object names, reads no holes, and rounds positions to 1/65536 (about 0.015 mm in
metres), so it isn't lossless for KiCad's metre exports.  None of the open formats fits as the main one: OBJ has no
skin, animation or hierarchy, glTF no polygons, smoothing groups or Blinn materials, FBX is closed, and COLLADA and
USD are large.

The old files (Into Oblivion's, Star Control's Melee.DRG and Test.DRG, and Goober.DRG) were converted once by
[`OldDragonLib/OldDragonConverter.h`], run by [`ConvertOldDragon/ConvertAll.py`]; OldDragonLib stays in the
repository, out of the regular builds, in case a conversion mistake turns up.  Quirks are fixed in the converter and
never reach a new file.  [`RenderSceneFile/RenderSceneFile.cpp`] draws scene files with the raster renderer.

## From a scene to what is drawn

Nothing in the scene knows a renderer exists, and a renderer only sees what it is given.

| Library | Holds |
|---|---|
| SupportLib | The scene.  Knows nothing of renderers. |
| RenderLib | Cooking ([`CCookedMesh`]), the abstract [`CRenderFactory`], [`CRenderer`] and [`CRenderMesh`], and [`CRenderScene`]. |
| RasterRenderLib | The raster renderer's factory: converts floats and scene types into what RasterLib wants. |
| VulkanLib | The Vulkan renderer's factory. |
| RasterLib | A standalone fixed point renderer: takes integers and draws, with no scene at all.  It includes only BaseLib and StandardLib. |

Transforms with parents, lights as objects, keyframes and time, and a list of what is in the world stay out of
RasterLib.  Drawing features in fixed point go in RasterLib; whatever touches floats or scene types goes in
RasterRenderLib, which is a library of its own so that RasterLib depends on neither RenderLib nor the scene.  A
[`CScene`] never creates a [`CRasterMesh`].

A mesh goes through four stages:

| Stage | When | What |
|---|---|---|
| Run the stack | When it changed | [`CMeshEditor::GetOutput`]. |
| Cook | When it changed | [`CCookedMesh::Cook`]: unique vertices, submeshes by material slot and transparency, skin weights as floats (normalised, most first), bounds.  Plain, for any renderer. |
| Create | Per device | The factory's [`CRenderFactory::CreateMesh`]: a raster or Vulkan render mesh from the cooked mesh. |
| Extract and draw | Every frame | A frame turns connections, camera and lights into what its renderer draws. |

A cooked mesh keeps positions and normals as streams of their own, so a change of shape or pose need only touch them,
and a table from its vertices to the mesh's corners for updating it in place.

Each frame [`CRenderScene::Update`] walks the scene and looks at the editors' counters; nothing has to notify it.  It
makes a render mesh for each editor the instances use (one for an editor many share), a new one when the editor's
structure, shape or modifier counter moves, and lets go of one no longer used.  A render mesh is never changed in
place.  Cooking and making a render mesh are done on the thread that draws, unless whoever made the output on
another thread gives it cooked ([`CRenderScene::SetCooked`]) or as a render mesh ([`CRenderScene::SetMesh`]); with
[`CMeshEditor::SetOutput`] a stack takes an output made elsewhere.  Sculpt's baker and the emulator's board model
loaders do this.

Lifetimes: a render mesh holds a Nes pointer to its [`CRenderer`], so every mesh is freed before the renderer and the
pointer is still valid in the mesh's Free (only a cycle made entirely of Nes pointers can't be ordered; see
[`TestStandardLib/TestStandardLib/TestObjectFreeOrdering.cpp`]).  The factory isn't an object in the graph: the app
makes it, as it does a [`CNativeWindowFactory`], and it outlives everything it makes.  A mesh's Free shouldn't read a
collection on the renderer: after an explicit Kill only pointer fields are put back for it.  Textures are held by the
meshes drawn with them and found by their renderer weakly, so a texture is freed with the last mesh that uses it
([Weak references](../standardlib/weak-references.md)).

Each renderer subclasses [`CRenderMesh`] directly.  A [`CCanvas`] has a native object beneath it because it has a lot
of logic that doesn't depend on the host; a render mesh has little, and a native object couldn't hold a [`Ptr`].

### The raster renderer

[`CRasterRenderer`] draws everything at one scale, integer units to each of the scene's, so meshes are drawn together
at their true sizes.  [`BuildRasterMesh`] makes a [`CRasterMesh`] from a cooked mesh and leaves out triangles its
rounding leaves with no area.  [`CRasterMesh`] is a [`CObject`], so anything can share one with or without a scene;
[`SRasterInstance`] stays a plain struct with a raw pointer, as per-frame data the worker threads use, and the frame
holds the [`Ptr`]s that keep the meshes alive during the draw.

[`CRasterFrame`] extracts a frame:

- each rigid mesh of each instance becomes an [`SRasterInstance`] whose axes are its connection's world matrix,
  scale and mirror included; its light axes are the rotation alone, and a mirrored instance's triangles are wound the
  other way;
- the camera is the scene camera: when the viewer's controls move [`CRasterCamera`] the next extract writes the view
  back into the camera's connection, and when the scene camera moves the view follows it;
- the lights are the scene's ([`CRasterLights`]): ambient lights add up ([`LT_Ambient`]), directional lights add up,
  and omni and spot lights are point lights (a spot's cone isn't used yet) that reach as far as they would add a
  shade, worked out only for instances and vertices within that reach;
- a skinned mesh is posed in a pass of its own ([`CRasterPosed`], integers only, from [`SRasterBone`]s each the
  skin's inverse matrix times the world matrix of the instance mesh's connection), into vertices the frame keeps from
  frame to frame.  A vertex has any number of weights, adding up to exactly `RASTER_SKIN_ONE` (15 bits), what
  rounding leaves over going to the largest.  A normal is turned by each bone's rotation, blended and made a unit by
  an integer square root.  A bone whose matrix mirrors is an error: the mesh isn't drawn, and it is logged the first
  frame only.

Textured triangles have a filler of their own, perspective correct at each pixel from the inverse depth it already
has ([`CRasterTexture`]).  Every mip level is kept ([`CTextureLevels`]: the image's own levels where it has them,
halved where it hasn't), a triangle picks a level from its texels over its pixels and each pixel moves a level for
each doubling of its distance, and [`CRasteriser::SetFilter`] chooses nearest ([`RF_Nearest`], the default),
bilinear or trilinear.

### The Vulkan renderer

[`VulkanLib/VulkanRenderer.h`] draws offscreen and reads the frame back as pixels, so it works in any window or
dialog and in tests; a swapchain on a native window is [`WindowLib/Vulkan.txt`]'s.  [`VulkanLib/VulkanFunctions.h`]
finds every Vulkan function at run time in vulkan-1.dll or libvulkan.so.1 (through [`CSharedLibrary`], with
VK_NO_PROTOTYPES), so nothing links against Vulkan and a program without it goes on with the rasteriser.  It needs
Vulkan 1.3 for dynamic rendering and a dynamic front face; a Debug build uses the validation layer if installed and
logs what it reports.

- A frame is drawn and read back synchronously: B8G8R8A8 colour, so a pixel is 0xAARRGGBB as a [`CCanvas`] has it,
  and D32 depth.  Nothing is in flight between frames, so a mesh's buffers are destroyed as soon as it is freed.
  Buffers are in memory the CPU sees; the frame is read back through cached memory, as copying 1080 by 400 pixels
  from memory meant only for writing took 8.4 ms against 0.4 ms (2026-11-17).
- A front face is counter-clockwise unless mirrored: Vulkan's y points down the target, so the rasteriser's
  clockwise is wrong there.
- Skinning is in the vertex shader ([`VulkanLib/Shaders/Mesh.vert`]): each vertex's first weight and count, its
  weights and the palette are in storage buffers, so a vertex keeps any number of weights.  Each skinned mesh has a
  descriptor set of its skin, and the frame's palettes are one buffer seen at a dynamic offset.
- It lights as the rasteriser does (ambient, directional, point lights with a reach), so the two can be compared
  within a tolerance, and draws a material's diffuse texture with every mip level, sampled trilinearly.  Other
  textures wait.  Transparent triangles go after opaque ones, unsorted.

TestVulkanLib proves it against the rasteriser's frames: lit and unlit boxes within a mean of 1.2 to 2.3 of 255, a
texture's texels, skinned boxes, and Goober's walk covering the rasteriser's pixels to about 1 in 500 (2026-11-14).

### Viewing a scene

SceneViewLib shows a scene in a window: [`SceneViewLib/SceneCanvasDraw.h`] draws a scene file with Vulkan or the
rasteriser (R switches between them; with no Vulkan 1.3 device it is the rasteriser), loops its animations, and has a
turntable view of its own ([`SceneViewLib/Turntable.h`]) that starts at the scene's first camera;
[`SceneViewLib/SceneView.h`] is the canvas with the view's mouse and keys.  HCP65-Sculpt uses it.

## Test scenes

- The emulator's board scene ([`CBoardSceneDraw`]) proves the rigid path and the parent chain: a mainboard instance
  with a connection for each socket, and each board an instance whose connection is a child of its socket's, all in
  millimetres; each model is an editor that every socket holding that board shares.
- Goober (TestSupportLib's Input/Scene/Goober.DRG), a skinned mesh walking a field of stars, proves the rest: 3241
  positions, 6134 faces, 67 bones in a skeleton 12 deep, three images and 1105 stars.  Played at its start every bone
  is where its file has it, through the walk no bone changes length, and a second Goober made from the first's
  skeleton, sharing its mesh and walk at another time, stays its own
  ([`TestSupportLib/TestSupportLib/TestSceneFileGoober.cpp`]).

## Decisions

1. The scene is in floats and knows nothing of renderers; RasterLib stays standalone (BaseLib and StandardLib
   only), and the float-to-integer conversion is RasterRenderLib's (2026-11-10).
2. The main file format is a chunk file of its own, written through calls rather than objects, with the rules above;
   OBJ is kept for plain meshes and glTF import may come later (2026-11-09).
3. Polygons and smoothing groups are opt-in details of [`CMesh`]; edge visibility and selections stay on the editor
   (2026-11-09).
4. An instance's axes may scale and mirror; its light axes are their rotation; the shade is clamped at 256; an uneven
   scale shades as if the normals were only turned (the user, 2026-11-10).
5. The camera is the scene camera and the viewer's controls write back to it; the field of view across the target's
   width makes the focal length (the user, 2026-11-10).
6. Ambient is a light, and ambient lights add up; point lights reach only as far as they would add a shade (the user,
   2026-11-10).
7. The board scene is in millimetres, one size for every mesh, and its pixels needn't stay byte-identical (the user,
   2026-11-10).
8. Keys are local matrices, blended by decomposing, the rotation by slerp; tracks hold [`Ptr`]s (the user,
   2026-11-10).
9. Skeletons are shared, bones known by identity not place, animations of two kinds never mixed, and the file holds
   a Skeleton chunk with the version raised to 2; a walk on another skeleton needs a bone map, which waits until
   something needs it (the user, 2026-11-14).
10. Raster skinning is a pass of its own, with any number of weights a vertex, not at most four; a missing bone stops
    the mesh drawing and is logged once; a mirroring bone is an error logged once.  A CLogger "error once" was
    offered and not added (the user, 2026-11-14).
11. The raster renderer needn't draw the same bytes once built from cooked data (the user, 2026-11-14).
12. Vulkan: the LunarG SDK for headers, glslc and validation; the library loaded at run time so a built app needs only
    the driver; offscreen and read back for now; skinning in the vertex shader with any number of weights; physics
    poses in floats in SupportLib, not from the renderer; it lights as the rasteriser does and draws the diffuse
    texture; proved by TestVulkanLib against the rasteriser (the user, 2026-11-14).
13. Textured triangles in the rasteriser have their own filler (the user, 2026-11-14).
14. Mipmaps in both renderers: the rasteriser picks a level per triangle moved by each pixel's distance, with
    nearest, bilinear and trilinear filters as an option (the user, 2026-11-16).
15. Mesh modifiers are [`CObject`]s (2026-11-15).

## Not done

- In-place updates of a mesh with no modifiers (plan step 10), if dragging plain meshes is slow.  Any modifier makes
  it impossible, as there may be no old vertex to overwrite; an editor may run a heavy stack when a drag ends.
- A 3ds Max exporter writing through [`CSceneFileWriter`] (format step F), if one is wanted.
- Vulkan in a window, with a swapchain: [`WindowLib/Vulkan.txt`].  Faster memory and frames in flight wait for it;
  with frames in flight a freed mesh's buffers would have to wait for its frame's fence.
- Particles have no cook or draw on either renderer: they are kept on the scene so nothing in a file is lost.
- Creating and removing an attached light: whatever creates a character must create its lantern too, and something
  must remove it with the character, or it stays lit where the hand was.
- Coordinates: a scene file is right handed with y up but the KiCad boards look z up, so [`CMesh`] has no one
  convention.  Set one, and importers convert to it.
- Threads: cooking on [`CJobs`] with draws skipping what isn't ready.  Giving a cooked mesh or render mesh made
  elsewhere is built ([`CRenderScene::SetCooked`]); that reading a [`CMesh`] off the main thread is safe in general
  isn't established.
- Modifiers that rebuild faces lose UVs, skin and materials, so they refuse such meshes rather than keep them.
- [`CMesh::MovePosition`], [`CMesh::MovePositions`], [`CMesh::RemoveFace`], [`CMesh::RemoveFaces`] and
  [`CMesh::RemoveCorner`] are empty, and whether a move is to a position or by an offset isn't decided.
- [`CMeshModifierStack`]'s Load and Save are not implemented, so editable stacks can't be saved through the object
  graph yet.
- [`CMeshEditor::Scale`] doesn't scale a shape modifier's own positions.
- A spot light's cone isn't used by either renderer.
- Both renderers draw only the diffuse texture: a bake's normals (the bump image, [Unwrapping](unwrapping.md)) are
  read by neither.
- Melee's one channel images aren't the alpha of their PNGs, and what they should do is open.  Arilou Headlights is
  wound the wrong way and left so.
