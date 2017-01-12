#ifndef _mc_mqtt_api_h
#define _mc_mqtt_api_h

typedef void (*mc_pfn_on_data)( void* context, const char* topic, const char* msg );
typedef void (*mc_pfn_on_connection_lost)( void* context );
typedef void (*mc_pfn_on_success)( void* context );
typedef void (*mc_pfn_on_error)( void* context, int error_code, const char* error_msg );


typedef struct {
    char* broker_url;
    char* client_name;
    char* username;
    char* password;
    void* context;
    mc_pfn_on_data on_data_cb;
    mc_pfn_on_connection_lost on_connection_lost_cb;
    void* client_magic;
} mc_mqtt_client_t;

int mc_mqtt_client_init( mc_mqtt_client_t* client, const char* broker_url, const char* client_name, const char* username, const char* password, 
        void* context, mc_pfn_on_connection_lost on_connection_lost_cb, mc_pfn_on_data on_data_cb );
void mc_mqtt_client_connect( mc_mqtt_client_t* client, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb );
void mc_mqtt_client_disconnect( mc_mqtt_client_t* client, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb );
void mc_mqtt_client_subscribe( mc_mqtt_client_t* client, const char* topic, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb );
void mc_mqtt_client_unsubscribe( mc_mqtt_client_t* client, const char* topic, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb );
void mc_mqtt_client_publish( mc_mqtt_client_t* client, const char* topic, const char* msg, mc_pfn_on_success success_cb, mc_pfn_on_error error_cb );
void mc_mqtt_client_release( mc_mqtt_client_t* client );

#endif
