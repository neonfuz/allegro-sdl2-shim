#ifndef ALLEGRO_CONFIG_H
#define ALLEGRO_CONFIG_H

#include "allegro_base.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AllegroConfig;
struct AllegroConfigSection;
struct AllegroConfigEntry;
struct ALLEGRO_FILE;

typedef struct AllegroConfig ALLEGRO_CONFIG;
typedef struct AllegroConfigSection ALLEGRO_CONFIG_SECTION;
typedef struct AllegroConfigEntry ALLEGRO_CONFIG_ENTRY;
typedef struct ALLEGRO_FILE ALLEGRO_FILE;

ALLEGRO_CONFIG* al_create_config(void);
void al_destroy_config(ALLEGRO_CONFIG* config);
ALLEGRO_CONFIG* al_load_config_file(const char* filename);
ALLEGRO_CONFIG* al_load_config_f(ALLEGRO_FILE* file, const char* origin);
bool al_save_config_file(const char* filename, const ALLEGRO_CONFIG* config);
bool al_save_config_f(ALLEGRO_FILE* file, const ALLEGRO_CONFIG* config);

void al_add_config_section(ALLEGRO_CONFIG* config, const char* name);
void al_add_config_comment(ALLEGRO_CONFIG* config, const char* section, const char* comment);
bool al_remove_config_section(ALLEGRO_CONFIG* config, const char* section);
bool al_remove_config_key(ALLEGRO_CONFIG* config, const char* section, const char* key);

const char* al_get_config_value(const ALLEGRO_CONFIG* config, const char* section, const char* key, const char* default_value);
void al_set_config_value(ALLEGRO_CONFIG* config, const char* section, const char* key, const char* value);

const char* al_get_first_config_section(const ALLEGRO_CONFIG* config, ALLEGRO_CONFIG_SECTION** iterator);
const char* al_get_next_config_section(ALLEGRO_CONFIG_SECTION** iterator);
const char* al_get_first_config_entry(const ALLEGRO_CONFIG* config, const char* section, ALLEGRO_CONFIG_ENTRY** iterator);
const char* al_get_next_config_entry(ALLEGRO_CONFIG_ENTRY** iterator);

ALLEGRO_CONFIG* al_merge_config(const ALLEGRO_CONFIG* cfg1, const ALLEGRO_CONFIG* cfg2);
void al_merge_config_into(ALLEGRO_CONFIG* master, const ALLEGRO_CONFIG* add);

#ifdef __cplusplus
}
#endif

#endif
