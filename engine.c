#include "engine.h"

static configuration config;
static FILE *ENTRY_FILE;

static Frame **frame_array = NULL;

static Frame *frame_head = NULL;
static Frame *frame_tail = NULL;

static int config_loader(void *user, const char *section, const char *name,
                         const char *value)
{
    configuration *pconfig = (configuration *)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("Engine", "version"))
        pconfig->version = atoi(value);
    else if (MATCH("App", "name"))
        pconfig->app_name = strdup(value);
    else if (MATCH("App", "author"))
        pconfig->app_author = strdup(value);
    else if (MATCH("App", "link"))
        pconfig->app_link = strdup(value);
    else if (MATCH("App", "root"))
        pconfig->app_root = strdup(value);
    else if (MATCH("App", "entry"))
        pconfig->app_entry = strdup(value);
    else
        return 0; // raise error is no such entry
    return 1;
}

Result create_frame()
{
    Frame *frame_buf;
    Result alloc_result;

    // create frame buffer
    frame_buf = (Frame *)malloc(sizeof(frame_buf));
    if (frame_buf == NULL)
    {
        debug("malloc frame_buf failed, when insert frame after id: %d", prev_id);
        return (Result){
            .is_ok = 0,
            .msg = "allocate frame_buf failed"};
    }

    return (Result) {
        .is_ok = 1,
        .ptr = (void *)frame_buf,
    };
}

/*
 * append the frame into the tail
 *
 */
Result append_frame(char action[ACTION_BUFFER_LEN], char res[RES_BUFFER_LEN], Gene gene)
{
    Frame *frame_buf;
    Result alloc_result;

    // create frame buffer
    alloc_result = create_frame();
    if (!alloc_result.is_ok) {
        return alloc_result;
    }
    frame_buf = (Frame *)alloc_result.ptr;

    frame_buf->id = arrlen(frame_array);
    frame_buf->gene = gene;
    frame_buf->next_id = -1;
    snprintf(frame_buf->action, ACTION_BUFFER_LEN, "%s", action);
    snprintf(frame_buf->res, RES_BUFFER_LEN, "%s", res);

    if (frame_tail == NULL)
    {
        // first node ever
        frame_buf->prev_id = -1;

        frame_head = frame_buf;
        frame_tail = frame_buf;
        arrput(frame_array, frame_buf);

        return (Result){
            .is_ok = 1,
        };
    }

    frame_buf->prev_id = frame_tail->id;
    frame_tail->next_id = frame_buf->id;

    frame_tail = frame_buf;
    return (Result){
        .is_ok = 1,
    };
}

/*
 * if no prev_id i.e. add to head, set it to -1
 *
 */
Result insert_frame(int prev_id, char action[ACTION_BUFFER_LEN], char res[RES_BUFFER_LEN], Gene gene)
{
    Frame *frame_buf;
    Result alloc_result;

    if (prev_id >= arrlen(frame_array))
    {
        fprintf(stderr, "prev_id: %d not exist\n", prev_id);
        Result response = {
            .is_ok = 0,
        };
        sprintf(response.msg, "prev_id: %d not exist\n", prev_id);
        return response;
    }

    // create frame buffer
    alloc_result = create_frame();
    if (!alloc_result.is_ok) {
        return alloc_result;
    }
    frame_buf = (Frame *)alloc_result.ptr;

    frame_buf->id = arrlen(frame_array);
    frame_buf->gene = gene;
    snprintf(frame_buf->action, ACTION_BUFFER_LEN, "%s", action);
    snprintf(frame_buf->res, RES_BUFFER_LEN, "%s", res);

    // add to head and handle inserting first node
    if (prev_id == -1)
    {
        frame_buf->prev_id = -1;
        frame_buf->next_id = frame_head != NULL ? frame_head->id : -1;

        if (frame_head != NULL)
        {
            frame_head->prev_id = frame_buf->id;
            frame_head = frame_buf;
        }
        else
        {
            // first node ever
            frame_head = frame_buf;
            frame_tail = frame_buf;
        }

        arrput(frame_array, frame_buf);

        return (Result){
            .is_ok = 1,
        };
    }

    // add to tail
    if (prev_id == frame_tail->id) {
        return append_frame(action, res, gene);
    }

    // insert into arbirary place
    frame_buf->prev_id = frame_array[prev_id]->id,
    frame_buf->next_id = frame_array[prev_id]->next_id,

    frame_array[prev_id]->next_id = frame_buf->id;
    frame_array[frame_buf->next_id]->prev_id = frame_buf->id;

    arrput(frame_array, frame_buf);
    return (Result){
        .is_ok = 1,
    };
}

int main()
{
    char buf[STD_BUFFER_LEN];

    // load config
    if (ini_parse(CONFIG_DIR, config_loader, &config) < 0)
    {
        printf("Can't load '%s'\n", CONFIG_DIR);
        return 1;
    }

    JOIN(buf, config.app_root, config.app_entry);

    // open/create the entry file
    ENTRY_FILE = fopen(buf, "r+");
    if (ENTRY_FILE == NULL)
    {
        ENTRY_FILE = fopen(buf, "w+");
        if (ENTRY_FILE == NULL)
        {
            perror("Error opening/creating file");
            return -EIO;
        }
        printf("create/");
    }

    // initialize the frame list
    frame_array = NULL; // TODO: change after finishing serialize part
    printf("bind to entry: %s\n", buf);
    return 0;
}
