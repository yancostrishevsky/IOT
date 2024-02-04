import logging
import time

from paho.mqtt import client as paho_client
import firebase_admin
import firebase_admin.db as fb_db


class DatabasePusher:

    def __init__(self, config):
        self._config = config

        cred_obj = firebase_admin.credentials.Certificate(
            self._config["firebase_credentials_path"]
        )

        self._app = firebase_admin.initialize_app(
            cred_obj, {"databaseURL": self._config["firebase_database_url"]}
        )

    def message_handler(
        self, client: paho_client.Client, userdata, message: paho_client.MQTTMessage
    ):
        """Handler for receiving messages from mqtt broker."""

        logging.debug(
            "Received message from topic %s: %s",
            message.topic,
            message.payload.decode(),
        )

        topic = message.topic.split("/")

        if len(topic) != 2:
            logging.error("Invalid topic: %s", message.topic)
            return

        device_id = topic[0]
        measurement_type = topic[1]

        single_float_measurement_types = ("humidity", "temperature", "movement")

        if measurement_type in single_float_measurement_types:

            ref = fb_db.reference(f"/measurements/{device_id}/{measurement_type}")

            ref.push().set(
                {"value": message.payload.decode(), "timestamp": time.time()}
            )

            return

        logging.error("Invalid measurement type: %s", measurement_type)
