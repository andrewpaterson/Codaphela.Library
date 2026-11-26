# Sound

A sound is a [`CSound`], made as an image is: frames of samples, as an image's pixels are of channels, one sample a
channel in each frame, interleaved in a StandardLib [`CChannels`].  Sounds are read from and written to WAVE files,
resampled, cut into cels that share one sound, and played by a ProTracker module player that runs on the host and
on the HCP65's W65C816.  Playing sounds through the computer's speakers is WindowLib's: a window with a
[`CNativeSoundFactory`] has a [`CSoundMixer`] ([WindowLib](../windowlib/index.md)).

## Sounds

A [`CSound`] has a sample rate ([`CSound::GetSampleRate`], a [`uint32`]), a number of frames and channels, each of
any primitive type.  It is made by one of its `Init`s or changed between [`CSound::BeginChange`] and
[`CSound::EndChange`], as [`CImage`] is ([Images and Drawing](images.md#images)), and changed further by modifiers
on a [`CSoundModifierStack`], in place so that everything pointing at the sound hears the change.

Samples are read and written as [`float64`] in [-1, 1) whatever their type ([`CSound::GetSample`],
[`CSound::SetSamples`]):

| Type | -1.0 | Silence | Top |
|---|---|---|---|
| Signed integers | the most negative (int16 -32768) | 0 | just under 1.0 |
| Unsigned integers | 0 | half their range (uint8 128) | just under 1.0 |
| Bit sized (crumb, tribble, nybble, nickle, sixbits) | 0 | half their range (nybble 8) | nybble 15 is 0.875 |
| A bit | 0 | none | 1 is 1.0 |
| Floats | not scaled | 0.0 | not scaled |

[`CChannels`]' own type conversion is for images (int16 to float is x / 65535), so sounds convert their own
([`ConvertSampleToFloat`], [`ConvertSampleFromFloat`]).  Samples needn't be byte aligned: bit sized and packed
channels (two nybbles a frame) are read and written bit by bit, the first channel in the low bits; byte aligned
channels are faster.  A new sound's data is zero bytes, which is silence only for signed and float samples:
[`CSound::Silence`] silences any sound.

Channels are named by speaker ([`ESoundSpeaker`], in WAVE channel mask order, [`SupportLib/SoundChannel.h`]'s
`SOUND_FRONT_LEFT` and the rest; mono is a speaker but has no mask bit), by number for channels with no position
(`SOUND_DISCRETE`) or as padding (`SOUND_IGNORED`).  Their purposes ([`ESoundPurpose`]) start at 0xF0, above every
image purpose so a sound channel never shares a number with an image channel, and below 0x100 so a channel fits the
W65C816's 16 bit [`size`].

## Cels

A [`CSoundCel`] is a run of frames in a sound, as an image cel is a rectangle of an image: where a sample starts,
how long it is and, if it loops, the frame it goes back to, counted from the cel's start.  It is an entry of the
SNES's sample directory; [`CSoundCel::Play`] wraps at the end to the loop.  A [`CSoundCelGroup`] is the sound bank
and its directory: one sound holding many back to back, and a cel for each found by index
([`CSoundCelGroup::AddSound`] copies a sound onto the bank's end, converting it to the bank's sample types).

## WAVE files

[`LoadWAV`] reads PCM of 8 (unsigned), 16, 24 and 32 bits and floats of 32 and 64, plain or
`WAVE_FORMAT_EXTENSIBLE`.  Samples keep their type, except that 24 bit samples are read into the top three bytes of
an [`int32`].  Channels are named from the channel mask, or by the usual layouts for the count without one.  A data
chunk that runs past the file's end is cut to the whole frames there.

[`SaveWAV`] writes the format asked for ([`EWAVSampleFormat`]) or, by default, the one the sound's type suggests: 8
bit and bit sized as PCM 8, 16 bit as PCM 16, 32 bit integers as PCM 32, floats as themselves.

## Resampling

[`CSoundResampler`] is a sound modifier, as [`CImageResampler`] is an image modifier.  It makes the sound at
another rate by nearest neighbour, linear, cubic (Catmull-Rom) or Kaiser windowed sinc ([`ESoundResample`]). Output
frame i is taken from source time i × source rate / new rate, so the first frames line up, and a sound already at
the rate is returned as it is.

The sinc filter has a Kaiser β of 9, 32 zero crossings each side and a cut-off at 0.95 of the lower Nyquist
frequency, so going down in rate also filters out what the new rate can't hold.  Its taps are tabled per phase when
phases × taps is at most 1,048,576.  On a 1 kHz sine its error is under 1e-5 RMS (about 3e-6 when written).  Past
the ends linear and cubic hold the end samples, and sinc reads silence.

## MOD player

The ProTracker module player plays a [`CModSong`] read by [`LoadMOD`]:

- [`LoadMOD`] reads 31 sample modules signed "M.K." and the other four and more channel signatures ("M!K!", "FLT4",
  "xCHN", "xxCH", "TDZx", "CD81", "OKTA" and others), and 15 sample Soundtracker modules with none.  FLT8, which
  splits its patterns in two, isn't read.
- A song's samples are one [`int8`] bank ([`CSoundCelGroup`]), a cel per sample with its loop; a sample longer than
  its loop's end is cut there.

[`CModPlayer`] plays a song as ProTracker 2.3D does, as read from Olav Sørensen's pt2-clone, whose period table it
copies with credit: speed and tempo (CIA) timing, every effect, finetune, and 8xx and E8x panning, which ProTracker
ignores.  Where trackers disagree it follows ProTracker:

- a note delay (EDx) sets the sample, volume and period on tick 0 and only starts the sample on tick x;
- periods are moved onto the table even at finetune 0;
- tremolo never changes the stored volume;
- F00 stops the song (libopenmpt ignores it).

The song ends when it would play a row it has already played (rows played inside a pattern loop don't count).
Channels are panned left, right, right, left as on the Amiga, scaled by the stereo separation.

It renders 16 bit stereo ([`CModPlayer::Render(int16*,lsize)`]), floats, the HCP65 sound board's bytes
([`CModPlayer::Render8`]) or a whole song as a sound ([`CModPlayer::RenderSong`], which first plays the song
through without mixing to find its length, then renders into the sound a block at a time). [`CModPlayer::SkipTick`]
plays a tick without mixing.

### Fixed point mixing

The mixer is fixed point on every target, so the host and the W65C816 make the same samples bit for bit:

- samples are Q8 (an 8 bit sample shifted up 8), gains Q12 (4096 is 1.0), positions a frame and a 16 bit fraction,
  and steps 16.16, worked out as a step numerator over the period;
- a frame of the mix is 16 bits, the sum of each channel's sample × gain >> 13 (`MOD_MIX_ONE`, 16384, is full
  scale).  The channels' gains add up to at most 2.0, so the sum never overflows;
- a channel's step and gains are cached and worked out again only when its period, volume or pan changes;
- an 8 bit mono bank, a MOD's, is read where it is rather than copied (Space Debris's samples as floats would be
  2.4 MB); other banks are copied as 16 bit samples.

Nearest sampling of 8 bit samples, as the Amiga plays them, goes through [`SupportLib/ModMixer.h`], written for the
W65C816's code generator:

- each side of each channel has a gain table ([`SModGainTable`]): the 256 values of sample × gain, built with adds
  only ([`ModBuildGainTable`], exact for every gain) and rebuilt only when the gain changes;
- [`ModMixNearest8`] adds a table entry per frame into a 256 frame block, reading the sample through a window of up
  to 64 KB ([`SModSampleWindow`]) at a 16 bit offset, at most `MOD_SAMPLE_WINDOW` (0xF000) frames at a time;
- a side whose gain is 0 isn't mixed, so with full stereo separation each channel is one pass;
- the tables, blocks and windows are arrays in structs under 64 KB, which the W65C816 indexes with 16 bit Y
  subscripts; a pointer indexed would add 32 bit addresses ([W65C816 Back End](../../compiler/design/w65c816.md));
- [`ModMixToUnsigned8`] turns the mix into the sound board's unsigned bytes, left and right apart
  ([`SModUnsigned8`], the board's RAM A and RAM B), the same as the 16 bit samples' high bytes plus $80.

### On the HCP65

HCP65's `modplay.exe` ([`ModPlay/ModPlay.cpp`]) plays Space Debris from the SD card through the sound board at
8,192 Hz: it renders 256 frame blocks with [`CModPlayer::Render8`], writes them through the kernal into the board's
buffer of 32 K frames (left in its RAM A, right in RAM B), which it plays round and round, starts the board after
four blocks ([`ModPlay/ModPlaySamples.h`]), and sleeps until the board's interrupt says there is room.  The board's
DAC is 8 bits: 16 was too expensive.

On 2026-10-31 rendering took 55 M W65C816 cycles per second of audio at 8,192 Hz with the first fixed point mixer
and about 11 M after the tables, cached steps and windows, so a 25 MHz HCP65 keeps up
([`TestHCP65Lib/TestHCP65Kernal.cpp`] with `TEST_HCP65_CLOCK=25000000 TEST_MODPLAY_SECONDS=10`).

## Tests

| Test | Covers |
|---|---|
| [`TestSupportLib/TestSound.cpp`] | making and changing sounds, sample conversion, silence, bit sized and packed samples |
| [`TestSupportLib/TestSoundCel.cpp`] | cels, loops, banks |
| [`TestSupportLib/TestSoundWAV.cpp`] | reading every format, bad files, writing, channel order, bit sized |
| [`TestSupportLib/TestSoundResampler.cpp`] | frame counts, sines, anti-aliasing, edges, the modifier stack |
| [`TestSupportLib/TestModPlayer.cpp`] | reading, periods, effects, finetune, stereo, a tune |

The WAVE inputs are made by `Input/WAV/MakeWAV.py` (RIFF by hand and Python's wave module), the modules by
`Input/MOD/MakeMOD.py`, all original, including the "Codaphela Tune".  The tune's test compares a fingerprint of
each row's RMS level (`Tune-levels-expected.txt`); each of its channels alone was checked against libopenmpt 0.7.3
when written (libopenmpt pans MOD channels 75/25, so compare one channel at a time).  The tune renders at 32 kHz in
16 bits because Debug builds stop at allocations over 4 MB.  TestSupportLib65 leaves TestModPlayer out: it takes
9.8 G 65C64 instructions, more than its 4 G; it passes on the W65C816 when given longer.

## Decisions

- Sound lives in SupportLib, not a library of its own; samples are interleaved channels, a frame like a pixel; the
  sample rate is a [`uint32`] (the user, 2026-10-23).
- The resampler is a sound modifier, following [`CImageResampler`] and [`CImageModifierStack`] (the user,
  2026-10-23).
- Bit sized and packed samples kept, though the HCP65's DAC turned out to be 8 bits (the user, 2026-10-23).  The
  DAC's samples are two's complement [`int8`], 0 silence and -128 -1.0 (the user, 2026-10-23); the sound board
  itself takes unsigned bytes with $80 the middle ([`SModUnsigned8`]).
- A MOD player first, XM later (the user asked for "a MOD (or XM) player", 2026-10-23).  It follows ProTracker
  2.3D, not libopenmpt, where they differ.
- Fixed point mixing on every target (the user, 2026-10-31).
- The mixer stays C++ that the code generator can optimise, no assembly (the user, 2026-10-31).

## Not done

- [`CModPlayer::Render(int16*,lsize)`] doubles the 16 bit mix, so its lowest bit is always clear: one bit less than
  full precision, inaudible (noted 2026-10-31).
- XM modules.
- The Amiga's LED filter and its resampling; E3x glissando, EFx funk repeat, and Soundtracker's own effects.
