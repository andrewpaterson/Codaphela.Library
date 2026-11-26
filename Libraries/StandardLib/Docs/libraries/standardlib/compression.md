# Deflate and PNG

The libraries read and write PNG images with their own code: deflate and inflate (zlib's format) in StandardLib,
the PNG reader and writer in SupportLib.  They replaced ImageLib, a copy of libpng 1.2 and zlib, which was deleted on
2026-10-16 (CommonLibraries 3904a75b1) because the user wants no third-party image or compression code in the
libraries.  Being our own code, they also compile for the 65C64 and the W65C816: TestStandardLib65 and
TestSupportLib65 run the same compression and PNG tests there.

## Where each part lives

| Part | Classes | Files |
|---|---|---|
| Checksums | [`CAdler32`] (ends a zlib stream), [`CCRC32`] (PNG chunks) | [`StandardLib/Adler32.h`], [`StandardLib/CRC32.h`] |
| Bits | [`CBitReader`], [`CBitWriter`]: least significant bit first, as deflate packs them | [`StandardLib/BitReader.h`], [`StandardLib/BitWriter.h`] |
| Deflate's tables | Length and distance bases and extra bits, the fixed codes, [`DeflateReverseBits`] | [`StandardLib/DeflateCommon.h`] |
| Huffman codes | [`CHuffmanDecoder`], [`CHuffmanEncoder`]: canonical codes (RFC 1951 3.2.2) | [`StandardLib/HuffmanDecoder.h`], [`StandardLib/HuffmanEncoder.h`] |
| Inflate | [`CInflater`] | [`StandardLib/Inflater.h`] |
| Deflate | [`CDeflater`] | [`StandardLib/Deflater.h`] |
| Big-endian integers | [`CFileReader::ReadBigEndianInt32`], [`CFileWriter::WriteBigEndianInt32`] and the 16 bit ones | BaseLib |
| PNG | [`LoadPNG`], [`SavePNG`], [`PNGPaethPredictor`], [`PNGWriteChunk`] | [`SupportLib/PNGReader.h`], [`SupportLib/PNGWriter.h`], [`SupportLib/PNGCommon.h`] |

Compression is in StandardLib because it is general (any file can be deflated); only PNG is about images.

## Inflate

[`CInflater`] reads deflate data (RFC 1951) from a [`CFileReader`], raw or in a zlib stream (RFC 1950), and appends
what it decompresses to a [`CArrayLargeUint8`].  A zlib header must be a multiple of 31, method 8, a window of at
most 32 KB and no preset dictionary (PNG never uses one); the Adler-32 at the end is checked.  Stored, fixed and
dynamic blocks are all read.  Corrupt, truncated or over-long input fails rather than reading or writing out of
bounds: the caller can give the most output it will accept ([`CInflater::Init`]'s uiMaxOutput), which the PNG reader
sets to exactly the size the image's rows need.  A dynamic block's code lengths must make a complete code, except
that a literal or distance code of a single symbol is allowed.

[`CHuffmanDecoder`] decodes a code of up to 9 bits with one lookup in a table indexed by the next 9 bits read
(`HUFFMAN_FAST_BITS`); longer codes, and bits that aren't a code, are decoded a bit at a time.  [`CBitReader`] reads
a byte from the file only when it needs one, so after [`CBitReader::AlignToByte`] the file is at the next whole byte
(a stored block's length, the zlib trailer).

## Deflate

[`CDeflater`] compresses a whole buffer as one stream, raw or with a zlib header (78 9C: a 32 KB window, the default
level).  Matches are found with hash chains over three bytes (2^15 heads) and lazy matching: a match is taken only if
the one starting at the next byte isn't longer.  Its limits are zlib's level 6: 128 chain links at most, fewer once
the match is 8 long, stop at 128, don't look further after 16; a three-byte match more than 4096 back is written as
literals, as it costs more.  Each block of 16384 symbols is written stored, with the fixed code or with its own
dynamic codes, whichever counts fewest bits.  [`CHuffmanEncoder`] builds code lengths by joining the two lightest
nodes, ties to the lowest node, and when a code would pass 15 bits flattens the frequencies and builds again; at
least two symbols always get a code, as some decoders reject a code of one.

So the same input always gives the same output, byte for byte, on every target: the test expected files depend on
it.  When written (2026-10-16) the output was within a few bytes of zlib's level 6, and the inflater and deflater
were checked against Python's zlib both ways and by a fuzz of about 26000 cases (the same inputs accepted and
rejected, the same output).

## PNG

[`LoadPNG`] reads grey, grey with alpha, RGB and RGBA images of any bit depth, and palette images, interlaced (Adam7)
or not.

- The file is read whole and its chunks walked; every chunk's CRC is checked.  Unknown ancillary chunks are skipped
  and unknown critical chunks fail.  The image data chunks are gathered and inflated to exactly the size the rows
  need.  An image may have at most 8192 x 8192 pixels, as compressed data can claim a huge image in a tiny file.
- 1, 2 and 4 bit grey is read as 8 bit; other images keep their bit depth.
- A palette is expanded to red, green, blue and alpha (alpha from tRNS), or to grey if every entry is grey, as the BMP
  reader does; an index past the palette's end is opaque black.  A tRNS chunk on a grey or RGB image is ignored, as
  the libpng-based reader ignored it.
- Opacity is the first channel.  The pixels go into the image through [`CImageCopier`], as the old reader's did, so
  the channel types and scaling are what they were.

[`SavePNG`] writes RGBA, RGB, grey with alpha or grey, whichever holds the image's channels, 16 bits a sample if any
written channel is 16 bits or wider, else 8.  Each row takes the filter with the smallest sum of absolute values of
its filtered bytes (as signed bytes), the usual heuristic; with AVX2 built in the filters and sums run 32 bytes at a
time.  The data is deflated with zlib's wrapper and split into 8 KB IDAT chunks, as most encoders do.

## Tests

- [`TestStandardLib/TestCompressionZLib.cpp`]: the checksums, bits, inflating zlib streams written by zlib (stored,
  fixed and dynamic blocks) and raw ones, bad streams, deflate round trips and their matches, code edges, bit
  reversal, and more bits or data than the reader, writer or deflater can count refused.
- [`TestSupportLib/TestImagePNG.cpp`]: interlaced images, every filter, palettes, chunks, bad files
  (`Input/PNG/Bad`) and writing (`Input/PNG/Written`).  The input PNGs were made by a scratch script, not kept.
  TestImageWriter also round-trips palette images.
- TestFileIO's big-endian test, for the integers PNG chunks are made of.

The expected PNGs that other SupportLib tests compare with were written again by the new writer once PIL showed the
same pixels and headers as before (46 files, 2026-10-16).  Any change to the deflater's output bytes means writing
them again.

## Decisions

- No third-party image or compression code: libpng and zlib replaced by our own, and ImageLib deleted (the user,
  2026-10-16).
- Deflate in StandardLib, which any library can use; PNG in SupportLib beside the other image readers (2026-10-16).
- Deterministic output, so expected files compare byte for byte (2026-10-16).
- Reading behaves as the libpng-based reader did where images depended on it: the same import into the image's
  channels, tRNS ignored on grey and RGB images (2026-10-16).

## Not done

- 290 SupportLib source files (2026-11-26 count) still carry "libpng is Copyright" and "zlib is Copyright" lines in
  their licence headers, from when ImageLib was linked.
- No gzip reader or writer, though [`CCRC32`] is gzip's checksum too; no preset dictionaries.
- No streaming: [`CDeflater::Deflate`] takes the whole input at once and [`CInflater::Inflate`] the whole stream.
- The writer doesn't write palette, 1, 2 or 4 bit, or interlaced PNGs, nor ancillary chunks (gamma, text,
  transparency).
