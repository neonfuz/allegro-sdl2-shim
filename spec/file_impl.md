# Allegro 5 File I/O Module Implementation Plan (SDL2 Backend)

## Overview

This document specifies the implementation of the Allegro 5 File I/O Module using standard C file I/O and SDL2 as the backend. The implementation provides configuration file management and file operations with feature parity to Allegro 5.

## Type Mappings

### Core File I/O Types

| Allegro Type | SDL2 Equivalent | Description |
|--------------|-----------------|-------------|
| `ALLEGRO_CONFIG` | Custom struct (`AllegroConfig`) | Configuration file in-memory representation |
| `ALLEGRO_CONFIG_SECTION` | Custom struct (`AllegroConfigSection`) | Section within config file |
| `ALLEGRO_CONFIG_ENTRY` | Custom struct (`AllegroConfigEntry`) | Key-value entry within section |
| `ALLEGRO_FILE` | Custom struct (`AllegroFile`) | File handle wrapper around FILE* |
| `ALLEGRO_FILE_INTERFACE` | Custom struct (`AllegroFileInterface`) | Function pointer struct for custom I/O |

### Enum Mappings

#### ALLEGRO_SEEK

| Allegro Value | SDL2 Equivalent | Notes |
|---------------|-----------------|-------|
| `ALLEGRO_SEEK_SET` | `SEEK_SET` | Seek from beginning |
| `ALLEGRO_SEEK_CUR` | `SEEK_CUR` | Seek from current position |
| `ALLEGRO_SEEK_END` | `SEEK_END` | Seek from end |

---

## Custom Struct Definitions

### AllegroConfig

```cpp
struct AllegroConfig {
    std::map<std::string, AllegroConfigSection*> sections;
    std::string filename;
};
```

### AllegroConfigSection

```cpp
struct AllegroConfigSection {
    std::string name;
    std::map<std::string, std::string> entries;
    std::vector<std::string> comments;
};
```

### AllegroConfigEntry

```cpp
struct AllegroConfigEntry {
    std::string key;
    std::string value;
};
```

### AllegroFile

```cpp
struct AllegroFile {
    FILE* file;
    bool is_memfile;
    void* mem_buffer;
    size_t mem_size;
    size_t mem_pos;
    bool owns_buffer;
    const AllegroFileInterface* interface;
    void* userdata;
    int error_code;
    std::string error_message;
};
```

### AllegroFileInterface

```cpp
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
    off_t (*fi_fsize)(ALLEGRO_FILE *f);
};
```

---

## Function Implementations

### 1. Configuration Creation and Destruction

#### al_create_config()

**Function Signature:**
```c
ALLEGRO_CONFIG* al_create_config(void);
```

**Standard C Equivalent:**
Create in-memory configuration structure.

**Implementation Details:**
1. Allocate new `AllegroConfig` structure
2. Create default "[general]" section
3. Initialize sections map
4. Return pointer to config or NULL on failure

---

#### al_destroy_config()

**Function Signature:**
```c
void al_destroy_config(ALLEGRO_CONFIG *config);
```

**Standard C Equivalent:**
Free configuration structure.

**Implementation Details:**
1. Check if config is NULL (return early)
2. Iterate through all sections
3. Free each section and its entries
4. Free config structure itself

---

### 2. Configuration Loading and Saving

#### al_load_config_file()

**Function Signature:**
```c
ALLEGRO_CONFIG* al_load_config_file(const char *filename);
```

**Standard C Equivalent:**
Parse INI-style file using standard C file I/O.

**Implementation Details:**
1. Open file with `fopen(filename, "r")`
2. Read file line by line using `fgets()`
3. Parse sections (lines starting with `[section_name]`)
4. Parse key=value pairs
5. Handle comments (lines starting with `;` or `#`)
6. Create `ALLEGRO_CONFIG` structure with parsed data
7. Close file
8. Return config or NULL on failure

---

#### al_save_config_file()

**Function Signature:**
```c
bool al_save_config_file(const char *filename, const ALLEGRO_CONFIG *config);
```

**Standard C Equivalent:**
Write configuration to file.

**Implementation Details:**
1. Open file with `fopen(filename, "w")`
2. Iterate through all sections
3. Write section headers: `[section_name]`
4. Write key=value pairs for each entry
5. Write comments if present
6. Close file
7. Return true on success, false on failure

---

#### al_load_config_file_f()

**Function Signature:**
```c
ALLEGRO_CONFIG* al_load_config_file_f(ALLEGRO_FILE *file);
```

**Standard C Equivalent:**
Parse configuration from ALLEGRO_FILE stream.

**Implementation Details:**
1. Read entire file contents using `al_fread()`
2. Parse using same logic as `al_load_config_file()`
3. Return parsed config

---

#### al_save_config_file_f()

**Function Signature:**
```c
bool al_save_config_file_f(ALLEGRO_FILE *file, const ALLEGRO_CONFIG *config);
```

**Standard C Equivalent:**
Write configuration to ALLEGRO_FILE stream.

**Implementation Details:**
1. Iterate through all sections
2. Write section headers and entries using `al_fprintf()`
3. Return true on success

---

### 3. Configuration Value Access

#### al_get_config_value()

**Function Signature:**
```c
const char* al_get_config_value(const ALLEGRO_CONFIG *config, const char *section, const char *key);
```

**Standard C Equivalent:**
Lookup value in section key-value map.

**Implementation Details:**
1. Find section by name in config sections map
2. If section not found, return NULL
3. Find key in section entries map
4. Return value string (owned by config, do not free)
5. Return NULL if key not found

---

#### al_set_config_value()

**Function Signature:**
```c
void al_set_config_value(ALLEGRO_CONFIG *config, const char *section, const char *key, const char *value);
```

**Standard C Equivalent:**
Set value in section key-value map.

**Implementation Details:**
1. Find existing section or create new one
2. Insert or update key-value pair in section entries
3. If value is NULL, treat as empty string

---

#### al_add_config_section()

**Function Signature:**
```c
void al_add_config_section(ALLEGRO_CONFIG *config, const char *name);
```

**Standard C Equivalent:**
Create new section in config.

**Implementation Details:**
1. Check if section already exists
2. If not, create new `AllegroConfigSection`
3. Add to config sections map

---

#### al_add_config_comment()

**Function Signature:**
```c
void al_add_config_comment(ALLEGRO_CONFIG *config, const char *section, const char *comment);
```

**Standard C Equivalent:**
Add comment to section.

**Implementation Details:**
1. Find or create section
2. Store comment in section's comments list

---

#### al_remove_config_section()

**Function Signature:**
```c
bool al_remove_config_section(ALLEGRO_CONFIG *config, const char *section);
```

**Standard C Equivalent:**
Remove section from config.

**Implementation Details:**
1. Find section in config sections map
2. If found, erase from map and free
3. Return true if removed, false if not found

---

#### al_remove_config_key()

**Function Signature:**
```c
bool al_remove_config_key(ALLEGRO_CONFIG *config, const char *section, const char *key);
```

**Standard C Equivalent:**
Remove key from section.

**Implementation Details:**
1. Find section
2. If section found, find and erase key from entries
3. Return true if removed, false otherwise

---

### 4. Configuration Iteration

#### al_get_first_config_section()

**Function Signature:**
```c
const char* al_get_first_config_section(const ALLEGRO_CONFIG *config, ALLEGRO_CONFIG_SECTION **iterator);
```

**Standard C Equivalent:**
Begin iteration over sections.

**Implementation Details:**
1. Initialize iterator to first section
2. Return section name or NULL if empty

---

#### al_get_next_config_section()

**Function Signature:**
```c
const char* al_get_next_config_section(ALLEGRO_CONFIG_SECTION **iterator);
```

**Standard C Equivalent:**
Continue iteration over sections.

**Implementation Details:**
1. Advance iterator to next section
2. Return section name or NULL if done

---

#### al_get_first_config_entry()

**Function Signature:**
```c
const char* al_get_first_config_entry(const ALLEGRO_CONFIG *config, const char *section, ALLEGRO_CONFIG_ENTRY **iterator);
```

**Standard C Equivalent:**
Begin iteration over entries in section.

**Implementation Details:**
1. Find section by name
2. Initialize iterator to first entry
3. Return key name or NULL if empty

---

#### al_get_next_config_entry()

**Function Signature:**
```c
const char* al_get_next_config_entry(ALLEGRO_CONFIG_ENTRY **iterator);
```

**Standard C Equivalent:**
Continue iteration over entries.

**Implementation Details:**
1. Advance iterator to next entry
2. Return key name or NULL if done

---

### 5. Configuration Merging

#### al_merge_config()

**Function Signature:**
```c
ALLEGRO_CONFIG* al_merge_config(const ALLEGRO_CONFIG *cfg1, const ALLEGRO_CONFIG *cfg2);
```

**Standard C Equivalent:**
Create new config from two configs merged.

**Implementation Details:**
1. Create new config
2. Copy all sections from cfg1
3. Copy all sections from cfg2 (cfg2 takes precedence)
4. Return merged config

---

#### al_merge_config_into()

**Function Signature:**
```c
void al_merge_config_into(ALLEGRO_CONFIG *master, const ALLEGRO_CONFIG *add);
```

**Standard C Equivalent:**
Merge second config into first.

**Implementation Details:**
1. Iterate through add config sections
2. Add sections/entries to master (add takes precedence)

---

### 6. Basic File Operations

#### al_fopen()

**Function Signature:**
```c
ALLEGRO_FILE* al_fopen(const char *path, const char *mode);
```

**Standard C Equivalent:**
Open file using standard C `fopen()`.

**Implementation Details:**
1. Call standard `fopen(path, mode)`
2. If successful, create `AllegroFile` wrapper
3. Store FILE* in wrapper
4. Set default file interface (stdio)
5. Return file handle or NULL on failure

---

#### al_fclose()

**Function Signature:**
```c
bool al_fclose(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Close file using `fclose()`.

**Implementation Details:**
1. Call `fclose(f->file)` to close underlying file
2. If memory file, free buffer if owned
3. Free `AllegroFile` structure
4. Return true on success

---

#### al_fread()

**Function Signature:**
```c
size_t al_fread(ALLEGRO_FILE *f, void *ptr, size_t size);
```

**Standard C Equivalent:**
Read bytes from file using `fread()`.

**Implementation Details:**
1. If using stdio interface: call `fread(ptr, 1, size, f->file)`
2. If using custom interface: call `f->interface->fi_fread(f, ptr, size)`
3. Update error state on failure
4. Return bytes read

---

#### al_fwrite()

**Function Signature:**
```c
size_t al_fwrite(ALLEGRO_FILE *f, const void *ptr, size_t size);
```

**Standard C Equivalent:**
Write bytes to file using `fwrite()`.

**Implementation Details:**
1. If using stdio interface: call `fwrite(ptr, 1, size, f->file)`
2. If using custom interface: call `f->interface->fi_fwrite(f, ptr, size)`
3. Update error state on failure
4. Return bytes written

---

#### al_fflush()

**Function Signature:**
```c
bool al_fflush(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Flush file buffers using `fflush()`.

**Implementation Details:**
1. If using stdio interface: call `fflush(f->file)`
2. If using custom interface: call `f->interface->fi_fflush(f)`
3. Return true on success

---

#### al_ftell()

**Function Signature:**
```c
int64_t al_ftell(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Get file position using `ftell()`.

**Implementation Details:**
1. If using stdio interface: call `ftell(f->file)`
2. If using custom interface: call `f->interface->fi_ftell(f)`
3. Return position or -1 on error

---

#### al_fseek()

**Function Signature:**
```c
bool al_fseek(ALLEGRO_FILE *f, int64_t offset, int whence);
```

**Standard C Equivalent:**
Seek file position using `fseek()`.

**Implementation Details:**
1. Map ALLEGRO_SEEK enum to SEEK_SET/SEEK_CUR/SEEK_END
2. If using stdio interface: call `fseek(f->file, offset, whence)`
3. If using custom interface: call `f->interface->fi_fseek(f, offset, whence)`
4. Return true on success

---

#### al_feof()

**Function Signature:**
```c
bool al_feof(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Check EOF flag using `feof()`.

**Implementation Details:**
1. If using stdio interface: call `feof(f->file)`
2. If using custom interface: call `f->interface->fi_feof(f)`
3. Return true if at EOF

---

#### al_ferror()

**Function Signature:**
```c
int al_ferror(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Check error flag using `ferror()`.

**Implementation Details:**
1. If using stdio interface: call `ferror(f->file)`
2. If using custom interface: return stored error code
3. Return non-zero if error

---

#### al_ferrmsg()

**Function Signature:**
```c
const char* al_ferrmsg(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Get error message string.

**Implementation Details:**
1. Return stored error message
2. Use strerror() to populate if needed

---

#### al_fclearerr()

**Function Signature:**
```c
void al_fclearerr(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Clear EOF and error flags using `clearerr()`.

**Implementation Details:**
1. If using stdio interface: call `clearerr(f->file)`
2. If using custom interface: call `f->interface->fi_fclearerr(f)`
3. Clear stored error code and message

---

#### al_fungetc()

**Function Signature:**
```c
int al_fungetc(ALLEGRO_FILE *f, int c);
```

**Standard C Equivalent:**
Push character back using `ungetc()`.

**Implementation Details:**
1. If using stdio interface: call `ungetc(c, f->file)`
2. For memory files, decrement position and store in buffer
3. Return c on success, EOF on failure

---

#### al_fsize()

**Function Signature:**
```c
int64_t al_fsize(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Get file size.

**Implementation Details:**
1. Save current position
2. Seek to end: `fseek(f, 0, SEEK_END)`
3. Get position: `ftell(f)`
4. Restore original position
5. Return file size or -1 on error

---

### 7. Character and String I/O

#### al_fgetc()

**Function Signature:**
```c
int al_fgetc(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Read single character using `fgetc()`.

**Implementation Details:**
1. Read one byte using `al_fread()`
2. Return byte as int or EOF on error/EOF

---

#### al_fputc()

**Function Signature:**
```c
int al_fputc(ALLEGRO_FILE *f, int c);
```

**Standard C Equivalent:**
Write single character using `fputc()`.

**Implementation Details:**
1. Write one byte using `al_fwrite()`
2. Return c on success, EOF on failure

---

#### al_fgets()

**Function Signature:**
```c
char* al_fgets(ALLEGRO_FILE *f, char * const p, size_t max);
```

**Standard C Equivalent:**
Read line using `fgets()`.

**Implementation Details:**
1. Read up to max-1 characters
2. Stop at newline or EOF
3. Null-terminate string
4. Return p on success, NULL on failure

---

#### al_fputs()

**Function Signature:**
```c
int al_fputs(ALLEGRO_FILE *f, const char *p);
```

**Standard C Equivalent:**
Write string using `fputs()`.

**Implementation Details:**
1. Write string using `al_fwrite(strlen(p))`
2. Return non-negative on success, EOF on failure

---

#### al_fget_ustr()

**Function Signature:**
```c
ALLEGRO_USTR* al_fget_ustr(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Read line as ALLEGRO_USTR (UTF-8 string).

**Implementation Details:**
1. Read line into buffer using `al_fgets()`
2. Convert to ALLEGRO_USTR if needed
3. Return string or NULL on failure

---

### 8. Formatted I/O

#### al_fprintf()

**Function Signature:**
```c
int al_fprintf(ALLEGRO_FILE *f, const char *format, ...);
```

**Standard C Equivalent:**
Formatted print using `fprintf()`.

**Implementation Details:**
1. Format using va_list
2. Write to file using `al_fwrite()`
3. Return characters written or negative on error

---

#### al_vfprintf()

**Function Signature:**
```c
int al_vfprintf(ALLEGRO_FILE *f, const char* format, va_list args);
```

**Standard C Equivalent:**
Formatted print with va_list.

**Implementation Details:**
1. Format string with args
2. Write to file using `al_fwrite()`
3. Return characters written or negative on error

---

### 9. Binary I/O (16-bit)

#### al_fread16le()

**Function Signature:**
```c
int16_t al_fread16le(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Read 16-bit little-endian integer.

**Implementation Details:**
1. Read 2 bytes using `al_fread()`
2. Combine bytes in little-endian order
3. Return 16-bit value or -1 on error

---

#### al_fread16be()

**Function Signature:**
```c
int16_t al_fread16be(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Read 16-bit big-endian integer.

**Implementation Details:**
1. Read 2 bytes using `al_fread()`
2. Combine bytes in big-endian order
3. Return 16-bit value or -1 on error

---

#### al_fwrite16le()

**Function Signature:**
```c
size_t al_fwrite16le(ALLEGRO_FILE *f, int16_t w);
```

**Standard C Equivalent:**
Write 16-bit little-endian integer.

**Implementation Details:**
1. Split 16-bit value into 2 bytes (little-endian)
2. Write bytes using `al_fwrite()`
3. Return 2 on success

---

#### al_fwrite16be()

**Function Signature:**
```c
size_t al_fwrite16be(ALLEGRO_FILE *f, int16_t w);
```

**Standard C Equivalent:**
Write 16-bit big-endian integer.

**Implementation Details:**
1. Split 16-bit value into 2 bytes (big-endian)
2. Write bytes using `al_fwrite()`
3. Return 2 on success

---

### 10. Binary I/O (32-bit)

#### al_fread32le()

**Function Signature:**
```c
int32_t al_fread32le(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Read 32-bit little-endian integer.

**Implementation Details:**
1. Read 4 bytes using `al_fread()`
2. Combine bytes in little-endian order
3. Return 32-bit value or -1 on error

---

#### al_fread32be()

**Function Signature:**
```c
int32_t al_fread32be(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Read 32-bit big-endian integer.

**Implementation Details:**
1. Read 4 bytes using `al_fread()`
2. Combine bytes in big-endian order
3. Return 32-bit value or -1 on error

---

#### al_fwrite32le()

**Function Signature:**
```c
size_t al_fwrite32le(ALLEGRO_FILE *f, int32_t l);
```

**Standard C Equivalent:**
Write 32-bit little-endian integer.

**Implementation Details:**
1. Split 32-bit value into 4 bytes (little-endian)
2. Write bytes using `al_fwrite()`
3. Return 4 on success

---

#### al_fwrite32be()

**Function Signature:**
```c
size_t al_fwrite32be(ALLEGRO_FILE *f, int32_t l);
```

**Standard C Equivalent:**
Write 32-bit big-endian integer.

**Implementation Details:**
1. Split 32-bit value into 4 bytes (big-endian)
2. Write bytes using `al_fwrite()`
3. Return 4 on success

---

### 11. File Interface Management

#### al_fopen_interface()

**Function Signature:**
```c
ALLEGRO_FILE* al_fopen_interface(const ALLEGRO_FILE_INTERFACE *vt, const char *path, const char *mode);
```

**Standard C Equivalent:**
Open file with custom interface.

**Implementation Details:**
1. Call `vt->fi_fopen(path, mode)` to get handle
2. Create `AllegroFile` wrapper
3. Store custom interface and userdata
4. Return file handle or NULL on failure

---

#### al_create_file_handle()

**Function Signature:**
```c
ALLEGRO_FILE* al_create_file_handle(const ALLEGRO_FILE_INTERFACE *vt, void *userdata);
```

**Standard C Equivalent:**
Create file handle with custom userdata.

**Implementation Details:**
1. Allocate `AllegroFile` structure
2. Set file to NULL (for custom I/O)
3. Store custom interface and userdata
4. Return file handle

---

#### al_get_new_file_interface()

**Function Signature:**
```c
const ALLEGRO_FILE_INTERFACE* al_get_new_file_interface(void);
```

**Standard C Equivalent:**
Get current file interface.

**Implementation Details:**
1. Return pointer to current thread-local file interface
2. Default is standard file interface

---

#### al_set_new_file_interface()

**Function Signature:**
```c
void al_set_new_file_interface(const ALLEGRO_FILE_INTERFACE *file_interface);
```

**Standard C Equivalent:**
Set current file interface.

**Implementation Details:**
1. Store pointer in thread-local storage
2. New files will use this interface

---

#### al_set_standard_file_interface()

**Function Signature:**
```c
void al_set_standard_file_interface(void);
```

**Standard C Equivalent:**
Reset to standard file interface.

**Implementation Details:**
1. Set file interface to stdio implementation
2. Use fopen/fread/fwrite/etc.

---

#### al_get_file_userdata()

**Function Signature:**
```c
void* al_get_file_userdata(ALLEGRO_FILE *f);
```

**Standard C Equivalent:**
Get userdata from file handle.

**Implementation Details:**
1. Return `f->userdata` pointer

---

### 12. Additional File Operations

#### al_fopen_fd()

**Function Signature:**
```c
ALLEGRO_FILE* al_fopen_fd(int fd, const char *mode);
```

**Standard C Equivalent:**
Open file from file descriptor.

**Implementation Details:**
1. Use `fdopen()` to wrap fd with FILE*
2. Create `AllegroFile` wrapper
3. Return handle or NULL on failure

---

#### al_make_temp_file()

**Function Signature:**
```c
ALLEGRO_FILE* al_make_temp_file(const char *tmpl, ALLEGRO_PATH **ret_path);
```

**Standard C Equivalent:**
Create temporary file.

**Implementation Details:**
1. Use `mkstemp()` to create temp file
2. Return file handle
3. Optionally return path via ret_path

---

#### al_fopen_slice()

**Function Signature:**
```c
ALLEGRO_FILE* al_fopen_slice(ALLEGRO_FILE *fp, size_t initial_size, const char *mode);
```

**Standard C Equivalent:**
Create view into existing file.

**Implementation Details:**
1. Create new AllegroFile wrapping same FILE*
2. Track slice boundaries
3. Return slice handle

---

### 13. Memory File Operations

#### al_open_memfile()

**Function Signature:**
```c
ALLEGRO_FILE* al_open_memfile(void *mem, size_t size, const char *mode);
```

**Standard C Equivalent:**
Open in-memory file using custom interface.

**Implementation Details:**
1. Create `AllegroFile` with memory file interface
2. Store pointer to memory buffer
3. Store buffer size
4. Set initial position to 0
5. Support read/write modes based on buffer ownership
6. Return memory file handle

**Memory File Interface Implementation:**
- `fi_fread`: Copy from buffer, advance position
- `fi_fwrite`: Copy to buffer if not read-only, advance position
- `fi_fseek`: Set position within bounds
- `fi_ftell`: Return current position
- `fi_feof`: Check if position >= size
- `fi_fsize`: Return buffer size

---

## Implementation Notes

### Configuration File Format

The configuration system uses INI-style format:
- Sections: `[section_name]`
- Key-Value: `key=value`
- Comments: Lines starting with `;` or `#`

### Thread Safety

- File interface is stored in thread-local storage
- Configuration structures are not inherently thread-safe
- User must provide synchronization for concurrent access

### Error Handling

- Store error code and message in `AllegroFile` structure
- Use `al_ferrmsg()` to retrieve human-readable error
- Clear errors with `al_fclearerr()` before operations

### Memory Management

- Memory files can own or borrow their buffer
- Set `owns_buffer` flag to determine deallocation behavior
- Configuration strings are owned by the config structure

---

## New Details from SDL Source Analysis

### SDL2 File I/O APIs

The repository contains **SDL2** with file I/O support:

1. **SDL_RWops**: Core file I/O abstraction
   - `SDL_RWFromFile()` - Open file
   - `SDL_RWFromMem()` - Memory buffer
   - `SDL_RWFromConstMem()` - Read-only memory
   - Read/write/seek operations

2. **SDL_filesystem.h** - File system utilities:
   - `SDL_GetBasePath()` - Application base directory
   - `SDL_GetPrefPath()` - User preferences directory
   - `SDL_GetCurrentDirectory()` - Current working directory

3. **Implementation**: Located in `src/file/`

### SDL2 Implementation Notes

1. **Memory Files**: Use `SDL_RWFromMem()` for in-memory file operations
2. **Custom I/O**: Implement `SDL_RWops` interface for custom sources
3. **Path Handling**: Use SDL_filesystem functions for portable path handling

### Dependencies Update

| Header | Notes |
|--------|-------|
| SDL2/SDL_filesystem.h | Directory/path operations |
| SDL2/SDL_rwops.h | File I/O |
