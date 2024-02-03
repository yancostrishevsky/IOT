import logging
import time

from paho.mqtt import client as paho_client


class MqttConnection:

    def __init__(self, config, on_message_callback):
        self._config = config
        self._client: paho_client.Client = None
        self._is_connected = False

        self._on_message_callback = on_message_callback

    @property
    def is_connected(self):
        return self._is_connected

    def setup_connection(self):

        try:

            self._is_connected = False

            self._client = paho_client.Client("PythonServer")  # create new instance

            self._client.username_pw_set(
                self._config["user"], password=self._config["password"]
            )  # set username and password
            self._client.on_connect = self._on_connect  # attach function to callback
            self._client.on_message = self._on_message_callback
            self._client.on_disconnect = self._on_disconnect

            self._client.connect(
                self._config["broker_address"], self._config["port"]
            )  # connect to broker

            self._client.loop_start()  # start the loop

            self._client.subscribe("+/humidity")
            self._client.subscribe("+/temperature")
            self._client.subscribe("+/movement")

            time.sleep(3)

        except ConnectionRefusedError:
            logging.debug("Connection refused")

    def tear_down_connection(self):

        self._is_connected = False
        self._client.disconnect()
        self._client.loop_stop()

    def _on_connect(self, client, userdata, flags, rc):
        """Handler for connection with mqtt broker."""

        if rc == 0:
            logging.debug("Connected to broker")

            self._is_connected = True

        else:
            logging.debug("Connection failed")

    def _on_disconnect(self, client: paho_client.Client, userdata, rc):
        """Handler for disconnection from mqtt broker."""

        logging.debug("Disconnected from broker")

        self.tear_down_connection()
