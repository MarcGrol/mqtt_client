#ifndef _mc_state_machine_h
#define _mc_state_machine_h

#include "mc_config.h"

typedef enum {
    MC_STATE_IDLE,
    MC_STATE_WAITFOR_MQTT_CONNECTED,
    MC_STATE_WAITFOR_MQTT_SUBSCRIBED,
    MC_STATE_WAITFOR_MQTT_UNSUBSCRIBED,
    MC_STATE_WAITFOR_MQTT_DATA,
    MC_STATE_WAITFOR_MQTT_PUBLISHED,
    MC_STATE_WAITFOR_MQTT_DISCONNECT,
    MC_STATE_DONE,
} mc_state_t;

typedef enum {
    MC_EVENT_STARTUP,
    MC_EVENT_MQTT_CONNECT_SUCCESS,
    MC_EVENT_MQTT_CONNECT_ERROR,
    MC_EVENT_MQTT_DISCONNECTED,
    MC_EVENT_MQTT_SUBSCRIBE_SUCCESS,
    MC_EVENT_MQTT_SUBSCRIBE_ERROR,
    MC_EVENT_MQTT_PUBLISH_SUCCESS,
    MC_EVENT_MQTT_PUBLISH_ERROR,
    MC_EVENT_MQTT_UNSUBSCRIBE_SUCCESS,
    MC_EVENT_MQTT_UNSUBSCRIBE_ERROR,
    MC_EVENT_MQTT_DISCONNECT_SUCCESS,
    MC_EVENT_MQTT_DISCONNECT_ERROR,
    MC_EVENT_MQTT_DATA,
    MC_EVENT_USER_ABORT,
} mc_event_t;

typedef struct {
    mc_state_t state;
    mc_config_t* cfg;
    void* client; // implementation specific
} mc_context_t;

typedef struct {
    char* error;
    char* topic;
    char* msg;
} mc_event_data_t;

void mc_processEvent( mc_context_t* context, mc_state_t state, mc_event_t event, mc_event_data_t* event_data );

#endif
