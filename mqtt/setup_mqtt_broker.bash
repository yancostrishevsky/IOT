#!/bin/bash

docker rm -f mosquitto-docker

sudo  rm -rf ./config/pwfile
sudo  rm -rf ./data
sudo  rm -rf ./log

echo 'server:admin123' | sudo tee -a ./config/pwfile

for i in {1..10}
do
    echo "device$i:device$i" | sudo tee -a ./config/pwfile
done

# sudo chmod 0700 ./config/pwfile

docker compose -f docker-compose.yml up -d

docker exec -it mosquitto-docker mosquitto_passwd -U /mosquitto/config/pwfile > /dev/null

sudo chmod 777 ./log/mosquitto.log


sleep 1

docker stop mosquitto-docker

sleep 1

docker start mosquitto-docker