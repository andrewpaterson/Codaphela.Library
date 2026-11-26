# The Codabase

The Codabase ([`CCodabase`]) is BaseLib's object database: records of any size kept by a 64 bit index ([`OIndex`]),
some of them also by a unique name, in files that survive a crash part way through a write.  It has write and
read-only transactions with locks and snapshots, and several Codabases can be stacked into one connection
([`CLayeredConnection`]) so a game can change a level it only reads.  StandardLib's object system ([`CObjects`]) keeps
objects in it: it writes each object as one record and reads it back when a pointer to it is used.

The Codabase knows nothing about objects; it stores bytes.  How objects live in memory (pointers, the root, distances
to the root, hollows, freeing) is on [Objects](../standardlib/objects.md); this page is about how they get to the
database and back.

Today only the tests use a database-backed [`CObjects`].  Every application (LogiCim, the HCP65 emulator and tester,
the IDE, WindowInputTester) calls [`ObjectsInit(void)`] with no data connection and saves in formats of its own.

## What it is for

The user wants one database for three uses, often at the same time:

| Use | Example | Needs |
|---|---|---|
| Virtual memory for objects | A game in limited RAM pages its changed map out to disk and back in | No transactions, no write-through, no durability |
| A document store | The level an artist saved; the game never changes it | Opened read-only |
| A transactional (ACID) database | A store changed only at transaction boundaries | Locks, snapshots, durable commits |

## Layers

From the disk up:

| Layer | Classes | Does |
|---|---|---|
| Durable files | [`CDurableFile`], [`CLogFile`] | A file with a primary and a backup copy.  Writes between Begin and End are logged in memory ([`CLogFile`]) and reach the files only at End. |
| Durable bracket | [`CDurableFileController`], [`CDurableSet`] | Begin, End, Abort and Recover for every durable file in one directory pair. |
| Index trees | [`CIndexTreeFile`], [`CIndexTreeEvicting`] | Keyed trees on disk with a cache in memory that evicts what wasn't used lately. |
| Indexed data | [`CIndexedData`], [`CIndexedDescriptorsFile`], [`CIndexedFilesEvicting`], [`CIndexedFiles`] | [`OIndex`] to record: a tree from index to descriptor (record size and where it is), the records in files of fixed-size slots (generally one file per record size), a data cache. |
| Names | [`CNamedIndexes`] | A tree from name to [`OIndex`]. |
| Named indexed data | [`CNamedIndexedData`] | Both together.  A record starts with a [`CNamedIndexedHeader`]: the name's length (4 bytes on every CPU) and the name, then the data. |
| The Codabase | [`CCodabase`] | One store mutex, the durable bracket, the direct session, transactions, old versions and locks. |
| Connections | [`CDataReader`], [`CDataConnection`] | What callers see: Get, Contains, GetName, GetIndex and iteration; [`CDataConnection`] adds Add, Set, Put, Remove and Flush.  [`CCodabase`], [`CWriteTransaction`] and [`CLayeredConnection`] are connections; [`CReadTransaction`] is a reader. |
| Sequences | [`CSequence`], [`CSequenceConnection`] | Hand out new [`OIndex`]es.  Kept apart from the Codabase: [`CObjects`] takes a sequence and a data connection. |

A Codabase directory holds `Primary` and `Backup`, each with `Index` (the records and the index tree) and `Names`.
[`CCodabaseFactory::Create`] makes one with caches by access order ([`ITESST_Access`]):

| Cache | Host | W65C816 | [`CCodabaseFactory::CreateSmall`] |
|---|---|---|---|
| Data | 16 MB | 2 MB | 256 KB |
| Index | 8 MB | 1 MB | 256 KB |
| Names | 1 MB | 256 KB | 64 KB |

The small one is for a Codabase that holds little, such as a layered connection's tombstones: the 65C64's test heap
didn't fit two full-sized ones.

### Starting, opening and closing

```{.cpp .no-check}
pcSequence = CSequenceFactory::Create("Output/Store", DS_No);
pcDatabase = CCodabaseFactory::Create("Output/Store", IWT_No, DS_No);
pcDatabase->Open();
ObjectsInit(pcDatabase, pcSequence);
...
ObjectsFlush();
ObjectsKill();
pcDatabase->Close();
SafeKill(pcDatabase);
SafeKill(pcSequence);
```

[`CCodabase::Init`] does no disk work.  [`CCodabase::Start`] recovers a commit a crash interrupted and then loads the
store; [`CCodabaseFactory::Create`] calls both and returns NULL if Start fails.  [`CSequence::Init`] recovers too, so an
interrupted sequence write never hands out an index twice.

A durable bracket is always open while a Codabase is started, because even reading the store needs one: Start begins
it, and every commit is flush, End, Begin.  [`CCodabase::Kill`] aborts the open bracket
([`CDurableFileController::Abort`]), so nothing uncommitted reaches the disk, as in a crash.

[`CCodabase::Open`] starts the direct session and [`CCodabase::Close`] commits it (see
[the direct session](#the-direct-session)).  Without a session, direct reads see the committed store and direct writes
fail.  Writes made in a session are kept in its write set, in memory, until Close: [`CCodabase::Flush`] flushes the
store's caches, not the session.

[`EDurableSync`] is given to each Codabase and sequence, not set globally: [`DS_Yes`] fsyncs every commit, [`DS_No`]
skips the fsyncs but not the marks or recovery.  [`DS_No`] is for tests that don't need durability (TestStandardLib
took 374 s with fsyncs and 108 s without).

## Writing an object

[`CObjects::ForceSave`] writes an object (the object it is embedded in, if it is embedded) with a
[`CInternalObjectSerialiser`]: a [`CObjectWriter`] writes it into memory and the result is Put into the data
connection by index, or by index and name if the object is named.  A successful save marks the object stored
([`CBaseObject::SetStored`]).

A record ([`CSerialisedObject`] maps onto its start) is:

| Part | Holds |
|---|---|
| Length | 4 bytes, the length of the object's part |
| `OBJECT_DATA` | Magic |
| Header | The identifier (type, [`OIndex`], and the name if it is named) and the class name |
| Fields | What the class's Save writes ([`CBaseObject::SaveManaged`]).  A pointer is the identifier of the object it points into, the number of objects embedded in that object and which of them it points to; NULL is a NULL identifier. |
| Length, `OBJECT_FROM_HEAP` | The froms part |
| Froms | Each embedded object's froms, then the number of holders and each holder's identifier ([`CBaseObject::SaveHeapFroms`]) |

Distances to the root are not saved: they describe objects in memory and are worked out there.  The holders saved are
only those in memory when the object was written, so **a stored from list can miss holders** that were never loaded.

## Reading an object

[`CObjects::GetPossiblyHollow(const char*)`] looks for a name in memory, then in the database, then in the external
object sources ([`CObjects::GetFromSources`]).  An object found by [`OIndex`] (a hollow being used,
[`CObjects::Dehollow(OIndex)`]) is looked for in memory and then in the database; sources are found by name only.

[`CInternalObjectDeserialiser`] reads the record into a [`CRecordMemory`] (most fit in 2 KB on the stack) and a
[`CObjectReader`] makes the object.  Each pointer becomes the object in memory, or a hollow for it, loaded when it is
used.  The froms are read and a hollow is made for each stored holder that isn't in memory, but they are not linked
to the object: what holds a loaded object in memory is only what is loaded.  An object that reads is marked stored; one
that fails to read leaves a hollow in its place.

A pointer to an [`OIndex`] the database doesn't have is a programming error: it is logged and the pointer doesn't load.

An object converted from an external source must be named what it was asked for, since that name is how objects in
the Codabase refer to it: [`CObjects::GetFromSources`] logs, frees and returns NULL for one that isn't.

## Flushing and freeing

An object is stored once it has been read from or written to the database ([`CBaseObject::IsStored`]).  An object
loaded by name alone, with its holders only in the database, can't find the root, but it is stored, so it is still
saved.  [`CBaseObject::NeedsSaving`] is the one rule:

> dirty, and stored or able to find the root.

| When | What happens to a dirty object |
|---|---|
| [`ObjectsFlush`] ([`CObjects::Flush`]) | Each that needs saving is saved and marked clean, then the connection is flushed.  With no data connection, dirty objects that can find the root are just marked clean. |
| Freed ([`CObjects::RemoveBatch`]: garbage collection, Kill) | A stored one is saved first ([`CObjects::SaveStoredDirty`]).  One that never reached the database is freed unsaved. |
| [`CObjects::EvictInMemory`] | Everything that needs saving is saved ([`CObjects::SaveDirtyInMemory`]), then everything is freed. |
| [`ObjectsKill`] | Nothing is saved.  [`CObjects::ValidateNoDirty`] logs any object that still needs saving: [`ObjectsFlush`] or [`ObjectsDiscard`] (every object marked clean, changes thrown away) comes first. |
| Write-through ([`IWT_Yes`]) | [`CObjects::LogicalDirty`] saves each object as soon as it changes. |

Pointer fields and collections mark their object dirty when they change ([`CCollection::MembersChanged`]).
Unmanaged fields need [`CBaseObject::SetDirty`] by hand.

### Kill and Destroy

| | Kill ([`CBaseObject::Kill`]) | Destroy ([`CBaseObject::Destroy`], [`CObjects::Destroy(CArrayOIndex*)`]) |
|---|---|---|
| Memory | Freed | Freed |
| Database | Unchanged | The record and its name are removed |
| A stored object's holders in memory | Point to a hollow that loads it again ([`CObjects::ReplaceWithHollow`]); the holders are not changed or made dirty | Loaded, the pointers cleared and saved |
| What it held | Unchanged | Loaded, saved without it as a holder; left in the database (no cascade) for the database's garbage collection |
| An object never stored | Freed, the pointers to it cleared | Killed |

Destroy reads the holders from the object's record ([`CObjects::GetStoredHeapFroms`]), loads them and what the
object points to, marks those dirty, clears the destroyed objects' pointers, kills them, saves the changed objects,
lets anything only they held be freed, and removes the records.  Destroying a set destroys pointers between its
members without saving them.  The root can't be destroyed.  [`CObjects::LoadForward`] loads everything an object
reaches, breadth first, up to a limit, to make such a set.

## Layered connections

[`CLayeredConnection`] stacks Codabases into the one [`CDataConnection`] that [`CObjects`] sees.  The top layer is a
writable Codabase; the layers below are read-only Codabases that are never opened.

- An object is read from the highest layer holding it; the same [`OIndex`] in two layers means the top one.
- Writes, including saving before a free, go to the writable layer.  A changed lower-layer object is copied up with
  its name and keeps its [`OIndex`], since references to it in a read-only layer can't be changed.
- Removing an object a lower layer holds puts a tombstone in a second, small "Removed" Codabase (a byte per object),
  so it doesn't reappear.  Names need no tombstones: a name found in a lower layer counts only if no higher layer holds
  (or removed) its [`OIndex`].
- The layered connection owns the writable sequence.  [`CLayeredConnection::AddReadOnly`] moves it above each
  read-only layer's own sequence ([`CSequence::AdvanceTo`] the layer's [`CSequence::PeekNext`]), so a new object never
  hides one below.
- A sequence can reserve a range ([`CSequence::ReserveRange`]), written to its file at once and never handed out by
  GetNext; a range-limited sequence ([`CSequenceFactory::CreateRange`]) hands out only from one range.  The plan: a
  released level reserves a range for the game, and the game's sequence (virtual memory and save games) numbers inside
  it, so a later update to the level can't collide with a save.

| Use | Layers, top first | Top layer at the end |
|---|---|---|
| Game | virtual memory (writable, not durable), level | Discarded at shutdown |
| Artist editing a level | level (writable) | Kept |
| Transaction | write set (spillable), base | Commit merges it into the base; abort drops it |

The game case has three stores: the level (unchanging, read-only), the virtual memory database (only the objects changed
since the level was loaded, discarded at shutdown) and save games.  A save game is a serialisation into its own
database of what is reachable from a list of named roots: unchanged level objects are written as references, changed
and new ones are copied, garbage never reaches a save.  Saves depend on a level's [`OIndex`]es staying put, so editing
a level must never renumber its objects.

The design and what is still open are in [`BaseLib/CodabaseLayers.txt`].

## Transactions

The transactions are built in BaseLib (2026-10-17 to 2026-10-19).  [`CObjects`] doesn't use them: it writes through
the direct session, which is an implicit write transaction.

### The guarantee

Strict serialisability for everything read or written by [`OIndex`]: the transactions run as if one at a time, in an
order that agrees with real time.  Read-only transactions are covered fully, names and iteration too, because they read
a snapshot.  Two things are not covered in this first version: a write transaction that looks up a name, finds
nothing and acts on it, and objects another write transaction inserts while a write transaction iterates (phantoms).

### Read and write transactions

| | Read ([`CCodabase::BeginReadTransaction`], [`CReadTransaction`]) | Write ([`CCodabase::BeginWriteTransaction`], [`CWriteTransaction`]) |
|---|---|---|
| Reads | The snapshot: the last commit published when it began | The latest committed version, after locking it, never a snapshot (that would allow write skew) |
| Locks | None; never waits, never aborts | Every object it reads or writes, held until the commit is durable (rigorous two-phase locking) |
| Changes | None | Private in a write set until Commit; Abort, or Kill before Commit, discards them |
| Interface | [`CDataReader`] | [`CDataConnection`] plus Commit and Abort, so code written for a connection runs on one unchanged |

The Codabase allocates transactions and Kill releases them.  Transactions use the Codabase only through its methods
(Stored..., Snapshot..., LockObject, ReserveName, [`CCodabase::CommitWriteSet`]), never its members.

Write transactions run at the same time.  A transaction that conflicts with an older one dies: its changes are thrown
away, its locks and names released, and the call that found the conflict returns false.
[`CWriteTransaction::HasDied`] tells that from other failures and [`CWriteTransaction::Restart`] begins it again with
its original start number, so it gets older and eventually succeeds.

### The direct session

[`CCodabase::Open`] starts an implicit write transaction with start number 0, so it is always the oldest: it waits
but never dies.  Only one session is open at a time; a second Open waits.  Direct calls between Open and Close go
through the session from any thread (a session mutex serialises them), and Close commits it.  NumIndices, NumNames and
the cache counts include what the session has added.  A thread with a session can't begin a write transaction, and a
thread with a write transaction can't Open: one would wait for the other for ever.

### Commit

[`CWriteTransaction::Commit`] hands its write set to [`CCodabase::CommitWriteSet`], which holds the store mutex for the
whole commit, so commits run one at a time:

1. Nothing can fail on a name: names were checked and reserved when they were added.  This matters because a durable
   bracket can't be rolled back; whatever is written inside it is committed by End.
2. Removals are applied before additions and changes, so a name taken from one object can be given to another.
   Before each change, if a read transaction is open, the store's current version (or "did not exist") is kept in the
   version store, valid until the new commit number.
3. Flush, End, Begin the next bracket.  The cache never holds unwritten changes between commits, so eviction only drops
   clean data.
4. The commit number is published, only now: any transaction begun after Commit returns sees the commit.
5. The transaction releases its name reservations and locks.

The store mutex is held through End's fsyncs, so read transactions and direct reads wait while a commit is written
(never for an uncommitted writer).  Every commit costs fsyncs; a write-ahead log could make it cheaper if needed.  If a
commit fails part way (a full disk, a failed write) the Codabase refuses writes until it is closed and started again,
and Start recovers from the marks.

### Names

Two transactions adding the same name: the second fails when it adds it, not at commit.  The Codabase keeps a table
of names being added by uncommitted transactions.  Adding a name fails at once if it is committed or reserved by
another transaction; it never waits, so it can't deadlock and isn't part of the lock manager.  Commit moves the name
into the store; abort frees it.  If the first transaction then aborts, the second has failed for a name that turned
out free; it can retry.

### Locks

[`CLockManager`] locks by [`OIndex`] only; names aren't locked.  A new object needs no lock: its index comes fresh
from the sequence and no other transaction knows it.

| Requested \ Held | Shared | Update | Exclusive |
|---|---|---|---|
| Shared ([`LM_Shared`]: "I read it") | yes | yes | no |
| Update ([`LM_Update`]: "I read it and will probably write it") | yes | no | no |
| Exclusive ([`LM_Exclusive`]: "I am writing it") | no | no | no |

Update lets readers carry on but lets only one transaction hold it, so two readers can't both upgrade and deadlock.
The caller chooses the mode; by default shared to read and exclusive to write.

Deadlocks are prevented by wait-die on start numbers (lower is older).  A requester older than every conflicting
holder waits; otherwise it dies ([`LR_Died`]).  An upgrade checks only the other holders.  Incompatible waiters
already queued count as conflicts, so a waiting exclusive lock isn't overtaken for ever by new shared ones; a waiting
upgrade goes to the front.  Each waiter waits on a condition variable of its own, woken only when its entry changes.
Locks belong to transactions, not threads, so a thread pool could serve one client's requests on any thread.

### Old versions

[`CCodabaseVersions`] keeps old versions in two maps in memory beside the store, [`OIndex`] to a chain of
[`CVersion`] (the record, or "did not exist") and name to a chain of [`SNameVersion`], each marked with the commit that
replaced it.  A reader at snapshot S takes the first version replaced after S, or the store's if there is none (the
usual case).  Versions are kept only while a read transaction is open, and dropped when no open snapshot needs them.
They are not durable: no transaction survives a restart.  A second durable indexed store wasn't used for them, since
those have brackets, backups and recovery that temporary versions don't need.

Iterating a transaction ([`CTransactionIteration`]) lists the indices or names visible to it when the iteration
starts, sorted and once each, and returns their data as of its snapshot (or with its changes).  One index iteration
and one name iteration at a time per transaction.  Iterating the Codabase directly is locked per call, so it may miss
or see changes other threads make between calls.

### Durability

[`CDurableSet::End`] is a double-write commit: write the start mark, write and fsync the primary files, write the
rewrite mark, write the backup files, delete the marks (the rewrite mark first).  The start mark lists the primary and
backup name of every file in the commit, ending `#End`, and is fsynced before any data is written; Recover copies
those files primary to backup or backup to primary, so it needs no other file list.  A start mark without `#End` was
cut short before any primary file was written.  The files, the marks and their directories are fsynced and every
result is checked.  The crash tests in [`TestDurableFileController.cpp`] stop End at every step and check Recover gives
either the old files or the new ones.

### Threads and processes

[`CCodabase`] holds one store mutex for each call; nothing below it is thread safe, and even Get changes the caches.
One process owns a Codabase; others are meant to reach it by IPC into that process, so the lock and name tables live
in ordinary memory.  [`CNativeThread`] and [`CConditionVariable`] moved from ThreadLib into BaseLib for this.

The design, step by step, is in [`BaseLib/CodabaseTransactions.txt`].  Every step in it is built; it is kept because
[`BaseLib/CodabaseLayers.txt`], [`StandardLib/ThreadSafetyPlan.txt`] and CommonLibraries' CLAUDE.md cite it.

## Tests

| Suite | Covers |
|---|---|
| [`TestDurableFileController.cpp`], [`TestDurableSet.cpp`] | The bracket, recovery, crashes at every step of End |
| [`TestCodabase.cpp`], [`TestSequence.cpp`] | The store and sequences |
| [`TestWriteTransaction.cpp`], [`TestReadTransaction.cpp`], [`TestTransactionConcurrency.cpp`] | Transactions, snapshots, locks, wait-die, names, a bank transfer stress test |
| [`TestLayeredConnection.cpp`], [`TestLayeredObjects.cpp`] | Layers, tombstones, sequences; objects over layers, Destroy through them |
| [`TestStoredObjects.cpp`] | The stored flag: flushing and freeing without a path to the root, eviction, Kill's checks, Kill leaving a hollow, Destroy |

## Decisions

- **Fully ACID transactions on the Codabase** (the user, 2026-10-17).  The old ones (CoreLib's Transaction,
  SubTransaction, TransactionController, a database per transaction holding copies of what others wrote) were removed
  in 2016 (commits b17537b5, fa534136): writers had to copy old values into every concurrent reader's database and
  phantoms couldn't be prevented.  Now there is one store of old versions and a private write set per transaction.
- **Read-only or write transactions; writers lock pessimistically, per object, by [`OIndex`] only** (the user,
  2026-10-17).  Optimistic validation for writers is not to be proposed again.
- **The second transaction to create a name fails when it creates it**, through a reservation table that never waits
  (the user, 2026-10-17, confirmed).
- **Threads only: one process owns a Codabase**, others reach it by IPC; no shared memory (the user, 2026-10-17).
- **Wait-die, with shared, update and exclusive modes** chosen by the caller (the user, 2026-10-17).
- **MVCC with strict serialisability** (the user, 2026-10-17); the first version's guarantee covers only what is
  accessed by [`OIndex`], not writers' failed name lookups or iteration phantoms (the user decided).
- **Old versions in two maps in memory, not a second indexed store**; spilling to disk only when needed (the user,
  2026-10-17).
- **No nested transactions** (the user: the old SubTransaction was a bad idea).  **[`CObjects`] is not in scope for the
  first version**; name edge cases (remove and re-add, uncommitted remove) are ignored for now (2026-10-17).
- **The direct API is one implicit write transaction from Open to Close**, keeping today's behaviour (the user,
  2026-10-17); it is the oldest under wait-die and waits, never dies (2026-10-18).
- **The write-through modes are left alone** in the first version: the user is unsure they are still worth having, and
  [`IWT_Yes`] also drives [`CObjects::LogicalDirty`] (2026-10-17).
- **fsync is chosen per Codabase and sequence ([`EDurableSync`]), not globally** (the user, 2026-10-17).
- **A durable bracket is always open; a commit that fails part way leaves the Codabase read-only until restarted**
  (2026-10-18).  [`CNativeThread`] and [`CConditionVariable`] moved into BaseLib (2026-10-18).
- **The cache counts and NumIndices include the session's changes** (the user chose, 2026-10-18).
- **Lists of [`OIndex`] are [`CArrayOIndex`]**, not [`CArrayInt64`] (the user, 2026-10-19).
- **A dirty object is flushed before it is freed** (the user, 2026-11-05).  The other option was to accept that
  freeing an unsaved object discards it, with write-through as the way to keep changes.  Only garbage (objects nothing
  in the store can reach) may be dropped unsaved.  This became a problem once loaded objects stopped taking their
  distance to the root from the file: before, they leaked until Flush, which saved them.
- **Objects that reached the database are stored, and stored objects are saved without a path to the root**
  (option A, the user, 2026-11-06; commits dff91473f, bbbe1f66a).  The others were loading the holders all the way to
  the root so the distance is real (the user's first intent, which the user called "potentially a terrible idea") and
  hollows with phantom pointers (rejected: every from must hold a pointer to the object, which
  [`CEmbeddedObject::ValidateFrom`] checks).  The user's concern stays true: many transient objects are made and
  collected without ever reaching the database, and they are freed as before.
- **Eviction saves dirty stored and rooted objects first** (the user, 2026-11-06).
- **[`ObjectsKill`] doesn't save: [`ObjectsFlush`] or [`ObjectsDiscard`] first**, and Kill reports what is still dirty
  (the user, 2026-11-06).
- **Kill frees from memory; Destroy deletes from the database** (the user, 2026-11-06).  Destroy deletes only the
  object; what it held is left to the database's garbage collection, not destroyed with it (the user, who had leaned
  towards destroying it too).  A stored object's holders keep pointing at it through a hollow, not NULL, and Kill
  doesn't make them dirty.
- **A pointer to an [`OIndex`] the database lacks is a programming error**: logged and not loaded, not made NULL (the
  user, 2026-11-06).
- **Every object converted from an external source is named**, its relative path with its extension (the user,
  2026-11-06): that is how Codabase objects refer to it.
- **Layering is one [`CDataConnection`], not a list of connections in [`CObjects`]** (option A, the user,
  2026-11-06); the writable layer is a second [`CCodabase`], not a write set that spills; the top layer wins; names
  resolve top down.  It lives below [`CObjects`] so that saving before a free stays "write to the connection".  The
  external sources couldn't hold the databases: they are found by name only and nothing is written to them.
- **Lookup is memory, the writable database, the read-only databases, then external sources; a changed object keeps
  its [`OIndex`]** (2026-11-06).
- **No database ID in the [`OIndex`]** (decided 2026-11-05, dropped by the user 2026-11-06 as "not worth it"): the
  top 8 bits would only have said where an object was made, and the lookup order finds it without them.  An
  [`OIndex`] stays a plain sequence counter.
- **The layered connection owns one sequence, started above every lower layer's own; sequences reserve ranges for
  the game's sub-sequence** (the user, 2026-11-06).  **Tombstones go in a second small Codabase** (the user,
  2026-11-06).
- **Save games: their own database, written by serialisation of what is reachable from a list of named roots**,
  unchanged level objects as references, changed and new ones copied (the user, 2026-11-05).
- **Database garbage collection** (the user, 2026-11-05): named objects are not roots; a subroot (typically a large
  store; a saved flag on collections) is always alive and is a collection boundary; collection runs on demand,
  offline, and may take days over tens of billions of objects, so a full mark and sweep in one go isn't practical.

## Not done

The user's open questions:

- **What a read-only database does with a refused write.**  The options: fail when the object is marked dirty
  (earliest, pointing at the code making the change); fail at flush or free and discard the change; keep the dirty
  object in memory (it can't be freed); allow changes in memory only, a freed dirty object losing its change.  With
  layering the game never writes to the level, so a refused write should only happen on a bug.  No read-only mode
  exists yet.
- **Whether new objects in a read-only session are an error**, or allowed until one has to be written.
- **Whether the write-through modes go**, or [`IWT_Yes`] becomes "autocommit per change" (both plans ask).
- **How a read-only transaction gets an object view in [`CObjects`]**: the snapshot gives the right bytes, but they must
  be read into a read-only view of the transaction's own, not the shared object graph (wanted for threads with a
  view of their own: level 3 of [`StandardLib/ThreadSafetyPlan.txt`]).
- **Protecting write transactions' name lookups and iteration** (a second version).
- **The database's garbage collection**: whether records gain a list of the indices they point to (so collection
  walks without deserialising) and drop the unreliable from list, perhaps with a version flag in the record header;
  how a subroot is freed; whether a run covers the whole database or one subroot's store (which needs the references
  coming in from other stores); a checkpointed walk with its marks on disk, resumable after a crash.  Claude's ideas,
  not decided: reference counts in the database, kept up by comparing to-lists at save, with trial deletion for
  cycles among suspects only; collection per subroot store with a record of references crossing a boundary.
- **Save games**, proposed with them: transient pointer fields (say `M_TransientPointer`) the save walk doesn't follow,
  loading as NULL; loading a save as a read-only layer between the virtual memory layer and the level, so loading
  needs no replay.
- **Finding dirty objects without SetDirty** (the user liked it, not decided): hash each object's unmanaged fields
  when it is loaded or saved and re-hash clean ones at flush and free; or a debug-only check that catches a missed
  SetDirty.

Not built:

- No application stores objects in a Codabase, and [`CObjects`] doesn't use transactions.
- A write set doesn't spill: a session keeps everything [`CObjects`] saves in memory until Close, so flushing before a
  free doesn't yet free memory for the virtual memory use.  A writable layer with no durability (no brackets, no
  fsync) isn't built either.
- Save games, and a level reserving and recording its range for the game.
- Nothing deletes garbage from the database; Destroy is the only delete, and nothing reads
  [`CCollection::IsSubRoot`] yet.
- Stored from lists miss holders that weren't loaded when an object was saved, so Destroy can miss a holder, which then
  fails to load the destroyed object later.  Not tested.
- Nothing stops a second process opening the same Codabase directory; an exclusive lock at Init would.
- IPC: no front end; when there is one, each transaction belongs to its client's session, closing or losing the session
  aborts them, and remote lock requests should time out.
- [`CFileUtil::SyncDirectory`] does nothing on Windows (it has no documented way to flush a directory), and a newly
  made directory isn't synced into its parent.
- Old versions grow with a long read transaction beside heavy writing; they could move to a [`CIndexTreeEvicting`] in
  a temporary directory if that matters.
- Readers could read through a commit if the store mutex were held per object rather than for the whole commit.
