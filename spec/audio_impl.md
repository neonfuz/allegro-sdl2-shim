# Allegro 5 Audio Module Implementation Plan (SDL2 Backend)

## Overview

This document specifies the implementation of the Allegro 5 Audio Module using SDL2_mixer as the backend. The implementation provides feature parity with Allegro 5's audio subsystem while mapping to SDL2_mixer's capabilities.

## Type Mappings

### Core Audio Types

| Allegro Type | SDL2 Equivalent | Description |
|--------------|------------------|-------------|
| `ALLEGRO_SAMPLE` | `Mix_Chunk*` | Audio sample data (raw PCM) |
| `ALLEGRO_SAMPLE_INSTANCE` | Custom struct (`AllegroSampleInstance`) | Sample with playback state |
| `ALLEGRO_AUDIO_STREAM` | Custom struct (`AllegroAudioStream`) | Streaming audio with buffer management |
| `ALLEGRO_MIXER` | `MixerWrapper*` (custom) | Audio mixer wrapping SDL2_mixer |
| `ALLEGRO_VOICE` | `VoiceWrapper*` (custom) | Hardware voice output |

### Enum Mappings

#### ALLEGRO_AUDIO_DEPTH

| Allegro Value | SDL2 Equivalent | Notes |
|---------------|-----------------|-------|
| `ALLEGRO_AUDIO_DEPTH_INT8` | `AUDIO_S8` | 8-bit signed integer |
| `ALLEGRO_AUDIO_DEPTH_INT16` | `AUDIO_S16LSB` | 16-bit signed little-endian |
| `ALLEGRO_AUDIO_DEPTH_INT24` | N/A | 24-bit (simulate with 32-bit) |
| `ALLEGRO_AUDIO_DEPTH_FLOAT32` | `AUDIO_F32LSB` | 32-bit float |
| `ALLEGRO_AUDIO_DEPTH_UNSIGNED` | Add flag | Unsigned sample format |

#### ALLEGRO_CHANNEL_CONF

| Allegro Value | SDL2 Equivalent | Notes |
|---------------|-----------------|-------|
| `ALLEGRO_CHANNEL_CONF_1` | 1 (mono) | Mono |
| `ALLEGRO_CHANNEL_CONF_2` | 2 (stereo) | Stereo |
| `ALLEGRO_CHANNEL_CONF_3` | 3 | 3-channel |
| `ALLEGRO_CHANNEL_CONF_4` | 4 | 4-channel |
| `ALLEGRO_CHANNEL_CONF_5_1` | 6 | 5.1 surround |
| `ALLEGRO_CHANNEL_CONF_6_1` | 7 | 6.1 surround |
| `ALLEGRO_CHANNEL_CONF_7_1` | 8 | 7.1 surround |

#### ALLEGRO_PLAYMODE

| Allegro Value | SDL2 Equivalent | Notes |
|---------------|-----------------|-------|
| `ALLEGRO_PLAYMODE_ONCE` | `0` | Play once and stop |
| `ALLEGRO_PLAYMODE_LOOP` | `1` | Loop continuously |
| `ALLEGRO_PLAYMODE_BIDIR` | N/A | Bidirectional (not supported) |

---

## Custom Struct Definitions

### AllegroSampleInstance

```cpp
struct AllegroSampleInstance {
    Mix_Chunk* chunk;              // SDL2_mixer chunk
    int channel;                   // Channel playing on (-1 if none)
    bool is_playing;               // Playing state
    ALLEGRO_PLAYMODE loop;         // Loop mode
    float gain;                    // Volume (0.0 - 1.0)
    float pan;                     // Pan (-1.0 left to 1.0 right)
    float speed;                   // Playback speed
    unsigned int position;         // Current sample position
    ALLEGRO_SAMPLE* sample;        // Reference to source sample
};
```

### AllegroAudioStream

```cpp
struct AllegroAudioStream {
    SDL_RWops* rwops;              // Source file handle
    Mix_Music* music;              // SDL2_mixer music
    bool is_playing;               // Playing state
    ALLEGRO_PLAYMODE loop;         // Loop mode
    float gain;                    // Volume (0.0 - 1.0)
    float pan;                     // Pan (not directly supported)
    float speed;                   // Playback speed
    unsigned int frequency;         // Sample frequency
    ALLEGRO_AUDIO_DEPTH depth;      // Audio depth
    ALLEGRO_CHANNEL_CONF channels;  // Channel configuration
    unsigned int buffer_samples;   // Buffer size in samples
    size_t buffer_count;           // Number of buffers
    void* user_data;               // User callback data
    ALLEGRO_EVENT_SOURCE event_source;  // Event source for stream events
};
```

### MixerWrapper

```cpp
struct MixerWrapper {
    Mix_Chunk* reserved_channels;  // Pre-allocated channels
    int frequency;                  // Output frequency
    ALLEGRO_AUDIO_DEPTH depth;      // Output depth
    ALLEGRO_CHANNEL_CONF channels;  // Output channels
    ALLEGRO_MIXER_QUALITY quality;  // Resampling quality
    float gain;                     // Master volume
    bool is_playing;                // Mixer playing state
    std::vector<AllegroSampleInstance*> attached_instances;
};
```

### VoiceWrapper

```cpp
struct VoiceWrapper {
    Mix_Chunk* chunk;                // Current chunk being played
    int channel;                     // SDL2_mixer channel
    bool is_playing;                 // Playing state
    unsigned int frequency;          // Output frequency
    ALLEGRO_AUDIO_DEPTH depth;       // Output depth
    ALLEGRO_CHANNEL_CONF channels;   // Output channels
    ALLEGRO_SAMPLE_INSTANCE* attached_instance;
    ALLEGRO_MIXER* attached_mixer;
};
```

---

## Function Implementations

### 1. Audio Initialization

#### al_install_audio()

**Function Signature:**
```c
bool al_install_audio(void);
```

**SDL2_Mixer Equivalent:**
Initialize SDL2_mixer with `Mix_OpenAudio()` using default or specified parameters.

**Implementation Details:**
1. Call `Mix_OpenAudio(44100, AUDIO_S16LSB, 2, 1024)` to initialize SDL2_mixer
2. Allocate internal audio state structures
3. Create default mixer if needed
4. Return true on success, false on failure
5. Set global audio installed flag

---

#### al_init_acodec_addon()

**Function Signature:**
```c
bool al_init_acodec_addon(void);
```

**SDL2_Mixer Equivalent:**
Register file type loaders with SDL2_mixer using `Mix_LoadWAV_RW()` and `Mix_LoadMUS()`.

**Implementation Details:**
1. Initialize internal sample loader registry
2. Register supported audio formats (WAV, OGG, MP3, FLAC, MOD)
3. Register audio stream loaders for each format
4. Return true (SDL2_mixer handles format detection internally)

---

#### al_uninstall_audio()

**Function Signature:**
```c
void al_uninstall_audio(void);
```

**SDL2_Mixer Equivalent:**
Call `Mix_CloseAudio()` to close audio device.

**Implementation Details:**
1. Stop all playing samples and streams
2. Destroy default mixer and voice
3. Free all allocated audio resources
4. Call `Mix_CloseAudio()`
5. Clear global audio installed flag

---

#### al_is_audio_installed()

**Function Signature:**
```c
bool al_is_audio_installed(void);
```

**SDL2_Mixer Equivalent:**
Check internal state flag.

**Implementation Details:**
1. Return value of internal `audio_installed` boolean flag
2. Flag is set true by `al_install_audio()` and false by `al_uninstall_audio()`

---

#### al_reserve_samples()

**Function Signature:**
```c
bool al_reserve_samples(int reserve_samples);
```

**SDL2_Mixer Equivalent:**
Allocate channels using `Mix_ReserveChannels()`.

**Implementation Details:**
1. Call `Mix_ReserveChannels(reserve_samples)` to reserve channels
2. Store reserved channel count for sample playback allocation
3. Return true on success

---

#### al_get_allegro_audio_version()

**Function Signature:**
```c
uint32_t al_get_allegro_audio_version(void);
```

**SDL2_Mixer Equivalent:**
Return version constant.

**Implementation Details:**
1. Return Allegro audio module version number
2. Format: (major << 24) | (minor << 16) | (revision << 8)

---

### 2. Sample Management

#### al_create_sample()

**Function Signature:**
```c
ALLEGRO_SAMPLE* al_create_sample(void *buf, unsigned int samples, 
    unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, 
    ALLEGRO_CHANNEL_CONF chan_conf, bool free_buf);
```

**SDL2_Mixer Equivalent:**
Convert raw PCM buffer to `Mix_Chunk` using `Mix_LoadWAV_RW()` or manual chunk creation.

**Implementation Details:**
1. Calculatesamples * channels * buffer size: ` bytes_per_sample`
2. Create `Mix_Chunk` structure with:
   - `allocated = free_buf ? 1 : 0`
   - `alen = buffer_size`
   - `abuf = free_buf ? copy_or_take_ownership(buf) : buf`
   - `volume = 128` (default)
3. Store sample metadata (frequency, depth, channels) in wrapper struct
4. Return pointer to sample wrapper

---

#### al_destroy_sample()

**Function Signature:**
```c
void al_destroy_sample(ALLEGRO_SAMPLE *spl);
```

**SDL2_Mixer Equivalent:**
Free the `Mix_Chunk` using `Mix_FreeChunk()`.

**Implementation Details:**
1. Check if sample is NULL (return early)
2. Free the underlying `Mix_Chunk` with `Mix_FreeChunk()`
3. Free any metadata stored in wrapper
4. Mark sample as invalid

---

#### al_get_sample_frequency()

**Function Signature:**
```c
unsigned int al_get_sample_frequency(const ALLEGRO_SAMPLE *spl);
```

**SDL2_Mixer Equivalent:**
Return stored frequency value.

**Implementation Details:**
1. Retrieve frequency from sample wrapper metadata
2. Return stored unsigned integer value

---

#### al_get_sample_length()

**Function Signature:**
```c
unsigned int al_get_sample_length(const ALLEGRO_SAMPLE *spl);
```

**SDL2_Mixer Equivalent:**
Return chunk size converted to sample count.

**Implementation Details:**
1. Get `alen` from `Mix_Chunk`
2. Divide by bytes per sample: `alen / (channels * bytes_per_sample)`
3. Return sample count

---

#### al_get_sample_depth()

**Function Signature:**
```c
ALLEGRO_AUDIO_DEPTH al_get_sample_depth(const ALLEGRO_SAMPLE *spl);
```

**SDL2_Mixer Equivalent:**
Return stored depth value.

**Implementation Details:**
1. Retrieve audio depth from sample wrapper metadata
2. Return `ALLEGRO_AUDIO_DEPTH` enum value

---

#### al_get_sample_channels()

**Function Signature:**
```c
ALLEGRO_CHANNEL_CONF al_get_sample_channels(const ALLEGRO_SAMPLE *spl);
```

**SDL2_Mixer Equivalent:**
Return stored channel configuration.

**Implementation Details:**
1. Retrieve channel configuration from sample wrapper
2. Return `ALLEGRO_CHANNEL_CONF` enum value

---

#### al_get_sample_data()

**Function Signature:**
```c
void* al_get_sample_data(const ALLEGRO_SAMPLE *spl);
```

**SDL2_Mixer Equivalent:**
Return pointer to chunk audio data.

**Implementation Details:**
1. Return `spl->chunk->abuf` pointer
2. Handle case where sample was created from file (data may be owned by SDL2_mixer)

---

#### al_load_sample()

**Function Signature:**
```c
ALLEGRO_SAMPLE* al_load_sample(const char *filename);
```

**SDL2_Mixer Equivalent:**
Load audio file using `Mix_LoadWAV()`.

**Implementation Details:**
1. Call `Mix_LoadWAV(filename)` to load file
2. If successful, create `ALLEGRO_SAMPLE` wrapper
3. Extract metadata from loaded chunk (frequency, channels)
4. Return sample or NULL on failure

---

#### al_save_sample()

**Function Signature:**
```c
bool al_save_sample(const char *filename, ALLEGRO_SAMPLE *spl);
```

**SDL2_Mixer Equivalent:**
Save to WAV file using SDL2 audio conversion.

**Implementation Details:**
1. Convert sample data to WAV format if needed
2. Write WAV header with correct format
3. Write raw PCM data
4. Return true on success

---

### 3. Sample Instance Management

#### al_create_sample_instance()

**Function Signature:**
```c
ALLEGRO_SAMPLE_INSTANCE* al_create_sample_instance(ALLEGRO_SAMPLE *data);
```

**SDL2_Mixer Equivalent:**
Create wrapper struct for sample instance.

**Implementation Details:**
1. Allocate `AllegroSampleInstance` structure
2. Initialize fields:
   - `chunk = data->chunk`
   - `channel = -1` (not playing)
   - `is_playing = false`
   - `loop = ALLEGRO_PLAYMODE_ONCE`
   - `gain = 1.0f`
   - `pan = 0.0f` (center)
   - `speed = 1.0f`
   - `position = 0`
   - `sample = data`
3. Return pointer to instance

---

#### al_destroy_sample_instance()

**Function Signature:**
```c
void al_destroy_sample_instance(ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Stop playback and free instance structure.

**Implementation Details:**
1. If instance is playing, call `al_set_sample_instance_playing(spl, false)`
2. If attached to mixer, detach first
3. Free the `AllegroSampleInstance` structure
4. Do NOT free the underlying sample (shared resource)

---

#### al_get_sample_instance_frequency()

**Function Signature:**
```c
unsigned int al_get_sample_instance_frequency(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Return sample frequency from source sample.

**Implementation Details:**
1. Return `spl->sample->frequency`
2. If no sample attached, return 0

---

#### al_get_sample_instance_length()

**Function Signature:**
```c
unsigned int al_get_sample_instance_length(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Return length from source sample.

**Implementation Details:**
1. Return `spl->sample->length`
2. Convert from bytes to samples if needed

---

#### al_get_sample_instance_position()

**Function Signature:**
```c
unsigned int al_get_sample_instance_position(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Get current playback position.

**Implementation Details:**
1. If playing, call `Mix_GetChunk(spl->channel)` and calculate position
2. Or use `Mix_Playing(spl->channel)` to check status
3. Return current sample position in samples

---

#### al_set_sample_instance_position()

**Function Signature:**
```c
bool al_set_sample_instance_position(ALLEGRO_SAMPLE_INSTANCE *spl, unsigned int val);
```

**SDL2_Mixer Equivalent:**
Seek to position in sample (limited SDL2_mixer support).

**Implementation Details:**
1. SDL2_mixer does not support seeking within chunks natively
2. Implementation may require stopping and restarting from new position
3. Return true if successful, false otherwise

---

#### al_get_sample_instance_speed()

**Function Signature:**
```c
float al_get_sample_instance_speed(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Return stored speed value.

**Implementation Details:**
1. Return `spl->speed` float value
2. Default is 1.0f (normal speed)

---

#### al_set_sample_instance_speed()

**Function Signature:**
```c
bool al_set_sample_instance_speed(ALLEGRO_SAMPLE_INSTANCE *spl, float val);
```

**SDL2_Mixer Equivalent:**
Set playback speed (SDL2_mixer doesn't support, store for future use).

**Implementation Details:**
1. Store speed value in instance
2. SDL2_mixer doesn't support pitch shifting; simulate with re-sampling if critical
3. Return true

---

#### al_get_sample_instance_gain()

**Function Signature:**
```c
float al_get_sample_instance_gain(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Return stored gain value.

**Implementation Details:**
1. Return `spl->gain` float value
2. Default is 1.0f (full volume)

---

#### al_set_sample_instance_gain()

**Function Signature:**
```c
bool al_set_sample_instance_gain(ALLEGRO_SAMPLE_INSTANCE *spl, float val);
```

**SDL2_Mixer Equivalent:**
Use `Mix_Volume()` to set volume.

**Implementation Details:**
1. Convert float gain (0.0-1.0) to integer volume (0-128)
2. Call `Mix_Volume(spl->channel, volume)` if playing
3. Store gain value in instance
4. Return true

---

#### al_get_sample_instance_pan()

**Function Signature:**
```c
float al_get_sample_instance_pan(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Return stored pan value.

**Implementation Details:**
1. Return `spl->pan` float value
2. Default is 0.0f (center)

---

#### al_set_sample_instance_pan()

**Function Signature:**
```c
bool al_set_sample_instance_pan(ALLEGRO_SAMPLE_INSTANCE *spl, float val);
```

**SDL2_Mixer Equivalent:**
Use `Mix_SetPanning()` to set stereo position.

**Implementation Details:**
1. Convert pan float (-1.0 to 1.0) to SDL2_mixer panning (left/right)
2. Call `Mix_SetPanning(spl->channel, left, right)`
3. Store pan value in instance
4. Return true on success

---

#### al_get_sample_instance_time()

**Function Signature:**
```c
float al_get_sample_instance_time(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Calculate time from position and frequency.

**Implementation Details:**
1. Get current position with `al_get_sample_instance_position()`
2. Divide by sample frequency: `position / frequency`
3. Return time in seconds

---

#### al_get_sample_instance_depth()

**Function Signature:**
```c
ALLEGRO_AUDIO_DEPTH al_get_sample_instance_depth(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Return depth from source sample.

**Implementation Details:**
1. Return `spl->sample->depth`
2. If no sample, return default

---

#### al_get_sample_instance_channels()

**Function Signature:**
```c
ALLEGRO_CHANNEL_CONF al_get_sample_instance_channels(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Return channels from source sample.

**Implementation Details:**
1. Return `spl->sample->channels`
2. If no sample, return default

---

#### al_get_sample_instance_playmode()

**Function Signature:**
```c
ALLEGRO_PLAYMODE al_get_sample_instance_playmode(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Return stored loop mode.

**Implementation Details:**
1. Return `spl->loop` enum value

---

#### al_set_sample_instance_playmode()

**Function Signature:**
```c
bool al_set_sample_instance_playmode(ALLEGRO_SAMPLE_INSTANCE *spl, ALLEGRO_PLAYMODE val);
```

**SDL2_Mixer Equivalent:**
Store loop mode for next playback.

**Implementation Details:**
1. Store loop mode in instance
2. If currently playing, restart with new loop mode
3. Return true

---

#### al_get_sample_instance_playing()

**Function Signature:**
```c
bool al_get_sample_instance_playing(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Check playing state.

**Implementation Details:**
1. Return `spl->is_playing` flag
2. Optionally verify with `Mix_Playing(spl->channel)`

---

#### al_get_sample_instance_attached()

**Function Signature:**
```c
bool al_get_sample_instance_attached(const ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Check if attached to mixer.

**Implementation Details:**
1. Check if instance has mixer parent
2. Return true if attached

---

#### al_set_sample_instance_playing()

**Function Signature:**
```c
bool al_set_sample_instance_playing(ALLEGRO_SAMPLE_INSTANCE *spl, bool val);
```

**SDL2_Mixer Equivalent:**
Use `Mix_PlayChannel()` to start, `Mix_HaltChannel()` to stop.

**Implementation Details:**
1. If val=true and not playing:
   - Determine loop count from `spl->loop`
   - Call `Mix_PlayChannel(-1, spl->chunk, loops)`
   - Store channel number
   - Set `is_playing = true`
2. If val=false and playing:
   - Call `Mix_HaltChannel(spl->channel)`
   - Set `is_playing = false`
3. Return true

---

#### al_detach_sample_instance()

**Function Signature:**
```c
bool al_detach_sample_instance(ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Remove instance from mixer routing.

**Implementation Details:**
1. Stop instance if playing
2. Clear mixer reference
3. Return true

---

#### al_set_sample()

**Function Signature:**
```c
bool al_set_sample(ALLEGRO_SAMPLE_INSTANCE *spl, ALLEGRO_SAMPLE *data);
```

**SDL2_Mixer Equivalent:**
Replace source sample data.

**Implementation Details:**
1. Replace `spl->chunk` with new sample's chunk
2. Update `spl->sample` reference
3. Reset position to 0
4. Return true

---

#### al_get_sample()

**Function Signature:**
```c
ALLEGRO_SAMPLE* al_get_sample(ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Return reference to source sample.

**Implementation Details:**
1. Return `spl->sample` pointer

---

#### al_play_sample_instance()

**Function Signature:**
```c
bool al_play_sample_instance(ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Start playback immediately.

**Implementation Details:**
1. Call `al_set_sample_instance_playing(spl, true)`
2. Return result

---

#### al_stop_sample_instance()

**Function Signature:**
```c
bool al_stop_sample_instance(ALLEGRO_SAMPLE_INSTANCE *spl);
```

**SDL2_Mixer Equivalent:**
Stop playback.

**Implementation Details:**
1. Call `al_set_sample_instance_playing(spl, false)`
2. Return result

---

### 4. Mixer Functions

#### al_create_mixer()

**Function Signature:**
```c
ALLEGRO_MIXER* al_create_mixer(unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, 
    ALLEGRO_CHANNEL_CONF chan_conf);
```

**SDL2_Mixer Equivalent:**
Create mixer wrapper structure.

**Implementation Details:**
1. Allocate `MixerWrapper` structure
2. Store frequency, depth, and channel configuration
3. Initialize attached instances vector
4. Set default gain to 1.0f
5. Return mixer pointer

---

#### al_destroy_mixer()

**Function Signature:**
```c
void al_destroy_mixer(ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Destroy mixer wrapper.

**Implementation Details:**
1. Detach all attached sample instances
2. Free mixer wrapper structure
3. Do NOT close audio device (managed globally)

---

#### al_attach_sample_instance_to_mixer()

**Function Signature:**
```c
bool al_attach_sample_instance_to_mixer(ALLEGRO_SAMPLE_INSTANCE *stream, 
    ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Associate instance with mixer.

**Implementation Details:**
1. Store mixer reference in sample instance
2. Add instance to mixer's attached list
3. Configure volume/gain routing
4. Return true

---

#### al_attach_audio_stream_to_mixer()

**Function Signature:**
```c
bool al_attach_audio_stream_to_mixer(ALLEGRO_AUDIO_STREAM *stream, 
    ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Associate stream with mixer.

**Implementation Details:**
1. Store mixer reference in audio stream
2. Route stream output to mixer
3. Return true

---

#### al_attach_mixer_to_mixer()

**Function Signature:**
```c
bool al_attach_mixer_to_mixer(ALLEGRO_MIXER *stream, ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Chain mixers together.

**Implementation Details:**
1. Set parent mixer reference
2. Configure audio routing
3. Return true

---

#### al_set_mixer_postprocess_callback()

**Function Signature:**
```c
bool al_set_mixer_postprocess_callback(ALLEGRO_MIXER *mixer, 
    void (*cb)(void *buf, unsigned int samples, void *data), void *data);
```

**SDL2_Mixer Equivalent:**
Set audio processing callback (limited SDL2_mixer support).

**Implementation Details:**
1. Store callback and user data
2. SDL2_mixer doesn't support this directly; implement via hook if possible
3. Return true if supported

---

#### al_get_mixer_frequency()

**Function Signature:**
```c
unsigned int al_get_mixer_frequency(const ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Return mixer frequency.

**Implementation Details:**
1. Return stored frequency value

---

#### al_get_mixer_channels()

**Function Signature:**
```c
ALLEGRO_CHANNEL_CONF al_get_mixer_channels(const ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Return mixer channel configuration.

**Implementation Details:**
1. Return stored channel configuration

---

#### al_get_mixer_depth()

**Function Signature:**
```c
ALLEGRO_AUDIO_DEPTH al_get_mixer_depth(const ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Return mixer audio depth.

**Implementation Details:**
1. Return stored audio depth

---

#### al_get_mixer_quality()

**Function Signature:**
```c
ALLEGRO_MIXER_QUALITY al_get_mixer_quality(const ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Return mixer resampling quality.

**Implementation Details:**
1. Return stored quality setting

---

#### al_get_mixer_gain()

**Function Signature:**
```c
float al_get_mixer_gain(const ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Return mixer master gain.

**Implementation Details:**
1. Return stored gain value

---

#### al_get_mixer_playing()

**Function Signature:**
```c
bool al_get_mixer_playing(const ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Check if mixer has active audio.

**Implementation Details:**
1. Check if any attached instances are playing
2. Return true if any playing

---

#### al_get_mixer_attached()

**Function Signature:**
```c
bool al_get_mixer_attached(const ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Check if mixer is attached to voice.

**Implementation Details:**
1. Check if mixer has parent voice
2. Return true if attached

---

#### al_mixer_has_attachments()

**Function Signature:**
```c
bool al_mixer_has_attachments(const ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Check if mixer has attached streams.

**Implementation Details:**
1. Check if attached instances list is non-empty
2. Return true if any attached

---

#### al_set_mixer_frequency()

**Function Signature:**
```c
bool al_set_mixer_frequency(ALLEGRO_MIXER *mixer, unsigned int val);
```

**SDL2_Mixer Equivalent:**
Set mixer output frequency (requires reinitialization).

**Implementation Details:**
1. SDL2_mixer doesn't support runtime frequency changes
2. Store new frequency for new playback instances
3. Return true if supported

---

#### al_set_mixer_quality()

**Function Signature:**
```c
bool al_set_mixer_quality(ALLEGRO_MIXER *mixer, ALLEGRO_MIXER_QUALITY val);
```

**SDL2_Mixer Equivalent:**
Set resampling quality.

**Implementation Details:**
1. Store quality setting
2. SDL2_mixer uses built-in resampling
3. Return true

---

#### al_set_mixer_gain()

**Function Signature:**
```c
bool al_set_mixer_gain(ALLEGRO_MIXER *mixer, float gain);
```

**SDL2_Mixer Equivalent:**
Set master volume.

**Implementation Details:**
1. Convert float (0.0-1.0) to integer (0-128)
2. Apply to all playing channels via mixer
3. Store gain value
4. Return true

---

#### al_set_mixer_playing()

**Function Signature:**
```c
bool al_set_mixer_playing(ALLEGRO_MIXER *mixer, bool val);
```

**SDL2_Mixer Equivalent:**
Start/stop all attached streams.

**Implementation Details:**
1. If val=true, start all attached instances
2. If val=false, stop all attached instances
3. Update playing state
4. Return true

---

#### al_detach_mixer()

**Function Signature:**
```c
bool al_detach_mixer(ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Detach mixer from parent.

**Implementation Details:**
1. Clear parent voice reference
2. Return true

---

### 5. Voice Functions

#### al_create_voice()

**Function Signature:**
```c
ALLEGRO_VOICE* al_create_voice(unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, 
    ALLEGRO_CHANNEL_CONF chan_conf);
```

**SDL2_Mixer Equivalent:**
Create voice wrapper structure.

**Implementation Details:**
1. Allocate `VoiceWrapper` structure
2. Store frequency, depth, channel configuration
3. Initialize attached instance/mixer as NULL
4. Return voice pointer

---

#### al_destroy_voice()

**Function Signature:**
```c
void al_destroy_voice(ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Destroy voice wrapper.

**Implementation Details:**
1. Detach any attached stream or mixer
2. Free voice wrapper structure

---

#### al_attach_sample_instance_to_voice()

**Function Signature:**
```c
bool al_attach_sample_instance_to_voice(ALLEGRO_SAMPLE_INSTANCE *stream, 
    ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Attach sample to voice for playback.

**Implementation Details:**
1. Store voice reference in sample instance
2. Store instance in voice
3. Configure for immediate playback when started
4. Return true

---

#### al_attach_audio_stream_to_voice()

**Function Signature:**
```c
bool al_attach_audio_stream_to_voice(ALLEGRO_AUDIO_STREAM *stream, 
    ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Attach stream to voice for playback.

**Implementation Details:**
1. Store voice reference in audio stream
2. Configure playback through voice
3. Return true

---

#### al_attach_mixer_to_voice()

**Function Signature:**
```c
bool al_attach_mixer_to_voice(ALLEGRO_MIXER *mixer, ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Attach mixer to voice for output.

**Implementation Details:**
1. Store voice reference in mixer
2. Store mixer in voice
3. Configure audio routing
4. Return true

---

#### al_detach_voice()

**Function Signature:**
```c
void al_detach_voice(ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Detach all streams from voice.

**Implementation Details:**
1. Stop any playing audio
2. Clear attached instance/mixer references

---

#### al_get_voice_frequency()

**Function Signature:**
```c
unsigned int al_get_voice_frequency(const ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Return voice frequency.

**Implementation Details:**
1. Return stored frequency value

---

#### al_get_voice_position()

**Function Signature:**
```c
unsigned int al_get_voice_position(const ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Return current playback position.

**Implementation Details:**
1. Get position from attached instance
2. Return sample position

---

#### al_get_voice_channels()

**Function Signature:**
```c
ALLEGRO_CHANNEL_CONF al_get_voice_channels(const ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Return voice channel configuration.

**Implementation Details:**
1. Return stored channel configuration

---

#### al_get_voice_depth()

**Function Signature:**
```c
ALLEGRO_AUDIO_DEPTH al_get_voice_depth(const ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Return voice audio depth.

**Implementation Details:**
1. Return stored audio depth

---

#### al_get_voice_playing()

**Function Signature:**
```c
bool al_get_voice_playing(const ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Check if voice is playing.

**Implementation Details:**
1. Return playing state from attached instance

---

#### al_voice_has_attachments()

**Function Signature:**
```c
bool al_voice_has_attachments(const ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Check if voice has attached stream.

**Implementation Details:**
1. Check if instance or mixer attached
2. Return true if attached

---

#### al_set_voice_position()

**Function Signature:**
```c
bool al_set_voice_position(ALLEGRO_VOICE *voice, unsigned int val);
```

**SDL2_Mixer Equivalent:**
Seek to position in voice output.

**Implementation Details:**
1. Seek in attached instance
2. Return true if successful

---

#### al_set_voice_playing()

**Function Signature:**
```c
bool al_set_voice_playing(ALLEGRO_VOICE *voice, bool val);
```

**SDL2_Mixer Equivalent:**
Start/stop voice output.

**Implementation Details:**
1. If val=true, start attached instance playback
2. If val=false, stop playback
3. Return true

---

### 6. Audio Stream Functions

#### al_create_audio_stream()

**Function Signature:**
```c
ALLEGRO_AUDIO_STREAM* al_create_audio_stream(size_t buffer_count, 
    unsigned int samples, unsigned int freq, 
    ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf);
```

**SDL2_Mixer Equivalent:**
Create streaming audio structure.

**Implementation Details:**
1. Allocate `AllegroAudioStream` structure
2. Initialize buffer arrays for streaming
3. Set frequency, depth, channels
4. Create event source for stream events
5. Return stream pointer

---

#### al_destroy_audio_stream()

**Function Signature:**
```c
void al_destroy_audio_stream(ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Destroy audio stream.

**Implementation Details:**
1. Stop stream if playing
2. Free audio buffers
3. Close file handle if open
4. Free stream structure

---

#### al_drain_audio_stream()

**Function Signature:**
```c
void al_drain_audio_stream(ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Drain remaining audio data.

**Implementation Details:**
1. Wait for playback to complete
2. Set draining flag
3. Return when finished

---

#### al_load_audio_stream()

**Function Signature:**
```c
ALLEGRO_AUDIO_STREAM* al_load_audio_stream(const char *filename, 
    size_t buffer_count, unsigned int samples);
```

**SDL2_Mixer Equivalent:**
Load streaming audio with `Mix_LoadMUS()`.

**Implementation Details:**
1. Open file with `Mix_LoadMUS(filename)`
2. Create `AllegroAudioStream` wrapper
3. Configure streaming parameters
4. Set up event source for buffer events
5. Return stream pointer or NULL on failure

---

#### al_get_audio_stream_frequency()

**Function Signature:**
```c
unsigned int al_get_audio_stream_frequency(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return stream frequency.

**Implementation Details:**
1. Return stored frequency value

---

#### al_get_audio_stream_length()

**Function Signature:**
```c
unsigned int al_get_audio_stream_length(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return stream length in samples.

**Implementation Details:**
1. Return total samples from music metadata

---

#### al_get_audio_stream_fragments()

**Function Signature:**
```c
unsigned int al_get_audio_stream_fragments(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return number of stream buffers.

**Implementation Details:**
1. Return `stream->buffer_count`

---

#### al_get_available_audio_stream_fragments()

**Function Signature:**
```c
unsigned int al_get_available_audio_stream_fragments(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return available buffer count.

**Implementation Details:**
1. Count buffers not currently in use

---

#### al_get_audio_stream_speed()

**Function Signature:**
```c
float al_get_audio_stream_speed(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return stream speed.

**Implementation Details:**
1. Return stored speed value

---

#### al_set_audio_stream_speed()

**Function Signature:**
```c
bool al_set_audio_stream_speed(ALLEGRO_AUDIO_STREAM *stream, float val);
```

**SDL2_Mixer Equivalent:**
Set playback speed (limited support).

**Implementation Details:**
1. Store speed value
2. SDL2_mixer doesn't support speed control natively
3. Return true

---

#### al_get_audio_stream_gain()

**Function Signature:**
```c
float al_get_audio_stream_gain(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return stream volume.

**Implementation Details:**
1. Return stored gain value

---

#### al_set_audio_stream_gain()

**Function Signature:**
```c
bool al_set_audio_stream_gain(ALLEGRO_AUDIO_STREAM *stream, float val);
```

**SDL2_Mixer Equivalent:**
Set stream volume.

**Implementation Details:**
1. Convert float (0.0-1.0) to integer (0-128)
2. Call `Mix_VolumeMusic(volume)`
3. Store gain value
4. Return true

---

#### al_get_audio_stream_pan()

**Function Signature:**
```c
float al_get_audio_stream_pan(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return stream pan.

**Implementation Details:**
1. Return stored pan value

---

#### al_set_audio_stream_pan()

**Function Signature:**
```c
bool al_set_audio_stream_pan(ALLEGRO_AUDIO_STREAM *stream, float val);
```

**SDL2_Mixer Equivalent:**
Set stream pan (limited SDL2_mixer support).

**Implementation Details:**
1. Store pan value
2. SDL2_mixer music doesn't support panning
3. Return true (stored but not applied)

---

#### al_get_audio_stream_channels()

**Function Signature:**
```c
ALLEGRO_CHANNEL_CONF al_get_audio_stream_channels(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return stream channel configuration.

**Implementation Details:**
1. Return stored channel configuration

---

#### al_get_audio_stream_depth()

**Function Signature:**
```c
ALLEGRO_AUDIO_DEPTH al_get_audio_stream_depth(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return stream audio depth.

**Implementation Details:**
1. Return stored audio depth

---

#### al_get_audio_stream_playmode()

**Function Signature:**
```c
ALLEGRO_PLAYMODE al_get_audio_stream_playmode(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Return stream loop mode.

**Implementation Details:**
1. Return stored loop mode

---

#### al_set_audio_stream_playmode()

**Function Signature:**
```c
bool al_set_audio_stream_playmode(ALLEGRO_AUDIO_STREAM *stream, ALLEGRO_PLAYMODE val);
```

**SDL2_Mixer Equivalent:**
Set stream loop mode.

**Implementation Details:**
1. Store loop mode
2. Apply to music with `Mix_PlayMusic` with loop parameter
3. Return true

---

#### al_get_audio_stream_playing()

**Function Signature:**
```c
bool al_get_audio_stream_playing(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Check if stream is playing.

**Implementation Details:**
1. Return `stream->is_playing`
2. Optionally verify with `Mix_PlayingMusic()`

---

#### al_set_audio_stream_playing()

**Function Signature:**
```c
bool al_set_audio_stream_playing(ALLEGRO_AUDIO_STREAM *stream, bool val);
```

**SDL2_Mixer Equivalent:**
Start/stop stream playback.

**Implementation Details:**
1. If val=true:
   - Call `Mix_PlayMusic(stream->music, loops)`
   - Set `is_playing = true`
2. If val=false:
   - Call `Mix_HaltMusic()`
   - Set `is_playing = false`
3. Return true

---

#### al_get_audio_stream_attached()

**Function Signature:**
```c
bool al_get_audio_stream_attached(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Check if stream is attached to mixer.

**Implementation Details:**
1. Check if mixer reference exists
2. Return true if attached

---

#### al_detach_audio_stream()

**Function Signature:**
```c
bool al_detach_audio_stream(ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Detach stream from mixer.

**Implementation Details:**
1. Clear mixer reference
2. Continue playing directly
3. Return true

---

#### al_get_audio_stream_played_samples()

**Function Signature:**
```c
uint64_t al_get_audio_stream_played_samples(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Track played samples counter.

**Implementation Details:**
1. Maintain internal counter updated on each buffer
2. Return total samples played

---

#### al_get_audio_stream_fragment()

**Function Signature:**
```c
void* al_get_audio_stream_fragment(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Get buffer for filling.

**Implementation Details:**
1. Return pointer to next available buffer
2. Mark buffer as in use

---

#### al_set_audio_stream_fragment()

**Function Signature:**
```c
bool al_set_audio_stream_fragment(ALLEGRO_AUDIO_STREAM *stream, void *val);
```

**SDL2_Mixer Equivalent:**
Submit filled buffer.

**Implementation Details:**
1. Mark buffer as filled
2. Queue for playback
3. Return true

---

#### al_rewind_audio_stream()

**Function Signature:**
```c
bool al_rewind_audio_stream(ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Rewind to beginning.

**Implementation Details:**
1. Call `Mix_RewindMusic()`
2. Reset position counter
3. Return true

---

#### al_seek_audio_stream_secs()

**Function Signature:**
```c
bool al_seek_audio_stream_secs(ALLEGRO_AUDIO_STREAM *stream, double time);
```

**SDL2_Mixer Equivalent:**
Seek to time position.

**Implementation Details:**
1. Call `Mix_SetMusicPosition(time)`
2. Update position counter
3. Return true on success

---

#### al_get_audio_stream_position_secs()

**Function Signature:**
```c
double al_get_audio_stream_position_secs(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Get current playback time.

**Implementation Details:**
1. SDL2_mixer doesn't expose position; calculate from elapsed time
2. Return time in seconds

---

#### al_get_audio_stream_length_secs()

**Function Signature:**
```c
double al_get_audio_stream_length_secs(const ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Get total stream length.

**Implementation Details:**
1. Get from music metadata if available
2. Calculate from file size and format
3. Return length in seconds

---

#### al_set_audio_stream_loop_secs()

**Function Signature:**
```c
bool al_set_audio_stream_loop_secs(ALLEGRO_AUDIO_STREAM *stream, double start, double end);
```

**SDL2_Mixer Equivalent:**
Set loop points.

**Implementation Details:**
1. Store loop start and end times
2. Implement with position callbacks
3. Return true if supported

---

#### al_get_audio_stream_event_source()

**Function Signature:**
```c
ALLEGRO_EVENT_SOURCE* al_get_audio_stream_event_source(ALLEGRO_AUDIO_STREAM *stream);
```

**SDL2_Mixer Equivalent:**
Get event source for stream events.

**Implementation Details:**
1. Return pointer to stream's event source
2. Used for receiving buffer refill events

---

### 7. Simple Audio Layer Functions

#### al_get_default_mixer()

**Function Signature:**
```c
ALLEGRO_MIXER* al_get_default_mixer(void);
```

**SDL2_Mixer Equivalent:**
Return global default mixer.

**Implementation Details:**
1. Return pointer to global default mixer
2. Create on first call if not exists

---

#### al_set_default_mixer()

**Function Signature:**
```c
bool al_set_default_mixer(ALLEGRO_MIXER *mixer);
```

**SDL2_Mixer Equivalent:**
Set global default mixer.

**Implementation Details:**
1. Store pointer as global default
2. Return true

---

#### al_restore_default_mixer()

**Function Signature:**
```c
bool al_restore_default_mixer(void);
```

**SDL2_Mixer Equivalent:**
Restore default SDL2_mixer.

**Implementation Details:**
1. Recreate default mixer with standard settings
2. Set as default
3. Return true

---

#### al_play_sample()

**Function Signature:**
```c
bool al_play_sample(ALLEGRO_SAMPLE *data, float gain, float pan, float speed, 
    ALLEGRO_PLAYMODE loop, ALLEGRO_SAMPLE_ID *ret_id);
```

**SDL2_Mixer Equivalent:**
Play sample immediately with parameters.

**Implementation Details:**
1. Get available channel with `Mix_PlayChannel(-1, chunk, loops)`
2. Apply volume: `Mix_Volume(channel, gain * 128)`
3. Apply panning if supported
4. Store channel in `ret_id` if provided
5. Return true

---

#### al_stop_sample()

**Function Signature:**
```c
void al_stop_sample(ALLEGRO_SAMPLE_ID *spl_id);
```

**SDL2_Mixer Equivalent:**
Stop specific sample by ID.

**Implementation Details:**
1. Extract channel from `spl_id`
2. Call `Mix_HaltChannel(channel)`
3. Clear playing state

---

#### al_stop_samples()

**Function Signature:**
```c
void al_stop_samples(void);
```

**SDL2_Mixer Equivalent:**
Stop all playing samples.

**Implementation Details:**
1. Call `Mix_HaltChannel(-1)` to stop all channels
2. Call `Mix_HaltMusic()` to stop music

---

#### al_get_default_voice()

**Function Signature:**
```c
ALLEGRO_VOICE* al_get_default_voice(void);
```

**SDL2_Mixer Equivalent:**
Return default voice.

**Implementation Details:**
1. Return global default voice
2. Create on first call if not exists

---

#### al_set_default_voice()

**Function Signature:**
```c
void al_set_default_voice(ALLEGRO_VOICE *voice);
```

**SDL2_Mixer Equivalent:**
Set default voice.

**Implementation Details:**
1. Store as global default voice

---

### 8. Utility Functions

#### al_get_channel_count()

**Function Signature:**
```c
size_t al_get_channel_count(ALLEGRO_CHANNEL_CONF conf);
```

**SDL2_Mixer Equivalent:**
Calculate channel count from configuration.

**Implementation Details:**
1. Extract channel count from channel configuration enum
2. Return number of channels

---

#### al_get_audio_depth_size()

**Function Signature:**
```c
size_t al_get_audio_depth_size(ALLEGRO_AUDIO_DEPTH conf);
```

**SDL2_Mixer Equivalent:**
Get bytes per sample for depth.

**Implementation Details:**
1. Return size based on depth:
   - INT8: 1 byte
   - INT16: 2 bytes
   - INT24: 3 bytes
   - FLOAT32: 4 bytes

---

#### al_fill_silence()

**Function Signature:**
```c
void al_fill_silence(void *buf, unsigned int samples, 
    ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf);
```

**SDL2_Mixer Equivalent:**
Fill buffer with silence.

**Implementation Details:**
1. Calculate buffer size
2. Fill with zero bytes (silence for most formats)
3. For float, fill with 0.0f

---

#### al_get_num_audio_output_devices()

**Function Signature:**
```c
int al_get_num_audio_output_devices(void);
```

**SDL2_Mixer Equivalent:**
Get available audio devices.

**Implementation Details:**
1. Use SDL2 `SDL_GetNumAudioDevices()`
2. Return count

---

#### al_get_audio_output_device()

**Function Signature:**
```c
const ALLEGRO_AUDIO_DEVICE* al_get_audio_output_device(int index);
```

**SDL2_Mixer Equivalent:**
Get audio device info.

**Implementation Details:**
1. Query SDL2 for device name
2. Return device structure

---

#### al_get_audio_device_name()

**Function Signature:**
```c
const char* al_get_audio_device_name(const ALLEGRO_AUDIO_DEVICE *device);
```

**SDL2_Mixer Equivalent:**
Get device name string.

**Implementation Details:**
1. Return stored device name

---

### 9. Sample Loader Registration

#### al_register_sample_loader()

**Function Signature:**
```c
bool al_register_sample_loader(const char *ext, 
    ALLEGRO_SAMPLE* (*loader)(const char *filename));
```

**SDL2_Mixer Equivalent:**
Register custom sample loader.

**Implementation Details:**
1. Store loader function in registry
2. Map extension to loader
3. Return true

---

#### al_register_sample_saver()

**Function Signature:**
```c
bool al_register_sample_saver(const char *ext, 
    bool (*saver)(const char *filename, ALLEGRO_SAMPLE *spl));
```

**SDL2_Mixer Equivalent:**
Register custom sample saver.

**Implementation Details:**
1. Store saver function in registry
2. Map extension to saver
3. Return true

---

#### al_register_audio_stream_loader()

**Function Signature:**
```c
bool al_register_audio_stream_loader(const char *ext, 
    ALLEGRO_AUDIO_STREAM* (*stream_loader)(const char *filename, 
       size_t buffer_count, unsigned int samples));
```

**SDL2_Mixer Equivalent:**
Register custom stream loader.

**Implementation Details:**
1. Store loader function in registry
2. Map extension to loader
3. Return true

---

#### al_load_sample_f()

**Function Signature:**
```c
ALLEGRO_SAMPLE* al_load_sample_f(ALLEGRO_FILE *fp, const char *ident);
```

**SDL2_Mixer Equivalent:**
Load sample from file handle.

**Implementation Details:**
1. Create SDL_RWops from ALLEGRO_FILE
2. Load with `Mix_LoadWAV_RW()`
3. Return sample

---

#### al_save_sample_f()

**Function Signature:**
```c
bool al_save_sample_f(ALLEGRO_FILE *fp, const char *ident, ALLEGRO_SAMPLE *spl);
```

**SDL2_Mixer Equivalent:**
Save sample to file handle.

**Implementation Details:**
1. Create SDL_RWops from ALLEGRO_FILE
2. Write WAV data
3. Return true on success

---

#### al_load_audio_stream_f()

**Function Signature:**
```c
ALLEGRO_AUDIO_STREAM* al_load_audio_stream_f(ALLEGRO_FILE *fp, const char *ident, 
    size_t buffer_count, unsigned int samples);
```

**SDL2_Mixer Equivalent:**
Load stream from file handle.

**Implementation Details:**
1. Create SDL_RWops from ALLEGRO_FILE
2. Load with `Mix_LoadMUS_RW()`
3. Return stream

---

#### al_identify_sample()

**Function Signature:**
```c
const char* al_identify_sample(const char *filename);
```

**SDL2_Mixer Equivalent:**
Identify audio file format.

**Implementation Details:**
1. Check file extension
2. Return format identifier string

---

#### al_identify_sample_f()

**Function Signature:**
```c
const char* al_identify_sample_f(ALLEGRO_FILE *fp);
```

**SDL2_Mixer Equivalent:**
Identify audio format from file.

**Implementation Details:**
1. Read file header bytes
2. Match against known signatures
3. Return format string

---

## Implementation Notes

### SDL2_mixer Limitations

1. **No pitch/speed control**: SDL2_mixer doesn't support playback speed adjustment
2. **Limited seeking**: Position queries are not fully supported
3. **No per-channel panning on music**: Only works for chunks
4. **Fixed sample rates**: May require resampling
5. **No 24-bit audio**: Must convert to 16-bit or 32-bit float

### Workaround Strategies

1. **Speed control**: Implement software resampling before passing to SDL2_mixer
2. **Position tracking**: Maintain software position counter synced with callbacks
3. **Loop points**: Use channel finished callbacks to implement loop regions
4. **24-bit audio**: Convert to 16-bit with dithering or use 32-bit float

### Thread Safety

1. SDL2_mixer callbacks occur on audio thread
2. Use mutex protection for shared state
3. Queue commands from main thread to audio thread

### Memory Management

1. Samples (Mix_Chunk*) are reference counted
2. Stream buffers must be managed carefully to avoid leaks
3. Clean up all resources on audio shutdown

---

## New Details from SDL Source Analysis

### SDL2 Audio (SDL_mixer)

The SDL source in this repository is **SDL2** with **SDL2_mixer** for audio:

1. **SDL_mixer**: Located in `src/audio/` as `SDL_mixer.c`
   - Provides music streaming (`Mix_Music*`) and sound effects (`Mix_Chunk*`)
   - Channel-based mixing with up to 8 channels (configurable)
   - Format support: WAV, OGG, MP3, FLAC, MOD

2. **Key SDL2_mixer Functions**:
   - `Mix_OpenAudio()` - Initialize audio device
   - `Mix_LoadWAV()` / `Mix_LoadMUS()` - Load audio files
   - `Mix_PlayChannel()` / `Mix_PlayMusic()` - Play audio
   - `Mix_Volume()` / `Mix_VolumeMusic()` - Volume control
   - `Mix_SetPanning()` - Stereo positioning

3. **Backend Implementation**: The `src/audio/` directory contains platform backends:
   - ALSA, PulseAudio, OSS (Linux)
   - CoreAudio (macOS)
   - DirectSound, WASAPI (Windows)

### Updated Implementation Notes

1. SDL2_mixer is the correct approach for this repository
2. Use `Mix_LoadWAV_RW()` for loading from custom sources
3. For streaming, use `Mix_Music*` with callback-based decoding
4. Leverage SDL2's built-in audio format conversion

### Dependencies Update

| Library | Notes |
|---------|-------|
| SDL2 | Core library |
| SDL2_mixer | Audio mixing (comes with SDL2) |
| SDL2_image | Image loading |
| SDL2_ttf | Font rendering |
