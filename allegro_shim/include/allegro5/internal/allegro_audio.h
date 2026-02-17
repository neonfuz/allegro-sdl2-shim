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

#endif
