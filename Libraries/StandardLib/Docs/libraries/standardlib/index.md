# StandardLib

StandardLib is the project's object system and what is built on it.  It sits on BaseLib (containers, text, files and
the [Codabase](../baselib/codabase.md), the database objects are stored in) and under everything else that keeps a
graph of objects: SupportLib's images, fonts and meshes, WindowLib's windows and components, MicroprocessorLib's
circuits, the HCP65 IDE, LogiCim and HCP65-Sculpt.  It compiles for the 65C64 and the W65C816 too, and its tests run
there (TestStandardLib65).

## The object system

An object is a [`CObject`], made with [`OMalloc`] and pointed at by [`Ptr`]s.  Each object knows what points at it,
and is freed the moment nothing reachable from the root ([`ORoot`]) or from the stack reaches it, in an order that
lets each `Free` still use what it needs.  Collections of objects ([`CArrayObject`], [`CSetObject`], [`CMapObject`],
[`CIndexObject`]) take part as pointer fields do.  Objects are saved to and loaded from the Codabase by index or by
name; an object in the database but not in memory is a hollow that loads it when used.

| Page | Covers |
|---|---|
| [Objects](objects.md) | objects and [`Ptr`], the root and the distance to it, holding from the stack, freeing and its order, where froms are kept, the unknown collections |
| [Weak References](weak-references.md) | [`Weak`] and [`CArrayWeakObject`]: finding objects without keeping them; makers of native resources as objects |
| [Objects and Threads](threads.md) | using objects from several threads: the lock, readers and writers, workers' own object systems |
| [Codabase](../baselib/codabase.md) | storing objects: saving, loading, hollows, flushing |

## Its parts

| Part | Main classes | Is |
|---|---|---|
| Objects | [`CObject`], [`CObjects`], [`Ptr`], [`CRoot`], [`CDistCalculator`], [`CKillOrderCalculator`] | the object system ([Objects](objects.md)) |
| Collections of objects | [`CArrayObject`], [`CSetObject`], [`CMapObject`], [`CIndexObject`], [`CArrayWeakObject`] | collections that hold objects as pointers do |
| Classes and fields | [`CClass`], [`CClasses`], [`CPointerField`] | what `Class()` declares: an object's pointers, embedded objects and data |
| Unknowns | [`CUnknown`], [`CUnknowns`], [`CArrayUnknown`], [`CMapStringUnknown`], [`CIndexUnknown`] | allocated things that aren't objects, and their collections ([Objects](objects.md#unknown-collections)) |
| Object files | [`CObjectWriter`], [`CExternalObjectSerialiser`], [`CNamedIndexedObjects`] | objects written to and read from files and the Codabase |
| Threads | [`ObjectsWorkerInit`], [`CObjects::Adopt`] | workers' object systems ([Objects and Threads](threads.md)) |
| Channels | [`CChannels`], [`CChannelsAccessor`] | interleaved channels of data, the pixel formats of SupportLib's images |
| Actions | [`CAction`], [`CMemberCalls`] | named events and the member functions called when they happen |
| Text diff | [`CMyersDiff`], [`CTextDiffProducer`], [`CTextContent`] | the IntelliJ-style diff ([Text Diff](text-diff.md)) |
| Compression | [`CDeflater`], [`CInflater`], [`CCRC32`], [`CAdler32`] | deflate, inflate and checksums ([Compression](compression.md)) |
| Ordering | [`COrderer`] | orders items so each comes after those that fulfil the names it requires |

A channel set ([`CChannels`]) checks a change's added channels only when it owns its data: one over data it was given
may name a channel twice on purpose (SupportLib's RGBA copy repeats its ignored channel).
