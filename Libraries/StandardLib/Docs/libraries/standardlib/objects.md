# Objects

StandardLib's objects are C++ objects whose memory the object system manages.  Every object knows what points at
it: its froms.  An object stays while something reachable from the root, or a pointer on the stack, reaches it, and
is freed as soon as nothing does.  There is no reference count and no collection pass: each time a pointer changes,
the object system works out what that change detached, and frees exactly that, at once, in an order that lets each
object's `Free` still use what it depends on.

This page is how that works: objects and pointers, the froms, the distance to the root, holding from the stack,
freeing and its order, where froms are kept so that big collections and hubs stay cheap, and the unknown
collections.  Saving objects to the database and loading them, hollows, flushing and the open decisions about them
are the [Codabase](../baselib/codabase.md)'s; weak references are [Weak References](weak-references.md); using
objects from several threads is [Objects and Threads](threads.md).

## Objects and pointers

An object is a subclass of [`CObject`] (through [`CBaseObject`] and [`CEmbeddedObject`]).  It implements `Init`,
`Class`, `Free`, `Save` and `Load` (the list is at the top of [`StandardLib/Object.h`]).  `Class` declares its
fields, so that the object system knows where its pointers are:

| Declared with | Field | The object system |
|---|---|---|
| `M_Pointer(mpX)` | a [`Ptr`] | records a heap from on what it points at, walks it, saves it |
| `Nes(M_Pointer(mpX))` | a [`Ptr`] the object needs in its `Free` | frees what it points at after this ([Freeing](#freeing)) |
| `M_Embedded(mcX)` | an object or collection inside this one | embeds it ([Embedded objects](#embedded-objects)) |
| `W_Pointer(mwX)` | a [`Weak`] | records a weak from that keeps nothing ([Weak References](weak-references.md)) |
| `U_Pointer(mpcX)` | a raw pointer | ignores it; it loads as NULL |

[`OMalloc`] allocates an object in the running thread's [`CObjects`] ([`gcObjects`] unless a worker has its own) and
calls its `Init` with the arguments given; [`ONMalloc`] gives it a name too.  Both return a [`Ptr`].  [`OMalloc`]
drops `Init`'s result, so a class whose setting up can fail has `Init(void)` and a separate `bool Setup(...)`, as
[`CIDEContent::Setup`] does.

A [`Ptr`] (a [`CPointer`]) is either a field of a [`CObject`] or, briefly, a local variable on the stack; nowhere
else (the user's ruling, written at the top of [`StandardLib/PointerObject.h`]).  So a class that holds a [`Ptr`], an
object or a collection is itself a [`CObject`], even when it is made on the stack.  Stack pointers are for using
objects, not keeping them.  A stack pointer is known to its object by its address, so one that has been moved (an
element of an array that grew) dangles; [`CEmbeddedObject::ValidateFroms`] reports a stack from that no longer points
at its object.

Pointing a [`Ptr`] somewhere adds the new from before it takes the old one off.  The other way round,
`p = p->mpNext` with `p` the only holder of a list freed the whole list and left `p` dangling (found in the review of
2026-11-03).  Collections do the same when an entry is replaced ([`CCollection::ReplaceObjectFrom`]): the new and the
old may be the same object, and when the argument [`Ptr`] lives inside the object being replaced, removing first is
a use after free.

`ValidatePtr` checks that a [`Ptr`]'s object is of its template class, with `dynamic_cast` (which the 65C64 and
W65C816 compilers support for it).

### Embedded objects

An object embedded in another (by `M_Embedded`) has no life of its own: it is allocated, freed, saved and loaded with
its container.  Pointers may point at it, and its froms are its own, but its distance to the root and all its flags
but `OBJECT_FLAGS_CALLED_CONSTRUCTOR` and `OBJECT_FLAGS_CALLED_CLASS` are its container's (`OBJECT_FLAGS_CONTAINER`
in [`StandardLib/BaseObject.h`]); its own [`CBaseObject::miDistToRoot`] is `EMBEDDED_DIST_TO_ROOT`, so reading a
container's distance stays one load.  A container keeps every object embedded in it, at any depth, in one flat array
([`CObject::mapEmbedded`]), so [`CEmbeddedObject::GetEmbeddedObject`]'s index, the one objects are saved by, reaches any
depth in one step.

## Froms

Every [`CEmbeddedObject`] records what points at it:

| Member | Holds |
|---|---|
| [`CEmbeddedObject::mapHeapFromObjects`] | the objects (not collections) that point at it, once per pointer |
| [`CEmbeddedObject::mapHeapFromCollections`] | the collections that hold it, once per entry |
| [`CEmbeddedObject::mpcStackFroms`] | the [`Ptr`]s and collections on the stack that point at it |
| [`CEmbeddedObject::mpsHub`] | NULL until it has 64 heap froms; then where each from is ([Hubs](#hubs)) |

[`CEmbeddedObject::NumHeapFroms`] and [`CEmbeddedObject::GetHeapFrom`] show the two heap arrays as one list, objects
first.  The walks read an object's heap froms, and those of the objects embedded in it, where they are, through
[`CBaseObject::StartHeapFromIteration`] and [`CBaseObject::IterateHeapFrom`]; a hub has thousands, and copying them
cost speed.  A hollow's embedded hollows have froms too, and are read the same way.

## The root and the distance to the root

[`ORoot`] is the root, a [`CRoot`] named `GraphRoot`.  It holds a set ([`CSetObject`]); [`CRoot::Add`] puts an
object in it.  Objects are held by objects reachable from the root, through pointer fields and collections
([`CArrayObject`], [`CSetObject`], [`CMapObject`], [`CIndexObject`], and their typed forms [`CArray`], [`CSet`],
[`CIndex`]).  An object with no more natural holder (a dialog, say) is made with [`OMalloc`] and added to the root,
and removed from it when it is time for it to go.

Every object has a distance to the root ([`CBaseObject::GetDistToRoot`]): the fewest heap pointers from the root to
it.  The root's is 0.  The other values are:

| Value | Name | Means |
|---|---|---|
| -1 | `UNATTACHED_DIST_TO_ROOT` | no path from the root |
| -2 | `CLEARED_DIST_TO_ROOT` | only during a calculation: cleared, to be given again |
| -3 | `EMBEDDED_DIST_TO_ROOT` | an embedded object's own: read its container's |
| -4 | `HELD_FROM_STACK_DIST_TO_ROOT` | unattached and known held from the stack ([Holding from the stack](#holding-from-the-stack)); read as -1 |

The distances are exact: each is what a breadth first search from the root would give.  They are kept as pointers
change, by [`CDistCalculator`], never worked out from scratch.

**A from is added** ([`CEmbeddedObject::AddHeapFrom`]).  If the from is attached, [`CDistCalculator::FromAdded`] gives
the object the from's distance plus one, if that is shorter.  An object that got closer gives what it points to its
distance plus one in turn, breadth first from a queue, so each object gets its final distance the first time it is
reached.  A new from can only bring objects closer.

**A heap from is removed** ([`CDistCalculator::CalculateDistToRoot`]).  First the distances that relied on the from
are cleared, depth first from the object that lost it ([`CDistCalculator::ClearInvalidDistToRoot`]):

- An object keeps its distance if it has a from closer to the root that isn't cleared
  ([`CDistCalculator::HasCloserFrom`]).  That from is valid by the same rule: every attached object had a closer from
  before the change, and clearing an object rechecks everything it points to.  So nothing walks back to the root.
- An object pointed at by a cleared object that was no closer to the root than it, or not attached at all, never
  relied on it and isn't checked.  Without this a window that every component points at is checked once for every
  cleared component.
- A cleared object puts everything it (and the objects embedded in it) points at on the walk's own stack of
  candidates.  The walk isn't recursive: a recursive one overflowed the W65C816's stack on a chain about a hundred
  objects long.

Then each cleared object with an attached from is given the closest from's distance plus one as its expected
distance, and they are taken closest first from a [`CDistToRootExpectedDists`] (a binary heap on distance, ties in the
order added, with an index from object to entry once there are many).  Each one taken gets its final distance and
offers what it points to one more ([`CDistCalculator::AddExpectedDistToRoot`]).  An object pointed at whose distance
is already valid and no longer is left alone: it wasn't cleared, so it has a valid path.  (Clearing it anyway was a
bug, fixed 2026-10-22: a component's back pointer to its window cleared the window, and the focus embedded in it,
and objects found through the window were given longer distances.  `TestDistToRootBackPointerToRootedObject` in
[`TestStandardLib/TestDistToRootCalculator.cpp`] is the repro.)  What is still cleared at the end is unattached.

Distances aren't saved.  They describe the objects in memory, so an object loaded from the database starts unattached
until a from attaches it, or takes the in-memory distance of the hollow it replaces (the user, 2026-11-04).

## Holding from the stack

An object detached from the root is still kept while the stack reaches it: a [`Ptr`] on the stack points at it, or at
an object that points at it, and so on.  When a stack from goes, [`CDistCalculator::Calculate`] frees nothing when the
object still has stack froms, or is known held, or [`CDistCalculator::IsHeldFromStackOrRoot`] finds, breadth first
up its froms, something held from the stack or attached to the root.  Only otherwise does it walk.

**Known held.**  An unattached object is known held from the stack when it has a stack from, or a from that is known
held and older (a smaller index) than it ([`CBaseObject::IsKnownHeldFromStack`]); following older froms always ends at
something held from the stack.  It is kept in the distance, as `HELD_FROM_STACK_DIST_TO_ROOT`.  Objects are built
from older ones (a child added to its parent, a chain grown at its tail), so letting go of the stack pointer that held
an object while it was built leaves it known held through the older one, with no walk.  The user asked for a distance
to the stack instead; it was tried first, but distances renumber whole subtrees as short-lived stack pointers come and
go: building an unrooted tree went from 30 to 119 ms.  Age ordering took building an unrooted tree beside a hub from
302 to 5 ms (2026-11-05, TestDistToRootUpdate speed tests on the host).

**The walk** ([`CDistCalculator::CalculateFromTouched`]).  From the objects whose distance was cleared it collects,
walking up their froms, everything detached from the root, counting those with stack froms.  None: all of them go.
All: none go.  Otherwise those reached from the stack are found breadth first from the ones pointed at by something
reached, and the rest go.  This walk keeps its own stack too.

**Walk stamps.**  What a walk finds about each object is kept in the object's [`CBaseObject::muiWalkStamp`]: the base
of the walk that last reached it ([`CObjects::StartWalk`], which steps by 8) plus a state in the low bits
(`WALK_STATE_TOUCHED`, `WALK_STATE_DETACHED_WALKED`, `WALK_STATE_REACHED_FROM_STACK`).  A stamp from an older walk reads
as not reached, so nothing is cleared after a walk; when the stamps come round to 0 every object's is cleared once.
These replaced seven flags that each needed a clearing pass.  A walk that calls code that walks (a validator) can't
rely on its stamps afterwards, so validation keeps flags (`OBJECT_FLAGS_TESTED_FOR_SANITY`,
`OBJECT_FLAGS_DUMPED`).

## Freeing

[`CObjects::FromRemoved`] is the one place that decides what is freed: every way a from goes (a [`Ptr`] repointed or
going out of scope, an entry removed from a collection, an object killed) unlinks it and then calls it.  It returns
straight away for the root, for an object already in the batch being freed, and for a stack from that went from an
object still attached; otherwise it runs [`CDistCalculator::Calculate`] and frees what that found completely detached.

### The order

[`CObjects::RemoveInKillOrder`] splits what is to go into two batches with [`CKillOrderCalculator`]:

- The objects nothing holds a Nes pointer to go first.
- Then those that are Nes pointed to, each after the others in the batch that hold Nes pointers to it, chains
  included (a component needs its panel, which needs its window).  A cycle of Nes pointers can't be ordered: it is
  logged as an error and freed in the order found.

Both keep the order the walk found the objects in, not their addresses, so `Free`s are called in the same order on
every platform and every run.

A Nes pointer says its holder uses the object in its own `Free`.  It is how an object that owns something made by
another keeps its maker alive until it has given it back: windows, canvases and clipboards hold their
[`CNativeWindowFactory`] by Nes [`Ptr`], a [`CSoundMixer`] its [`CNativeSoundFactory`], a [`CCircuit`] its
[`CCircuitSimulation`] (whose timeline its components take events off).  Makers of native resources are objects for
this reason ([Weak References](weak-references.md#makers-as-objects)).

### A batch

[`CObjects::Remove`] registers both batches with the [`CObjectRemovalContext`] before anything is freed: a `Free` that
lets go of more objects starts a removal, which is queued on the context and done after, and an object in a queued
batch isn't freed twice.  On a thread that isn't the objects' home thread the batches wait to be freed by it
([Objects and Threads](threads.md)).  Each batch is then [`CObjects::RemoveBatch`]:

| Step | Does |
|---|---|
| [`CObjects::SaveStoredDirty`] | saves the dirty objects that are in the database ([Codabase](../baselib/codabase.md)) |
| [`CObjects::DoneKill`] | marks each `OBJECT_FLAGS_FREED` and calls the destruction callback |
| [`CObjects::ClearWeakFroms`] | NULLs every weak reference to them |
| [`CObjects::FreeInternal`] | calls each object's `Free`, embedded objects first |
| [`CObjects::RemoveAllPointerTos`] | clears their pointers, unlinking the froms they gave |
| [`CObjects::RemoveAllHeapFroms`] | drops their own froms |
| [`CObjects::KillObjects`] | gives their memory back |

The batch is marked freed before any `Free` runs.  A `Free` that changes a [`Ptr`] then doesn't start a distance walk
through objects that are going anyway: [`CObjects::FromRemoved`] and both walks pass over freed objects.  Freeing 4096
objects whose `Free`s changed pointers took 33 s before and 61 ms after (2026-11-04,
`TestDistToRootUpdateFreeBatchChangingPointers`); the HCP65 IDE had hung on it.  For the same reason clearing a
pointer whose object is in the same batch only unlinks it ([`CEmbeddedObject::IsBeingFreed`],
[`CCollection::UnlinkFromUnlessFreed`]): before, freeing a hub's children searched the hub's froms once per child.

An object's pointers are cleared one at a time ([`CBaseObject::ClearPointerTos`]), each followed, when it may free,
by freeing what it detached.  Clearing them all and then freeing isn't safe: freeing one object can free another that
the pointers not yet cleared still point at.

### Killing one object

[`CBaseObject::Kill`] frees an object now, as though its last pointer had been set to NULL.  A stored object that is
still held is replaced by a hollow instead ([Codabase](../baselib/codabase.md)).  Otherwise
[`CBaseObject::KillInternal`] takes its froms off, works out what that detached, and frees it in kill order.  The
pointer fields of objects freed with it that pointed at it are put back first, so their `Free`s see it as they would
in an ordinary batch; the objects freed with it are stamped, so this is one comparison per pointer.  Collections
aren't put back: no `Free` reads a collection yet.

An object not allocated in [`CObjects`] (one on the stack) is freed alone: its weak references cleared, its `Free`
and its pointers cleared in Nes order, then its froms dropped.

### Shutting down

[`ObjectsKill`] (through [`CObjects::Kill`]) and [`CObjects::EvictInMemory`] free everything in memory as one batch,
with [`CObjects::FreeAllInKillOrder`]: the kill order is calculated before anything is freed (freeing clears the Nes
pointers it looks for), dependants are freed before Nes pointed to objects, nothing is deallocated until every object
has been freed, and removals started meanwhile are queued on the removal context.  Before 2026-11-03 these freed in
index order, ignoring Nes pointers: a window could be freed before its canvas, which then found its factory NULL.

## Where froms are kept

### Objects and collections apart

Loading a big collection was quadratic: remapping a hollow's froms onto the loaded object, and validating froms,
asked each holder whether it pointed at the object, and a collection answered by scanning all its elements.  The
user's design (2026-10-22) was to split an object's heap froms into froms from objects and froms from collections,
and later to have the collections know where each member is.  The split came first
(`TestObjectHeapFromsObjectsAndCollections` in [`TestStandardLib/TestObject.cpp`]).

### Member locations

A collection with 64 or more members keeps where each member is, in a [`CPointerLocations`]
([`CCollection::mpcMemberLocations`]): open addressing from the member to any number of locations, made the first
time it is wanted.  What a location is depends on the collection: a slot for arrays and sets, the key for maps, a
copy of the key for indexes.  Removing or remapping a member is then [`CCollection::RemoveLocatedMember`] and
[`CCollection::RemapLocatedMember`], not a scan.  An array's members move when one is inserted; rather than move
every location with each change, the array says its members have moved ([`CCollection::HaveMembersMoved`]) and the
locations are made again when next wanted.

Before it, killing the members of an index that was still alive took 289, 1096 and 4817 ms for 4K, 8K and 16K
members (2026-11-05, host): quadratic.  The user chose these collection-side tables over a hint on each object of
where it is in its collection.

An object that is in one collection several times has a from for each entry; removing the entries removes that many
froms, and only the last can free the object ([`CCollection::RemoveObjectCopiesTryFree`]).

### Hubs

A hub is an object with thousands of froms: a window that every component points back at.  Once an object has 64
heap froms it gets an [`SHeapFromHub`] ([`CEmbeddedObject::mpsHub`]): where each from is in its arrays, so unlinking
one isn't a search, and a from that was closer to the root the last time one was asked for
([`CBaseObject::HasHeapFromCloserThan`] starts there and stops at the first).  It goes again below 32 froms.
Profiling showed the cost was mostly [`CDistCalculator::HasCloserFrom`] scanning every from (about 56%), more than the
unlinking (about 35%).  The user chose one pointer on every object for this (2026-11-05), which made every object 8
bytes bigger on 64-bit hosts.  At depth 9 of the speed tests, letting go of a hub went from 533 to 1 ms and freeing a
batch from 4966 to 1776 ms (2026-11-05, host).

## Unknown collections

The unknown collections ([`CArrayUnknown`], [`CSetUnknown`], [`CMapStringUnknown`], [`CMapUnknownUnknown`],
[`CIndexUnknown`]) hold [`CUnknown`]s, not objects: nothing points back and nothing is freed by reachability.  They
share these rules (2026-10-23):

- An element is saved with [`CUnknowns::SaveUnknown`] and loaded with [`CUnknowns::LoadUnknown`], which write and read
  a flag first saying whether it is NULL.  The arrays save every slot, NULLs too, so the count in the header matches
  and indices are kept; loading counts the non-NULL elements again.  A [`CMapUnknownUnknown`]'s keys are never NULL
  and have no flag; its values do.
- Putting a key that is already there replaces the value in place (and kills the old one if the collection kills its
  elements, unless it is the same value), or fails if the collection was made not to overwrite
  ([`CMapStringUnknown::Init`], [`CIndexUnknown::Init`]).  The flags that say so, `MAP_COMMON_NO_OVERWRITE` and
  `INDEX_TREE_NO_OVERWRITE`, are set for "don't overwrite", so files saved before they existed load as overwriting,
  which they always did.
- [`CArrayCommonUnknown::AddAll`] counts only the source's non-NULL elements, and clears `ARRAY_COMMON_IS_PTR_SORTED`
  because the appended elements aren't in address order.

## Validation and tests

In Debug builds every change is followed by [`CObjects::ValidateObjectsConsistency`], which walks memory once.
[`CBaseObject::ValidateCanFindRoot`] checks each distance locally: an attached object's closest attached from is
exactly one closer, and an unattached object has no attached froms; checked for every object, that is the same as
every object finding the root by its shortest path, without walking to it.

[`TestStandardLib/TestDistToRootUpdate.cpp`] checks the distances against a breadth first search from the root after
every step of random graphs (through arrays, sets, indexes and maps, freeing as it goes), and has the hub,
back-pointer, alternate-path, long-chain and unattached cases and the speed tests; on the 65C64 and W65C816 it runs
with smaller sizes.  The speed tests turn validation off.  The free order is tested in
[`TestStandardLib/TestObjectFreeOrdering.cpp`].

## What profiling and the review changed

The design above is mostly the result of three rounds of work in October and November 2026.

**The review of 2026-11-03** looked at objects, pointers and memory in StandardLib and what uses it.  It fixed
`PointTo`'s order and the shutdown order (above); made the classes that held [`Ptr`]s outside the stack-or-field rule
into objects ([`CCircuitSimulation`] and LogiCim's editor classes, dialogs, the mesh cacher, InputLib's
[`CInputDataFormat`] and its holders) or gave them raw pointers; made native canvases' images own their pixels, so a
kept image no longer outlives them; and found the native window factory killed before [`ObjectsKill`], fixed by making
the factories objects (2026-11-16).

**Speeding up the distances** (2026-11-03, the user: as fast as possible, keeping them exact).  Each change was kept
only if a test failed without it or it was measured faster.  On a focus sweep over a three-way window tree
(RelWithDebInfo):

| Change | Without | With |
|---|---|---|
| All of them, depth 7 | 5647 ms | 22 ms |
| All of them, depth 8 | 47.9 s | about 195 ms |
| Breadth first queue when a from is added, depth 7 | 4660 ms | 23 ms |
| Skipping objects the cleared from was no closer than, depth 7 | 1201 ms | 23 ms |
| [`CDistCalculator::HasCloserFrom`] instead of a walk to the root, depth 8 | 356 ms | 186 ms |
| The expected distances as a heap, reattaching a tree | 14 ms | 2 ms |

Hovering in a 200-row WindowLib list went from 262 ms to 0.6 ms per move.  The random freeing test found a map that
added a second key from when a key was put again, so the key was never freed.

**The cleanup of 2026-11-04 and 2026-11-05** (the user: "memory management code is becoming a bit tangled").  Each
container writes one walk of its pointers ([`CEmbeddedObject::BaseGetPointerTos`]) instead of about ten; the walks
moved from the objects into one [`CDistCalculator`], the objects keeping only access to their froms, pointers,
distances and flags; walk stamps replaced the walk flags; [`CObjects::FromRemoved`] replaced seven free entry points;
the kill order stopped sorting by address; embedded objects read their container's distance and flags instead of
copies; and the hubs, member locations and known-held distances above were added.  Dead paths went with it.

## Decisions

- Distances to the root stay exact shortest paths; no "any valid path" relaxation (the user, 2026-11-03).
- A [`Ptr`] lives on the stack or as a field of a [`CObject`], nowhere else, and whatever holds one is an object (the
  user, 2026-11-03).
- Froms from objects and from collections are kept apart, and collections know where their members are; sorted
  lookup in sets and maps instead was rejected as too complex (the user, 2026-10-22).  The locations are kept on the
  collection side, not as hints on the objects (the user, 2026-11-05).
- Hubs get one pointer on every object, rather than a table elsewhere (the user, 2026-11-05).
- Known held from the stack is by age, not a distance to the stack: distances renumber subtrees (2026-11-05, after
  the user asked for a distance and it was measured).
- `DIRTY` is the container's: embedded objects share it (the user, 2026-11-05).
- Loaded objects start unattached and distances aren't in the file format (the user, 2026-11-04: "Remove the unused
  distances from the written file").
- Objects are freed in the order found, not by address, so `Free`s run in the same order everywhere (2026-11-05).
- Makers of native resources are objects, held by Nes [`Ptr`] from what they make (the user, 2026-11-16, choosing
  "factory as an object").

## Not done

- An object built bottom up (an older object held only by a newer one) isn't known held, so letting go of a stack
  pointer to it still walks.
- A `Free` that reads a collection freed in the same kill as the object being killed doesn't see that object: only
  pointer fields are put back ([Killing one object](#killing-one-object)).
- Test-only methods (`TestGetPointerTo`, `TestRemoveHeapFrom` and others) are still members of the classes.
- Test helper structures in TestLogicimLib and TestMicroprocessorLib hold [`Ptr`]s outside the stack-or-field rule;
  left as they are (2026-11-03).
