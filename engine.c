#include "engine.h"

static configuration config;

// data structure to maintain the frame list
// be careful that we need to ensure the id for each frame_array
// must corresponded to its index at list in memory!!!
static Frame **frame_array = NULL;

static int frame_head = -1;
static int frame_tail = -1;

/*
 * ini file loader callback
 *
 */
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
    else if (MATCH("App", "meta"))
        pconfig->meta_entry = strdup(value);

    else
        return 0; // raise error is no such entry
    return 1;
}

/*
 * free frame list
 *
 */
static void free_frame_array()
{
    for (int i = 0; i < arrlen(frame_array); ++i)
    {
        debug("free: %d", frame_array[i]->id);
        free(frame_array[i]);
    }
    arrfree(frame_array);
    frame_array = NULL;
    frame_head = -1;
    frame_tail = -1;
}

static int load_config()
{
    if (ini_parse(CONFIG_DIR, config_loader, &config) < 0)
    {
        printf("Can't load '%s'\n", CONFIG_DIR);
        return 1;
    }
    return 0;
}

static void free_config()
{
    free((void *)config.app_author);
    free((void *)config.app_entry);
    free((void *)config.app_link);
    free((void *)config.app_name);
    free((void *)config.app_root);
    free((void *)config.meta_entry);
}

/*
 * allocate frame on heap
 *
 */
Result create_frame()
{
    Frame *frame_buf;

    // create frame buffer
    frame_buf = (Frame *)malloc(sizeof(Frame));
    if (frame_buf == NULL)
    {
        debug("malloc frame_buf failed, when insert frame after id");
        return (Result){
            .is_ok = 0,
            .msg = "allocate frame_buf failed"};
    }

    return (Result){
        .is_ok = 1,
        .ptr = (void *)frame_buf,
    };
}

/*
 * append frame into the tail
 *
 */
Result append_frame(char *action, char *res, Gene gene)
{
    Frame *frame_buf;
    Result ret;

    // create frame buffer
    ret = create_frame();
    if (!ret.is_ok)
    {
        return ret;
    }
    frame_buf = (Frame *)ret.ptr;

    frame_buf->id = arrlen(frame_array);
    frame_buf->gene = gene;
    frame_buf->next_id = -1;
    snprintf(frame_buf->action, ACTION_BUFFER_LEN, "%s", action);
    snprintf(frame_buf->res, RES_BUFFER_LEN, "%s", res);

    if (frame_tail == -1)
    {
        // first node ever
        frame_buf->prev_id = -1;

        frame_head = frame_buf->id;
        frame_tail = frame_buf->id;
        arrput(frame_array, frame_buf);

        return (Result){
            .is_ok = 1,
            .ptr = (void *)frame_buf,
        };
    }

    frame_buf->prev_id = frame_tail;
    frame_array[frame_tail]->next_id = frame_buf->id;
    frame_tail = frame_buf->id;

    arrput(frame_array, frame_buf);

    return (Result){
        .is_ok = 1,
        .ptr = (void *)frame_buf,
    };
}

/*
 * Insert frame into arbitrary place in list
 * if no prev_id i.e. add to head, set it to -1
 *
 */
Result insert_frame(int prev_id, char *action, char *res, Gene gene)
{
    Frame *frame_buf;
    Result ret;

    if (prev_id >= arrlen(frame_array))
    {
        debug("prev_id: %d not exist", prev_id);
        sprintf(ret.msg, "prev_id: %d not exist, max size %ld", prev_id, arrlen(frame_array));
        return ret;
    }

    // add to tail, use append function
    if (prev_id == frame_tail)
    {
        debug("use append method");
        return append_frame(action, res, gene);
    }

    // create frame buffer
    ret = create_frame();
    if (!ret.is_ok)
        return ret;

    frame_buf = (Frame *)ret.ptr;

    frame_buf->id = arrlen(frame_array);
    frame_buf->gene = gene;
    snprintf(frame_buf->action, ACTION_BUFFER_LEN, "%s", action);
    snprintf(frame_buf->res, RES_BUFFER_LEN, "%s", res);

    // add to head and handle inserting first node
    if (prev_id == -1)
    {
        frame_buf->prev_id = -1;
        frame_buf->next_id = frame_head;

        if (frame_head != -1)
        {
            frame_array[frame_head]->prev_id = frame_buf->id;
            frame_head = frame_buf->id;
        }
        else
        {
            // first node ever
            frame_head = frame_buf->id;
            frame_tail = frame_buf->id;
        }

        arrput(frame_array, frame_buf);

        return (Result){
            .is_ok = 1,
            .ptr = (void *)frame_buf,
        };
    }

    // insert into arbirary place
    frame_buf->prev_id = frame_array[prev_id]->id,
    frame_buf->next_id = frame_array[prev_id]->next_id,

    frame_array[prev_id]->next_id = frame_buf->id;
    frame_array[frame_buf->next_id]->prev_id = frame_buf->id;

    arrput(frame_array, frame_buf);
    return (Result){
        .is_ok = 1,
        .ptr = (void *)frame_buf,
    };
}

/*
 * find the next frame that is not dirty from start
 *
 */
static int get_next_frame_id(int start_id)
{
    Frame *cur;

    if (start_id == -1)
        return -1;

    cur = frame_array[start_id];

    if (~cur->gene & IS_DIRTY)
        return start_id;

    return get_next_frame_id(cur->next_id);
}

/*
 * find the previous frame that is not dirty from start
 *
 */
static int get_prev_frame_id(int start_id)
{
    Frame *cur;

    if (start_id == -1)
        return -1;

    cur = frame_array[start_id];

    if (~cur->gene & IS_DIRTY)
        return start_id;

    return get_prev_frame_id(cur->prev_id);
}

/*
 * soft delete frame from frame list
 * notice that it will not really delete it from memory
 *
 */
Result delete_frame(int frame_id)
{
    Result ret;
    int new_next_id;
    int new_prev_id;

    if (frame_id >= arrlen(frame_array))
    {
        debug("frame id %d not exist", frame_id);
        sprintf(ret.msg, "frame id %d not exist", frame_id);
        return ret;
    }

    frame_array[frame_id]->gene |= IS_DIRTY;

    // next and prev update
    new_next_id = get_next_frame_id(frame_id);
    new_prev_id = get_prev_frame_id(frame_id);

    if (new_next_id == -1)
    {
        frame_tail = new_prev_id;
    }
    else
    {
        frame_array[new_next_id]->prev_id = new_prev_id;
    }

    if (new_prev_id == -1)
    {
        frame_head = new_next_id;
    }
    else
    {
        frame_array[new_prev_id]->next_id = new_next_id;
    }

    ret.is_ok = 1;
    ret.ptr = (void *)frame_array[frame_id];
    return ret;
}

/*
 * serialize a single frame into game file entry,
 * not ensure the serialized version always optimized
 *
 */
static Result serialize_frame_unsafe(Frame *frame, FILE *fp)
{
    if (fp == NULL)
    {
        debug("file is not opened");
        return (Result){
            .msg = "file is not opened"};
    }
    if (frame == NULL)
    {
        return (Result){.is_ok = 1};
    }

    // serialize frame
    fwrite(&(frame->id), sizeof(frame->id), 1, fp);
    fwrite(&(frame->prev_id), sizeof(frame->prev_id), 1, fp);
    fwrite(&(frame->next_id), sizeof(frame->next_id), 1, fp);
    fwrite(frame->action, sizeof(frame->action), 1, fp);
    fwrite(&(frame->gene), sizeof(frame->gene), 1, fp);
    fwrite(frame->res, sizeof(frame->res), 1, fp);

    return (Result){.is_ok = 1};
}

/*
 * deserialize a single frame from game file entry,
 * check of the expect id match with the real id
 *
 */
static Result deserialize_frame(Frame *dest, int exp_id, FILE *fp)
{
    Result ret;

    if (fp == NULL)
    {
        debug("file is not opened");
        return (Result){
            .msg = "file is not opened"};
    }

    if (dest == NULL)
    {
        return (Result){
            .msg = "dest cannot be NULL",
        };
    }

    // deserialize frame
    fread(&(dest->id), sizeof(dest->id), 1, fp);
    fread(&(dest->prev_id), sizeof(dest->prev_id), 1, fp);
    fread(&(dest->next_id), sizeof(dest->next_id), 1, fp);
    fread(dest->action, sizeof(dest->action), 1, fp);
    fread(&(dest->gene), sizeof(dest->gene), 1, fp);
    fread(dest->res, sizeof(dest->res), 1, fp);

    if (dest->id != exp_id)
    {
        sprintf(ret.msg, "local frame id %d mismatched with expected %d", dest->id, exp_id);
        return ret;
    }

    return (Result){.is_ok = 1};
}

/*
 * optimize and serialize/overwrite everything into the game entry and meta files
 *
 */
Result serialize_all()
{
    FILE *fp_entry;
    FILE *fp_meta;
    char buf[STD_BUFFER_LEN];
    size_t n_written;
    Frame *cur;
    Result ret;

    // bind entry and meta files on fp
    JOIN(buf, config.app_root, config.app_entry);
    fp_entry = fopen(buf, "w");
    if (fp_entry == NULL)
    {
        perror("Error opening entry file");
        return (Result){.msg = "Error opening entry file"};
    }

    JOIN(buf, config.app_root, config.meta_entry);
    fp_meta = fopen(buf, "w");
    if (fp_meta == NULL)
    {
        perror("Error opening meta file");
        return (Result){.msg = "Error opening meta file"};
    }

    if (frame_array == NULL)
    {
        return (Result){.is_ok = 1};
    }

    /*
     * first serialize the whole list, in optimized mode
     * we always need to keep the local game data optimized
     *
     */
    n_written = 0;
    for (int i = 0; i < arrlen(frame_array); ++i)
    {
        cur = frame_array[i];
        if (cur->gene & IS_DIRTY)
            continue;
        cur->id = n_written;
        serialize_frame_unsafe(cur, fp_entry);
        n_written++;
    }

    /*
     * then serialize the meta information
     *
     */
    fwrite(&n_written, sizeof(size_t), 1, fp_meta);
    fwrite(&frame_head, sizeof(frame_head), 1, fp_meta);
    fwrite(&frame_tail, sizeof(frame_tail), 1, fp_meta);

    fclose(fp_entry);
    fclose(fp_meta);

    printf("serialized %ld frames!\n", n_written);

    deserialize_all();

    if ((size_t)arrlen(frame_array) != n_written)
    {
        fprintf(stderr, "critical error! serializer misbehavior, expect %ld real %ld", n_written, arrlen(frame_array));
        sprintf(ret.msg, "serializer misbehavior, expect %ld real %ld", n_written, arrlen(frame_array));

        return ret;
    }

    return (Result){.is_ok = 1};
}

/*
 * deserialize from local game data to init frame list
 *
 */
Result deserialize_all()
{
    FILE *fp_entry, *fp_meta;
    char buf[STD_BUFFER_LEN];
    size_t n_frames;
    Frame *new_frame;
    Result ret;

    // Open entry and meta files
    JOIN(buf, config.app_root, config.app_entry);
    fp_entry = fopen(buf, "r");
    if (!fp_entry)
    {
        perror("Error opening entry file");
        return (Result){.msg = "Error opening entry file"};
    }

    JOIN(buf, config.app_root, config.meta_entry);
    fp_meta = fopen(buf, "r");
    if (!fp_meta)
    {
        perror("Error opening meta file");
        fclose(fp_entry);
        return (Result){.msg = "Error opening meta file"};
    }

    // Clean current frame array
    free_frame_array();

    // Read metadata n_frame
    fread(&n_frames, sizeof(size_t), 1, fp_meta);

    // Read and reconstruct each frame
    new_frame = malloc(sizeof(Frame));
    for (size_t i = 0; i < n_frames; ++i)
    {
        ret = deserialize_frame(new_frame, i, fp_entry);
        if (!ret.is_ok)
        {
            fprintf(stderr, "serialize failed due to: %s\n", ret.msg);
            break;
        }
        ret = append_frame(new_frame->action, new_frame->res, new_frame->gene);
        if (!ret.is_ok)
        {
            fprintf(stderr, "serialize failed due to: %s\n", ret.msg);
            break;
        }
    }

    // Read rest of metadata to init head and tail
    fread(&frame_head, sizeof(frame_head), 1, fp_meta);
    fread(&frame_tail, sizeof(frame_tail), 1, fp_meta);

    free(new_frame);
    fclose(fp_entry);
    fclose(fp_meta);

    return ret;
}

/*
 * Test case define
 *
 */
static void run_test()
{
#ifdef TEST
    Result ret;

    // Test append functionality
    for (int i = 0; i < 50; ++i)
    {
        ret = append_frame(NULL, NULL, FRAME_IS_DEFAUT | MUSIC_IS_DEFAULT);
        // print_result(ret);
        // if (ret.is_ok)
        //     test("added id: %d", ((Frame *)ret.ptr)->id);
    }

    // Test insert functionality
    for (int i = 0; i < 50; ++i)
    {
        ret = insert_frame(i, NULL, NULL, FRAME_IS_DEFAUT | MUSIC_IS_DEFAULT);
        // print_result(ret);
        // if (ret.is_ok)
        //     printf("added id: %d\n", ((Frame *)ret.ptr)->id);
    }

    // for (int i = 0; i < arrlen(frame_array); ++i)
    // {
    //     test("id: %d\t prev: %d\t next: %d", frame_array[i]->id, frame_array[i]->prev_id, frame_array[i]->next_id);
    // }

    // Test delete
    for (int i = 0; i < 50; i += 2)
    {
        ret = delete_frame(i);
        // if (ret.is_ok)
        //     test("delete id: %d prev: %d", ((Frame *)ret.ptr)->id, ((Frame *)ret.ptr)->prev_id);
        // else
        //     print_result(ret);
    }

    for (int i = 0; i < arrlen(frame_array); ++i)
    {
        if (~frame_array[i]->gene & IS_DIRTY)
        {
            test("id: %d\t prev: %d\t next: %d", frame_array[i]->id, frame_array[i]->prev_id, frame_array[i]->next_id);
        }
    }

    // Test serialization
    serialize_all();

    (void)ret;
#endif
}

void initialize_engine()
{
    load_config();
    printf("engine loaded!\n");
}

void exit_engine()
{
    free_config();
    free_frame_array();
    printf("bye\n");
}

int main()
{
    // load config
    if (load_config() == 1)
        return 1;

    // run test case if needed
    run_test();

    debug("size of frame:%ld", sizeof(Frame));

    free_frame_array();
    free_config();
    return 0;
}
