#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>

#include "mc_config.h"

static struct option options[] = {
    {"remote-host", required_argument, NULL, 'r' },
    {"client-name", required_argument, NULL, 'c' },
    {"username", required_argument, NULL, 'u' },
    {"password", required_argument, NULL, 's' },
    {"subscribe-topic", required_argument, NULL, 's' },
    {"publish-topic", required_argument, NULL, 'p' },
    {"verbose", no_argument, NULL, 'v' },
    {"help", no_argument, NULL, 'h' },
    {NULL, 0, NULL, 0 },
};

static void usage( const char* progname ) {
    fprintf( stderr, "Usage: %s [options]\n", progname );
    fprintf( stderr, "where options:\n" );
    fprintf( stderr, "\t-r --remote-host: mandatory, default=tcp://myhost.nl:4455\n" );
    fprintf( stderr, "\t-n --client-name: mandatory, default=testclient\n" ); 
    fprintf( stderr, "\t-u --username: mandatory, default=testclient\n" ); 
    fprintf( stderr, "\t-c --password: mandatory, default=grol\n" ); 
    fprintf( stderr, "\t-s --subscribe-topic: optional, default=/example\n" );
    fprintf( stderr, "\t-p --publish-topic: optional, default=none\n" );
    fprintf( stderr, "\t-v --verbose: optional, default=0\n" );
    fprintf( stderr, "\t-h --help: this help\n" );
    fprintf( stderr, "\n" );

    exit( EXIT_FAILURE );
}

static void mc_config_defaults( mc_config_t* cfg ) {
    assert( cfg != NULL );

    cfg->verbose = 0;
    strcpy( cfg->remote_hostname_port, "tcp://m20.cloudmqtt.com:10750");
    strcpy( cfg->client_name, "testclient");
    strcpy( cfg->username, "grol");
    strcpy( cfg->password, "grol");
    strcpy( cfg->subscribe_topic, "/example" );
    strcpy( cfg->publish_topic, "" );
}

 int mc_config_read( int argc, char** argv, mc_config_t* cfg ) {
    char progname[255] = "";
    strcpy( progname, argv[0] );

    mc_config_defaults( cfg );

    int ch = 0;
    while ((ch = getopt_long(argc, argv, "r:n:u:c:p:s:vh", options, NULL)) != -1) {
        switch (ch) {
             case 'r':
                     strcpy(cfg->remote_hostname_port, optarg);
                     break;
             case 'n':
                     strcpy(cfg->client_name, optarg );
                     break;
             case 'u':
                     strcpy(cfg->username, optarg );
                     break;
             case 'c':
                     strcpy(cfg->password, optarg );
                     break;
             case 's':
                    strcpy(cfg->subscribe_topic, optarg );
                     break;
             case 'p':
                    strcpy( cfg->publish_topic,optarg );;
                     break;
             case 'v':
                    cfg->verbose = 1;
                     break;
             case 'h':
                    usage(progname);
                     break;
             default:
                     usage(progname);
        }
    }
    argc -= optind;
    argv += optind;

    if( cfg->verbose == 1 ) {
        mc_config_print( cfg );
    }

    return 1;
}

void mc_config_print( const mc_config_t* cfg ) {
    assert( cfg != NULL );
    fprintf( stderr, "remote_hostname_port:%s\n", cfg->remote_hostname_port);
    fprintf( stderr, "client_name:%s\n", cfg->client_name);
    fprintf( stderr, "username: %s\n", cfg->username);
    fprintf( stderr, "password: %s\n", cfg->password);
    fprintf( stderr, "subscribe-topic: %s\n", cfg->subscribe_topic);
    fprintf( stderr, "publish-topic: %s\n", cfg->publish_topic);
    fprintf( stderr, "verbose:%d\n", cfg->verbose);
}
