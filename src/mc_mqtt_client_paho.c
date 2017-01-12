#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include "MQTTAsync.h"
#include "MQTTClient.h"
//#include "Log.h"

#include "mc_mqtt_client_api.h"

#define QOS         1

// ami: asynchrnous method invocation
typedef struct {
    mc_mqtt_client_t* client;
    mc_pfn_on_success on_success_cb;
    mc_pfn_on_error on_error_cb;
} mc_ami_t;

static mc_ami_t* mc_ami_create( mc_mqtt_client_t* client, mc_pfn_on_success on_success_cb,  mc_pfn_on_error on_error_cb ) {

    assert( client != NULL );
    assert( on_success_cb != NULL );
    assert( on_error_cb != NULL );

    mc_ami_t* ami = (mc_ami_t*)calloc(1,sizeof(mc_ami_t));
    if( ami != NULL ) {
        ami->client = client;
        ami->on_success_cb = on_success_cb;
        ami->on_error_cb = on_error_cb;
    }
    return ami;
}

static void  mc_ami_free( mc_ami_t* ami ) {
    if( ami != NULL ) {
        free( ami );
    }
}

int mc_mqtt_client_init( mc_mqtt_client_t* client, 
        const char* broker_url, const char* client_name, const char* username, const char* password, 
        void* context, mc_pfn_on_connection_lost on_connection_lost_cb, mc_pfn_on_data on_data_cb ) {
    assert( client != NULL );
    assert( broker_url != NULL );
    assert( client_name != NULL );
    assert( username != NULL );
    assert( password != NULL );
    assert( on_data_cb != NULL );

    client->client_magic = NULL;
    client->broker_url = strdup(broker_url);
    client->client_name = strdup(client_name);
    client->username = strdup(username);
    client->password = strdup(password);
    client->context = context;
    client->on_data_cb = on_data_cb;
    client->on_connection_lost_cb = on_connection_lost_cb;

    return  1;
}

static void onConnectionLost(void *context, char *cause) {
    // no ami here
    mc_mqtt_client_t* client = (mc_mqtt_client_t*)context;
    assert( client != NULL );

    fprintf(stderr, "Connection lost\n");

    // call global callback
    client->on_connection_lost_cb( client->context );
}

static int onMsgArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    // no ami here
    mc_mqtt_client_t* client = (mc_mqtt_client_t*)context;
    assert( client != NULL );

    fprintf(stderr, "onMsgArrived\n");

    // expect strings and no binary data
    char* topic = (char*)calloc(1,topicLen+1);
    memcpy(topic, topicName, topicLen );
    char* msg = (char*)calloc(1,message->payloadlen+1);
    memcpy(msg, message->payload, message->payloadlen );

    // call global callback
    client->on_data_cb( client->context, topic, msg );

    // cleanup
    free( topic );
    free( msg );

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return 1;
}

static void onMsgDelivered(void *context, MQTTAsync_token token) {
    // no ami here
    mc_mqtt_client_t* client = (mc_mqtt_client_t*)context;
    assert( client != NULL );

    fprintf(stderr, "onMsgDelivered %d\n", token);
}

static void onConnectSuccess(void* context, MQTTAsync_successData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );
    assert( ami->client != NULL );
    assert( ami->client->context != NULL );

    fprintf(stderr, "onConnectSuccess\n");

    ami->on_success_cb( ami->client->context );
    mc_ami_free( ami );
}

static void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );

    fprintf(stderr, "Connect failed, rc %d\n", response ? response->code : 0);

    ami->on_error_cb( ami->client->context, response->code, strerror(errno));
    mc_ami_free( ami );
}

void mc_mqtt_client_connect( mc_mqtt_client_t* client, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb ) {
    int rc;

    assert( client != NULL );
    assert( success_cb != NULL );
    assert( error_cb != NULL );
    
    mc_ami_t* ami = mc_ami_create(client, success_cb, error_cb );

    if( client->client_magic != NULL ) {
        MQTTAsync_destroy(&client->client_magic);
        client->client_magic = NULL;
    }

    // configure and start mqtt client
    MQTTAsync_create(&client->client_magic, client->broker_url, client->client_name, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(client->client_magic, client, onConnectionLost, onMsgArrived, onMsgDelivered);
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username =  client->username;
    conn_opts.password = client->password;
    conn_opts.onSuccess = onConnectSuccess;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = ami;

    if ((rc = MQTTAsync_connect(client->client_magic, &conn_opts)) != MQTTASYNC_SUCCESS) {
        fprintf(stderr, "Failed to start connect, return code %d (%s)\n", rc, strerror(rc));
        error_cb( client, rc, strerror(errno));
        mc_ami_free( ami );
    } else {
        fprintf( stderr, "Initiated connect\n" );
    }
}

static void onDisconnectSuccess(void* context, MQTTAsync_successData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );

    fprintf(stderr, "onDisonnectSuccess\n");

    ami->on_success_cb( ami->client->context );
    mc_ami_free( ami );
}

static void onDisconnectFailure(void* context, MQTTAsync_failureData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );

    fprintf(stderr, "Disconnect failed, rc %d\n", response ? response->code : 0);

    ami->on_error_cb( ami->client->context, response->code, strerror(errno));
    mc_ami_free( ami );
}

void mc_mqtt_client_disconnect( mc_mqtt_client_t* client, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb ) {
    int rc;

    assert( client != NULL );
    assert( success_cb != NULL );
    assert( error_cb != NULL );

    mc_ami_t* ami = mc_ami_create(client, success_cb, error_cb );

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    disc_opts.context = ami;
    disc_opts.onSuccess = onDisconnectSuccess;
    disc_opts.onFailure = onDisconnectFailure;

    if ((rc = MQTTAsync_disconnect(client->client_magic, &disc_opts)) != MQTTASYNC_SUCCESS) {
        fprintf(stderr, "Failed to start disconnect, return code %d\n", rc);
        ami->on_error_cb( ami->client->context, rc, strerror(errno));
        mc_ami_free( ami );
    } else {
        fprintf( stderr, "Initiated disconnect\n" );
    }
}

static void onSubscribeSuccess(void* context, MQTTAsync_successData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );

    fprintf(stderr, "onSubscribeSucces\n");

    ami->on_success_cb( ami->client->context);
    mc_ami_free( ami );
}

static void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );

    fprintf(stderr, "onSubscribeFailure: rc %d\n", response ? response->code : 0);

    ami->on_error_cb( ami->client->context, response->code, strerror(errno));
    mc_ami_free( ami );
}

void mc_mqtt_client_subscribe( mc_mqtt_client_t* client, const char* topic, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb ) {
    int rc;

    assert( client != NULL );
    assert( topic != NULL );
    assert( success_cb != NULL );
    assert( error_cb != NULL );

    mc_ami_t* ami = mc_ami_create(client, success_cb, error_cb );

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    opts.onSuccess = onSubscribeSuccess;
    opts.onFailure = onSubscribeFailure;
    opts.context = ami;

    if ((rc = MQTTAsync_subscribe(client->client_magic, topic, QOS, &opts)) != MQTTASYNC_SUCCESS) {
        fprintf(stderr, "Failed to start subscribe for %s, return code %d\n", topic, rc);
        ami->on_error_cb( ami->client->context, rc, strerror(errno));
        mc_ami_free( ami );
    } else {
        fprintf( stderr, "Initiated subscribe on topic %s\n", topic );
    }
}

static void onUnSubscribeSuccess(void* context, MQTTAsync_successData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );

    fprintf(stderr, "onUnSubscribeSuccess\n" );

    ami->on_success_cb( ami->client->context);
    mc_ami_free( ami );
}

static void onUnSubscribeFailure(void* context, MQTTAsync_failureData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );

    fprintf(stderr, "onUnSubscribeFailure: rc %d\n", response ? response->code : 0);

    ami->on_error_cb( ami->client->context, response->code, strerror(errno));
    mc_ami_free( ami );
}

void mc_mqtt_client_unsubscribe( mc_mqtt_client_t* client, const char* topic, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb ) {
    int rc;

    assert( client != NULL );
    assert( topic != NULL );
    assert( success_cb != NULL );
    assert( error_cb != NULL );

    mc_ami_t* ami = mc_ami_create(client, success_cb, error_cb );

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = onUnSubscribeSuccess;
    opts.onFailure = onUnSubscribeFailure;
    opts.context = ami;

    if ((rc = MQTTAsync_unsubscribe(client->client_magic, topic, &opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start unsubscribe, return code %d\n", rc);
        ami->on_error_cb( ami->client->context, rc, strerror(errno));
        mc_ami_free( ami );
    } else {
        fprintf( stderr, "Initiated unsubscribe from topic %s\n", topic );
    }
}

static void onPublishSuccess(void* context, MQTTAsync_successData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );

    fprintf(stderr, "onPublishSuccess\n");

    ami->on_success_cb( ami->client->context);
    mc_ami_free( ami );
}

static void onPublishFailure(void* context, MQTTAsync_failureData* response) {
    mc_ami_t* ami = (mc_ami_t*)context;
    assert( ami != NULL );

    fprintf(stderr, "onPublishFailure: rc %d\n", response ? response->code : 0);

    ami->on_error_cb( ami->client->context, response->code, strerror(errno));
    mc_ami_free( ami );
}

void mc_mqtt_client_publish( mc_mqtt_client_t* client, const char* topic, const char* msg, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb ) {
    int rc;

    assert( client != NULL );
    assert( topic != NULL );
    assert( msg != NULL );
    assert( success_cb != NULL );
    assert( error_cb != NULL );

    mc_ami_t* ami = mc_ami_create(client, success_cb, error_cb );

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = onPublishSuccess;
    opts.onFailure = onPublishFailure;
    opts.context = ami;
    if(( rc = MQTTAsync_send(client->client_magic, (char*)topic, strlen(msg), (char*)msg, QOS, 0, &opts ))  != MQTTASYNC_SUCCESS) {
        fprintf(stderr, "Failed to publish msg on topic %s, return code %d (%s)\n", topic, rc, strerror(errno));
        ami->on_error_cb( ami->client->context, rc, strerror(errno));
        mc_ami_free( ami );
    } else {
        fprintf( stderr, "Initiated publish of msg %s on topic %s\n", msg, topic );
    }
}

void mc_mqtt_client_release( mc_mqtt_client_t* client ) {
    if( client != NULL ) {
        free( client->broker_url );
        free( client->client_name );
        free( client->username );
        free( client->password );
        MQTTAsync_destroy(&client->client_magic);
    }
}

