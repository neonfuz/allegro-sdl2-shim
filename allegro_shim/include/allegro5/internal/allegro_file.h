#ifndef ALLEGRO_INTERNAL_FILE_H
#define ALLEGRO_INTERNAL_FILE_H

#include <cstdio>
#include <string>
#include "../allegro_file.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AllegroFile {
    FILE* file;
    bool is_memfile;
    void* mem_buffer;
    size_t mem_size;
    size_t mem_pos;
    bool owns_buffer;
    const ALLEGRO_FILE_INTERFACE* interface;
    void* userdata;
    int error_code;
    std::string error_message;
};

#ifdef __cplusplus
}
#endif

#endif
