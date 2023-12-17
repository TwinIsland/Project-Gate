#include "lib/mongoose-7.12/mongoose.h"
#include "lib/inih-r57/ini.h"

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
#define STD_BUFFER_LEN 128

typedef struct
{
    int port;
    int max_app;
    int max_client;
    int max_usr_uptime;
    char *app_name;
    char *app_author;
    char *app_link;
    char *app_root;
    char *app_entry;
} configuration;

/*
 * Configurations loader callback
 *
 */
static int config_handler(void *user, const char *section, const char *name,
                          const char *value)
{
    configuration *pconfig = (configuration *)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("App", "name"))
        pconfig->app_name = strdup(value);
    else if (MATCH("App", "author"))
        pconfig->app_author = strdup(value);
    else if (MATCH("App", "link"))
        pconfig->app_link = strdup(value);
    else if (MATCH("App", "root"))
        pconfig->app_root = strdup(value);
    else if (MATCH("App", "entry"))
        pconfig->app_entry = strdup(value);
    else if (MATCH("Server", "port"))
        pconfig->port = atoi(value);
    else if (MATCH("Server", "max_client"))
        pconfig->max_client = atoi(value);
    else if (MATCH("Server", "max_usr_uptime"))
        pconfig->max_usr_uptime = atoi(value);
    else
        return 0; /* unknown section/name, error */
    return 1;
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    (void)fn_data;
    char buf[STD_BUFFER_LEN];

    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        if (mg_http_match_uri(hm, "/api/hello"))
        { // On /api/hello requests,
            mg_http_reply(c, 200, "", "{%m:%d}\n",
                          MG_ESC("status"), 1); // Send dynamic JSON response
        }
        else if (mg_http_match_uri(hm, "/") || mg_http_match_uri(hm, "/index.html"))
        {
            // Serve index.html for root or /index.html requests
            debug("serve /index");
            struct mg_http_serve_opts opts = {.root_dir = "."};
            mg_http_serve_file(c, hm, "index.html", &opts);
        }
        else
        {
            struct mg_http_serve_opts opts = {.root_dir = "."};
            snprintf(buf, sizeof("game_data") + hm->uri.len, "%s%s", "game_data", hm->uri.ptr);
            debug("serve: %s", buf);

            mg_http_serve_file(c, hm, buf, &opts);
        }
    }
}

int main()
{
    struct mg_mgr mgr;
    configuration config;
    char buf[STD_BUFFER_LEN];

    debug("DEBUG MODE: don't distribute it in production env\n");

    // Load config
    if (ini_parse(CONFIG_DIR, config_handler, &config) < 0)
    {
        printf("Can't load %s\n", CONFIG_DIR);
        return 1;
    }

    // start server
    mg_mgr_init(&mgr);
    sprintf(buf, "http://0.0.0.0:%d", config.port); // Init manager
    mg_http_listen(&mgr, buf, fn, &mgr);            // Setup listener

    printf("server start at: %s\n", buf);
    for (;;)
        mg_mgr_poll(&mgr, 1000); // Event loop
    mg_mgr_free(&mgr);           // Cleanup
    return 0;
}