#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "mc_config.h"
#include "mc_mqtt_client_api.h"
#include "mc_state_machine.h"

static void waitforUserAbort( mc_context_t* ctx ) {
    int ch;

    assert( ctx != NULL );

    do {
        ch = getchar();
    } while (ch!='Q' && ch != 'q');

    mc_processEvent( ctx, ctx->state, MC_EVENT_USER_ABORT, NULL );
}

static void ignore_publish_success( void* context ) { }
static void ignore_publish_error( void* context, int error_code, const char* error_msg ) { }

void on_data( void* context, const char* topic, const char* msg ) {
    mc_context_t* ctx = ( mc_context_t*)context;
    assert( ctx != NULL );

    fprintf( stderr, "Recived msg %s on topic %s\n", msg, topic );
    // bounce back
    if( strlen(ctx->cfg->publish_topic) > 0 ) {
        char mymsg[256] = "";
        sprintf( mymsg, "%s got msg %s", ctx->cfg->client_name, msg );
        mc_mqtt_client_publish(ctx->client, ctx->cfg->publish_topic, mymsg, ignore_publish_success, ignore_publish_error );
    }
}

void on_disconnect( void* context ) {
    mc_context_t* ctx = ( mc_context_t*)context;
    assert( ctx != NULL );

    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_DISCONNECTED, NULL );
}

int main(int argc, char* argv[]) {

    // read config
    mc_config_t cfg;
    memset( &cfg, 0x00, sizeof(mc_config_t) );
    mc_config_read(argc, argv, &cfg );

    // setup context
    mc_context_t ctx;
    memset(&ctx, 0x00,sizeof(mc_context_t));
    ctx.cfg = &cfg;
    ctx.state = MC_STATE_IDLE;

    mc_mqtt_client_t client_api;
    mc_mqtt_client_init( &client_api, cfg.remote_hostname_port, cfg.client_name, cfg.username, cfg.password, 
            &ctx, on_disconnect, on_data );
    ctx.client = &client_api;

    // kick off state machine
    mc_processEvent( &ctx, ctx.state, MC_EVENT_STARTUP, NULL );

    // waitfor completion
    waitforUserAbort( &ctx );

    // some time to cleanup
    sleep( 3 );

    exit( EXIT_SUCCESS );
}
