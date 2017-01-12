
CC=gcc $(CFLAGS)
CFLAGS=-I../lib_paho -ggdb -Wall
LD=gcc $(LDFLAGS)
LDFLAGS=-o0 -ggdb
LIB=../lib_paho/libmqttpaho.ar
OBJS=mc_main.o mc_config.o mc_state_machine.o mc_action.o mc_mqtt_client_paho.o

all: mqtt_client

mqtt_client: $(OBJS)
	$(LD) -o mqtt_client $(OBJS) $(LIB)

.PHONY: clean
clean:
	$(RM) mqtt_client *.o
