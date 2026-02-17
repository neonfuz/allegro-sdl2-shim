#ifndef ALLEGRO_AUDIO_H
#define ALLEGRO_AUDIO_H

#include "allegro_base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_AUDIO_VERSION 0x150000

typedef struct ALLEGRO_AUDIO_DEVICE ALLEGRO_AUDIO_DEVICE;
typedef struct ALLEGRO_SAMPLE ALLEGRO_SAMPLE;
typedef struct ALLEGRO_SAMPLE_INSTANCE ALLEGRO_SAMPLE_INSTANCE;
typedef struct ALLEGRO_AUDIO_STREAM ALLEGRO_AUDIO_STREAM;
typedef struct ALLEGRO_MIXER ALLEGRO_MIXER;
typedef struct ALLEGRO_VOICE ALLEGRO_VOICE;
typedef struct ALLEGRO_SAMPLE_ID ALLEGRO_SAMPLE_ID;
typedef struct ALLEGRO_FILE ALLEGRO_FILE;

typedef enum {
    ALLEGRO_AUDIO_DEPTH_INT8 = 0x01,
    ALLEGRO_AUDIO_DEPTH_INT16 = 0x02,
    ALLEGRO_AUDIO_DEPTH_INT24 = 0x04,
    ALLEGRO_AUDIO_DEPTH_INT32 = 0x08,
    ALLEGRO_AUDIO_DEPTH_FLOAT32 = 0x10,
    ALLEGRO_AUDIO_DEPTH_UNSIGNED = 0x20
} ALLEGRO_AUDIO_DEPTH;

typedef enum {
    ALLEGRO_CHANNEL_CONF_1 = 1,
    ALLEGRO_CHANNEL_CONF_2 = 2,
    ALLEGRO_CHANNEL_CONF_3 = 3,
    ALLEGRO_CHANNEL_CONF_4 = 4,
    ALLEGRO_CHANNEL_CONF_5_1 = 6,
    ALLEGRO_CHANNEL_CONF_6_1 = 7,
    ALLEGRO_CHANNEL_CONF_7_1 = 8
} ALLEGRO_CHANNEL_CONF;

typedef enum {
    ALLEGRO_PLAYMODE_ONCE = 0,
    ALLEGRO_PLAYMODE_LOOP = 1,
    ALLEGRO_PLAYMODE_BIDIR = 2
} ALLEGRO_PLAYMODE;

typedef enum {
    ALLEGRO_MIXER_QUALITY_LOW = 0,
    ALLEGRO_MIXER_QUALITY_MEDIUM = 1,
    ALLEGRO_MIXER_QUALITY_HIGH = 2
} ALLEGRO_MIXER_QUALITY;

struct ALLEGRO_SAMPLE_ID {
    int _index;
    int _id;
};

bool al_install_audio(void);
void al_uninstall_audio(void);
bool al_init_acodec_addon(void);
bool al_is_audio_installed(void);
uint32_t al_get_allegro_audio_version(void);
bool al_reserve_samples(int reserve_samples);

ALLEGRO_SAMPLE* al_create_sample(void* buf, unsigned int samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf, bool free_buf);
void al_destroy_sample(ALLEGRO_SAMPLE* spl);
unsigned int al_get_sample_frequency(const ALLEGRO_SAMPLE* spl);
unsigned int al_get_sample_length(const ALLEGRO_SAMPLE* spl);
ALLEGRO_AUDIO_DEPTH al_get_sample_depth(const ALLEGRO_SAMPLE* spl);
ALLEGRO_CHANNEL_CONF al_get_sample_channels(const ALLEGRO_SAMPLE* spl);
void* al_get_sample_data(const ALLEGRO_SAMPLE* spl);
ALLEGRO_SAMPLE* al_load_sample(const char* filename);
ALLEGRO_SAMPLE* al_load_sample_f(ALLEGRO_FILE* fp, const char* ident);
bool al_save_sample(const char* filename, ALLEGRO_SAMPLE* spl);

ALLEGRO_SAMPLE_INSTANCE* al_create_sample_instance(ALLEGRO_SAMPLE* data);
void al_destroy_sample_instance(ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_play_sample_instance(ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_stop_sample_instance(ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_get_sample_instance_playing(const ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_set_sample_instance_playing(ALLEGRO_SAMPLE_INSTANCE* spl, bool val);
unsigned int al_get_sample_instance_position(const ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_set_sample_instance_position(ALLEGRO_SAMPLE_INSTANCE* spl, unsigned int pos);
unsigned int al_get_sample_instance_length(const ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_set_sample_instance_length(ALLEGRO_SAMPLE_INSTANCE* spl, unsigned int len);
float al_get_sample_instance_speed(const ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_set_sample_instance_speed(ALLEGRO_SAMPLE_INSTANCE* spl, float val);
float al_get_sample_instance_gain(const ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_set_sample_instance_gain(ALLEGRO_SAMPLE_INSTANCE* spl, float val);
float al_get_sample_instance_pan(const ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_set_sample_instance_pan(ALLEGRO_SAMPLE_INSTANCE* spl, float val);
ALLEGRO_PLAYMODE al_get_sample_instance_playmode(const ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_set_sample_instance_playmode(ALLEGRO_SAMPLE_INSTANCE* spl, ALLEGRO_PLAYMODE val);
ALLEGRO_AUDIO_DEPTH al_get_sample_instance_depth(const ALLEGRO_SAMPLE_INSTANCE* spl);
ALLEGRO_CHANNEL_CONF al_get_sample_instance_channels(const ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_get_sample_instance_attached(const ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_detach_sample_instance(ALLEGRO_SAMPLE_INSTANCE* spl);
bool al_set_sample(ALLEGRO_SAMPLE_INSTANCE* spl, ALLEGRO_SAMPLE* data);
ALLEGRO_SAMPLE* al_get_sample(ALLEGRO_SAMPLE_INSTANCE* spl);

ALLEGRO_MIXER* al_create_mixer(unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf);
void al_destroy_mixer(ALLEGRO_MIXER* mixer);
bool al_attach_sample_instance_to_mixer(ALLEGRO_SAMPLE_INSTANCE* stream, ALLEGRO_MIXER* mixer);
bool al_mixer_attach_sample(ALLEGRO_MIXER* mixer, ALLEGRO_SAMPLE* sample);
bool al_mixer_detach_sample(ALLEGRO_MIXER* mixer);
bool al_attach_audio_stream_to_mixer(ALLEGRO_AUDIO_STREAM* stream, ALLEGRO_MIXER* mixer);
unsigned int al_get_mixer_frequency(const ALLEGRO_MIXER* mixer);
ALLEGRO_CHANNEL_CONF al_get_mixer_channels(const ALLEGRO_MIXER* mixer);
ALLEGRO_AUDIO_DEPTH al_get_mixer_depth(const ALLEGRO_MIXER* mixer);
ALLEGRO_MIXER_QUALITY al_get_mixer_quality(const ALLEGRO_MIXER* mixer);
float al_get_mixer_gain(const ALLEGRO_MIXER* mixer);
bool al_get_mixer_playing(const ALLEGRO_MIXER* mixer);
bool al_get_mixer_attached(const ALLEGRO_MIXER* mixer);
bool al_set_mixer_frequency(ALLEGRO_MIXER* mixer, unsigned int val);
bool al_set_mixer_quality(ALLEGRO_MIXER* mixer, ALLEGRO_MIXER_QUALITY val);
bool al_set_mixer_gain(ALLEGRO_MIXER* mixer, float gain);
bool al_set_mixer_playing(ALLEGRO_MIXER* mixer, bool val);
bool al_detach_mixer(ALLEGRO_MIXER* mixer);

ALLEGRO_VOICE* al_create_voice(unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf);
void al_destroy_voice(ALLEGRO_VOICE* voice);
bool al_attach_sample_instance_to_voice(ALLEGRO_SAMPLE_INSTANCE* stream, ALLEGRO_VOICE* voice);
bool al_attach_audio_stream_to_voice(ALLEGRO_AUDIO_STREAM* stream, ALLEGRO_VOICE* voice);
bool al_attach_mixer_to_voice(ALLEGRO_MIXER* mixer, ALLEGRO_VOICE* voice);
void al_detach_voice(ALLEGRO_VOICE* voice);
unsigned int al_get_voice_frequency(const ALLEGRO_VOICE* voice);
unsigned int al_get_voice_position(const ALLEGRO_VOICE* voice);
ALLEGRO_CHANNEL_CONF al_get_voice_channels(const ALLEGRO_VOICE* voice);
ALLEGRO_AUDIO_DEPTH al_get_voice_depth(const ALLEGRO_VOICE* voice);
bool al_get_voice_playing(const ALLEGRO_VOICE* voice);
bool al_set_voice_position(ALLEGRO_VOICE* voice, unsigned int pos);
bool al_set_voice_playing(ALLEGRO_VOICE* voice, bool val);

ALLEGRO_AUDIO_STREAM* al_create_audio_stream(size_t buffer_count, unsigned int samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf);
void al_destroy_audio_stream(ALLEGRO_AUDIO_STREAM* stream);
void al_drain_audio_stream(ALLEGRO_AUDIO_STREAM* stream);
ALLEGRO_AUDIO_STREAM* al_load_audio_stream(const char* filename, size_t buffer_count, unsigned int samples);
ALLEGRO_AUDIO_STREAM* al_load_audio_stream_f(ALLEGRO_FILE* fp, const char* ident, size_t buffer_count, unsigned int samples);
unsigned int al_get_audio_stream_frequency(const ALLEGRO_AUDIO_STREAM* stream);
unsigned int al_get_audio_stream_length(const ALLEGRO_AUDIO_STREAM* stream);
unsigned int al_get_audio_stream_fragments(const ALLEGRO_AUDIO_STREAM* stream);
unsigned int al_get_available_audio_stream_fragments(const ALLEGRO_AUDIO_STREAM* stream);
float al_get_audio_stream_speed(const ALLEGRO_AUDIO_STREAM* stream);
bool al_set_audio_stream_speed(ALLEGRO_AUDIO_STREAM* stream, float val);
float al_get_audio_stream_gain(const ALLEGRO_AUDIO_STREAM* stream);
bool al_set_audio_stream_gain(ALLEGRO_AUDIO_STREAM* stream, float val);
float al_get_audio_stream_pan(const ALLEGRO_AUDIO_STREAM* stream);
bool al_set_audio_stream_pan(ALLEGRO_AUDIO_STREAM* stream, float val);
ALLEGRO_CHANNEL_CONF al_get_audio_stream_channels(const ALLEGRO_AUDIO_STREAM* stream);
ALLEGRO_AUDIO_DEPTH al_get_audio_stream_depth(const ALLEGRO_AUDIO_STREAM* stream);
ALLEGRO_PLAYMODE al_get_audio_stream_playmode(const ALLEGRO_AUDIO_STREAM* stream);
bool al_set_audio_stream_playmode(ALLEGRO_AUDIO_STREAM* stream, ALLEGRO_PLAYMODE val);
bool al_get_audio_stream_playing(const ALLEGRO_AUDIO_STREAM* stream);
bool al_set_audio_stream_playing(ALLEGRO_AUDIO_STREAM* stream, bool val);
bool al_get_audio_stream_attached(const ALLEGRO_AUDIO_STREAM* stream);
bool al_detach_audio_stream(ALLEGRO_AUDIO_STREAM* stream);
uint64_t al_get_audio_stream_played_samples(const ALLEGRO_AUDIO_STREAM* stream);
void* al_get_audio_stream_fragment(const ALLEGRO_AUDIO_STREAM* stream);
bool al_set_audio_stream_fragment(ALLEGRO_AUDIO_STREAM* stream, void* val);
bool al_rewind_audio_stream(ALLEGRO_AUDIO_STREAM* stream);
bool al_seek_audio_stream_secs(ALLEGRO_AUDIO_STREAM* stream, double time);
double al_get_audio_stream_position_secs(const ALLEGRO_AUDIO_STREAM* stream);
double al_get_audio_stream_length_secs(const ALLEGRO_AUDIO_STREAM* stream);

ALLEGRO_MIXER* al_get_default_mixer(void);
bool al_set_default_mixer(ALLEGRO_MIXER* mixer);
bool al_restore_default_mixer(void);

bool al_play_sample(ALLEGRO_SAMPLE* data, float gain, float pan, float speed, ALLEGRO_PLAYMODE loop, ALLEGRO_SAMPLE_ID* ret_id);
void al_stop_sample(ALLEGRO_SAMPLE_ID* spl_id);
void al_stop_samples(void);

#ifdef __cplusplus
}
#endif

#endif
