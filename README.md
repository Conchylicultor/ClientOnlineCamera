ClientOnlineCamera
==================

Mqtt client which extract features from camera and send them over the network

In order to run the program, the folder ClientOnlineCamera must contain a config.yml indicating the ip adress of the mqtt brocker:

```
%YAML:1.0
brokerIp: '192.168.100.13'
clientId: 6
```

If the clientId parameter is not set, the program will automatically attribute an id corresponding to the pid of the program.
