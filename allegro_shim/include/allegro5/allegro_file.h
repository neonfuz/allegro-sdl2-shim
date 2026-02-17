#ifndef ALLEGRO_FILE_H
#define ALLEGRO_FILE_H

#include "allegro_base.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

struct AllegroFile;
struct AllegroFileInterface;
struct AllegroPath;
struct AllegroFsEntry;

typedef struct AllegroFile ALLEGRO_FILE;
typedef struct AllegroFileInterface ALLEGRO_FILE_INTERFACE;
typedef struct AllegroPath ALLEGRO_PATH;
typedef struct AllegroFsEntry ALLEGRO_FS_ENTRY;

enum {
    ALLEGRO_SEEK_SET = 0,
    ALLEGRO_SEEK_CUR = 1,
    ALLEGRO_SEEK_END = 2
};

enum ALLEGRO_FILE_MODE {
    ALLEGRO_FILEMODE_READ    = 1,
    ALLEGRO_FILEMODE_WRITE   = 2,
    ALLEGRO_FILEMODE_EXECUTE = 4,
    ALLEGRO_FILEMODE_HIDDEN  = 8,
    ALLEGRO_FILEMODE_ISFILE  = 16,
    ALLEGRO_FILEMODE_ISDIR   = 32
};

struct AllegroFileInterface {
    void* (*fi_fopen)(const char *path, const char *mode);
    bool (*fi_fclose)(ALLEGRO_FILE *handle);
    size_t (*fi_fread)(ALLEGRO_FILE *f, void *ptr, size_t size);
    size_t (*fi_fwrite)(ALLEGRO_FILE *f, const void *ptr, size_t size);
    bool (*fi_fflush)(ALLEGRO_FILE *f);
    int64_t (*fi_ftell)(ALLEGRO_FILE *f);
    bool (*fi_fseek)(ALLEGRO_FILE *f, int64_t offset, int whence);
    bool (*fi_feof)(ALLEGRO_FILE *f);
    int (*fi_ferror)(ALLEGRO_FILE *f);
    const char* (*fi_ferrmsg)(ALLEGRO_FILE *f);
    void (*fi_fclearerr)(ALLEGRO_FILE *f);
    int (*fi_fungetc)(ALLEGRO_FILE *f, int c);
    int64_t (*fi_fsize)(ALLEGRO_FILE *f);
};

ALLEGRO_FILE* al_fopen(const char *path, const char *mode);
ALLEGRO_FILE* al_fopen_interface(const ALLEGRO_FILE_INTERFACE *vt, const char *path, const char *mode);
ALLEGRO_FILE* al_create_file_handle(const ALLEGRO_FILE_INTERFACE *vt, void *userdata);
bool al_fclose(ALLEGRO_FILE *f);

size_t al_fread(ALLEGRO_FILE *f, void *ptr, size_t size);
size_t al_fwrite(ALLEGRO_FILE *f, const void *ptr, size_t size);
bool al_fflush(ALLEGRO_FILE *f);
int64_t al_ftell(ALLEGRO_FILE *f);
bool al_fseek(ALLEGRO_FILE *f, int64_t offset, int whence);
bool al_feof(ALLEGRO_FILE *f);
int al_ferror(ALLEGRO_FILE *f);
const char* al_ferrmsg(ALLEGRO_FILE *f);
void al_fclearerr(ALLEGRO_FILE *f);
int al_fungetc(ALLEGRO_FILE *f, int c);
int64_t al_fsize(ALLEGRO_FILE *f);

int al_fgetc(ALLEGRO_FILE *f);
int al_fputc(ALLEGRO_FILE *f, int c);
char* al_fgets(ALLEGRO_FILE *f, char * const p, size_t max);
int al_fputs(ALLEGRO_FILE *f, const char *p);

int al_fprintf(ALLEGRO_FILE *f, const char *format, ...);
int al_vfprintf(ALLEGRO_FILE *f, const char* format, va_list args);

int16_t al_fread16le(ALLEGRO_FILE *f);
int16_t al_fread16be(ALLEGRO_FILE *f);
size_t al_fwrite16le(ALLEGRO_FILE *f, int16_t w);
size_t al_fwrite16be(ALLEGRO_FILE *f, int16_t w);

int32_t al_fread32le(ALLEGRO_FILE *f);
int32_t al_fread32be(ALLEGRO_FILE *f);
size_t al_fwrite32le(ALLEGRO_FILE *f, int32_t l);
size_t al_fwrite32be(ALLEGRO_FILE *f, int32_t l);

const ALLEGRO_FILE_INTERFACE* al_get_new_file_interface(void);
void al_set_new_file_interface(const ALLEGRO_FILE_INTERFACE *file_interface);
void al_set_standard_file_interface(void);
void* al_get_file_userdata(ALLEGRO_FILE *f);

ALLEGRO_FILE* al_fopen_fd(int fd, const char *mode);
ALLEGRO_FILE* al_make_temp_file(const char *tmpl, ALLEGRO_PATH **ret_path);
ALLEGRO_FILE* al_fopen_slice(ALLEGRO_FILE *fp, size_t initial_size, const char *mode);

ALLEGRO_FILE* al_open_memfile(void *mem, size_t size, const char *mode);

#ifdef __cplusplus
}
#endif

#endif
