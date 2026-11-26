# Memory

Everything in BaseLib that allocates does it through a [`CMallocator`], given to it when it is initialised: an
array, a map or a list holds one in its [`CMalloc`] base and never calls [`malloc`] itself.  So a container can be
pointed at a pool, a counting or tracking allocator, a scratch pad, or a memory that knows the container's
allocation pattern, without changing its code.

## Mallocators

A [`CMallocator`] has Malloc, Realloc and Free, a short name, and says whether it is local: one owned by a container
([`CLocalMallocator`], which can write and read its settings) or a global one.  [`CMallocators`] ([`gcMallocators`])
registers them by short name, so a container written to a file names its mallocator and gets the same one back when
it is read ([`CMallocators::Write`], [`CMallocators::Read`]).

| Mallocator | What it is |
|---|---|
| [`CSystemAllocator`] ([`gcSystemAllocator`]) | The default for every container.  On the hosts [`CThreadMallocator`]; on the 65C64 and the W65C816 Libc65's [`malloc`]; and [`malloc`] in an AddressSanitizer build, so that ASan and LeakSanitizer see the blocks (or when `SYSTEM_ALLOCATOR_USES_MALLOC` is defined, to run under valgrind). |
| [`CGlobalMemoryAllocator`] ([`gcMemoryAllocator`]) | The global [`CGeneralMemory`], [`gcMemory`], set up by [`MemoryInit`]. |
| [`CMemoryAllocator`] | A [`CGeneralMemory`] of its own. |
| [`CDataMemoryAllocator`] | A [`CDataMemory`] of its own (each [`CMarkup`] has one). |
| [`CSlabAllocator`] ([`gcSlabAllocator`]) | Whole 64KB slabs, each on a 64KB boundary. |
| [`CReuseAllocator`] | One buffer's allocator: keeps the block it gave out, so a string emptied and filled over and over is allocated once. |
| [`CCountingAllocator`], [`CTrackingAllocator`], [`CNullAllocator`], [`CScratchPadAllocator`], [`CFreeListAllocator`] | Counting, leak tracking, nothing, a scratch pad, one free list. |

## The general and data memories

[`CGeneralMemory`] and [`CDataMemory`] sit behind many small allocations of similar sizes (the Codabase's objects,
markup trees, maps of strings).  Each has a free list for each size class ([`CMemoryFreeListParams`]: element sizes
up to a limit, each class a range of sizes), and puts anything larger in a linked list of its own.  An allocation
has a small header before it ([`SGeneralMemoryAllocation`]: its size, an allocation count for breaking on the n-th,
and four characters of debug name): 24 bytes on 64 bit and 16 on 32 bit, padded so that 8 byte aligned elements have
8 byte aligned data.

[`CFreeList`] keeps fixed size elements in nodes, a bit for each element saying whether it is used.  Its counts are
kept, not walked: a node has its used count and is on a list of nodes that aren't full, so Add, Remove,
[`CFreeList::NumElements`] and [`CFreeList::HasElements`] are O(1), and the first clear bit is found with a count
trailing zeros instruction ([`FindFirstClearBit(uint32)`]).

[`CGeneralMemory`]'s free lists put each node in a 64KB slab from [`gcSlabAllocator`] ([`CSlabAllocator`]:
`VirtualAlloc` on Windows, `posix_memalign` on Linux), so the node an allocation is in is found by masking its
address ([`CFreeList::GetSlabNode`]) and the allocation's header needs no pointer to it.  A free list for a size
class is found through an index by the class's params, not a search.  [`CDataMemory`] isn't slabbed: there is one
per markup, map of strings or variable list, and a slab for each size class would cost each of them 64KB per class.
Empty free lists are kept until [`CGeneralMemory::Compact`] or [`CDataMemory::Compact`] frees them.

## CTrueMallocator

[`CTrueMallocator`] is a general purpose allocator over regions of memory it is given: a two level segregated fit
(TLSF) allocator.  A free block is on one of a list for each size range, found from two levels of bitmaps (the power
of two of its size, then which of 16 parts of that power), so Malloc and Free take the same time however many blocks
there are.  Blocks have boundary tags (the previous block's size while it is free, and the block's own size with two
flag bits), so a freed block is joined to free blocks either side of it at once, and each region ends in a used
block of no size.  Blocks are aligned to 8 bytes (16 in a 64 bit build), and the largest is 1GB, so sizes and their
rounding fit in 32 bits.

- Malloc rounds the size up to the next list's, so any block on the list it finds fits.  If no list from there on
  has one, it looks through the request's own list (which can hold smaller blocks) for one that is big enough, so
  that what [`CTrueMallocator::LargestFree`] says can be had can be.
- Realloc shrinks in place, and grows in place when the block after it is free and big enough.
- [`CTrueMallocator::MallocWithin`] is Malloc for memory that mustn't cross a boundary in its addresses: a W65C816
  bank, which an index register doesn't count past.
- [`CTrueMallocator::IsValid`] walks every block, list and bitmap.

It uses nothing else from BaseLib but [`BaseLib/Atomic.h`], so a program that only uses Libc65 links it alone, and
it isn't a [`CMallocator`] for the same reason.  It isn't safe between threads.  Three allocators are built on it:

- Libc65's [`malloc`] on the 65C64.  On the W65C816 it is [`CBankMallocator`]: a [`CTrueMallocator`] region for each
  64KB bank that small blocks are in, so no block crosses a bank, and runs of whole banks for what doesn't fit one.
  Banks are asked of a [`CBankSource`] as they are needed (under the HCP65 kernal, mapped into the program by system
  calls) and given back as they empty, but for a spare.
- [`CThreadMallocator`] ([`gcThreadMallocator`]), the hosts' system allocator: an arena per thread, each a
  [`CTrueMallocator`] over 4MB chunks from the operating system, so a thread allocates without a lock.  In front of
  each arena are lists of freed blocks by size, a push and a pop with no joining or splitting.  A block's arena is
  found from its address (chunks are aligned and name their arena); a block freed by another thread goes on its
  arena's remote list with a compare and exchange.  Blocks too big for an arena are each the operating system's, and
  the last few freed are kept for the next of about the same size, since getting memory from the operating system
  costs far more than anything else here.

## What profiling found

TestBaseLib and TestStandardLib were profiled on Linux (64 bit, `-O2`, with `perf`) on 2026-10-21.  Measured then,
64 bit `-O2`, nanoseconds an operation before and after the free list rework: bulk FIFO frees 67 to 44, churn 35 to
16, ping-pong 49 to 14, realloc 205 to 124, a growing heap 1715 and 3005 to 49 and 39.  A 40 byte allocation costs
64 bytes (it was 81); the price is that a tiny heap takes a 64KB slab for each size class it uses.

- [`CountBits`] counted bit by bit; it counts 32 bit words with a population count.  [`CFreeList::HasElements`], run
  on every [`CGeneralMemory`] free, used it: FIFO frees became twice as fast before the rework made it O(1).
- The free lists' counts and not-full list (above), the index of free lists by params, and the slabs, which took the
  node pointer out of every allocation's header.
- Pre-existing bugs found on the way and fixed: growing a large allocation left out its header (in both memories);
  [`CGeneralMemory`]'s FreeFreeList rebuilt its order key wrongly when the header wasn't a multiple of the
  alignment; [`CIndexTreeDataNode`]'s RemapListNode never moved the orderer's head and tail.
- System time was larger than user time in TestStandardLib (1.8 million system calls, 169,000 after; the suite 4.4 s
  to 1.3 s): check `strace -c` as well as `perf`.  [`CSequence`] hands out indexes in blocks (`SEQUENCE_BLOCK_SIZE`,
  256), writing "reserved up to" once a block rather than once an index; [`CFileUtil::TouchDir`] stats a directory
  that exists instead of making it; [`CLogFile`] grows its last write in place, doubling ([`CListVariable::Grow`]),
  instead of joining writes with a [`memmove`] each time.

Measured 2026-10-24 on the 65C64 simulator, in instructions: [`CTrueMallocator`] took TestStandardLib65's
TestArrayObject from over 60G to 23.4G, TestSetObject from over 60G to 20.7G (with a word-wise [`memmove`]) and
TestMapObject from 28G to 11.3G.  TestSetUnknown and TestCompressionZLib barely changed: their time is in free list
bit scans and [`memcpy`].

## Decisions

- [`CTrueMallocator`] is standalone, not a [`CMallocator`], so that a program using only Libc65 doesn't pull in the
  rest of BaseLib (Claude's choice, 2026-10-24, not ruled on by the user; the user had asked for "a proper
  TrueMallocator (malloc) in BaseLib that Libc65 malloc can use").  A [`CMallocator`] over it could be added.
- The free list rework, in four parts (O(1) counts, the params index, slabs, empty lists kept until Compact), was
  the user's choice of the options profiling offered (2026-10-21).
- [`CDataMemory`] isn't slabbed: a slab per size class is too much for one memory per container (2026-10-21).
- A debugging walk of the evicted data written for the descriptor cache bug (WriteEvictedData3) was removed with all
  its pass-throughs at the user's request (2026-10-21).

## Not done

- StandardLib's CIterableList::Remove is a linear scan (128 microseconds a Kill at 200,000 objects), but only test
  classes are iterable: left for the user to decide (2026-10-21).
- TestStandardLib65 still leaves out four tests that take more than its 4G instructions a program
  (TestCompressionZLib, TestArrayObject, TestSetObject, TestMapObject), and two more on the W65C816 (TestSetUnknown,
  TestMapUnknownUnknown): [`Tests/TestStandardLib65/TestStandardLib65/TestStandardLib65Tests.cpp`].
