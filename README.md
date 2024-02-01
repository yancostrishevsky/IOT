# Server room monitoring

Project structure:

```yaml
app:    # contains mobile application sources
mqtt:   # contains sources for docker container running mqtt broker
server: # contains sources for server transmitting messages from mqtt broker to database
```


# Launching MQTT server

```sh
cd mqtt
# Requires docker installed
bash setup_mqtt_broker.bash
```

# Starting server

```sh
cd server
bash setup_server.bash
```