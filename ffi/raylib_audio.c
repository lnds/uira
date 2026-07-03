/*
 * Audio shim — reduced to the one thing the FFI can't express.
 *
 * Sound and Music now cross the FFI by value (their AudioStream
 * pointers ride along as opaque words), so LoadSound / PlaySound /
 * the whole streaming API bind straight to raylib. What stays here is
 * gen_tone: synthesising a tone means allocating a sample buffer and
 * filling it, which needs a real C `short *`. It returns the Sound by
 * value, ABI-identical to the one kaikai declares.
 *
 * This file includes raylib.h (it needs the Wave struct and
 * LoadSoundFromWave). The emitted code must NOT include raylib.h —
 * that would clash with kaikai's generated typedefs — which is why
 * gen_tone lives here rather than as a direct bind.
 */

#include <raylib.h>
#include <stdint.h>
#include <math.h>

/* Generate a `seconds`-long mono sine at `freq` Hz, 22050 Hz / 16-bit,
 * with a 5 ms attack/release ramp, and load it as a Sound. Returns an
 * empty Sound (frameCount 0) on failure — every play/stop call on it
 * is a harmless no-op in raylib. */
Sound kai_raylib_gen_tone(float freq, float seconds, float amp) {
  Sound empty = {0};
  const unsigned int rate = 22050;
  unsigned int frames = (unsigned int)(seconds * (float)rate);
  if (frames == 0) return empty;

  short *samples = (short *)MemAlloc((unsigned int)(frames * sizeof(short)));
  if (!samples) return empty;

  unsigned int ramp = rate / 200; /* ~5 ms */
  if (ramp * 2 > frames) ramp = frames / 2;

  for (unsigned int n = 0; n < frames; n++) {
    float env = 1.0f;
    if (n < ramp)                 env = (float)n / (float)ramp;
    else if (n >= frames - ramp)  env = (float)(frames - n) / (float)ramp;
    float s = sinf(6.2831853f * freq * (float)n / (float)rate);
    samples[n] = (short)(s * env * amp * 32767.0f);
  }

  Wave w;
  w.frameCount = frames;
  w.sampleRate = rate;
  w.sampleSize = 16;
  w.channels   = 1;
  w.data       = samples;

  Sound snd = LoadSoundFromWave(w);
  MemFree(samples);
  return snd;
}
