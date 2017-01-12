#ifndef _mc_action_h
#define _mc_action_h

#include "mc_state_machine.h"

void mc_action_connect(mc_context_t* context, mc_event_data_t* event_data );
void mc_action_reconnect(mc_context_t* context, mc_event_data_t* event_data );
void mc_action_subscribe(mc_context_t* context, mc_event_data_t* event_data );
void mc_action_unsubscribe(mc_context_t* context, mc_event_data_t* event_data );
void mc_action_publish_up(mc_context_t* context, mc_event_data_t* event_data );
void mc_action_respond(mc_context_t* context, mc_event_data_t* event_data );
void mc_action_user_abort(mc_context_t* context, mc_event_data_t* event_data );
void mc_action_disconnect(mc_context_t* context, mc_event_data_t* event_data );
void mc_action_quit(mc_context_t* context, mc_event_data_t* event_data );
void mc_action_ignore(mc_context_t* context, mc_event_data_t* event_data );

#endif
