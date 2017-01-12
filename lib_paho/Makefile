CC=gcc -I ../include -Wall -ggdb -D NO_PERSISTENCE
AR=ar rcs

OBJS=Clients.o MQTTProtocolClient.o Socket.o utf-8.o \
	Heap.o MQTTPacket.o	MQTTProtocolOut.o SocketBuffer.o \
	LinkedList.o MQTTPacketOut.o StackTrace.o \
	Log.o MQTTPersistence.o	Messages.o Thread.o \
	MQTTAsync.o	MQTTPersistenceDefault.o SSLSocket.o Tree.o 

libmqttpaho.ar: $(OBJS)
	$(AR) libmqttpaho.ar $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	$(RM) libmqttpaho.ar *.o
