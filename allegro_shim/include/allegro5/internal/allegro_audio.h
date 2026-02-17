#ifndef ALLEGRO_INTERNAL_AUDIO_H
#define ALLEGRO_INTERNAL_AUDIO_H

#include <stdint.h>

struct ALLEGRO_SAMPLE {
    unsigned int num_samples;
    unsigned int frequency;
    int depth;
    int chan_conf;
    void* data;
    bool free_buffer;
};

struct ALLEGRO_VOICE {
    unsigned int frequency;
    int depth;
    int chan_conf;
    bool is_playing;
    unsigned int position;
    void* source;
    enum { SOURCE_NONE, SOURCE_SAMPLE, SOURCE_STREAM, SOURCE_MIXER } source_type;
};

#endif
