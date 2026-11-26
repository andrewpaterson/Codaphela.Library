# Objects and Threads

StandardLib's object system ([Objects](objects.md)) keeps, for every object, what points at it, how far it is from
the root and whether it is being freed.  Every [`Ptr`] copy changes some of that.  Until November 2026 all of it
assumed one thread, and programs with threads kept objects off them.  This page is how the object system was made
safe to use from several threads, what a program does to use it, and what the applications that use it showed.

The plan it was built from, [`StandardLib/ThreadSafetyPlan.txt`], is finished.  It is kept as the record that the
code and tests cite by section and step: the full audit of every way in, and each step's measurements, are there.

## The contract

The object system promises:

1. Its own bookkeeping (froms, stack pointers, distances, flags, the memory index, weak froms, the allocator) is
   never corrupted, whatever threads do with pointers, collections, [`OMalloc`] and [`CObjects::Get`].
2. An object is never freed while any thread holds a [`Ptr`] to it, or while it is reachable from the root.
3. Freeing stays exact and immediate: when the last holder lets go, the object is freed then, or, if another thread
   let go, queued for the home thread (below).

It does not promise that an object's own fields are safe to read on one thread while another writes them.  That is
the class's business (a [`CMutex`] of its own), as with any data, and it includes a collection's contents: two
threads adding to one [`CArrayObject`] need a lock of their own, though the froms of what they add are always right.

Two rules for users follow.  A thread that uses an object across a point where another thread could let go of it
holds a [`Ptr`] of its own: `pWindow->mpCanvas->Draw()` through a field, with no stack [`Ptr`], is only safe while
whatever guards `mpCanvas` is held.  And with a database, [`CObjects::Flush`] and the save of a dirty stored object
as it is freed read the object's fields on the thread doing it, so a class whose fields other threads write saves
under its own lock (in its Save), or the program flushes between jobs.

## Using objects from threads

The thread that called [`ObjectsInit()`] is the home thread.  Four ways to give threads work, cheapest first:

| Pattern | The worker threads | Example |
|---|---|---|
| Plain data | Never touch objects. | LogiCim's KiCad import lays out each board's circuit, plain data, as a job. |
| Plain pointers below one [`Ptr`] | Not registered; read through raw pointers and `operator->` what the home thread holds while it waits.  No [`Ptr`] is copied. | [`CMeshBake::SetJobs`]: a bake's rows in bands on a [`CJobs`]. |
| Registered threads | [`ObjectsThreadInit()`] / [`ObjectsThreadKill()`]; may copy and drop [`Ptr`]s freely, load stored objects. | A bake whose texture takes a [`Ptr`] at every sample. |
| Workers with object systems of their own | [`ObjectsWorkerInit()`] / [`ObjectsThreadKill()`]; build graphs in parallel and hand them over with [`ObjectsAdopt`]. | Sculpt's baker, the emulator's model loaders. |

The pattern to prefer for reading is the second: one [`Ptr`] at the top, plain pointers below it, no locking at
all and full speed on every core.  A Debug build reports any way into the object system taken by a thread that isn't
the home thread while the locking is off ([`ObjectsNumThreadErrors`] counts them), so a texture that does copy a
[`Ptr`] where it is sampled is caught by the tests.

For the last two patterns:

- The home thread switches the locking on, [`ObjectsLocking(bool)`] with true, before any other thread touches an
  object, and off when they have all gone.  Registration can't do it: the registering thread would race a home
  thread half way through something it began with the lock off.  [`ObjectsLocking(bool)`] refuses from any other
  thread, from inside a Free or an Init, and (off) while other threads are registered.
- Each other thread registers before it touches an object and unregisters when done, having let go of every
  [`Ptr`] (a Debug build reports any it kept).  Pools do it for their threads:
  [`CWorkerThreadPool::Init`] and [`CWorkerThreadJobs::Init`] take a function each thread calls on starting and one
  it calls on ending, which are [`ObjectsThreadInit()`] or [`ObjectsWorkerInit()`] and [`ObjectsThreadKill()`].
  A thread that isn't registered is told when it comes in with the locking on (every build).
- Only the home thread runs [`CBaseObject::Free`] (a worker is the home thread of its own object system).  What
  another thread lets go of is freed when the home thread next takes the lock (or, if it only reads, at its next
  read: with frees waiting it takes the lock instead) or calls [`ObjectsCollect()`].  A home thread that waits for
  its threads without doing either frees nothing meanwhile.
- A Free runs with the lock held, so it must not wait for a thread that uses objects, nor take a lock another thread
  holds while it uses objects.  A class with a lock of its own can check the second where it takes that lock:
  [`ObjectsAssertNoFree`] says, in a Debug build, when it is reached from inside a Free.  This is why the emulator's
  [`CBoardSceneDraw::StopLoading`] and Sculpt's baker's Stop wait for their threads and mustn't be called from a Free.

## Three levels

Each level is complete and was built in turn (decision 2; level 3 once it was needed).

| Level | What it is | What it is for |
|---|---|---|
| 1 | One recursive spinning lock for the process, taken by every way in. | Correctness; threads that seldom touch a [`Ptr`].  Holds about four threads at one copy per 0.8 us of work. |
| 2 | Stack pointers to rooted objects taken and dropped as readers, without the lock; each registered thread keeps its stack froms in a table of its own. | Threads that read objects and copy pointers: they cost each other nothing. |
| 3 | A worker thread has a [`CObjects`] of its own; the finished graph is adopted by the main one. | Building objects in parallel, which levels 1 and 2 serialise. |

### Level 1: the lock and its guards

[`StandardLib/ObjectsLock.h`] says how it works.  Every public way in was audited (step 8 of the plan, the table is
there) as one of:

| Mark | Meaning | Guard |
|---|---|---|
| W | Changes what the object system keeps, or frees, loads or saves. | `OBJECTS_WRITE()` |
| R | Only reads it. | `OBJECTS_READ()` |
| I | Machinery that is public only because C++ makes it so, reached from a W or an R. | `OBJECTS_INSIDE()`: Debug asserts the lock is held |
| - | Touches only the caller's own object or what never changes once an object exists. | none |
| H | The home thread alone, no other thread registered: set-up and shut-down. | none |

The guard ([`CObjectsGuard`]) is one inline test of [`gbObjectsLocking`] with the locking itself out of line, so a
program with one thread pays a test and a branch never taken.  Nothing at all is in `operator->`
([`CPointer::Dereference`]): a lock there took it from 1.3 to 9.2 ns, and even an unused test cost 0.5 ns.  The lock
([`gcObjectsLock`], a [`CRecursiveSpinLock`]) is recursive because a Free, an Init or a Save is the user's code and
comes back in, and it spins then yields because a fought-over pthread mutex puts waiters to sleep: 12 threads
copying pointers through one took 5 to 6 us a copy.  It is one lock for the process, not one for each
[`CObjects`]: a pointer must be locked before its object can be asked which [`CObjects`] it is in, and stack
pointers and threads are the process's anyway.

Two rules came out of proving the guards with a mutation harness (step 16):

- A way in that only passes what it is given to other ways in has no guard of its own (a pointer's Kill is its
  object's Kill; a Dump is its Print written to the console).
- A way in that reads a pointer out of the object system's own tables (the memory index, a weak reference) and must
  hold the object before the lock is let go does it with the machinery beneath ([`CPointer::AssignObjectInside`],
  [`CPointer::PointToInside`]), not another way in, so it takes the lock exactly once.

The audit also found reads that wrote: getting a collection's element or using a pointer wrote the element or
pointer back whether or not it had just loaded a hollow, so two threads reading were two threads writing.  They
write only what changed now (and since step 24d, nothing: the remap has rewritten the pointer already).

[`CUnknowns`] is not under the object lock: unknowns that aren't objects are made with no object system in sight,
so it has a spin lock of its own round its allocator, its count and its iterables ([`CUnknownsGuard`], taken only
while the locking is on).

### Level 2: readers and writers

[`StandardLib/ObjectsThreads.h`] says how it works.  Most pointer traffic walks nothing: a stack [`Ptr`] taken to, or
dropped from, an object that stays attached to the root.  Those are readers; anything that changes a heap from,
walks, frees, loads or evicts is a writer and runs alone, as in level 1.

- A reader sets its own flag, on a cache line of its own, and looks at the lock ([`ObjectsReadBegins`]).  If the lock
  is held it drops the flag, waits and tries again; after four tries it takes the lock (writers that follow each
  other closely would otherwise keep it waiting for good).  It doesn't take the lock at once: readers that did would
  each be a writer to the rest.
- A writer takes the lock and waits for every other thread's flag to drop ([`ObjectsWriteBegins`]).  The lock's own
  word is the writers' flag.  Each side sets its own before it looks at the other's, so one always sees the other.
- Where a worker's stack froms are kept is what decides scaling.  On the object, under a small lock picked by the
  object's address, they scale when threads hold different objects and not at all when they hold the same one.  So
  each registered thread keeps a table of (its stack pointer, the object) and writes nothing to the object; the home
  thread keeps its froms on the objects as with one thread.  A walk asking "has this object a stack from?" looks in
  the tables of the threads that keep any (counted, so idle registered threads cost nothing), and only of objects
  found detached from the root.  Stack pointers to unattached objects stay on the object, under the lock.
- The home thread reads too: with the locking on and nobody else working its copy and drop went from 25.6 to 17 ns.
  A home thread that only read would never free what others let go of, so with frees waiting it takes the lock.
- Nothing reads where the database is written through to ([`CObjects::IsWriteThrough`]): pointing a pointer at an
  object there can save it, so that is level 1.

A pointer's own ways in (constructors, destructor, assignments, and `AssignObject`, which is how a collection hands
out an element) do their own locking with no guard object ([`CPointer::PointToReading`]): one thread with the
locking off is back to 324 instructions a copy and drop.

### Level 3: a worker's own object system

[`StandardLib/ObjectsWorker.h`] says how it works.  Levels 1 and 2 serialise every heap pointer change and every
allocation and free (each a writer), so threads that build graphs gain nothing; [`ObjectsWorkerInit()`] gives the
thread an [`SObjectsWorker`]: a [`CObjects`] with its own root, classes and [`CStackPointers`], no database, and
unknowns of its own that share the main unknowns' allocator and constructors ([`CUnknowns::InitSharing`]).
[`ObjectsCurrent()`] is the running thread's object system ([`gcObjects`] unless it has one), and [`OMalloc`],
[`ORoot`] and the like go to it.  The worker is its home thread: Free runs there and nothing is queued; its own
objects' ways in take no lock ([`ObjectsEnterFor`] decides by the object system a way in is about).

- Indexes: a worker takes 256 at a time from the main object system's sequence ([`CObjectsWorkerSequence`],
  [`CObjects::ReserveIndexes`]) under the sequence's own lock ([`CObjects::mcIndexLock`]), not the main lock, so its
  objects are unique in the process and keep their index when adopted.
- A worker's object may point at a main object (decision 11).  The field is held as the worker's stack pointers hold
  main objects, in its thread table ([`CPointer::PointFieldAcross`], [`CCollection::AddAcrossFrom`]), never as a heap
  from on the main object, whose froms are the main lock's.  No walk crosses: to a worker's walk the main object is
  a leaf, to a main walk the field is a stack pointer.  A main object pointing at an unadopted worker object is an
  error, said in Debug ([`CEmbeddedObject::ValidateWorkersOwn`]).
- [`ObjectsAdopt`] ([`CObjects::Adopt`], [`StandardLib/ObjectsAdopt.cpp`]), by the worker under the main lock: the
  graph is the top and every worker object it reaches.  A name the main object system has (in memory or its
  database) refuses the whole graph and nothing moves (decision 14).  Otherwise each object moves to main's memory
  index, classes (completed now if main has none, embedded classes first) and unknowns ([`CUnknowns::MoveTo`]); its
  pointers at main objects become ordinary heap froms; weak references within the graph move and those across its
  edge are cleared; the worker's stack pointers and outside objects that held it become stack froms.  The graph is
  unattached in main until something attached points at it.  Anything else the worker made stays with it.
- [`ObjectsThreadKill()`] discards and kills what is left in a worker's object system.

Before level 3 the memory index (a trie by the index's bytes, about 1800 instructions to put and 1600 to take out,
37% of an OMalloc and free) was replaced by [`CIndexedObjects`] (decision 15): an index's bits above 16 pick a
directory, the next 8 a page and the low 8 a slot, so a get is three loads; the last page used is tried first, and
an emptied page is kept for the next one needed.  Iteration goes in index order from an index
([`SIndexesIterator`]), so objects can come and go as it runs and one left early has nothing to free.  A hash table
got faster in a model but has no order, and Flush, Kill, Evict and validation go in index order.

## Stored objects: forwarding hollows

With a database, using a [`Ptr`] to a hollow loads the object ([`CObjects::Dehollow`]); every pointer to the hollow is
rewritten to the object and, with one thread, the hollow is freed there and then.  With threads that breaks: another
thread may have read the hollow's address in `operator->` and not yet called into it, and nothing can be put in
`operator->`.  So a hollow loaded while another thread is registered forwards:

- It is remapped as before ([`CObjectRemapFrom::RemapFroms`]) but put aside on its [`CObjects`] with the object it
  was loaded as; nothing is added to the hollow, whose size the 65 targets assert.  Its Dehollow
  ([`CObjects::Dehollow(CHollowObject*)`]) takes the lock before reading anything and answers the object, loading
  nothing.
- Each thread counts its reads and its takings of the lock ([`SObjectsReader`]); it can't do either inside the
  window.  The home thread's [`ObjectsCollect()`] marks the counts, and a later one frees what was forwarding at the
  mark once every count has moved ([`CObjects::FreeForwarders`]).  Not when the home thread takes the lock, as queued
  frees are: that can be inside the hollow's own Dehollow.  With no other thread registered they all go, and a
  loaded hollow is freed at once, as before.
- The words a load rewrites are read and written whole as relaxed atomics on the hosts ([`CPointer::LoadObject`],
  [`CPointer::UnsafePointTo`], a collection's elements); a pointer's load is an acquire.  On x86-64 these are the
  instructions used before.  Dereference no longer writes back what a load answered.
- Eviction ([`CObjects::EvictInMemory`]) is refused while another thread holds any stack pointer: in practice it is
  done between jobs.  The Kill of a held stored object is refused while another thread holds a stack pointer to it
  or one of its embedded objects ([`CObjects::ValidateNoOtherThreadHolds`]).

Sets and maps used to keep their members in address order, and a fat hollow was allocated at the object's size so
a load kept the address.  Loaded in place, though, the object's vtable is there from the start of its load, so
another thread could call into a half loaded object.  Decision 10 orders sets and map keys by object index instead
([`CEmbeddedObject::CompareIndex`]: an embedded object by its container's index, then its place), so a load changes
no order; fat hollows are retired and their size left the record format.  The order is also the order objects were
made in, the same on every run under every allocator (address order broke four tests when the allocator changed).
A hollow made for a name alone takes the database's index for that name from the start.

Separate from the Codabase transaction work ([Codabase](../baselib/codabase.md)): every thread here shares one
[`CObjects`] and one data connection, called only with the lock held.  A thread wanting a database view of its own
(a read-only snapshot) would want a [`CObjects`] of its own over its transaction: a worker with a database, not built.

## The allocator

Nearly half of an OMalloc and free (15400 instructions in callgrind, 2026-11-17) was glibc malloc, realloc and free
under [`gcSystemAllocator`], for the memory index's nodes and scratch arrays.  The user's ruling (decision 5) was to
make [`CTrueMallocator`] faster than malloc and safe for threads, or fall back to a lock.  It could be:
[`CThreadMallocator`] ([`BaseLib/ThreadMallocator.h`]) gives each thread an arena, a [`CTrueMallocator`] over 4 MB
aligned chunks from the system, so a block's arena is found from its address.  In front of each arena are lists per
block size (16 sizes to each power of two up to 64 KB, 2 MB in all), so the common malloc is a pop and the common
free a push.  A block freed by another thread goes on its arena's remote list with one compare and exchange; the
owner takes the list back when a front list runs dry.  A chunk left with nothing allocated goes back to the system,
except one kept so memory rising and falling across a chunk's end doesn't map and unmap each time.  Blocks over 1 MB
are mapped from the system, the last few kept for reuse.  A thread that ends leaves its arena to the next
one.  [`gcSystemAllocator`] uses it on the hosts, and the global unknowns are allocated by it too (decision 9,
[`UnknownsInit()`]).  The 65C64 and the W65C816 keep [`CTrueMallocator`] alone.

With the allocator no longer thrown away whole, LeakSanitizer saw what had leaned on that: [`CObjects::Kill`] now
gives each object's memory back, and a [`CObject`] made by [`CUnknowns`] without Objects is given back to its
unknowns when killed.  [`CMallocator::FreeMultiple`] now returns how many it freed.

## The 65C64 and W65C816

They run one thread.  The guards and every function a way in names compile to nothing or to stubs in the
`__LIBC65__` branches of [`StandardLib/ObjectsLock.h`] and [`StandardLib/ObjectsThreads.h`]; [`ObjectsOwn()`] is
NULL and [`ObjectsAdopt`] refuses.  Object sizes don't change on any target (the 65 suites' size asserts; the
slots-moved bit of a large set, which threads iterating read, became a byte of its own in the padding after the
flags, which fits on all three targets).

## How it was proved

[`TestStandardLib/TestObjectsThreads.cpp`] holds the threads tests: Ptrs copied on many threads to one object and to
objects being re-pointed, unrooted graphs built and dropped on threads, the last Ptr dropped on a worker (freed on
the home thread, once, in Nes order), weak Get against the last strong Ptr going, stored trees walked and loaded by
8 threads, stored sets and maps iterated while others load their members, flush while threads read, workers
building and adopting named trees, with and without a database.  Every test ends with exact counts and
[`CObjects::ValidateObjectsConsistency`].  One test calls every guarded way in with the locking on and checks it came
in exactly once ([`ObjectsNumWaysIn`]) and reached nothing without the lock ([`ObjectsNumThreadErrors`]).

Each level was proved by mutation: a harness (kept outside the repositories with the prototypes, in
`~/Codaphela/documents/ThreadSafetyPrototype/mutation`) takes out each guard or protocol step in turn and requires a
test to fail.  Level 1: 106 of 109 guards killed.  Level 2: 97 of 100, and 10 of 11 protocol slips always fail (the
eleventh, a reader not looking at the lock again after waiting, is found by ThreadSanitizer in about one run in
three).  Forwarding hollows: 21 of 22.  Level 3: all 16.  Survivors, and why they stay:

| Survivor | Why it stays |
|---|---|
| The guard of [`CObjects::Destroy`] | "Stored? destroy it in the database, else kill it" is one step and both branches are guarded ways in. |
| The guard of [`CRoot::UnsafeGet`] | The set's own UnsafeGet takes the lock once to hold what it read. |
| The guard of [`CObjects::DumpMemoryUseIteration`] | Writes the memory index to the console; not run in a test. |
| A pointer's load made relaxed | The race (a key a load rewrote, followed before the object's index is seen) was seen once in 12 runs before the fix and not in 6 tries after; readers mostly synchronise through the reader protocol anyway. |

ThreadSanitizer (`BASELIB_TSAN`) and ASan builds of TestStandardLib, TestSupportLib and TestSculptLib report nothing;
the threads tests were run 16 times under ThreadSanitizer and 10 under ASan.  The final full sweep (2026-11-19, of
CommonLibraries 1e0edb4d6): 147 builds, every StandardLib suite and every suite built on it passing in every
configuration, the 65 suites with their sizes unchanged.

## In the applications

Each application uses one pattern, and each showed something different.  Dates are 2026-11-19 unless given; the
applications' own timings were taken in the running apps, their builds not recorded.

**Sculpt's baker** (HCP65-Sculpt, `Libraries/SculptLib/SculptBaker.h`): a [`CObject`] owning a thread started with
[`ObjectsWorkerInit()`], so the stack a mesh is made with and the meshes made on the way are the worker's and freed
there; the finished mesh, and the rasteriser's cooked mesh made on the same thread, are adopted before the bake is
said to be baked.  A new bake replaces one waiting; one at the whole density being made is stopped
([`CMeshBake::SetStop`]), a coarse one finished so something shows.  The page polls every 2 ms (WindowLib has no
cross-thread wake).  The app switches the locking on at start unless `HCP65_SCULPT_NO_BAKER` is set.  The longest
frame with the marble material went from about 70 ms to 13 ms (TestSculptLib's page test,
`SCULPT_BAKE_REPORT`).  Inside a bake the rows are shared out on a [`CWorkerThreadJobs`] by plain pointers (step
17): a unit sphere's marble at 128 texels a unit with 4 samples, 638 ms on one thread and 58 ms on 24 (Release, 64
bit, 2026-11-17); with registered threads and a texture that takes a [`Ptr`] at every sample, 57 ms against 54 with
plain pointers.  It proved level 3 under a real load, and level 2's claim that workers may copy Ptrs to rooted
objects freely.

**The emulator's board models** ([`CBoardSceneLoad`], [`CBoardSceneDraw::Load(const char**)`]): each model's load
is a main object whose Load runs on one of 4 loader threads ([`CWorkerThreadJobs::Init`] with
[`ObjectsWorkerInit()`] and [`ObjectsThreadKill()`]), reading a KiCad VRML export into the worker's object system
and adopting the editor and its render mesh.  The loaders are started on demand with the locking switched on, and
killed, the locking off again, when nothing is loading ([`CBoardSceneDraw::StopLoading`]).  Nine models from the
exports took 21 s on 4 loaders (72 s summed) and 0.9 s from saved .DRG files.  Moving the cooking and render meshes
onto the loaders too ([`CRenderScene::SetMesh`], [`CBoardSceneDraw::GiveRenderMesh`]) took the longest gap between
window ticks from 406-452 ms to 140-144 ms.  It proved adoption of large graphs and the start-stop of locking
around a burst of work.  At the user's ruling it has no test of its own; it was checked with a scratch harness under
ASan and ThreadSanitizer.

**LogiCim's KiCad import** (LogiCim, `Libraries/KicadImporterLib/KicadSchemBoards.h`): an import is built on the
caller's thread in the old order (all the work that touches the registry, fonts and other boards, about 0.1 s), then
each board's layout, 99% of the time and only its own circuit, is a job on a [`CJobs`].  It uses no object system on
the threads at all: the plain data pattern.  Output is byte for byte the same with 13 boards on 13 threads; the
13-board import went from 12.2 s to about 6 s.  A [`CProgressDialog`] shows progress and cancels (a stop flag, as
[`CMeshBake::SetStop`]), stopping in 0.2 to 1.0 s.  It proved that the first question is whether the threads need
objects at all.

## Measurements

The numbers that chose the design (Release, 64 bit, -O3 with LTO; ns for each operation on each thread; the plan
has the rest).  Short operations move a nanosecond or two between builds from where code lands in memory alone, so
those were judged by callgrind instruction counts: the guards add 23 instructions to a stack Ptr's copy and drop of
349 (2026-11-17).

| Ptr copy + drop, at 1 / 4 / 12 threads (2026-11-17 prototype) | own object | one shared object |
|---|---|---|
| pthread mutex | 42 / 624 / 5372 | 34 / 659 / 6509 |
| spinning lock | 29 / 380 / 2500 | 30 / 644 / 3129 |
| lock on the object | 42 / 40 / 47 | 40 / 584 / 1396 |
| per-thread table | 17 / 17 / 14 | 16 / 17 / 25 |

| As built | Result |
|---|---|
| Guards, locking off, one thread (2026-11-17) | Stack Ptr copy + drop 14.2 -> 14.8 ns; OMalloc + free 789 -> 805 ns; a naive guard object cost +25% and +37% on `operator->` |
| Level 2 readers, 1 / 4 / 11-12 / 23 other threads (step 19) | 13 / 14 / 16 / 22 ns a copy and drop; level 1 was 77 ns / 1.7 us / 11 us |
| OMalloc + free on 1 / 2 / 4 threads at level 2 (step 22) | 0.8 / 1.7 / 4-5 us: four threads do less than one |
| OMalloc + free on 1 / 4 / 12 workers at level 3 (step 25f, 2026-11-19) | 367 / 380 / 422 ns (before the index lock and own unknowns: 415 / 1252 / 13821) |
| Adopt (step 25f) | 147 ns an object |
| Two-level memory index (step 25a) | OMalloc + free 9097 -> 5441 instructions (-40%), 478 -> 299 ns; a tree built -40%, freed -32% |
| [`CThreadMallocator`] against glibc 2.39 (prototype, 2026-11-17) | 1.2 to 4.8 times faster in all 8 single-thread patterns; flat from 1 to 12 threads |
| Global unknowns on it (step 23, decision 9) | OMalloc + free 586 -> 490 ns (-16%); a tree built +0.9%, freed +2.6% in instructions |
| Sets by index (step 24a, decision 10) | Contains 1.6-1.9 times as long (4 ns more at 10 members, 90 at 100000); an Add +41% at 1000, +8% at 100000 |

## Decisions

The user's, asked one at a time with measurements.

1. The contract: the object system protects itself and lifetimes; a class protects its own fields (2026-11-17).
2. Level 1 first, a spinning lock, not [`CMutex`]; then level 2's per-thread stack froms (2026-11-17).
3. Free runs on the home thread, queued from other threads; the thread that disconnects a graph still does the walk
   that finds it dead, "that's as expected" (2026-11-17).
4. Threaded programs with a database: stored objects one thread at a time and eviction only between jobs, for now;
   nothing added to `operator->` (2026-11-17).  The first part was lifted by forwarding hollows (2026-11-18).
5. "The TrueMallocator exists, see if you can make it faster than malloc and thread safe.  Fall back to lock now and
   measure if not." (2026-11-17): it could, so no fallback.
6. Threads register explicitly; an unregistered thread touching objects is an error (2026-11-17).
7. The half nanosecond is accepted on the hosts: one configuration, guards always built; compiled out only for the
   65C64 and the W65C816 (2026-11-17).
8. Allocation off the lock (plan step 21) is not built (2026-11-18): every free is a writer, so allocating as a reader
   can't let threads make and free more objects, and a lock of the memory index's own is too wide a change for 1.3
   to 1.5 times.  Level 3 is the way to build objects in parallel.
9. The global unknowns go onto the new allocator, "Switch anyway", for the 16% on OMalloc and free (2026-11-18).  The
   user was first told a tree's build and free would be 17 and 23% slower; measured like for like (one commit, the
   whole test program, instruction counts and a cache simulation) they are about the same.  There is no follow-up
   to "keep same-sized blocks together": the cache simulation refuted the need.
10. Sets and maps are ordered by object index, not address, so a load never changes an order; fat hollows retired
    and their size taken out of the record format, as nothing outside the tests reads it yet (2026-11-18).
11. A worker's objects may point at main objects, held as its stack pointers hold them and made ordinary pointers at
    Adopt; main objects may not point into a graph not yet adopted (2026-11-18).
12. [`ObjectsAdopt`] moves the top and every object it reaches that is still the worker's; anything else stays with
    the worker, which carries on (2026-11-18).
13. A worker's object system has no database: its objects take their indexes from the main sequence and are saved
    when the main one flushes after Adopt (2026-11-18).
14. A name clash is found at Adopt, which refuses the whole graph, not at creation (the user's choice against the
    recommendation, 2026-11-18).
15. The memory index is replaced before level 3 is built on it (2026-11-18).
16. In the applications (2026-11-19): Sculpt's baker on by default with the locking on while the app runs; Sculpt's
    page polls every 2 ms for a finished bake; the emulator has 4 loader threads (3.4 GB peak the first time the
    exports are converted); the board scene has no test; a LogiCim import can be cancelled, its progress in the
    dialog rather than the status bar; threads inside one board's layout, not now.

## Not done

- Evicting part of memory while threads run (a game's paging): an object another thread uses mustn't be evicted, and
  the only sign of use the object system sees is a stack [`Ptr`].  Needs a design of its own.
- A worker with a database of its own (a thread's own snapshot of the database), the meeting point with Codabase
  transactions.
- An index object's values are copied in and out of its tree with memcpy, so a read while a load rewrites one is a
  race as far as ThreadSanitizer goes ([`CIndexObject`]), though the hardware reads it whole.
- [`ORoot`] finds the root by name on every call ([`CObjects::Root`] is a writer): a search where a pointer would do.
- [`CBaseObject::SetDirty`] is a writer called by every assignment that changes a primitive field: a class whose
  primitives are written often on several threads will feel the lock.  The dirty bit alone could be an atomic OR.
- No application calls [`ObjectsCollect()`] from a tick (2026-11-26).  Queued frees don't need it, but forwarding
  hollows are freed only by it, by [`ObjectsLocking(bool)`] going off, or by [`ObjectsKill`]; no application yet
  shares stored objects among threads.
- A tree loaded by its name alone while its holders are hollows isn't attached and goes again when the pointer to it
  does: the open item on objects loaded by name, not a threads one ([Objects](objects.md)).
- [`CThreadMallocator`]: long runs (fragmentation) and Debug not measured.
- The LogiCim import's Cancel was run headless only, not clicked on a real display (2026-11-19).
