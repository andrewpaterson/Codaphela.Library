# Weak References

A weak reference points at an object without keeping it.  The object is freed when nothing strong reaches it, as any
object is ([Objects](objects.md)), and every weak reference to it becomes NULL.  Before them there were only
[`Ptr`]s, which keep their objects, and raw pointers (`U_Pointer`), which don't but dangle.  Wherever something had
to find objects without keeping them it was built by hand: raw lists on both sides, each side's `Free` unhooking the
other, and care about the order a batch was freed in.  Weak references are the one general version.

The plan they were built from, [`StandardLib/WeakReferencesPlan.txt`], is finished.  It is kept as the record that
the code's comments cite by name ([`StandardLib/WeakPointer.h`], [`StandardLib/WeakFroms.h`],
[`SupportLib/ImageBlitter.h`] and others).

## Why

The renderers' texture caches held every texture they made, so a texture outlived the meshes that drew with it: a
mesh baked again and again leaked a texture each time.  The caches have to find textures by their image without
keeping them.  SupportLib's blitter cache did this by hand: the cache and the cel each kept a raw list of the other,
and each `Free` unhooked the other side.

Asked whether this was the same problem as canvases outliving their window factory, and whether a general solution
was missing, the answer (2026-11-16) was that there are two problems:

- An object that owns an external resource needs its maker alive until it has given the resource back.  The fix is
  makers as objects held by Nes [`Ptr`] ([Makers as objects](#makers-as-objects)).
- Shared objects found by a key without being kept need weak references.

## Weak froms

What a weak reference registers on its object is a weak from: its holder (a [`CWeakHolder`]) and a slot in it.  A
weak pointer has one slot, 0; a weak array has one per entry.  Weak froms are kept apart from the heap and stack froms,
in a [`CWeakFroms`] in the [`CObjects`] ([`CObjects::mcWeakFroms`]), a map from each weakly held object to its weak
froms.  No distance walk or from scan reads them, so a weak reference can't keep anything, by construction rather
than by care in each walk.

An object that may have weak froms has `OBJECT_FLAGS_WEAKLY_HELD` set on its container, so an object never weakly
held costs one bit and the free path one flag test.  The flag isn't cleared when the last weak reference lets go: a
stale flag costs one look in the map.  An embedded object's weak froms are keyed by the embedded object itself, not by
its container.

A holder implements three calls: [`CWeakHolder::ClearWeak`] (its object is being freed: NULL the slot and do nothing
else), [`CWeakHolder::RemapWeak`] (its object has been replaced: point at the new one) and [`CWeakHolder::GetWeak`]
(for validation).

## Weak pointers

[`Weak`] (a [`CWeakPointer`]) is a field of a [`CObject`], declared in `Class()` by `W_Pointer(mwX)`
([`CBaseObject::WeakPointer`], making a [`CWeakPointerField`]), so the object system knows where it is.  There are no
weak locals: a weak reference on the stack has no use.

- It is used through [`Weak::Get`], a [`Ptr`] that holds the object for as long as it is used, NULL once the object
  has been freed.  It has no `operator ->`, so a weak reference is never used without something strong holding its
  object for the length of the use.
- Assigning it (a [`Ptr`], an object, NULL, another [`Weak`]) takes it off its old object's weak froms and puts it on
  the new one's ([`CWeakPointer::PointTo`]).  Pointed at an object that is being freed, it is NULL.
- When its holder is freed, [`CObject::ClearPointerTos`] takes it off its object's weak froms
  ([`CWeakPointer::Release`]).
- It isn't copied: a weak pointer is a field.

## Weak arrays

[`CArrayWeakObject`] is a [`CObject`] and a [`CWeakHolder`] whose entries are weak, embedded in its holder by
`M_Embedded`.

- [`CArrayWeakObject::Add`] returns the entry's index; NULL, or an object being freed, isn't added.
- An entry whose object is freed becomes NULL.  A free never takes an entry out, so nothing iterating the array, or
  freeing, sees it change size.
- [`CArrayWeakObject::Remove`] and [`CArrayWeakObject::Compact`] take entries out, moving the last into the gap, so
  the order isn't kept.
- [`CArrayWeakObject::Get`] is a [`Ptr`], NULL where the object has been freed.
- When the array is freed its entries come off their objects' weak froms.

There is no typed `CArrayWeak<M>`: [`CArrayWeakObject::Get`]'s `Ptr<>` assigns to a `Ptr<M>` as it is.

## Clearing

When an object is freed, every weak reference to it, or to an object embedded in it, is set to NULL before any `Free`
of the objects going with it runs, so no `Free` can reach a dying object through a weak reference:

| Where | When |
|---|---|
| [`CObjects::RemoveBatch`] | after [`CObjects::DoneKill`], before [`CObjects::FreeInternal`] |
| [`CObjects::FreeAllInKillOrder`] (shutdown, eviction) | before any `Free` |
| [`CBaseObject::KillInternal`], an object not in [`CObjects`] | first |

A weak reference made during a batch to an object in it is NULL: the random distance test found one made to an
object freed that step, and a `Free` pointing a weak reference at another dying object would do the same.

## Hollows

Weak references follow an object's identity, as [`Ptr`]s do.  An object replaced by a hollow while strong holders
keep it isn't freed: its weak froms move to the hollow, and from the hollow to the loaded object when it is loaded
again ([`CObjectRemapFrom::RemapEmbedded`] calls [`CObjects::MoveWeakFroms`], which repoints each holder with
[`CWeakHolder::RemapWeak`]).  [`Weak::Get`] on a hollow loads it, as a [`Ptr`] would.  The references are cleared only
when the object is freed.  Hollows and loading are the [Codabase](../baselib/codabase.md)'s.

## Saving, validation and printing

Weak references aren't saved.  A weak pointer loads as NULL, as a `U_Pointer` does, and a weak array loads empty; the
file format didn't change.

[`CWeakFroms::ValidateConsistency`], called from [`CObjects::ValidateObjectsConsistency`], checks that every holder
points back at its object, that the object's container has the flag, and that no freed object is weakly held.
[`CEmbeddedObject::PrintObject`] shows a count of weak froms; a dump lists them apart from the others
([`CEmbeddedObject::PrintWeakFroms`]).

Threads: [`Weak::Get`] takes the object system's lock, and an adopted graph's weak froms move with it ([Objects and
Threads](threads.md)).

## Uses

- The renderers find textures in a [`CArrayWeakObject`] ([`CRasterRenderer::maTextures`], and the Vulkan renderer's
  in [`VulkanLib/VulkanRenderer.h`]); the meshes drawn with them hold them by [`Ptr`]
  ([`CRasterMesh::AddTexture`]).  A Vulkan texture is an object holding its renderer by Nes [`Ptr`], and gives back
  its image and descriptor set in its `Free` ([`VulkanLib/VulkanTexture.h`]).
- A blitter holds its cel weakly ([`CImageBlitter::mwSourceCel`]).  The [`CImageCelBlitterCache`] finds a cel's
  blitter by the blitter's cel and drops blitters whose cel is NULL the next time it is used.  The cel's and the
  cache's raw lists of each other, and their `Free`s' unhooking, are gone.

## Makers as objects

Not weak references, but the other half of the same question.  A window's native window, a canvas's native canvas and
a clipboard are made by the [`CNativeWindowFactory`], and must be given back to it.  It used to be a plain class that
the application killed itself, which could happen before [`ObjectsKill`] freed the windows.  Now
[`CNativeWindowFactory`] is a [`CObject`] that [`CWindow`], [`CCanvas`] and [`CClipboard`] hold by Nes [`Ptr`], so the
kill order frees it after them.  Applications make it with [`OMalloc`] after [`ObjectsInit`] and add it to the root;
where they used to kill it they remove it from the root.  Tests hold it on the stack and let go of it.  Nothing calls
its `Kill`.  The sound factory is the same: [`CNativeSoundFactory`] is a [`CObject`] held by Nes [`Ptr`] from each
[`CSoundMixer`], and the plain base both used to share is gone.

A window that isn't in [`CObjects`] (one on the stack) and is Nes pointed to clears its pointer fields before its own
`Free`, so [`CWindow::GetFactory`] asks the native window for its factory rather than reading the [`Ptr`].

The tests are `TestRefWindowFactoryOutlivesWindows` (TestWindowLib's TestRefWindow.cpp, with a counting allocator; it
crashes without the Nes pointer) and `TestSoundMixerFactoryOutlivesMixers`.

## Decisions

The user took each proposal on 2026-11-16:

1. Weak froms are kept in a map in [`CObjects`] behind a flag (`OBJECT_FLAGS_WEAKLY_HELD`, 0x1000), not in a pointer
   in every object as [`CEmbeddedObject::mpsHub`] is: an object never weakly held pays one bit, not four bytes on the
   65 targets and eight on 64-bit hosts.
2. Eviction: weak references follow the object to its hollow and back, and are cleared only when it is freed.
   Clearing them on eviction ("while it's in memory") would suit caches but surprise anything else.
3. Weak references aren't saved and load as NULL: they are for lookups rebuilt in memory.  Saving the index and
   reconnecting when both are loaded would need the reader to resolve weak froms as it does heap froms.
4. A weak reference is used only through `Get`, a [`Ptr`], never `->`, so something strong always holds the object
   while it is used.
5. The names: [`Weak`], `W_Pointer`, [`CArrayWeakObject`], [`CWeakPointerField`].

Then the user said to do every item left as not done (2026-11-16): printing, the blitter cache, and the window
factory as an object, which the user chose over detaching what was left or only reporting it.  The sound factory
followed on the user's request.

## Not done

- Weak maps (weak keys or weak values), if a lookup by key turns out to need one; the texture caches don't (a linear
  find over a weak array).
- A typed `CArrayWeak<M>` (not needed, above).
