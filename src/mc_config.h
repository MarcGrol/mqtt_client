#ifndef _mc_config_h
#define _mc_config_h

#define BUFLEN 64

typedef struct {
    int verbose;
    char remote_hostname_port[BUFLEN];
    char client_name[BUFLEN];
    char username[BUFLEN];
    char password[BUFLEN];
    char subscribe_topic[BUFLEN];
    char publish_topic[BUFLEN];
} mc_config_t;

void mc_config_print( const mc_config_t* cfg );
int mc_config_read( int argc, char** argv, mc_config_t* cfg );

#endif
