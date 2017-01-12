#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "mc_state_machine.h"
#include "mc_action.h"
#include "mc_config.h"
#include "mc_mqtt_client_api.h"

static int msg_counter  = 0;

static void on_connect_success( void* context ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_CONNECT_SUCCESS, NULL );
}

static void on_connect_error( void* context, int error_code, const char* error_msg ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_CONNECT_ERROR, NULL );
}

void mc_action_connect(mc_context_t* context, mc_event_data_t* event_data ) {
    assert( context != NULL );
    fprintf( stderr, "%s: start connecting\n", __func__ );
    mc_mqtt_client_connect(context->client, on_connect_success, on_connect_error);
}

void mc_action_reconnect(mc_context_t* context, mc_event_data_t* event_data ) {
    assert( context != NULL );
    fprintf( stderr, "%s: start reconnecting\n", __func__ );
    mc_mqtt_client_connect(context->client, on_connect_success, on_connect_error);
}

static void on_subscribe_success( void* context ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_SUBSCRIBE_SUCCESS, NULL );
}

static void on_subscribe_error( void* context, int error_code, const char* error_msg ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_SUBSCRIBE_ERROR, NULL );
}

static void ignore_publish_success( void* context ) { }

static void ignore_publish_error( void* context, int error_code, const char* error_msg ) { }

void mc_action_subscribe(mc_context_t* context, mc_event_data_t* event_data ) {
    assert( context != NULL );

    if( strlen(context->cfg->publish_topic) > 0 ) {
        // report successfull startup
        char msg[256] = "";
        sprintf( msg, "%s is connected", context->cfg->client_name );
        mc_mqtt_client_publish(context->client, context->cfg->publish_topic, msg, ignore_publish_success, ignore_publish_error );
    }

    fprintf( stderr, "%s: start subscribing\n", __func__ );
    mc_mqtt_client_subscribe(context->client, context->cfg->subscribe_topic, on_subscribe_success, on_subscribe_error);
}

static void on_unsubscribe_success( void* context ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_UNSUBSCRIBE_SUCCESS, NULL );
}

static void on_unsubscribe_error( void* context, int error_code, const char* error_msg ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_UNSUBSCRIBE_ERROR, NULL );
}

void mc_action_unsubscribe(mc_context_t* context, mc_event_data_t* event_data ) {
    assert( context != NULL );
    fprintf( stderr, "%s: start unsubscribing\n", __func__ );
    mc_mqtt_client_unsubscribe(context->client, context->cfg->subscribe_topic, on_unsubscribe_success, on_unsubscribe_error);
}

void mc_action_publish_up(mc_context_t* context, mc_event_data_t* event_data ) {
    assert( context != NULL );
    if( strlen(context->cfg->publish_topic) > 0 ) {
        // report we are up
        char msg[256] = "";
        sprintf( msg, "%s is subscribed", context->cfg->client_name );
        fprintf( stderr, "%s: start publishing %s\n", __func__, msg );
        mc_mqtt_client_publish(context->client, context->cfg->publish_topic, msg, ignore_publish_success, ignore_publish_error );
    }
}

void mc_action_user_abort(mc_context_t* context, mc_event_data_t* event_data ) {
    assert( context != NULL );
    fprintf( stderr, "%s: start unsubscribing\n", __func__ );
    if( strlen(context->cfg->publish_topic) > 0 ) {
        // report going down
        char msg[256] = "";
        sprintf( msg, "%s is terminating", context->cfg->client_name );
        mc_mqtt_client_publish(context->client, context->cfg->publish_topic, msg, ignore_publish_success, ignore_publish_error );
    }
    mc_mqtt_client_unsubscribe(context->client, context->cfg->subscribe_topic, on_unsubscribe_success, on_unsubscribe_error);
}

static void on_disconnect_success( void* context ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_DISCONNECT_SUCCESS, NULL );
}

static void on_disconnect_error( void* context, int error_code, const char* error_msg ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_DISCONNECT_ERROR, NULL );
}

void mc_action_disconnect(mc_context_t* context, mc_event_data_t* event_data ) {
    assert( context != NULL );
    fprintf( stderr, "%s: start disconnecting\n", __func__ );
    mc_mqtt_client_disconnect(context->client, on_disconnect_success, on_disconnect_error);
}

static void on_publish_success( void* context ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_PUBLISH_SUCCESS, NULL );
}

static void on_publish_error( void* context, int error_code, const char* error_msg ) {
    mc_context_t* ctx = (mc_context_t*)context;
    assert( ctx != NULL );
    mc_processEvent( ctx, ctx->state, MC_EVENT_MQTT_PUBLISH_ERROR, NULL );
}

void mc_action_respond(mc_context_t* context, mc_event_data_t* event_data ) {

    assert( context != NULL );
    assert( event_data != NULL );

    fprintf( stderr, "Got data: %s on topic %s\n", event_data->msg, event_data->topic );

    if( strlen(context->cfg->publish_topic) > 0 ) {
        // pong it back
        char msg[32] = "";
        sprintf( msg, "msg %d", msg_counter++ );
        fprintf( stderr, "%s: start responding %s\n", __func__, msg );
        mc_mqtt_client_publish(context->client, context->cfg->publish_topic, msg, on_publish_success, on_publish_error );
    }
}

void mc_action_quit(mc_context_t* context, mc_event_data_t* event_data ) {
    assert( context != NULL );
    fprintf( stderr, "%s: quit\n", __func__ );
    mc_mqtt_client_release( context->client );
    exit( EXIT_SUCCESS );
}

void mc_action_ignore(mc_context_t* context, mc_event_data_t* event_data ) { }

