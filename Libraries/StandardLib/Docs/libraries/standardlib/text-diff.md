# Text Diff

StandardLib compares two texts as IntelliJ IDEA's diff does: the changed blocks of lines, and inside each changed
block the words or characters that changed.  It always works from the full text of both sides, wherever they came
from, and never reads git's diff output or follows git's diff settings.  [The Diff View](../windowlib/diff-view.md)
shows what it produces side by side; the IDE's gutter change bars ([`CGitLineChanges`]) use it lines only.

## The classes

| Class | What it is (IntelliJ's name) |
|---|---|
| [`CTextContent`] | One side: a title and the text split into lines (DocumentContent). |
| [`CSimpleTextDiff`] | The result: two contents, the blocks and their fragments (SimpleDiffRequest). |
| [`CTextDiffProducer`] | Fills a [`CSimpleTextDiff`] under its whitespace and highlight policies (ChangeDiffRequestProducer). |
| [`CTextComparison`] | Tokenises, interns and compares lines, then a block's words or characters (ComparisonManager). |
| [`CMyersDiff`] | Myers' difference algorithm over sequences of ids. |
| [`CTextDiffLineMapper`] | Maps a line of one side to the other, for a view's scrolling (on the view's page). |

They are plain Init/Kill classes, not objects.  A view only reads a [`CSimpleTextDiff`]: its mutators are protected,
and [`CTextDiffProducer`] is its friend.  [`CTextDiffProducer::Produce`] recomputes a diff whose contents are already
set, so a view can change policy without reading the texts again; [`CTextDiffProducer::ProduceFromText`] and
[`CTextDiffProducer::ProduceFromFiles`] set the contents first.  A file that can't be read is an empty side, and the
diff is still produced.

## Lines

[`CTextContent`] splits at LF, CRLF or a lone CR and keeps each line's separator ([`ETextLineSeparator`]) so a view
can show it.  A line doesn't include its separator, and text ending in a separator has no empty line after it:
"a\n" and "a" are both one line, the second's separator [`TLS_None`]; empty text has no lines.

## Myers' algorithm

[`CMyersDiff`] is the O(ND) algorithm from Myers' 1986 paper in linear space (the middle snake bisection), as
IntelliJ and git use by default.  It compares sequences of ids, so whatever is compared (lines, words, characters)
is first interned to ids.  Common prefixes and suffixes are trimmed before each bisection, and the halves left to
compare are kept on an explicit stack, not the call stack, so very different sequences can't overflow it.  Changes
come out in order as [`SDiffRange`]s; touching changes from the same [`CMyersDiff::Diff`] are merged, but never with
changes that were in the array before the call (the fragments of earlier blocks).

## Interning

A token ([`STextToken`]) is a line, a word, a whitespace run, a punctuation character or a line separator.  What it
compares as isn't always the text it covers: every separator compares as "\n", and a trimmed line covers its whole
line.  Each token is hashed with FNV-1a as it is made (skipping whitespace when whitespace is ignored, and inverted
for an unterminated last line), and [`CTextComparison::Intern`] gives every token of both sides the index of the
first equal token, through a BaseLib [`CHashMapTemplate`] keyed by those hashes with [`STextToken::Equals`] deciding.
Myers then compares integers, whatever the tokens were.

## Blocks, then fragments

[`CTextComparison::CompareLines`] interns each side's lines under the whitespace policy and diffs them: the result is
the blocks, which are inserted, deleted or modified ([`ETextDiffBlockType`]).

| Whitespace ([`ETextDiffWhitespace`]) | Lines compare | Line separators inside a block | A missing final newline |
|---|---|---|---|
| [`TDW_Default`] | exactly | tokens (any kind equals any other) | a difference |
| [`TDW_Trim`] | without leading and trailing whitespace | ignored | not a difference |
| [`TDW_Ignore`] | without any whitespace | ignored | not a difference |

Whitespace is spaces, tabs, form feeds and vertical tabs ([`IsTextDiffWhitespace`]); line separators are never inside
a line.  Under [`TDW_Default`] the separators are tokens of a block, so text moved across a line break shows.

Then, unless the highlight is [`TDH_Lines`], each modified block (one with lines on both sides) is compared
again ([`CTextComparison::CompareFragments`]) to find its fragments ([`STextDiffFragment`]: character offsets into
each side's whole text; an empty side is the point inserted at or deleted from):

- [`TDH_Chars`]: a token for each character (whitespace left out under [`TDW_Ignore`]).
- [`TDH_Words`], the default: words (letters, digits, underscores and the bytes of UTF-8 sequences,
  [`IsTextDiffWordChar`]), whitespace runs (left out under [`TDW_Ignore`]) and single punctuation characters.  Words
  are compared whole, as IntelliJ does: a changed word is highlighted whole, never "the last letter of it".  Then two
  passes over the word fragments:
    1. Gaps ([`CTextComparison::RefineGaps`]): a fragment that on both sides is only whitespace and punctuation (no
       word characters, no line breaks) is compared again by character, so `a   b` to `a b` highlights the two
       spaces that went, not the whole run.
    2. Squash ([`CTextComparison::Squash`]): fragments separated only by whitespace on both sides are joined into one,
       so `x = a b;` to `x = c d;` is one change `a b` to `c d`, not two.  Line breaks aren't whitespace here, so
       fragments on different lines stay apart.

A block with more than `TEXT_DIFF_MAX_REFINE_LENGTH` (20000) characters on either side
([`StandardLib/TextDiffPolicies.h`]) gets no fragments, only its lines.  Myers is O(ND) and character highlighting
takes a token per character, 40 bytes each on the 64-bit hosts and 24 on 32-bit (sizeof, 2026-11-26).

## Printing

[`CSimpleTextDiff::Print`] writes the diff as a unified diff with no context lines and zero-based line numbers (the
blocks'), each fragment after its block as `~[left]>[right]`, and a missing final newline as git marks it.  It is for
reading and for the tests, not a patch:

```text
--- A
+++ B
@@ -0,1 +0,1 @@
-x = a b;
+x = c d;
~[a b]>[c d]
```

## Tests

[`TestStandardLib/TestTextDiff.cpp`]: lines, separators, a missing final newline, each whitespace policy, words, gaps,
squash, characters, offsets, block types, the refine limit, many lines, files (`Input/TextDiff`), and Myers on its
edges, reused, and on random sequences checked to be a minimal edit (n + m - 2 LCS, against a dynamic programming
table).  Gap refinement, squash and unterminated lines were checked by mutation when written (2026-11-05).  The line
mapper's tests are there too.

## Decisions

- Modelled on IntelliJ: the full text of both sides is fetched and diffed here, git's diff output and settings
  ignored (2026-11-05).
- Plain Init/Kill classes, not objects; Myers O(ND) in its linear-space middle-snake form with an explicit stack; the
  first step covers line blocks and word and character fragments (the user's choices, 2026-11-05).
- Words compared whole, then gaps refined by character and neighbours squashed, as IntelliJ shows them (2026-11-05).
- Under [`TDW_Default`] line separators are tokens and a missing final newline is a difference; under
  [`TDW_Trim`] and [`TDW_Ignore`] both are whitespace (2026-11-05).
- A view only reads [`CSimpleTextDiff`]; only [`CTextDiffProducer`] changes it (2026-11-05).

## Not done

- A git revision as a source inside [`CTextDiffProducer`]: the IDE reads revisions itself
  ([`CGitRepository::ReadRevisionText`], [`CGitRepository::ReadHeadText`]) and passes text, so only another user
  would need it.
- The line pass has no size limit: two large, wholly different texts cost Myers' O(ND) in full.
- Blocks over the refine limit show lines only, with no hint in the result that fragments were skipped.
