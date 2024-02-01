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
#include <string.h>
#include "ssd1306.h"
#include "font8x8_basic.h"
#include <dht.h>
#include "mqtt_client.h"
#define SENSOR_TYPE DHT_TYPE_DHT11
#define DHT_GPIO_PIN GPIO_NUM_18
#define tag "SSD1306"
#define CONFIG_SDA_GPIO  21
#define CONFIG_SCL_GPIO  22
#define CONFIG_RESET_GPIO  15
static const char *TAG = "app";

static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_EVENT = BIT0;

const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.hostname = "192.168.100.15", // Zmień na adres Twojego brokera MQTT
    .broker.address.port = 1883, // Standardowy port MQTT
    .broker.address.transport = MQTT_TRANSPORT_OVER_TCP, // Transport TCP (dla MQTTS użyj MQTT_TRANSPORT_OVER_SSL)
    // Inne opcjonalne konfiguracje, jak certyfikaty, użytkownik, hasło itp.
};
esp_mqtt_client_handle_t client = NULL;





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
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Connected with IP Address");
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    }
}

static void wifi_init_sta(void) {
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
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
    
            esp_mqtt_client_publish(client, "topic", message, 0, 1, 0);

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

void app_main(void) {


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

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
        const char *pop = "abcd1234";  // Proof of possession
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *) pop, "PROV_", NULL));
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");
        wifi_prov_mgr_deinit();
        wifi_init_sta();
    }
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);
    int center, top, bottom;
	char lineChar[20];

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
    xTaskCreate(dht_test, "dht_test", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
    while (1) {
        ESP_LOGI(TAG, "Running");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
