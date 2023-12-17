#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/inih-r57/ini.h"

typedef struct
{
    int version;
    const char* app_ext;
    const char* dump_dir; 
} configuration;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("Engine", "version")) {
        pconfig->version = atoi(value);
    } else if (MATCH("App", "app_ext")) {
        pconfig->app_ext = strdup(value);
    } else if (MATCH("App", "dump_dir")) {
        pconfig->dump_dir = strdup(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

int main(int argc, char* argv[])
{
    configuration config;

    if (ini_parse("config.ini", handler, &config) < 0) {
        printf("Can't load 'config.ini'\n");
        return 1;
    }
    printf("Config loaded from 'config.ini': version=%d, app_ext=%s, dump_dir=%s\n",
        config.version, config.app_ext, config.dump_dir);
    return 0;
}
