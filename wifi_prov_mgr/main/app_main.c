#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>
#include <stdlib.h>
#include "ssd1306.h"
#include "font8x8_basic.h"
#include <dht.h>
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <math.h>

#define SENSOR_TYPE DHT_TYPE_DHT11
#define DHT_GPIO_PIN GPIO_NUM_18
#define tag "SSD1306"
#define CONFIG_SDA_GPIO  21
#define CONFIG_SCL_GPIO  22
#define CONFIG_RESET_GPIO  15
#define TRIG_PIN 25
#define ECHO_PIN 34
#define BUZZER_PIN 32
#define SOUND_SPEED 0.034
#define MOVEMENT_THRESHOLD 10.0 
#define MEASUREMENT_DELAY 500  
#define BUTTON_PIN GPIO_NUM_0
#define DEVICE_ID "device1"
#define PROVISIONING_PREFIX "SMR_device1"

// MQTT configuration
// [device_id]/temperature - float
// [device_id]/humidity - float
// [device_id]/movement - float


static const char *TAG = "app";
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_EVENT = BIT0;
const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.hostname = "192.168.43.111", 
    .broker.address.port = 1883, 
    .broker.address.transport = MQTT_TRANSPORT_OVER_TCP, 
};
esp_mqtt_client_handle_t client = NULL;


static void handle_wifi_reconnect(void) {
    ESP_LOGI(TAG, "Trying to reconnect to the WiFi network...");
    esp_wifi_connect();
}


static void handle_mqtt_reconnect(esp_mqtt_client_handle_t client) {
    ESP_LOGI(TAG, "Trying to reconnect to the MQTT broker...");
    esp_mqtt_client_reconnect(client);
}


static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data) {
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG, "Received Wi-Fi credentials\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
                break;
            case WIFI_PROV_END:
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
                esp_wifi_connect();
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_EVENT); 
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Connected with IP Address");
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    }
}


static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id) {
        case MQTT_EVENT_DISCONNECTED:
            handle_mqtt_reconnect(client);
            break;
        default:
            break;
    }
    return ESP_OK;
}


static void wifi_init_sta(void) {
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
}


static void mqtt_publish(const char *topic, const char *message) {
    if (client != NULL) {
        esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    }
}


void dht_test(void *pvParameters)
{   SSD1306_t dev;
    float temperature, humidity;
    ESP_LOGI(tag, "INTERFACE is i2c");
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    ESP_LOGI(tag, "Panel is 128x64");
	ssd1306_init(&dev, 132, 64);
    while (1)
    {
        if (dht_read_float_data(SENSOR_TYPE, DHT_GPIO_PIN, &humidity, &temperature) == ESP_OK)
            {
            char message[100];
            snprintf(message, sizeof(message), "Temperatura: %.1f, Wilgotnosc: %.1f", temperature, humidity);
            char temperatureMessage[50];
            char humidityMessage[50];

            snprintf(temperatureMessage, sizeof(temperatureMessage), "%.1f", temperature);
            snprintf(humidityMessage, sizeof(humidityMessage), "%.1f", humidity);

            mqtt_publish(DEVICE_ID"/temperature", temperatureMessage);
            mqtt_publish(DEVICE_ID"/humidity", humidityMessage);

            printf("reading data from sensor\n");
            ssd1306_clear_screen(&dev, false);
	        ssd1306_contrast(&dev, 0xff);
            char humidityBuffer[32]; // Bufor dla wilgotności
            snprintf(humidityBuffer, sizeof(humidityBuffer), "Wilgotnosc: %.1f%%", humidity);
            ssd1306_display_text(&dev, 0, humidityBuffer, strlen(humidityBuffer), false);
            char temperatureBuffer[32]; // Bufor dla temperatury
            snprintf(temperatureBuffer, sizeof(temperatureBuffer), "Temp: %.1fC", temperature);
            ssd1306_display_text(&dev, 1, temperatureBuffer, strlen(temperatureBuffer), false);
            memset(humidityBuffer, 0, sizeof(humidityBuffer));
            memset(temperatureBuffer, 0, sizeof(temperatureBuffer)); 
            }
        else
            printf("Could not read data from sensor\n");

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}


static void ultrasonic_sensor_task(void *pvParameters) {
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    long duration;
    float distanceCm;
    float lastDistanceCm = 0;

    while (1) {
        gpio_set_level(TRIG_PIN, 0);
        vTaskDelay(2 / portTICK_PERIOD_MS); 
        gpio_set_level(TRIG_PIN, 1);
        esp_rom_delay_us(10); 
        gpio_set_level(TRIG_PIN, 0);

    
        int64_t start_time = esp_timer_get_time();
        while (gpio_get_level(ECHO_PIN) == 0); 
        int64_t signal_start = esp_timer_get_time();
        while (gpio_get_level(ECHO_PIN) == 1); 
        int64_t signal_end = esp_timer_get_time();
        duration = signal_end - signal_start; 

        distanceCm = (duration * SOUND_SPEED) / 2.0;

        if (fabs(distanceCm - lastDistanceCm) > MOVEMENT_THRESHOLD) {
            printf("Ruch wykryty! W odleglosci: %.2f cm\n", distanceCm);
            char movementMessage[50];
            snprintf(movementMessage, sizeof(movementMessage), "%.2f", distanceCm);
            mqtt_publish(DEVICE_ID"/movement", movementMessage);
            gpio_set_level(BUZZER_PIN, 1); 
            vTaskDelay(1000 / portTICK_PERIOD_MS); 
            gpio_set_level(BUZZER_PIN, 0);
        } else {
            printf("Brak ruchu w odleglosci (cm): %.2f\n", distanceCm);
        }

        lastDistanceCm = distanceCm;
        vTaskDelay(MEASUREMENT_DELAY / portTICK_PERIOD_MS);
    }
}


void app_main(void) {


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(BUTTON_PIN);

    xTaskCreate(ultrasonic_sensor_task, "ultrasonic_sensor_task", 4096, NULL, 5, NULL);

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");
        wifi_prov_mgr_config_t config = {
            .scheme = wifi_prov_scheme_ble,
            .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
        };
        ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
        const char *pop = DEVICE_ID;  // Proof of possession - device_id
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *) pop, PROVISIONING_PREFIX, NULL));
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");
        wifi_prov_mgr_deinit();
        wifi_init_sta();
    }

    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);
    int center, top, bottom;
	char lineChar[20];

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
    xTaskCreate(dht_test, "dht_test", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);

    while (1) {
        ESP_LOGI(TAG, "Running");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}