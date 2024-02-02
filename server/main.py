import logging
import time
import pathlib

from paho.mqtt import client as paho_client

from src import mqtt_connection
from src import database_pusher

HOME_PATH = pathlib.Path(__file__).absolute().parent.as_posix()

INTER_CONNECTIONS_TIMEOUT = 5
BROKER_ADDRESS = "127.0.0.1"
PORT = 8126
USER = "server"
PASSWORD = "admin123"


if __name__ == "__main__":

    logging.getLogger('urllib3.connectionpool').setLevel(logging.ERROR)
    logging.getLogger('urllib3.util.retry').setLevel(logging.ERROR)
    logging.getLogger().setLevel(logging.DEBUG)

    pusher_config = {
        'firebase_credentials_path': f"{HOME_PATH}/assets/firebase_config.json",
        'firebase_database_url': 'https://server-room-monitoring-fe8b6-default-rtdb.europe-west1.firebasedatabase.app'
    }

    database_pusher = database_pusher.DatabasePusher(pusher_config)

    connection_config = {
        'broker_address': BROKER_ADDRESS,
        'port': PORT,
        'user': USER,
        'password': PASSWORD
    }

    connection = mqtt_connection.MqttConnection(
        connection_config, database_pusher.message_handler)

    while True:

        for time_waited in range(INTER_CONNECTIONS_TIMEOUT):
            logging.debug("Reconnecting in %d seconds...", INTER_CONNECTIONS_TIMEOUT - time_waited)
            time.sleep(1)

        connection.setup_connection()

        try:
            while connection.is_connected:
                time.sleep(1)

            logging.debug("exiting...")
            connection.tear_down_connection()

        except KeyboardInterrupt:
            logging.debug("exiting...")
            connection.tear_down_connection()
