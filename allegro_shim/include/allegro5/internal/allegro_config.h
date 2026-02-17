#ifndef ALLEGRO_INTERNAL_CONFIG_H
#define ALLEGRO_INTERNAL_CONFIG_H

#include <map>
#include <string>
#include <vector>
#include "allegro5/allegro_config.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AllegroConfig {
    std::map<std::string, AllegroConfigSection*> sections;
    std::string filename;
};

struct AllegroConfigSection {
    std::string name;
    std::map<std::string, std::string> entries;
    std::vector<std::string> comments;
};

struct AllegroConfigEntry {
    std::string key;
    std::string value;
};

#ifdef __cplusplus
}
#endif

#endif
