# mqtt_client

C-based mqtt-client to be used on iot devices

## Design

Implements MQTT using an asynchronous, event-driven state-machine. This allows iot devices to communicate with the outside world. The current implementation uses the 'paho'-library to do the tcp-transport and payload-marshalling.
Other embedded implementations can be easily added, without disturbing the core logic.


## Building

    $ make
This builds the third party "poha"-library and our mqtt-client
The clients-executable ends up in src/mqtt_client

## Usage:

    $ ./src/mqtt_client -h
    Usage: ./mqtt_client [options]
    where options:
	    -r --remote-host: mandatory, default=tcp://myhost.nl:4455
	    -n --client-name: mandatory, default=testclient
	    -u --username: mandatory, default=grol
	    -c --password: mandatory, default=xxx
 	    -s --subscribe-topic: optional, default=/example
	    -p --publish-topic: optional, default=none
	    -v --verbose: optional, default=false
	    -h --help: this help


## Example
- I created an account https://www.cloudmqtt.com/. This server acts as a broker that allows clients to communicate with each other. Make sure to configure your own remote-host (-r), username (-u) and password (-s).

- Start a process thats acts as logger that monitors sensors by listening at '/logger'-channel

    $ ./mqtt_client -v -n logger-client -s /logger
    
- Start a process that acts as sensor that listens for commands on the '/sensor'-channel and publishes its status on '/logger'- channel:

    $ ./mqtt_client -v -n sensor-client -s /sensor -p /logger
    
    

