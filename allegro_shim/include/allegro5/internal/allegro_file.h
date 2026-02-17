#ifndef ALLEGRO_INTERNAL_FILE_H
#define ALLEGRO_INTERNAL_FILE_H

#include <cstdio>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
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

struct AllegroFsEntry {
    std::string path;
    uint32_t mode;
    time_t atime;
    time_t mtime;
    time_t ctime;
    off_t size;
    bool exists;
    DIR* dir;
};

#ifdef __cplusplus
}
#endif

#endif
