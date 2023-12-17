#ifndef ENGINE_H
#define ENGINE_H

#define _POSIX_C_SOURCE 200809L

#ifdef DEBUG
#define debug(msg, ...)                             \
    do                                              \
    {                                               \
        printf("(DEBUG) " msg "\n", ##__VA_ARGS__); \
    } while (0)
#else
#define debug(msg, ...)
#endif

#define CONFIG_DIR "config.ini"

#define RES_BUFFER_LEN          512
#define RES_BUFFER_LEN_SM       8
#define ACTION_BUFFER_LEN       64

#define STD_BUFFER_LEN      128
#define GENE_LEN            16

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "lib/inih-r57/ini.h"

#define STB_DS_IMPLEMENTATION
#include "lib/stb/stb_ds.h"

typedef struct
{
    int version;
    const char *app_name;
    const char *app_author;
    const char *app_link;
    const char *app_root;
    const char *app_entry;
} configuration;

// Frame relevant function
typedef uint16_t Gene;

typedef struct Frame_t
{
    int id;
    int prev_id;
    int next_id;
    char action[ACTION_BUFFER_LEN];
    Gene gene;
    char res[RES_BUFFER_LEN];
} Frame;

typedef struct Result_t
{
    int is_ok;
    char msg[STD_BUFFER_LEN];
    void *ptr;
} Result;

// Utils
/*
 * Join the two path
 */
#define JOIN(dest, path1, path2) \
    sprintf(dest, "%s/%s", path1, path2);


/*
 * Gene structure
 * 
 *  ____________________
 * |_|_|__|____|________|
 *  1 2  3   4      5
 * 
 * 1. 1bit using extra flag map
 * 2. 1bit diry bit
 * 3. 2bit reserved
 * 4. 4bit gene info
 * 5. 8bit flags
 * 
 */

#define USE_EXTRA_MAP 0x8000    // 1000 0000 0000 0000 
#define IS_DIRTY 0x4000         // 0100 0000 0000 0000 


// Flag MAP
#define FRAME_IS_DEFAUT 0b00000000
#define FRAME_IS_BRANCH 0b00000001
#define FRAME_IS_BACKGROUND 0b00000010

#define MUSIC_IS_DEFAULT 0b00000000
#define MUSIC_IS_STOP_PERM 0b00000100
#define MUSIC_IS_STOP_TMP 0b00001000
#define MUSIC_IS_SWITCHED 0b00001100

#ifdef USE_EXTRA_MAP

/*
 * define your own map here, when match the flag
 * it will prior match the extra map, then use xor
 * to get other default flag. Each frame can only
 * have one extra flag.
 *
 */
#define EX_FRAME_IS_DEMO 0b00000001

#endif

#endif