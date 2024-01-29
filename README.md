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
sudo ./generate_passwords.bash
docker-compose up -d
```