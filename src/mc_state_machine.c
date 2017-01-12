
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "mc_state_machine.h"
#include "mc_action.h"

typedef void (*mc_pfn_action_t)( mc_context_t* context, mc_event_data_t* event_data );

typedef struct {
    mc_state_t state;
    mc_event_t event;
    mc_pfn_action_t action;
    mc_state_t next_state;
} mc_transition_t;

static const mc_transition_t transitions[] = {
    { MC_STATE_IDLE,                      MC_EVENT_STARTUP,                  mc_action_connect,         MC_STATE_WAITFOR_MQTT_CONNECTED },

    { MC_STATE_WAITFOR_MQTT_CONNECTED,    MC_EVENT_MQTT_CONNECT_SUCCESS,     mc_action_subscribe,       MC_STATE_WAITFOR_MQTT_SUBSCRIBED },
    { MC_STATE_WAITFOR_MQTT_CONNECTED,    MC_EVENT_MQTT_CONNECT_ERROR,       mc_action_reconnect,       MC_STATE_WAITFOR_MQTT_CONNECTED },
    { MC_STATE_WAITFOR_MQTT_CONNECTED,    MC_EVENT_USER_ABORT,               mc_action_quit,            MC_STATE_DONE },

    { MC_STATE_WAITFOR_MQTT_SUBSCRIBED,   MC_EVENT_MQTT_SUBSCRIBE_SUCCESS,   mc_action_publish_up,      MC_STATE_WAITFOR_MQTT_DATA },
    { MC_STATE_WAITFOR_MQTT_SUBSCRIBED,   MC_EVENT_MQTT_SUBSCRIBE_ERROR,     mc_action_subscribe,       MC_STATE_WAITFOR_MQTT_SUBSCRIBED },
    { MC_STATE_WAITFOR_MQTT_SUBSCRIBED,   MC_EVENT_MQTT_DISCONNECTED,        mc_action_reconnect,       MC_STATE_WAITFOR_MQTT_CONNECTED },
    { MC_STATE_WAITFOR_MQTT_SUBSCRIBED,   MC_EVENT_USER_ABORT,               mc_action_disconnect,      MC_STATE_WAITFOR_MQTT_DISCONNECT },

    { MC_STATE_WAITFOR_MQTT_UNSUBSCRIBED, MC_EVENT_MQTT_UNSUBSCRIBE_SUCCESS, mc_action_disconnect,      MC_STATE_WAITFOR_MQTT_DISCONNECT },
    { MC_STATE_WAITFOR_MQTT_UNSUBSCRIBED, MC_EVENT_MQTT_UNSUBSCRIBE_ERROR,   mc_action_disconnect,      MC_STATE_WAITFOR_MQTT_DISCONNECT },
    { MC_STATE_WAITFOR_MQTT_UNSUBSCRIBED, MC_EVENT_MQTT_DISCONNECTED,        mc_action_quit,            MC_STATE_DONE },
    { MC_STATE_WAITFOR_MQTT_UNSUBSCRIBED, MC_EVENT_USER_ABORT,               mc_action_disconnect,      MC_STATE_WAITFOR_MQTT_UNSUBSCRIBED },

    { MC_STATE_WAITFOR_MQTT_DATA,         MC_EVENT_MQTT_DATA,                mc_action_respond,         MC_STATE_WAITFOR_MQTT_DATA },
    { MC_STATE_WAITFOR_MQTT_DATA,         MC_EVENT_MQTT_DISCONNECTED,        mc_action_reconnect,       MC_STATE_WAITFOR_MQTT_CONNECTED },
    { MC_STATE_WAITFOR_MQTT_DATA,         MC_EVENT_USER_ABORT,               mc_action_user_abort,      MC_STATE_WAITFOR_MQTT_UNSUBSCRIBED },

    // should this section be present??
    { MC_STATE_WAITFOR_MQTT_PUBLISHED,    MC_EVENT_MQTT_PUBLISH_SUCCESS,     mc_action_ignore,          MC_STATE_WAITFOR_MQTT_DATA },
    { MC_STATE_WAITFOR_MQTT_PUBLISHED,    MC_EVENT_MQTT_PUBLISH_ERROR,       mc_action_ignore,          MC_STATE_WAITFOR_MQTT_DATA },
    { MC_STATE_WAITFOR_MQTT_PUBLISHED,    MC_EVENT_MQTT_DISCONNECTED,        mc_action_reconnect,       MC_STATE_WAITFOR_MQTT_CONNECTED },
    { MC_STATE_WAITFOR_MQTT_PUBLISHED,    MC_EVENT_USER_ABORT,               mc_action_user_abort,      MC_STATE_WAITFOR_MQTT_UNSUBSCRIBED },

    { MC_STATE_WAITFOR_MQTT_DISCONNECT,   MC_EVENT_MQTT_DISCONNECT_SUCCESS,  mc_action_quit,            MC_STATE_DONE  },
    { MC_STATE_WAITFOR_MQTT_DISCONNECT,   MC_EVENT_MQTT_DISCONNECT_ERROR,    mc_action_quit,            MC_STATE_DONE  },
};

static const int num_transitions = sizeof(transitions)/sizeof(mc_transition_t);

static mc_pfn_action_t getAction( mc_state_t state, mc_event_t event ) {
    int idx = 0;
    for( idx=0; idx<num_transitions; idx++ ) {
        if( transitions[idx].state == state && transitions[idx].event == event ) {
            return transitions[idx].action;
        }
    }
    return NULL;
}

static mc_state_t getNextState( mc_state_t state, mc_event_t event ) {
    int idx = 0;
    for( idx=0; idx<num_transitions; idx++ ) {
        if( transitions[idx].state == state && transitions[idx].event == event ) {
            return transitions[idx].next_state;
        }
    }
    return -1;
}
typedef struct {
    mc_state_t state;
    char* name;
} mc_state_name_t;

static const mc_state_name_t state_names[] = {
    { MC_STATE_IDLE,                      "STATE_IDLE" },
    { MC_STATE_WAITFOR_MQTT_CONNECTED,    "STATE_WAITFOR_MQTT_CONNECTED" },
    { MC_STATE_WAITFOR_MQTT_SUBSCRIBED,   "STATE_WAITFOR_MQTT_SUBSCRIBED" },
    { MC_STATE_WAITFOR_MQTT_UNSUBSCRIBED, "STATE_WAITFOR_MQTT_UNSUBSCRIBED" },
    { MC_STATE_WAITFOR_MQTT_DATA,         "STATE_WAITFOR_MQTT_DATA" },
    { MC_STATE_WAITFOR_MQTT_PUBLISHED,    "STATE_WAITFOR_MQTT_PUBLISHED" },
    { MC_STATE_WAITFOR_MQTT_DISCONNECT,   "STATE_WAITFOR_MQTT_DISCONNECT" },
    { MC_STATE_DONE,                      "STATE_DONE" },
};
static const int num_state_names = sizeof(state_names)/sizeof(mc_state_name_t);
static char* getStateName( mc_state_t state ) {
    int idx = 0;
    for( idx=0; idx<num_state_names; idx++ ) {
        if( state_names[idx].state == state ) {
            return state_names[idx].name;
        }
    }
    return "STATE_UNKNOWN";
}

typedef struct {
    mc_event_t event;
    char* name;
} mc_event_name_t;

static const mc_event_name_t event_names[] = {
    { MC_EVENT_STARTUP,                  "EVENT_STARTUP" },
    { MC_EVENT_MQTT_CONNECT_SUCCESS,     "EVENT_MQTT_CONNECT_SUCCESS" },
    { MC_EVENT_MQTT_CONNECT_ERROR,       "EVENT_MQTT_CONNECT_ERROR" },
    { MC_EVENT_MQTT_DISCONNECTED,        "EVENT_MQTT_DISCONNECTED" },
    { MC_EVENT_MQTT_SUBSCRIBE_SUCCESS,   "EVENT_MQTT_SUBSCRIBE_SUCCESS" },
    { MC_EVENT_MQTT_SUBSCRIBE_ERROR,     "EVENT_MQTT_SUBSCRIBE_ERROR" },
    { MC_EVENT_MQTT_PUBLISH_SUCCESS,     "EVENT_MQTT_PUBLISH_SUCCESS" },
    { MC_EVENT_MQTT_PUBLISH_ERROR,       "EVENT_MQTT_PUBLISH_ERROR" },
    { MC_EVENT_MQTT_UNSUBSCRIBE_SUCCESS, "EVENT_MQTT_UNSUBSCRIBE_SUCCESS" },
    { MC_EVENT_MQTT_UNSUBSCRIBE_ERROR,   "EVENT_MQTT_UNSUBSCRIBE_ERROR" },
    { MC_EVENT_MQTT_DISCONNECT_SUCCESS,  "EVENT_MQTT_DISCONNECT_SUCCESS" },
    { MC_EVENT_MQTT_DISCONNECT_ERROR,    "EVENT_MQTT_DISCONNECT_ERROR" },
    { MC_EVENT_MQTT_DATA,                "EVENT_MQTT_DATA" },
    { MC_EVENT_USER_ABORT,               "EVENT_USER_ABORT" },
};
static const int num_event_names = sizeof(event_names)/sizeof(mc_event_name_t);
static char* getEventName( mc_event_t event ) {
    int idx = 0;
    for( idx=0; idx<num_event_names; idx++ ) {
        if( event_names[idx].event == event ) {
            return event_names[idx].name;
        }
    }
    return "EVENT_UNKNOWN";
}

void mc_processEvent( mc_context_t* context, mc_state_t state, mc_event_t event, mc_event_data_t* event_data ) {
    mc_pfn_action_t action = getAction( state, event );
    mc_state_t next_state = getNextState( state, event );
    if( action == NULL ) {
        fprintf( stderr, "Unsupported transtion %s %s\n", getStateName(state), getEventName(event) );
        assert( action == NULL );
    } else {
        fprintf( stderr, "Transition %s %s -> %s\n", getStateName(state), getEventName(event), getStateName(next_state) );
        context->state = next_state;
        action( context, event_data );
    }
}
