# Containers

BaseLib's containers are written once and compiled once.  A family of classes is a template over what varies (a size
policy, an element type, the class it derives from); its code is instantiated in one `.cpp` and declared
`extern template` in its header, so no other unit compiles it.  The classes people use are thin: a typed template
inlined into its caller over a block class that knows only sizes and offsets.  This keeps the W65C816's code small
and the compile time where it was, and it lets the HCP65 IDE index, rename and refactor all of it as ordinary
source.

## Size policies

Every array comes in two sizes: natural, counted in [`size`], and large, counted in [`lsize`].  On the hosts they
are the same type; on the W65C816 [`size`] is 16 bits, so a natural array holds at most `MAX_SIZE` bytes and a large
one is needed past 64KB.  What the two differ in is a size policy, a struct given to the templates as `SIZE`:

| | [`SArrayNaturalSize`] | [`SArrayLargeSize`] |
|---|---|---|
| Count | [`size`] | [`lsize`] |
| Max, NotFound | `MAX_SIZE`, `ARRAY_ELEMENT_NOT_FOUND` | `MAX_LSIZE`, `ARRAY_LARGE_ELEMENT_NOT_FOUND` |
| CheckSize | true where `SIZE_16` is defined, else false | false |
| Copy, Move, Set, Compare, Swap | [`memcpy`], [`memmove`], [`memset`], [`memcmp`], [`MemSwp`] | the 32 bit forms ([`BaseLib/Memory32.h`]) |

The functions only forward and are inlined everywhere.  Their counts are constants the compiler folds:
`if (SIZE::CheckSize && ...)` costs nothing where it is false.  Where [`size`] is 16 bits, an array that would need
more than `MAX_SIZE` bytes fails as if out of memory, and a debug build says why ([`CArrayBlockBase::Overflowed`]:
make it a large array).

## The array templates

Each family is a template whose second parameter is the class it derives from, with the old names kept by classes
that derive from it and add nothing:

| Template | Derives from | Natural | Large |
|---|---|---|---|
| [`CArrayBlockBase`]`<SIZE>` | [`CMalloc`], [`SArrayHeaderBase`] | [`CArrayBlock`] | [`CArrayLargeBlock`] |
| [`CArrayTemplateBase`]`<M, BLOCK>` | `BLOCK` | [`CArrayTemplate`]`<M>` | [`CArrayLargeTemplate`]`<M>` |
| [`CArrayTemplatePrimitiveBase`]`<M, TEMPLATE>` | `TEMPLATE` | [`CArrayTemplatePrimitive`]`<M>` | [`CArrayLargeTemplatePrimitive`]`<M>` |
| [`CArrayCharBase`]`<PRIMITIVE>` | `PRIMITIVE` | [`CArrayChar`] | [`CArrayLargeChar`] |
| [`CCharsBase`]`<STRING, TEXT>` | nothing: it holds a `TEXT` | [`CChars`] | [`CLargeChars`] |

The second parameter is the base class, not the size policy, so that the hierarchy is what it always was: a
[`CArrayTemplate`]`<M>` is a [`CArrayBlock`] and a [`CArrayTemplatePrimitive`]`<M>` is a [`CArrayTemplate`]`<M>`,
which code relies on (calling [`CArrayBlockBase::QuickSort`] on a typed array, passing a [`CArrayTemplate`] pointer
as a [`CArrayBlock`] pointer).  With the policy as the parameter, [`CArrayBlock`] would be a sibling of
[`CArrayTemplate`]`<M>`, not its base.

The names are classes, not typedefs, because a typedef can't be declared ahead and `class CChars;` is declared ahead
in many headers.  The cost is that a function of the template that returns its own type returns the template's:
[`CCharsBase`] takes `STRING`, the class deriving from it, so that its functions take and return [`CChars`] or
[`CLargeChars`] (`return (STRING*)this;`).

[`CArrayBlockBase`], [`CArrayCharBase`] and [`CCharsBase`] are instantiated in [`BaseLib/ArrayBlock.cpp`],
[`BaseLib/ArrayChar.cpp`] and [`BaseLib/Chars.cpp`]; [`CArrayTemplateBase`] and [`CArrayTemplatePrimitiveBase`] are
in their headers, since their functions are short forwarders that take the element type.  Each template has a
`Count` typedef (`typename SIZE::Count`, repeated in each class because a dependent base isn't looked in), and its
members, parameters and locals are `Count`.  Members of a dependent base are named with `using` declarations in the
class, so that both g++ and MSVC find them.

On the 65 targets a function's label is the template's: `_CCharsBase__T_C6CChars_C10CArrayChar__Init`, not
`_CChars__Init`.  Hand-written assembly that calls one ([`BaseLib/FatVolumeW65C816.asm`]) names that label, and
TestCppCompilerLib's `TestCPP816AssemblyReplaces` checks that the labels it calls exist.

### Growing and shrinking

An array's memory comes from its [`CMallocator`] ([`gcSystemAllocator`] unless it is given one).  It grows by a
chunk that gets larger as the array does ([`CalculateChunkSize`]): 1 element at first, 32 at 64, 486 at 1458, then a
third of the elements past 182,250, so a large array is copied a few times rather than once a chunk.  The chunk size
never gets smaller.  The array shrinks to the chunk that holds what is left as elements are removed, and frees its
memory when it is empty.

All of it goes through [`CArrayBlockBase::SetArraySize`].  When it can't grow (a realloc that fails, or more than
`MAX_SIZE` bytes where [`size`] is 16 bits) it changes nothing and returns false: the elements, the counts and the
memory are as they were.  Every function that grows passes the failure on as it already did for no memory: a NULL
pointer, a not found index, or false.  A shrink whose realloc fails keeps the bigger memory.

A pointer to an element lasts only until the array next changes.  [`CListBlock`] is for elements that must stay
where they are, [`CListVariable`] for elements of different sizes.

## Strings

[`CChars`] is a string kept in a [`CArrayChar`], NULL terminated; [`CLargeChars`] is the same over a
[`CArrayLargeChar`], and also appends natural strings.  Counting a character is [`CCharsBase::Occurrences(char)`]:
the name `Count` is the count type's.  [`CArrayChars`] is an array of [`CChars`].

## Hash maps and sets

Every other map in BaseLib is ordered: [`CMapBlock`] and [`CSetBlock`] (a sorted array, each element its own
allocation), the [`CIndexTree`] trie and the red-black tree.  The hash map gives O(1) lookups, and replaced eight
hash tables written by hand across the compiler, SupportLib and StandardLib.

### Layout

[`CHashBlockBase`]`<SIZE>` ([`CHashBlock`], [`CHashLargeBlock`]) knows only entry sizes and offsets:

- The entries are in one array in the order they were added: its hash (a [`uint32`], first), its key and its value.
  Iterating is indexing it, and an entry's index is its own until something is removed.
- The table holds only an entry's index + 1 (0 an empty slot), found by linear probing from the hash's low bits. It
  is at most half full and doubles when it would be fuller; growing places the entries by their kept hashes, so no
  key is hashed or read again.  A slot is a `Count`, so 2 bytes on the W65C816.
- A lookup compares the kept hash first and the key only when the hashes match.  Keys are compared by their bytes
  unless the map has a [`HashEquals`].
- Removing an entry shifts the following slots back (no tombstones) and moves the last entry into its place: O(1),
  with no holes to skip.  After a remove the order is no longer the order added, and the moved entry's index
  changes.
- A pointer to an entry, key or value lasts only until the next Put or Remove.  Something that must last keeps a
  pointer as the value.
- The entries' memory doubles as it fills, copied rather than realloced, so a failed Put leaves the map as it was.
  It is given back by ReInit and Kill, not as entries are removed.
- [`CHashBlockBase::Write`] writes the entries; [`CHashBlockBase::Read`] reads them and builds the table again.

The order added is the point of the layout: iteration is the same on every build whatever the table's size, which
matters because the compiler's output is proved byte-identical between builds.  Three of the hand-written tables it
replaced had arrived at the same layout.

### The templates

| Class | Key | Value |
|---|---|---|
| [`CHashMapTemplate`]`<K, V, HASH, SIZE>` | `K`, by pointer | `V`, zeroed when added |
| [`CHashSetTemplate`]`<K, HASH, SIZE>` | `K` | none |
| [`CHashMapPrimitiveTemplate`]`<K, V>` ([`CHashMapIntTemplate`], [`CHashMapLongTemplate`]) | an integer, enum or pointer, by value | `V` |
| [`CHashMapString`]`<V, SIZE, HASH>` | text, copied into the map | `V` |
| [`CHashSetString`]`<SIZE, HASH>` | text, copied: interning, an index for each | none |

The templates are inline: they hash the key with `HASH::Hash` and pass the hash in, so the hash is inlined and the
block makes one indirect call, to `HASH::Equals`, only when two hashes match.  [`CHashMapIntTemplate`] and
[`CHashMapLongTemplate`] are drop-ins for [`CMapIntTemplate`] and [`CMapLongTemplate`].  The text maps keep their
copies in a [`CMemoryStackExtended`] ([`CHashStringBase`]); a key's text lasts until the map is killed, even after
the key is removed.

A `HASH` says what a key hashes to and how two are compared:

- [`SHashOf`] (the default) is for a key whose bytes are all it is: up to 4 bytes mixed by [`HashMix32`], up to 8 by
  [`HashMix64`], more by [`HashBytes`] (FNV-1a with its high bits folded down).  The table takes the hash's low
  bits, and an address's low bits are always 0, so the mix matters.  A struct key with padding needs a `HASH` of its
  own: padding isn't cleared.
- [`SHashText`] is a key that is text kept somewhere else, a pointer and a length, which must outlive the map.
  [`SHashOfText`] hashes and compares it exactly, [`SHashOfTextIgnoreCase`] with ASCII letters in either case alike,
  [`SHashOfFileName`] with either slash alike too.

## The queue, the heap and the union-find

[`CQueueTemplate`]`<M, BLOCK>` is a [`CArrayTemplateBase`] and a front index: pushed at the back, taken or dropped
at the front, cut from the back ([`CQueueTemplate::RemoveBack`], [`CQueueTemplate::RemoveFrom`]).  Taking the front
moves the index on; when the dead elements before it outnumber the live ones, and are at least
`QUEUE_TEMPLATE_MIN_COMPACT` (8), one RemoveRange moves the live ones down, so taking the front is amortised O(1)
and the queue stays one array in order.  It isn't a ring: no code found pushes at the front, and a ring's contents
wrap past the end of its memory, which an array's growth doesn't know to move.  It has no maximum; a bounded history
drops its front at the caller, which knows what dropping an element costs (UndoStack, the IDE's navigation history
and recent edits, LogiCim's navigation history, the HCP65 debug port's events).  A new element gets its virtual
function table but isn't zeroed.

[`BinaryHeapPush`] and [`BinaryHeapPop`] keep a min-heap in a [`CArrayTemplate`]: the array is the heap's only
state.

[`CUnionFindBase`]`<SIZE>` ([`CUnionFind`], [`CUnionFindLarge`]) is a parent array with path halving.
[`CUnionFindBase::Join`] makes the lower root the other's parent, so a set's root is its lowest element (as KiCad's
netlist and the mesh charts want); [`CUnionFindBase::JoinToFirst`] keeps the first element's root, as LogiCim's
circuit wiring needs, since which node is a root decides the net numbers and the order of the traces made.

[`CArrayBit::CountSet`], [`CArrayBit::CountClear`], [`CArrayBit::FindSet`] and [`CArrayBit::FindClear`] use the bit
scans in [`BaseLib/IntegerHelper.h`] ([`CountBits`], [`FindNextSetBitLarge`]) rather than a Get per bit.

## The red-black tree

[`CRedBlackTree`] is Logicim's RedBlackTree ported: fixed size elements, smallest key first, each element holding
its own key at an offset and compared by a [`DataCompare`].  Keys are unique.  [`CRedBlackTreeTemplate`]`<M>` types
it. MicroprocessorLib's timeline keeps its events in one ([`CSimultaneousEventsTree`]).

The nodes are in a [`CFreeList`], each element's data after its [`SRedBlackNode`], and an element stays at its
address until it is removed.  Logicim removed a node with two children by copying its successor's object into it and
swapped objects while rebalancing; here the successor's node is moved into the removed node's place instead
([`CRedBlackTree::RemoveNode`]), so no element moves.  So removing an element other than the one being iterated
doesn't disturb an iteration.  Adding keeps Logicim's trinode [`CRedBlackTree::FixDoubleRed`]; removing is the
textbook (CLRS) fix-up rather than Logicim's double black bubbling.  The checker is [`CRedBlackTree::IsValid`]:
`Validate` is a BaseLib macro.

## Decisions

- The array classes made by including one text twice between macros (`ARRAY_BLOCK`, `CHARS` and the rest) became
  real templates, so the HCP65 IDE can index and refactor them: code a macro makes has no source of its own (the
  user, 2026-11-15).  Done in five steps the same day; sizeof every array class, and the cycles and bytes of every
  65 test program, were the same before and after.
- The converted text compiles under both g++ and MSVC: `typename` and `using` (or `this->`) wherever a template
  needs them (the user, 2026-11-15).
- Where the CPP65 compiler didn't honour something the templates need (`extern template`, explicit function template
  instantiation, `static const` members as constants), the compiler was fixed, not worked around (the user,
  2026-11-15).
- An array's shrink and free behaviour stays as it is (the user, 2026-11-15).
- The Class and Code `.inl` files were folded into the `.h` and `.cpp` that included them (the user's choice,
  2026-11-15).
- Each template has a `Count` typedef, and [`CChars`]'s `Count(char)` became [`CCharsBase::Occurrences(char)`] for
  it (the user's choice, 2026-11-15); a typedef of a type doesn't conflict with the IDE's rule that instances are
  named by deriving classes.
- On the 65C64 one difference remains from the conversion: [`CCharsBase::AppendData`] inlines four callees fewer,
  because the policy's forwarders each take one of the 32 calls the 65C64 inlines into a function
  (TestIndexTreeEvicting 416 cycles more, +0.0007%).  The user, 2026-11-15: leave it, it isn't significant.
- A hash map and a set, the set being the map with no value; after a Remove the last entry moves into the gap;
  pointers last until the next Put or Remove; the kept hash is a [`uint32`] on every target, so a map written on a
  PC reads on the W65C816; at most half full; no multimap mode (the user, 2026-11-23, taking every recommendation).
- WeakFroms and MapUnknownUnknown don't need a consistent order (the user, 2026-11-23).
- The queue is [`CQueueTemplate`], the bounded histories use it too though they gain no speed (each says what it
  is), and it compacts when the dead front outnumbers the live elements and is at least 8 (the user, 2026-11-23).
- A failed grow leaves an array as it was, rather than emptying it and leaking its memory (the user, 2026-11-23; it
  was found on the W65C816, where memory runs out before `MAX_SIZE`).
- The red-black tree's elements never move, and its removal is CLRS's rather than Logicim's (at the port,
  2026-10-23).

The array templates' plan, [`BaseLib/ArrayTemplatesPlan.txt`], is finished and kept only as the record a test cites;
the hash map's, the queue's, the array grow fix's, the union-find's and the bit scans' is `BaseLib/HashMapPlan.txt`, in
the history up to CommonLibraries dbc0e9d8f.

## Not done

- Left as they are on purpose: StandardLib's CPointerLocations (a multimap: a key with many values, iterated by
  key), MapUnknownUnknown (keys compared by content), InputLib's device copy maps, SFTWriter's map, VRMLReader and
  ImageCombiner (small, not hot), UndoLog (it removes from anywhere, which a queue doesn't) and
  ObjectsRemovalContext (an embedded array that doesn't allocate).
- Sorted maps iterated by key or ID (the Codabase's records, the profiler, PropertiesFile, ChunkFile) stay sorted:
  their order may be what their output relies on.
- No ring buffer or deque that pushes at the front, until something needs one; no Write and Read for the queue.
- No and, or or xor of two bit arrays: nothing was found that needs them.
- The hash maps have no CMapAccess-style family (PutIntInt and the rest).
