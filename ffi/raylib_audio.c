/*
 * Audio shim — like Font, raylib's Sound and Music can't cross the
 * FFI by value: both embed an AudioStream that holds pointers into
 * raudio's internals, so neither is a fixed-width `extern "C" type`.
 * We park loaded sounds / music in slot tables and expose scalar-only
 * entry points the kaikai bindings call.
 *
 * This file includes raylib.h (it needs LoadSound, PlayMusicStream,
 * the Wave struct, …). The emitted out.c must NOT include raylib.h —
 * that would clash with kaikai's generated typedefs — which is why
 * the audio entry points live here rather than as direct binds.
 *
 * Loading returns a slot index, or -1 when the table is full or the
 * asset is invalid. Every play/stop/volume call is a no-op on an
 * out-of-range or unloaded slot, so a -1 from load propagates
 * harmlessly through the call sites.
 *
 * gen_tone synthesises a sine wave straight to a Sound (no file on
 * disk) so the audiopad demo is self-contained. A short linear
 * attack/release envelope keeps note on/off from clicking.
 */

#include <raylib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#define KAI_RAYLIB_MAX_SOUNDS 32
#define KAI_RAYLIB_MAX_MUSIC   4

static Sound sounds[KAI_RAYLIB_MAX_SOUNDS];
static int   snd_loaded[KAI_RAYLIB_MAX_SOUNDS];

static Music music[KAI_RAYLIB_MAX_MUSIC];
static int   mus_loaded[KAI_RAYLIB_MAX_MUSIC];

/* ----- device ----- */

void kai_raylib_init_audio(void)  { InitAudioDevice(); }
void kai_raylib_close_audio(void) { CloseAudioDevice(); }
int64_t kai_raylib_audio_ready(void) { return IsAudioDeviceReady() ? 1 : 0; }
void kai_raylib_set_master_volume(float v) { SetMasterVolume(v); }

/* ----- sounds ----- */

static int snd_alloc(Sound s) {
  if (!IsSoundValid(s)) { UnloadSound(s); return -1; }
  for (int i = 0; i < KAI_RAYLIB_MAX_SOUNDS; i++) {
    if (!snd_loaded[i]) {
      sounds[i] = s;
      snd_loaded[i] = 1;
      return i;
    }
  }
  UnloadSound(s);
  return -1;
}

static int snd_ok(int64_t slot) {
  int i = (int)slot;
  return i >= 0 && i < KAI_RAYLIB_MAX_SOUNDS && snd_loaded[i];
}

int64_t kai_raylib_load_sound(const char *path) {
  return (int64_t)snd_alloc(LoadSound(path));
}

/* Generate a `seconds`-long mono sine at `freq` Hz, 22050 Hz / 16-bit,
 * with a 5 ms attack/release ramp, and load it as a Sound. */
int64_t kai_raylib_gen_tone(float freq, float seconds, float amp) {
  const unsigned int rate = 22050;
  unsigned int frames = (unsigned int)(seconds * (float)rate);
  if (frames == 0) return -1;

  short *samples = (short *)MemAlloc((unsigned int)(frames * sizeof(short)));
  if (!samples) return -1;

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
  return (int64_t)snd_alloc(snd);
}

void kai_raylib_play_sound(int64_t slot)  { if (snd_ok(slot)) PlaySound(sounds[(int)slot]); }
void kai_raylib_stop_sound(int64_t slot)  { if (snd_ok(slot)) StopSound(sounds[(int)slot]); }
int64_t kai_raylib_sound_playing(int64_t slot) {
  return snd_ok(slot) && IsSoundPlaying(sounds[(int)slot]) ? 1 : 0;
}
void kai_raylib_set_sound_volume(int64_t slot, float v) { if (snd_ok(slot)) SetSoundVolume(sounds[(int)slot], v); }
void kai_raylib_set_sound_pitch(int64_t slot, float p)  { if (snd_ok(slot)) SetSoundPitch(sounds[(int)slot], p); }
void kai_raylib_unload_sound(int64_t slot) {
  if (snd_ok(slot)) { UnloadSound(sounds[(int)slot]); snd_loaded[(int)slot] = 0; }
}

/* ----- music (streaming) ----- */

static int mus_ok(int64_t slot) {
  int i = (int)slot;
  return i >= 0 && i < KAI_RAYLIB_MAX_MUSIC && mus_loaded[i];
}

int64_t kai_raylib_load_music(const char *path) {
  Music m = LoadMusicStream(path);
  if (!IsMusicValid(m)) { UnloadMusicStream(m); return -1; }
  for (int i = 0; i < KAI_RAYLIB_MAX_MUSIC; i++) {
    if (!mus_loaded[i]) { music[i] = m; mus_loaded[i] = 1; return (int64_t)i; }
  }
  UnloadMusicStream(m);
  return -1;
}

void kai_raylib_play_music(int64_t slot)   { if (mus_ok(slot)) PlayMusicStream(music[(int)slot]); }
void kai_raylib_update_music(int64_t slot) { if (mus_ok(slot)) UpdateMusicStream(music[(int)slot]); }
void kai_raylib_stop_music(int64_t slot)   { if (mus_ok(slot)) StopMusicStream(music[(int)slot]); }
void kai_raylib_pause_music(int64_t slot)  { if (mus_ok(slot)) PauseMusicStream(music[(int)slot]); }
void kai_raylib_resume_music(int64_t slot) { if (mus_ok(slot)) ResumeMusicStream(music[(int)slot]); }
int64_t kai_raylib_music_playing(int64_t slot) {
  return mus_ok(slot) && IsMusicStreamPlaying(music[(int)slot]) ? 1 : 0;
}
void kai_raylib_set_music_volume(int64_t slot, float v) { if (mus_ok(slot)) SetMusicVolume(music[(int)slot], v); }
void kai_raylib_set_music_looping(int64_t slot, int64_t on) { if (mus_ok(slot)) music[(int)slot].looping = on != 0; }
void kai_raylib_seek_music(int64_t slot, float pos)     { if (mus_ok(slot)) SeekMusicStream(music[(int)slot], pos); }
void kai_raylib_unload_music(int64_t slot) {
  if (mus_ok(slot)) { UnloadMusicStream(music[(int)slot]); mus_loaded[(int)slot] = 0; }
}
